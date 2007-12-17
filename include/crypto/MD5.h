#pragma once

#include "crypto/MessageDigest.h"

class MD5 :  public MessageDigest {
    /** containss the computed message digest */

private: 
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];

	void MD5Update (const unsigned char *input, unsigned int inputLen);

public: 
    MD5();
    ~MD5();

public:

    void init();

    void updateByte(const unsigned char b);
    void updateArray(const unsigned char* input, int len, int offset=0);

    void finish();

    const char * getAlg() const { return "MD5"; }
};
