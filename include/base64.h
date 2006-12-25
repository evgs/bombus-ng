#pragma once

#include <string>

namespace base64{
	// Base64 Encoding
	unsigned long base64EncodeGetLength( unsigned long size );
	unsigned long base64Encode( void* dest, const void* src, unsigned long size );

	const std::string base64Encode(const std::string &inbuf);

	// Base64 Decoding
	unsigned long base64DecodeGetLength( unsigned long size );
	unsigned long base64Decode( void* dest, const void* src, unsigned long size );
    const std::string base64Decode(const std::string &inbuf);
}

