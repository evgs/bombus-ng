#include "dnsquery.h"
#include <Iphlpapi.h>
#include "boostheaders.h"
using namespace dns;

DnsSrvQuery::DnsSrvQuery() {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    Buffer=new char[8192];
    BufferPos=0;

    DNSHost=DnsSrvQuery::getSystemDnsServer();
    if (DNSHost.length()==0) DNSHost="127.0.0.1";

}
DnsSrvQuery::~DnsSrvQuery(){
    if (Buffer) delete[] Buffer;
    shutdown(sock, SD_BOTH);
    closesocket(sock);
}

int DnsSrvQuery::PrepDnsName(const std::string & name, char *out){
    size_t i=0;
    char *l=out;
    char *pos=out+1;
    while (i<name.length()) {
        char c=name[i++];
        if (c=='.') {
            *l=(char)(pos-l-1);
            l=pos;
        } else {
            *pos=c;
        }
        pos++;
    }
    //finalizing
    int flen=(int)(pos-l-1);
    if (flen>0) *l=(char)flen;

    *(pos++)=0;
    return (int)(pos-out);
}

typedef boost::scoped_array<char> FixedInfoPtr;

const std::string DnsSrvQuery::getSystemDnsServer() {

    FixedInfoPtr fp;
    ULONG    ulOutBufLen;
    DWORD    dwRetVal;

    ulOutBufLen = sizeof(FIXED_INFO);

    do {
        fp.swap(FixedInfoPtr(new char[ulOutBufLen]));
        dwRetVal = GetNetworkParams((FIXED_INFO *)(fp.get()), &ulOutBufLen);
    } while (dwRetVal==ERROR_BUFFER_OVERFLOW);

    std::string result;

    if (dwRetVal==0) result=((FIXED_INFO *)(fp.get()))->DnsServerList.IpAddress.String;

    return result;
}

#pragma pack(push, 1)

struct DNS_HEADER
{
    unsigned	short id;		    // identification number

    unsigned	char rd     :1;		// recursion desired
    unsigned	char tc     :1;		// truncated message
    unsigned	char aa     :1;		// authoritive answer
    unsigned	char opcode :4;	    // purpose of message
    unsigned	char qr     :1;		// query/response flag

    unsigned	char rcode  :4;	    // response code
    unsigned	char cd     :1;	    // checking disabled
    unsigned	char ad     :1;	    // authenticated data
    unsigned	char z      :1;		// its z! reserved
    unsigned	char ra     :1;		// recursion available

    unsigned    short q_count;	    // number of question entries
    unsigned	short ans_count;	// number of answer entries
    unsigned	short auth_count;	// number of authority entries
    unsigned	short add_count;	// number of resource entries
};


//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};


//Constant sized fields of the resource record structure
struct  R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int   ttl;
    unsigned short data_len;
};
struct SRV_DATA {
    unsigned short priority;
    unsigned short weight;
    unsigned short port;    
};
#pragma pack(pop)


std::string ReadName(unsigned char* reader, unsigned char* buffer,int* count)
{
    unsigned char name[256];
    unsigned int len=0,offset;
    unsigned int i;

    bool jumped=false;

    *count=1;


    name[0]='\0';

    //read the names in 3www6google3com format
    while(*reader!=0) {
        BOOST_ASSERT(len<256);

        if(*reader>=0xC0) {
            offset = ((*reader)<<8) + *(reader+1) - 0xc000; 
            reader = buffer + offset - 1;
            jumped = true;  //we have jumped to another location so counting wont go up!
        }
        else name[len++]=*reader;

        reader++;

        if(jumped==0) (*count)++; //if we havent jumped to another location then we can count up
    }

    name[len]='\0';    //string complete

    if(jumped) (*count)++;  //number of steps we actually moved forward in the packet

    //now convert 3www6google3com0 to www.google.com
    i=0;
    while (i<len) {
        int l=name[i];
        while (l--) {
            name[i]=name[i+1];
            i++;
        }
        name[i]='.';
        i++;
    }
    name[i-1]='\0';	  //remove the last dot
    return std::string((char *)name);
}



bool DnsSrvQuery::doQuery(const std::string &hostname){

    char buf[65536],*qname,*reader;
    int i , scount;

    //sockaddr_in addr;

    sockaddr_in dest;

    DNS_HEADER *dns = NULL;
    QUESTION   *qinfo = NULL;

    dest.sin_family=AF_INET;
    dest.sin_port=htons(53);
    dest.sin_addr.s_addr=inet_addr(getDnsHost().c_str());  //dns servers

    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)&buf;

    dns->id = htons( (unsigned short) GetCurrentProcessId());
    dns->qr = 0;      //This is a query
    dns->opcode = 0;  //This is a standard query
    dns->aa = 0;      //Not Authoritative
    dns->tc = 0;      //This message is not truncated
    dns->rd = 1;      //Recursion Desired
    dns->ra = 0;      //Recursion not available! hey we dont have it (lol)
    dns->z  = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);   //we have only 1 question
    dns->ans_count  = 0;
    dns->auth_count = 0;
    dns->add_count  = 0;

    //point to the query portion
    qname =&buf[sizeof(struct DNS_HEADER)];

    PrepDnsName(hostname, qname);

    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it

    qinfo->qtype = htons(dns::Qtype_SRV);  
    qinfo->qclass = htons(1); //internet

    connect(sock, (sockaddr*) &dest, sizeof(dest));

    size_t qlen=sizeof(DNS_HEADER) 
        + (strlen((const char*)qname)+1) 
        + sizeof(QUESTION);

    if (send(sock, (char*)buf, (int)qlen, 0)==SOCKET_ERROR) {
        return false;
    }

    int sel=0;
    //MessageBoxW(NULL,L"1",L"1",MB_OK);
    do {
        timeval t={5, 0};
        fd_set fds;
        fds.fd_count=1;
        fds.fd_array[0]=sock;

        sel=select(0, &fds, NULL, NULL, &t);
        if (sel==0) return false; //timeout
        if (sel==SOCKET_ERROR) return false;
    } while (!sel);

    if(recv(sock,(char*)buf, 65536, 0)==SOCKET_ERROR) { return false; }

    dns=(struct DNS_HEADER*)buf;

    reader=&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

    //reading answers
    scount=0;

    for(i=0; i<ntohs(dns->ans_count); i++) {
        const std::string & name=ReadName((unsigned char *)reader, (unsigned char *)buf, &scount);
        reader+=scount;

        R_DATA *resource= (R_DATA*)(reader);
        reader+=sizeof(R_DATA);

        int dataLen=ntohs(resource->data_len);

        int ttl=ntohl(resource->ttl);

        switch (ntohs(resource->type)) {

        case dns::Qtype_A: //if its an ipv4 address
        {
            std::string ipAddr=boost::str(
                boost::format("%d.%d.%d.%d")
                % *((char*)(reader))
                % *((char*)(reader+1))
                % *((char*)(reader+2))
                % *((char*)(reader+3))
                );
            reader+=dataLen;
            break;
        }

        case dns::Qtype_SRV: {
            SRVAnswer::ref srv(new SRVAnswer());
            srv->service=name;
            srv->ttl=ttl;

            SRV_DATA *sd=(SRV_DATA *)reader;
            srv->priority=ntohs(sd->priority);
            srv->weight=ntohs(sd->weight);
            srv->port=ntohs(sd->port);
            reader+=sizeof(SRV_DATA);

            srv->target=ReadName((unsigned char *)reader, (unsigned char *)buf,&scount);
            reader+=scount;

            results.push_back(srv);
            break;
        }

        default:
            reader+=dataLen;
        }
    }

    //read authorities
    for(i=0;i<ntohs(dns->auth_count);i++)
    {
        const std::string & authName=ReadName((unsigned char *)reader, (unsigned char *)buf,&scount);
        reader+=scount;

        R_DATA *resource= (R_DATA*)(reader);
        reader+=sizeof(R_DATA);

        const std::string & ns=ReadName((unsigned char *)reader, (unsigned char *)buf,&scount);
        reader+=scount;
    }

    //read additional - the same as answers, dropped for SRV

    return true;

}
