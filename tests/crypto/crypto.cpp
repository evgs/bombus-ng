// crypto.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <string>
#include <tchar.h>
#include <assert.h>

#include "crypto/MD5.h"

void testMD5(const std::string & str, const std::string & hash) {
	MessageDigest *md=new MD5();
	md->init();
	md->updateASCII(str);
	md->finish();
	const std::string &result=md->getDigestHex();
	std::cout<< "MD5('" << str << "') = " << result;

	if (hash == result) {
		std::cout << " - OK" << std::endl;
	} else {
		std::cout << " - FAILED!!! Excepted=" << hash << std::endl;
	}
}

int _tmain(int argc, _TCHAR* argv[]){
	testMD5 ("", "d41d8cd98f00b204e9800998ecf8427e");
	testMD5 ("a", "0cc175b9c0f1b6a831c399e269772661");
	testMD5 ("abc", "900150983cd24fb0d6963f7d28e17f72");
	testMD5 ("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
	testMD5 ("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
	testMD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		"d174ab98d277d9f5a5611c2c9f419d9f");
	testMD5 ("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"57edf4a22be3c955ac49da2e2107b67a");
	return 0;
}

