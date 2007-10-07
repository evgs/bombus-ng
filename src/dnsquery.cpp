#include "dnsquery.h"
#include <Iphlpapi.h>

using namespace dns;

DNSQuery::DNSQuery() {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    Buffer=new char[8192];
    BufferPos=0;

    DNSHost=DNSQuery::getSystemDnsServer();
    if (DNSHost.length()==0) DNSHost="127.0.0.1";

}
DNSQuery::~DNSQuery(){
    if (Buffer) delete[] Buffer;
    shutdown(sock, SD_BOTH);
    closesocket(sock);
}

int DNSQuery::CompressName(const std::string & name, char *out){
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
    int flen=(pos-l-1);
    if (flen>0) *l=(char)flen;

    *(pos++)=0;
    return pos-out;
}

const std::string DNSQuery::getSystemDnsServer() {

    FIXED_INFO * FixedInfo=NULL;
    ULONG    ulOutBufLen;
    DWORD    dwRetVal;

    ulOutBufLen = sizeof(FIXED_INFO);

    do {
        if (FixedInfo) GlobalFree(FixedInfo);
        FixedInfo=(FIXED_INFO *) GlobalAlloc(GPTR, ulOutBufLen);
        dwRetVal = GetNetworkParams(FixedInfo, &ulOutBufLen);
    } while (dwRetVal==ERROR_BUFFER_OVERFLOW);

    std::string result;
    if (dwRetVal==0) result=FixedInfo->DnsServerList.IpAddress.String;

    GlobalFree( FixedInfo );

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
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char  *name;
    struct R_DATA  *resource;
    unsigned char  *rdata;
};

//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct QUESTION      *ques;
} QUERY;


unsigned char* ReadName(char* reader,char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name   = (unsigned char*)malloc(256);

    name[0]='\0';

    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000  ;)
            reader = buffer + offset - 1;
            jumped = 1;  //we have jumped to another location so counting wont go up!
        }
        else 
            name[p++]=*reader;

        reader=reader+1;

        if(jumped==0) *count = *count + 1; //if we havent jumped to another location then we can count up
    }

    name[p]='\0';    //string complete
    if(jumped==1) *count = *count + 1;  //number of steps we actually moved forward in the packet

    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0';	  //remove the last dot
    return name;		
}



void DNSQuery::doQuery(const std::string &hostname){

    char buf[65536],*qname,*reader;
    int i , j , stop;

    SOCKET s;
    sockaddr_in a;

    RES_RECORD answers[20],auth[20],addit[20];  //the replies from the DNS server
    sockaddr_in dest;

    DNS_HEADER *dns = NULL;
    QUESTION   *qinfo = NULL;

    s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);  //UDP packet for DNS queries

    dest.sin_family=AF_INET;
    dest.sin_port=htons(53);
    dest.sin_addr.s_addr=inet_addr(getDnsHost().c_str());  //dns servers

    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)&buf;

    dns->id = (unsigned short) htons(GetCurrentProcessId());
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

    CompressName(hostname, qname);

    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it

    qinfo->qtype = htons(dns::Qtype_SRV);  
    qinfo->qclass = htons(1); //internet

    printf("\nSending Packet...");
    if(sendto(s,(char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION),0,(struct sockaddr*)&dest,sizeof(dest))==SOCKET_ERROR)
    {
        printf("%d  error",WSAGetLastError());
        return;
    }
    printf("Sent");

    int sel=0;
    do {
        timeval t={20, 0};
        fd_set fds;
        fds.fd_count=1;
        fds.fd_array[0]=sock;

        sel=select(0, &fds, NULL, NULL, &t);
        if (sel==0) return; //timeout
        if (sel==SOCKET_ERROR) return;
    } while (!sel);

    i=sizeof(dest);
    printf("\nReceiving answer...");
    if(recvfrom (s,(char*)buf,65536,0,(struct sockaddr*)&dest,&i)==SOCKET_ERROR)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return;
    }
    printf("Received.");

    dns=(struct DNS_HEADER*)buf;

    reader=&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

    printf("\nThe response contains : ");
    printf("\n %d Questions.",ntohs(dns->q_count));
    printf("\n %d Answers.",ntohs(dns->ans_count));
    printf("\n %d Authoritative Servers.",ntohs(dns->auth_count));
    printf("\n %d Additional records.\n\n",ntohs(dns->add_count));


    //reading answers
    stop=0;

    for(i=0;i<ntohs(dns->ans_count);i++)
    {
        answers[i].name=ReadName(reader,buf,&stop);
        reader = reader + stop;

        answers[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);

        if(ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));

            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
                answers[i].rdata[j]=reader[j];

            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

            reader = reader + ntohs(answers[i].resource->data_len);

        }
        else
        {
            answers[i].rdata = ReadName(reader,buf,&stop);
            reader = reader + stop;
        }


    }

    //read authorities
    for(i=0;i<ntohs(dns->auth_count);i++)
    {
        auth[i].name=ReadName(reader,buf,&stop);
        reader+=stop;

        auth[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);

        auth[i].rdata=ReadName(reader,buf,&stop);
        reader+=stop;
    }

    //read additional
    for(i=0;i<ntohs(dns->add_count);i++)
    {
        addit[i].name=ReadName(reader,buf,&stop);
        reader+=stop;

        addit[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);

        if(ntohs(addit[i].resource->type)==1)
        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len));
            for(j=0;j<ntohs(addit[i].resource->data_len);j++)
                addit[i].rdata[j]=reader[j];

            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            reader+=ntohs(addit[i].resource->data_len);

        }
        else
        {
            addit[i].rdata=ReadName(reader,buf,&stop);
            reader+=stop;
        }
    }


    //print answers
    for(i=0;i<ntohs(dns->ans_count);i++)
    {
        //printf("\nAnswer : %d",i+1);
        printf("Name  :  %s ",answers[i].name);

        if(ntohs(answers[i].resource->type)==1)   //IPv4 address
        {

            long *p;
            p=(long*)answers[i].rdata;
            a.sin_addr.s_addr=(*p);    //working without ntohl
            printf("has IPv4 address :  %s",inet_ntoa(a.sin_addr));
        }
        if(ntohs(answers[i].resource->type)==5)   //Canonical name for an alias
            printf("has alias name : %s",answers[i].rdata);

        printf("\n");
    }

    //print authorities
    for(i=0;i<ntohs(dns->auth_count);i++)
    {
        //printf("\nAuthorities : %d",i+1);
        printf("Name  :  %s ",auth[i].name);
        if(ntohs(auth[i].resource->type)==2)
            printf("has authoritative nameserver : %s",auth[i].rdata);
        printf("\n");
    }

    //print additional resource records
    for(i=0;i<ntohs(dns->add_count);i++)
    {
        //printf("\nAdditional : %d",i+1);
        printf("Name  :  %s ",addit[i].name);
        if(ntohs(addit[i].resource->type)==1)
        {
            long *p;
            p=(long*)addit[i].rdata;
            a.sin_addr.s_addr=(*p);    //working without ntohl
            printf("has IPv4 address :  %s",inet_ntoa(a.sin_addr));
        }
        printf("\n");
    }

    return;

}

/*
namespace Dimon.Net
{
    	public class DNSQuery
    	{
    		private byte[] CodeInt(ushort Value)
    		{
    			byte[] retArr = new byte[2];
    			retArr[0] = (byte)(Value / 256);
    			retArr[1] = (byte)(Value % 256);
    			return (retArr);
    		}
    		private byte[] CodeHeader()
    		{
    			byte[] ret = new byte[12];
    			Random rnd = new Random(32767);
    			CodeInt((ushort)rnd.Next()).CopyTo(ret, 0);
    
    			CodeInt(256).CopyTo(ret, 2);
    			CodeInt(1).CopyTo(ret, 4);
    			CodeInt(0).CopyTo(ret, 6);
    			CodeInt(0).CopyTo(ret, 8);
    			CodeInt(0).CopyTo(ret, 10);
    			return ret;
    		}
    		private int CodeQuery(string Name, int Qtype, out byte[] ret)
    		{
    			byte[] res = new byte[255];
    			int i;
    			i = CompressName(Name, out res);
    			CodeInt((ushort)Qtype).CopyTo(res, i);
    			i += 2;
    			CodeInt(1).CopyTo(res, i);
    			i += 2;
    			ret = res;
    			return i;
    		}
    		private string DecodeLabels(ref int From)
    		{
    			string ret = "";
    			int l, f;
    			while (true)
    			{
    				l = (int) Buffer[From++];
    				if (l == 0)
    				{
    					break;
    				}
    				if (ret != "")
    				{
    					ret += ".";
    				}
    				if ((l & 0xC0) != 0)
    				{
    					f = l & 0x3F;
    					f = f * 256 + (int) Buffer[From++]; // + 1;
    					ret += this.DecodeLabels(ref f);
    					break;
    				}
    				else
    				{
    					ret += Encoding.ASCII.GetString(Buffer, From, l);
    					From += l;
    				}
    			}
    			return ret;
    		}
    
    		private ushort DecodeInt(byte[] Value, int Index)
    		{
    			byte x, y;
    			if (Value.Length > Index)
    			{
    				x = Value[Index];
    			}
    			else
    			{
    				x = 0;
    			}
    			if (Value.Length > Index + 1)
    			{
    				y = Value[Index + 1];
    			}
    			else
    			{
    				y = 0;
    			}
    			return (ushort) (x * 256 + y);
    		}
    		private string DecodeResource(ref int i, string Name, DNSQueryType Qtype)
    		{
    			string Rname;
    			int Rtype, len, j, x, n;
    			string ret = "";
    
    			Rname = DecodeLabels(ref i);
    			Rtype = DecodeInt(Buffer,i);
    			i += 8;
    			len = DecodeInt(Buffer,i);
    			i += 2;
    			j = i;
    			i += len;   //i point to next record
    			if ((Name == Rname) && ((int)Qtype == Rtype)) 
    			{
    				switch(Qtype)
    				{
    					case DNSQueryType.Qtype_A:
    						ret = ((byte)(Buffer[j++])).ToString();
    						ret += "." + ((byte)(Buffer[j++])).ToString();
    						ret += "." + ((byte)(Buffer[j++])).ToString();
    						ret += "." + ((byte)(Buffer[j++])).ToString();
    						break;
    					case DNSQueryType.Qtype_NS:
    					case DNSQueryType.Qtype_MD:
    					case DNSQueryType.Qtype_MF:
    					case DNSQueryType.Qtype_CNAME:
    					case DNSQueryType.Qtype_MB:
    					case DNSQueryType.Qtype_MG:
    					case DNSQueryType.Qtype_MR:
    					case DNSQueryType.Qtype_PTR:
    					case DNSQueryType.Qtype_X25:
    					case DNSQueryType.Qtype_NSAP:
    					case DNSQueryType.Qtype_NSAPPTR:
    						ret = DecodeLabels(ref j);
    						break;
    
    					case DNSQueryType.Qtype_SOA:
    						ret =  DecodeLabels(ref j);
    						ret += "," + DecodeLabels(ref j);
    						for (n = 1; n <= 5; n++)
    						{
    							x = DecodeInt(Buffer,j)*65536 + DecodeInt(Buffer,j+2);
    							j += 4;
    							ret += "," + x.ToString();
    						}
    						break;
    					case DNSQueryType.Qtype_NULL:
    					case DNSQueryType.Qtype_WKS:
    						break;
    					case DNSQueryType.Qtype_HINFO:
    					case DNSQueryType.Qtype_MINFO:
    					case DNSQueryType.Qtype_RP:
    					case DNSQueryType.Qtype_ISDN:
    						ret = DecodeLabels(ref j);
    						ret += "," + DecodeLabels(ref j);
    						break;
    					case DNSQueryType.Qtype_MX:
    					case DNSQueryType.Qtype_AFSDB:
    					case DNSQueryType.Qtype_RT:
    					case DNSQueryType.Qtype_KX:
    						x = DecodeInt(Buffer,j);
    						j += 2;
    						ret = x.ToString();
    						ret += "," + DecodeLabels(ref j);
    						break;
    					case DNSQueryType.Qtype_TXT:
    						ret = DecodeLabels(ref j);
    						break;
    					case DNSQueryType.Qtype_GPOS:
    						ret = DecodeLabels(ref j);
    						ret += "," + DecodeLabels(ref j);
    						ret += "," + DecodeLabels(ref j);
    						break;
    					case DNSQueryType.Qtype_PX:
    						x = DecodeInt(Buffer,j);
    						j += 2;
    						ret = x.ToString();
    						ret += "," + DecodeLabels(ref j);
    						ret += "," + DecodeLabels(ref j);
    						break;
    				}  
    			}
    			return ret;
    		}
    		private bool IsIP(string Value)
    		{
    			int x = 0;
    			string correctChars = "0123456789.";
    			bool ret = true;
    			for (int n = 1; n <= Value.Length; n++)
    			{
    				if (correctChars.IndexOf(Value[n]) == -1)
    				{
    					ret = false;
    					break;
    				}
    				else
    				{
    					if (Value[n] == '.')
    					{
    						x++;
    					}
    				}
    			}
    			if (x != 3)
    			{
    				ret = false;
    			}
    			return ret;
    		}
    		private string ReverseIP(string Value)
    		{
    			int x;
    			string ret = "";
    			x = Value.LastIndexOf(".");
    			while (x > 0)
    			{
    				ret += Value.Substring(x + 1);
    				Value.Remove(x, Value.Length - x + 1);
    				x = Value.LastIndexOf(".");
    			}
    			if (ret.Length > 0)
    			{
    				if(ret[0] == '.')
    				{
    					ret.Remove(0, 1);
    				}
    			}
    			return ret;
    		}
    		public bool MakeQuery(string Name, DNSQueryType QType)
    		{
    			int n, i;
    			int flag,qdcount, ancount, nscount, arcount;
    			string s;
    			byte[] tmp;
    			bool ret = false;
    			results.Clear();
    			if (IsIP(Name))
    			{
    				Name = ReverseIP(Name) + ".in-addr.arpa";
    			}
    			CodeHeader().CopyTo(Buffer, 0);
    			BufferPos += 12;
    			int len = CodeQuery(Name, (int)QType, out tmp);
    			tmp.CopyTo(Buffer, BufferPos);
    			BufferPos += len;
    			sock.Connect(new IPEndPoint(IPAddress.Parse(DNSHost), 53));
    			sock.Send(Buffer, BufferPos, 0);
    			int bytes = sock.Receive(Buffer, 8192, 0);
    			flag = DecodeInt(Buffer, 2);
    			byte RCode = (byte)(flag & 0x000F);
    			if (RCode == 0)
    			{
    				qdcount = DecodeInt(Buffer, 4);
    				ancount = DecodeInt(Buffer, 6);
    				nscount = DecodeInt(Buffer, 8);
    				arcount = DecodeInt(Buffer, 10);
    				i = 13;
    				if (qdcount > 0)
    				{
    					for(n = 1; n <= qdcount ; n++)
    					{
    						while ((Buffer[i] != 0) && ((Buffer[i] & 0xC0) != 0xC0))
    						{
    							i++;
    						}
    						i += 5;
    					}
    				}
    				if (ancount > 0)
    				{
    					for(n = 1; n <= ancount; n++)
    					{
    						s = DecodeResource(ref i, Name, QType);
    						if (s != "")
    						{
    							results.Add(s, s);
    						}
    					}
    				}
    				ret = true;
    			}
    			return ret;
    		}
    		public bool MakeQuery(string Name, int QType)
    		{
    			return MakeQuery(Name, (DNSQueryType) QType);
    		}
    		public bool GetMXRecords(string Name)
    		{
    			bool retValue;
    			if(hash[Name] != null)
    			{
    				results = (System.Collections.SortedList) hash[Name];
    				return true;
    			}
    			retValue = MakeQuery(Name, DNSQueryType.Qtype_MX);
    			if (retValue)
    			{
    				string val;
    				for (int i = 0; i < results.Count; i++)
    				{
    					val = (string)results.GetByIndex(i);
    					val = val.Remove(0, val.IndexOf(",") + 1);
    					results.SetByIndex(i, val);
    				}
    				hash.Add(Name, results.Clone());
    			}
    			return retValue;
    		}

            }
    }  
  */