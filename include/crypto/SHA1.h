#pragma once

#include "crypto/MessageDigest.h"
/*
 * SHA1.java - An implementation of the SHA-1 Algorithm
 *
 * This version by Chuck McManis (cmcmanis@netcom.com) and
 * still public domain.
 *
 * Based on the C code that Steve Reid wrote his header
 * was :
 *      SHA-1 in C
 *      By Steve Reid <steve@edmweb.com>
 *      100% Public Domain
 *
 *      Test Vectors (from FIPS PUB 180-1)
 *      "abc"
 *      A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 *      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *      84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 *      A million repetitions of "a"
 *      34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 * 
 * This file was obtained from: http://www.mcmanis.com/~cmcmanis/java/src/util/crypt/SHA1.java
 * More information can be found here: http://www.mcmanis.com/~cmcmanis/java/
 */

/**
 * This is a simple port of Steve Reid's SHA-1 code into Java.
 * I've run his test vectors through the code and they all pass.
 *
 */
class SHA1 : MessageDigest {
private:
    int state[5];
    long count;

public:
    SHA1();
    ~SHA1();

        /*
     * The following array forms the basis for the transform
     * buffer. Update puts bytes into this buffer and then
     * transform adds it into the state of the digest.
     */
private:
    int block[16];
    int blockIndex;

    /*
     * These functions are taken out of #defines in Steve's
     * code. Java doesn't have a preprocessor so the first
     * step is to just promote them to real methods.
     * Later we can optimize them out into inline code,
     * note that by making them final some compilers will
     * inline them when given the -O flag.
     */
    int rol(int value, int bits); 

    int blk0(int i);
    int blk(int i);

    void R0(int data[], int v, int w, int x , int y, int z, int i);
    void R1(int data[], int v, int w, int x, int y, int z, int i);
    void R2(int data[], int v, int w, int x, int y, int z, int i);
    void R3(int data[], int v, int w, int x, int y, int z, int i); 
    void R4(int data[], int v, int w, int x, int y, int z, int i);


    /*
     * Steve's original code and comments :
     *
     * blk0() and blk() perform the initial expand.
     * I got the idea of expanding during the round function from SSLeay
     *
     * #define blk0(i) block->l[i]
     * #define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
     *   ^block->l[(i+2)&15]^block->l[i&15],1))
     *
     * (R0+R1), R2, R3, R4 are the different operations used in SHA1
     * #define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
     * #define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
     * #define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
     * #define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
     * #define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);
     */

    int dd[5];

    /**
     * Hash a single 512-bit block. This is the core of the algorithm.
     *
     * Note that working with arrays is very inefficent in Java as it
     * does a class cast check each time you store into the array.
     *
     */

    void transform();

    /**
     *
     * SHA1Init - Initialize new context
     */
public:
    virtual void init();

    /**
     * Add one byte to the digest. When this is implemented
     * all of the abstract class methods end up calling
     * this method for types other than bytes.
     */
    virtual void update(const unsigned char b);

    /**
     * Complete processing on the message digest.
     */
    virtual void finish() ;

    /** Return a string that identifies this algorithm */
    const char * getAlg() const { return "SHA1"; }

    
};