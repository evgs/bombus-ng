#include "base64.h"

#include <stdexcept> //for std::runtime_error
#include <assert.h> //for assert()

typedef unsigned char BYTE, *LPBYTE;
typedef unsigned long DWORD;
typedef const char* LPCSTR;

////////////////////////////////////////
// Constants

namespace base64{

    DWORD BASE64_8BIT = 3;
    DWORD BASE64_6BIT = 4;
    LPCSTR BASE64_DICT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";



    BYTE BASE64_TABLE[ 0x80 ] = {
        /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*28-2f*/ 0xFF, 0xFF, 0xFF, 0x3e, 0xFF, 0xFF, 0xFF, 0x3f, //2 = '+' and '/'
        /*30-37*/ 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, //8 = '0'-'7'
        /*38-3f*/ 0x3c, 0x3d, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, //2 = '8'-'9' and '='
        /*40-47*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //7 = 'A'-'G'
        /*48-4f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, //8 = 'H'-'O'
        /*50-57*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, //8 = 'P'-'W'
        /*58-5f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //3 = 'X'-'Z'
        /*60-67*/ 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, //7 = 'a'-'g'
        /*68-6f*/ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, //8 = 'h'-'o'
        /*70-77*/ 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, //8 = 'p'-'w'
        /*78-7f*/ 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  //3 = 'x'-'z'
    };

	const char* c_pszErrorInvalidParameter = "Invalid parameter";
	const char* c_pszErrorNotMultipleOf2 = "Invalid source, size is not a multiple of 2";
	const char* c_pszErrorNotMultipleOf4 = "Invalid source, size is not a multiple of 4";
	const char* c_pszErrorNotMultipleOf8 = "Invalid source, size is not a multiple of 8";

}


////////////////////////////////////////
// base64 Encoding

unsigned long base64::base64EncodeGetLength( unsigned long size )
{
    // output 4 bytes for every 3 input:
    //                1        2        3
    // 1 = --111111 = 111111--
    // 2 = --11XXXX = ------11 XXXX----
    // 3 = --1111XX =          ----1111 XX------
    // 4 = --111111 =                   --111111

    return (((size + BASE64_8BIT - 1) / BASE64_8BIT) * BASE64_6BIT);
}

unsigned long base64::base64Encode( void* dest, const void* src, unsigned long size )
{
    if (!dest || !src)
        throw std::runtime_error( c_pszErrorInvalidParameter );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        unsigned long dwBlockSize = __min( dwSrcSize, BASE64_8BIT );
        assert( 1 <= dwBlockSize && dwBlockSize <= BASE64_8BIT );

        // Encode inputs...
        BYTE n1, n2 = 0, n3 = 0, n4 = 0;
        switch (dwBlockSize)
        {
        case 3:
            n4  =  (pSrc[ 2 ] & 0x3f);
            n3  = ((pSrc[ 2 ] & 0xc0) >> 6);
        case 2:
            n3 |= ((pSrc[ 1 ] & 0x0f) << 2);
            n2  = ((pSrc[ 1 ] & 0xf0) >> 4);
        case 1:
            n2 |= ((pSrc[ 0 ] & 0x03) << 4);
            n1  = ((pSrc[ 0 ] & 0xfc) >> 2);
            break;

        default:
            assert( false );
        }
        pSrc += dwBlockSize;
        dwSrcSize -= dwBlockSize;

        // Validate...
        assert( 0 <= n1 && n1 <= 63 );
        assert( 0 <= n2 && n2 <= 63 );
        assert( 0 <= n3 && n3 <= 63 );
        assert( 0 <= n4 && n4 <= 63 );

        // Padding...
        switch (dwBlockSize)
        {
        case 1: n3 = 64;
        case 2: n4 = 64;
        case 3:
            break;

        default:
            assert( false );
        }

        // 4 outputs...
        *pDest++ = BASE64_DICT[ n1 ];
        *pDest++ = BASE64_DICT[ n2 ];
        *pDest++ = BASE64_DICT[ n3 ];
        *pDest++ = BASE64_DICT[ n4 ];
        dwDestSize += BASE64_6BIT; //4
    }

    return dwDestSize;
}

unsigned long base64::base64DecodeGetLength( unsigned long size )
{
    // output 3 bytes for every 4 input:
    //                1        2        3
    // 1 = --111111 = 111111--
    // 2 = --11XXXX = ------11 XXXX----
    // 3 = --1111XX =          ----1111 XX------
    // 4 = --111111 =                   --111111

    if (size % BASE64_6BIT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf4 );

    return (((size + BASE64_6BIT - 1) / BASE64_6BIT) * BASE64_8BIT);
}

unsigned long base64::base64Decode( void* dest, const void* src, unsigned long size )
{
    if ((dest == NULL) || (src == NULL))
        throw std::runtime_error( c_pszErrorInvalidParameter );

    if (size % BASE64_6BIT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf4 );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        // 4 inputs...
        BYTE in1 = *pSrc++;
        BYTE in2 = *pSrc++;
        BYTE in3 = *pSrc++;
        BYTE in4 = *pSrc++;
        dwSrcSize -= BASE64_6BIT; //4

        // Validate ascii...
        assert( 0 <= in1 && in1 <= 0x7f );
        assert( 0 <= in2 && in2 <= 0x7f );
        assert( 0 <= in1 && in3 <= 0x7f );
        assert( 0 <= in2 && in4 <= 0x7f );

        // Convert ascii to base16...
        in1 = BASE64_TABLE[ in1 ];
        in2 = BASE64_TABLE[ in2 ];
        in3 = BASE64_TABLE[ in3 ];
        in4 = BASE64_TABLE[ in4 ];

        // Validate base16...
        assert( in1 != 0xff );
        assert( in2 != 0xff );
        assert( in3 != 0xff );
        assert( in4 != 0xff );
        assert( 0 <= in1 && in1 <= 63 );
        assert( 0 <= in2 && in2 <= 63 );
        assert( 0 <= in3 && in3 <= 64 ); //possible padding
        assert( 0 <= in4 && in4 <= 64 ); //possible padding

        // 3 outputs...
        *pDest++ = ((in1 & 0x3f) << 2) | ((in2 & 0x30) >> 4);
        *pDest++ = ((in2 & 0x0f) << 4) | ((in3 & 0x3c) >> 2);
        *pDest++ = ((in3 & 0x03) << 6) | (in4 & 0x3f);
        dwDestSize += BASE64_8BIT; //3

        // Padding...
        if (in4 == 64)
        {
            --dwDestSize;
            if (in3 == 64)
            {
                --dwDestSize;
            }
        }
    }

    return dwDestSize;
}

const std::string base64::base64Encode(const std::string &inbuf) {
	int ilen=inbuf.length();
	int olen=base64EncodeGetLength(ilen);
	char *buf=new char[olen+1];
	base64Encode(buf, inbuf.c_str(), ilen);

	buf[olen]=0;
	return std::string(buf);
}