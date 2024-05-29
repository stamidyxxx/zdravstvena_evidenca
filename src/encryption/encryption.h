#pragma once
#include "../includes.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>

#define AES_KEY_LENGTH 256
#define AES_BLOCK_SIZE 128


class Encryption
{
public:
	string Encrypt(const string& password, const string& salt);
	string ByteArrayToHexString(const unsigned char* byteArray, int length);
};

extern Encryption encryption;