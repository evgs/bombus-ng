//#include "stdafx.h"

#include "CompressedSocket.h"
#include "boostheaders.h"
#include <stdio.h>
/**
 * CHUNK is simply the buffer size for feeding data to and pulling data 
 * from the zlib routines. Larger buffer sizes would be more efficient, 
 * especially for inflate(). If the memory is available, buffers sizes 
 * on the order of 128K or 256K bytes should be used.
 */ 
#define CHUNK 16384

CompressedSocket::CompressedSocket(ConnectionRef pack){
	this->pack=pack;

	zinbuf=new char[CHUNK];
	zoutbuf=new char[CHUNK];

	/* allocate deflate state */
    ostr.zalloc = Z_NULL;
    ostr.zfree = Z_NULL;
    ostr.opaque = Z_NULL;
    int ret = deflateInit(&ostr, Z_BEST_COMPRESSION);
    BOOST_ASSERT( ret == Z_OK );

	/* allocate inflate state */
    istr.zalloc = Z_NULL;
    istr.zfree = Z_NULL;
    istr.opaque = Z_NULL;
    istr.avail_in = 0;
	istr.avail_out=1;
    istr.next_in = Z_NULL;
    ret = inflateInit(&istr);
    BOOST_ASSERT( ret == Z_OK );
}

CompressedSocket::~CompressedSocket(void){
	deflateEnd(&ostr);
	inflateEnd(&istr);

	delete zoutbuf;
	delete zinbuf;
}
const std::string CompressedSocket::getStatistics(){
	char *fmt="--- ZLib ---\n"
	"sent=%d  (unpacked=%d)\n"
	"recv=%d  (unpacked=%d)\n";

	char buf[256];

	//sprintf_s(buf, 256, fmt, ostr.total_out, ostr.total_in, istr.total_in, istr.total_out);
	sprintf(buf, fmt, ostr.total_out, ostr.total_in, istr.total_in, istr.total_out);

	return std::string(buf) + (pack->getStatistics());
}
int CompressedSocket::read(char *buf, int len){

	//if (istr.avail_out!=0){
    if (istr.avail_in==0){
		istr.avail_in=pack->read(zinbuf, CHUNK);
		istr.next_in= (unsigned char *) const_cast<char *>( zinbuf );
	}
	if (istr.avail_in==0) return 0;

	istr.avail_out=len;
	istr.next_out = (unsigned char *) ( buf );

	int ret=inflate(&istr, Z_NO_FLUSH);
	BOOST_ASSERT(ret != Z_STREAM_ERROR);
	//TODO: handle zlib errors

	int have=len-istr.avail_out;
	return have;
}
	
int CompressedSocket::write(const char *buf, int len){

	ostr.avail_in=len;
	ostr.next_in= (unsigned char *) const_cast<char *>( buf );

	do {
		ostr.avail_out=CHUNK;
		ostr.next_out = (unsigned char *) const_cast<char *>( zoutbuf );

		int ret=deflate(&ostr, Z_SYNC_FLUSH);
		BOOST_ASSERT(ret != Z_STREAM_ERROR);

		int have=CHUNK-ostr.avail_out;

		pack->write(zoutbuf, have);
	} while (ostr.avail_out==0);

	BOOST_ASSERT(ostr.avail_in == 0);
	return len;
}

void CompressedSocket::close(){}