#include "crypto/MessageDigest.h"
#include "base64.h"
/*
* @(#)MessageDigest.java	1.7 95/08/15
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
* Updated to JDK 1.0.2 levels by Chuck McManis
* 
* This file was obtained from: http://www.mcmanis.com/~cmcmanis/java/src/util/crypt/MessageDigest.java
* More information can be found here: http://www.mcmanis.com/~cmcmanis/java/
*/

/**
* The MessageDigest class defines a general class for computing digest
* functions. It is defined as an abstract class that is subclassed by
* message digest algorithms. In this way the PKCS classes can be built
* to take a MessageDigest object without needing to know what 'kind'
* of message digest they are computing.
*
* This class defines the standard functions that all message digest
* algorithms share, and ways to put all Java fundamental types into
* the digest. It does not define methods for digestifying either
* arbitrary objects or arrays of objects however.
*
* @version 	5 Oct 1996, 1.8
* @author 	Chuck McManis
*/

/**
* Add a short value to the digest.
*/
void MessageDigest::updateShort(short aValue) {
    unsigned char	b1, b2;

    b1 = (unsigned char)((aValue >> 8) & 0xff);
    b2 = (unsigned char)(aValue & 0xff);
    updateByte(b1);
    updateByte(b2);
}

/**
* Add an integer value to the digest.
*/
void MessageDigest::updateInt(int aValue) {
    unsigned char	b;

    for (int i = 3; i >= 0; i--) {
        b = (unsigned char)((aValue >> (i * 8)) & 0xff);
        updateByte(b);
    }
}

/**
* Add a long to the digest.
*/
void MessageDigest::updateLong(long aValue) {
    unsigned char b;

    for (int i = 7; i >= 0; i--) {
        b = (unsigned char)((aValue >> (i * 8)) & 0xff);
        updateByte(b);
    }
}

/**
* Add specific bytes to the digest.
*/
void MessageDigest::updateArray(const unsigned char* input, int len, int offset) {
    for (int i = 0; i < len; i++) {
        updateByte(input[i+offset]);
    }
}

/**
* Add the bytes in the String 'input' to the current digest.
* Note that the string characters are treated as unicode chars
* of 16 bits each. To digestify ISO-Latin1 strings (ASCII) use
* the updateASCII() method.
*/
void MessageDigest::updateW(const std::wstring &input) {
    int	i, len;
    unsigned short	x;

    len = input.length();
    for (i = 0; i < len; i++) {
        x = (unsigned short) input[i];
        updateShort(x);
    }
}

/**
* Treat the string as a sequence of ISO-Latin1 (8 bit) characters.
*/
void MessageDigest::updateASCII(const std::string &input) {
	updateArray((const unsigned char *)input.c_str(), input.length());
}

const unsigned char * MessageDigest::getDigestBits() const{
    return (digestValid)? digestBits: 0;
}

const std::string MessageDigest::getDigestHex() const{
    std::string result;
    if (!digestValid) return result;

    for(int i = 0; i < digestBitsLen; i++) {
        char c;

        c = (char) ((digestBits[i] >> 4) & 0xf);
        if (c > 9)   c = (char) ((c - 10) + 'a');
        else  c = (char) (c + '0');

        result+=c;

        c = (char) (digestBits[i] & 0xf);
        if (c > 9)
            c = (char)((c-10) + 'a');
        else
            c = (char)(c + '0');
        
        result+=c; 
    }

    return result;
}

const std::string MessageDigest::getDigestBase64() const{
    if (!digestValid) return std::string();

    return base64::base64Encode(digestBits, digestBitsLen);
}