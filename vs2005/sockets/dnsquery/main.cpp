// dnsquery.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dnsquery.h"
#include "iostream"

#pragma comment(lib,"Ws2_32.lib")


int _tmain(int argc, _TCHAR* argv[]){
    WSADATA wsaData;

    int err=WSAStartup( 0x202, &wsaData );

    dns::DnsSrvQuery d;

    do { std::cout << "querying jabber.ru..." << std::endl; } while ( !d.doQuery("_xmpp-client._tcp.jabber.ru") );
    do { std::cout << "querying gmail.com..." << std::endl; } while ( !d.doQuery("_xmpp-client._tcp.gmail.com") );
    do { std::cout << "querying ya.ru..." << std::endl; } while ( !d.doQuery("_xmpp-client._tcp.ya.ru") );

    int i=0;
    while (i<d.getCount()) {
        dns::SRVAnswer::ref srv(d.getResult(i));
        std::cout << srv->service 
            << " TTL=" << srv->ttl 
            << " Pr=" << srv->priority 
            << " W=" << srv->weight
            << " T=" << srv->target
            << ":" << srv->port
            << std::endl;
        i++;
    }

    WSACleanup();
    
    return 0;
}

