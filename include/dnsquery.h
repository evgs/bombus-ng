#pragma once

#include <string>
#include <vector>
#include <Winsock2.h>

#pragma comment(lib,"Iphlpapi.lib")

namespace dns {
    enum DNSQueryType
    {
        Qtype_A     =1,
        Qtype_NS    =2,
        Qtype_MD    =3,
        Qtype_MF    =4,
        Qtype_CNAME =5,
        Qtype_SOA   =6,
        Qtype_MB    =7,
        Qtype_MG    =8,
        Qtype_MR    =9,
        Qtype_NULL  =10,
        Qtype_WKS   =11,  //
        Qtype_PTR   =12,
        Qtype_HINFO =13,
        Qtype_MINFO =14,
        Qtype_MX    =15,
        Qtype_TXT   =16,
        Qtype_RP    =17,
        Qtype_AFSDB =18,
        Qtype_X25   =19,
        Qtype_ISDN  =20,
        Qtype_RT    =21,
        Qtype_NSAP  =22,
        Qtype_NSAPPTR=23,
        Qtype_SIG   =24, //RFC-2065
        Qtype_KEY   =25, //RFC-2065
        Qtype_PX    =26,
        Qtype_GPOS  =27,
        Qtype_AAAA  =28, //IP6 Address
        Qtype_LOC   =29, //RFC-1876
        Qtype_NXT   =30, //RFC-2065
        Qtype_SRV   =33, //RFC-2052
        Qtype_NAPTR =35, //RFC-2168
        Qtype_KX    =36,
        Qtype_AXFR  =252, //
        Qtype_MAILB =253, //
        Qtype_MAILA =254, //
        Qtype_ALL   =255 //
    };

    class DNSQuery {
    private:
        std::string DNSHost;
        char *Buffer;
        int BufferPos;

        SOCKET sock;

        std::vector<std::string> results;


        static const std::string getSystemDnsServer();
        int CompressName(const std::string & name, char *out);

    public:
        DNSQuery();
        ~DNSQuery();

        const std::string & getDnsHost() const { return DNSHost; }

        void setDnsHost(const std::string &dnsHost) { this->DNSHost=dnsHost; }

        int getCount() const { return results.size(); }

        void doQuery(const std::string &hostname);

        const std::string & getResult(int index) const { return results[index]; } //todo: operator[]

    };

}