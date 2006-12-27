#pragma once

#include "crypto/MessageDigest.h"
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
class MD5 :  public MessageDigest {
    /** containss the computed message digest */

private: 
    int * state;
    unsigned long count;
    unsigned char * buffer;
    int *transformBuffer;

    enum { 
        S11=7,
        S12= 12,
        S13= 17,
        S14= 22,
        S21= 5,
        S22= 9,
        S23= 14,
        S24= 20,
        S31= 4,
        S32= 11,
        S33= 16,
        S34= 23,
        S41= 6,
        S42= 10,
        S43= 15,
        S44= 21
    };

    /**
     * Standard constructor, creates a new MD5 instance, allocates its
     * buffers from the heap.
     */
public: 
    MD5();
    ~MD5();


    /* **********************************************************
     * The MD5 Functions. These are copied verbatim from
     * the RFC to insure accuracy. The results of this
     * implementation were checked against the RSADSI version.
     * **********************************************************
     */

private:
    int F(int x, int y, int z); 
    int G(int x, int y, int z);
    int H(int x, int y, int z);
    int I(int x, int y, int z);

    int rotateLeft(int a, int n);

    int FF(int a, int b, int c, int d, int x, int s, int ac);

    int GG(int a, int b, int c, int d, int x, int s, int ac);

    int HH(int a, int b, int c, int d, int x, int s, int ac);

    int II(int a, int b, int c, int d, int x, int s, int ac);

    /**
     * This is where the functions come together as the generic MD5
     * transformation operation, it is called by update() which is
     * synchronized (to protect transformBuffer)
     */
    void transform(unsigned char * buf, int offset);

public:
    /**
     * Initialize the MD5 state information and reset the bit count
     * to 0. Given this implementation you are constrained to counting
     * 2^64 bits.
     */

    void init();

    /**
     * update adds the passed type to the input buffer
     */
    void updateByte(const unsigned char b);

    /**
     * Perform the final computations, any buffered bytes are added
     * to the digest, the count is added to the digest, and the resulting
     * digest is stored. After calling final you will need to call
     * init() again to do another digest.
     */
    void finish();

    const char * getAlg() const { return "MD5"; }
};
