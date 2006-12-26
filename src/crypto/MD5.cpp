#include "crypto/MD5.h"
/*
* @(#)MD5.java	1.9 95/08/07
*
* Copyright (c) 1994 Sun Microsystems, Inc. All Rights Reserved.
*
* Permission to use, copy, modify, and distribute this software
* and its documentation for NON-COMMERCIAL purposes and without
* fee is hereby granted provided that this copyright notice
* appears in all copies. Please refer to the file "copyright.html"
* for further important copyright and licensing information.
*
* SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
* THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
* ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
* DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
*
* Updated to the JDK 1.0.2 release by Chuck McManis
* 
* This file was obtained from: http://www.mcmanis.com/~cmcmanis/java/src/util/crypt/MD5.java
* More information can be found here: http://www.mcmanis.com/~cmcmanis/java/
*/


/**
* The MD5 class is used to compute an MD5 message digest over a given
* buffer of bytes. It is an implementation of the RSA Data Security Inc
* MD5 algorithim as described in internet RFC 1321.
* @version 	06 Oct 1996, 1.9.1
* @author 	Chuck McManis
*/

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


/**
* Standard constructor, creates a new MD5 instance, allocates its
* buffers from the heap.
*/
MD5::MD5() {
    state = new int[4];
    for (int i=0; i<4; i++) { state[i]=0; }
    count = 0;

    transformBuffer = new int[16];
    digestBits = new unsigned char[16];
    digestBitsLen=16;
    digestValid = false;
    for (int i=0; i<16; i++) { transformBuffer[i]=0; }

    buffer = new unsigned char[64];
    for (int i=0; i<64; i++) { buffer[i]=0; }
}

MD5::~MD5(){
    delete state;
    delete buffer;
    delete transformBuffer;
}


/* **********************************************************
* The MD5 Functions. These are copied verbatim from
* the RFC to insure accuracy. The results of this
* implementation were checked against the RSADSI version.
* **********************************************************
*/

int MD5::F(int x, int y, int z) {
    return ((x & y) | ((~x) & z));
}

int MD5::G(int x, int y, int z) {
    return ((x & z) | (y & (~z)));
}

int MD5::H(int x, int y, int z) {
    return ((x ^ y) ^ z);
}

int MD5::I(int x, int y, int z) {
    return (y ^ (x | (~z)));
}

int MD5::rotateLeft(int a, int n) {
    return ((a << n) | (((unsigned int) a) >> (32 - n)));
}

int MD5::FF(int a, int b, int c, int d, int x, int s, int ac) {
    a += F(b, c, d) + x + ac;
    a = rotateLeft(a, s);
    a += b;
    return a;
}

int MD5::GG(int a, int b, int c, int d, int x, int s, int ac) {
    a += G(b, c, d) + x + ac;
    a = rotateLeft(a, s);
    a += b;
    return a;
}

int MD5::HH(int a, int b, int c, int d, int x, int s, int ac) {
    a += H(b, c, d) + x + ac;
    a = rotateLeft(a, s);
    a += b;
    return a;
}

int MD5::II(int a, int b, int c, int d, int x, int s, int ac) {
    a += I(b, c, d) + x + ac;
    a = rotateLeft(a, s);
    a += b;
    return a;
}

/**
* This is where the functions come together as the generic MD5
* transformation operation, it is called by update() which is
* synchronized (to protect transformBuffer)
*/
void MD5::transform(unsigned char * buf, int offset) {
    int a, b, c, d;
    int *x = transformBuffer;

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

    for (int i = 0; i < 16; i++) {
        x[i] = (int)buf[i*4+offset] & 0xff;
        for (int j = 1; j < 4; j++) {
            x[i] += ((int)(buf[i*4+j+offset])) << (j * 8);
        }
    }

    /* Round 1 */
    a = FF ( a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    d = FF ( d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    c = FF ( c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    b = FF ( b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    a = FF ( a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    d = FF ( d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    c = FF ( c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    b = FF ( b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    a = FF ( a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    d = FF ( d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    c = FF ( c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    b = FF ( b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    a = FF ( a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    d = FF ( d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    c = FF ( c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    b = FF ( b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    a = GG ( a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    d = GG ( d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    c = GG ( c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    b = GG ( b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    a = GG ( a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    d = GG ( d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    c = GG ( c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    b = GG ( b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    a = GG ( a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    d = GG ( d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    c = GG ( c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    b = GG ( b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    a = GG ( a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    d = GG ( d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    c = GG ( c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    b = GG ( b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    a = HH ( a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    d = HH ( d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    c = HH ( c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    b = HH ( b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    a = HH ( a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    d = HH ( d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    c = HH ( c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    b = HH ( b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    a = HH ( a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    d = HH ( d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    c = HH ( c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    b = HH ( b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    a = HH ( a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    d = HH ( d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    c = HH ( c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    b = HH ( b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    a = II ( a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    d = II ( d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    c = II ( c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    b = II ( b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    a = II ( a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    d = II ( d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    c = II ( c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    b = II ( b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    a = II ( a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    d = II ( d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    c = II ( c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    b = II ( b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    a = II ( a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    d = II ( d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    c = II ( c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    b = II ( b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

/**
* Initialize the MD5 state information and reset the bit count
* to 0. Given this implementation you are constrained to counting
* 2^64 bits.
*/
void MD5::init() {
    count = 0;
    // Load magic initialization constants.
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
    digestValid = false;
    for (int i = 0; i < digestBitsLen; i++)
        digestBits[i] = 0;
}

/**
* update adds the passed type to the input buffer
*/
void MD5::updateByte(unsigned char b) {
    int	index;

    index = (int) ((count >> 3) & 0x3f);
    count += 8;
    buffer[index] = b;
    if (index  >= 63) {
        transform(buffer, 0);
    }
}

/**
* Perform the final computations, any buffered bytes are added
* to the digest, the count is added to the digest, and the resulting
* digest is stored. After calling final you will need to call
* init() again to do another digest.
*/
void MD5::finish() {
    unsigned char 	bits[8];
    int	i, index, padLen;

    for (i = 0; i < 8; i++) {
        bits[i] = (unsigned char)((count >> (i * 8)) & 0xff);
    }

    index = (int)(count >> 3) & 0x3f;
    padLen = (index < 56) ? (56 - index) : (120 - index);
    updateByte(0x80);
    for (int padding=1; padding<padLen; padding++)
        updateByte(0);
    MessageDigest::updateArray(bits, 8);

    for (i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            digestBits[i*4+j] = (unsigned char)((state[i] >> (j * 8)) & 0xff);
        }
    }
    digestValid = true;
}

