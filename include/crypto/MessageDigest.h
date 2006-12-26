#pragma once
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
#include <string>

class MessageDigest {

public:

    /** the actual digest bits. */
    unsigned char *digestBits;

    int digestBitsLen;

    /** status of the digest */
    bool digestValid;

    /**
    * Return a string that represents the algorithim of this
    * message digest.
    */
    virtual const char * getAlg() const =0 ;

    /**
     * This function is used to initialize any internal digest
     * variables or parameters.
     */
    virtual void init()=0;

    /**
     * The basic unit of digestifying is the byte. This method is
     * defined by the particular algorithim's subclass for that
     * algorithim. Subsequent versions of this method defined here
     * decompose the basic type into bytes and call this function.
     * If special processing is needed for a particular type your
     * subclass should override the method for that type.
     */
    virtual void updateByte(const unsigned char aValue)=0;

    /**
    * Perform the final computations and cleanup.
    */
    virtual void finish()=0;


    /**
     * Add a short value to the digest.
     */
    void updateShort(short aValue);

    /**
     * Add an integer value to the digest.
     */
    void updateInt(int aValue);

    /**
     * Add a long to the digest.
     */
    void updateLong(long aValue);

    /**
     * Add specific bytes to the digest.
     */
    void updateArray(const unsigned char* input, int len, int offset=0);

    /**
     * Add the bytes in the String 'input' to the current digest.
     * Note that the string characters are treated as unicode chars
     * of 16 bits each. To digestify ISO-Latin1 strings (ASCII) use
     * the updateASCII() method.
     */
    void updateW(const std::wstring &input);

    /**
     * Treat the string as a sequence of ISO-Latin1 (8 bit) characters.
     */
    void updateASCII(const std::string &input);


    const unsigned char * getDigestBits() const;
    
    const std::string getDigestHex() const;


};
