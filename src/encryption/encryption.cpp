#include "encryption.h"

string Encryption::Encrypt(const string& password, const string& salt)
{
    std::string saltedPassword = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;

    if (!SHA256_Init(&sha256)) {
        throw std::runtime_error("SHA256_Init failed");
    }
    if (!SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.size())) {
        throw std::runtime_error("SHA256_Update failed");
    }
    if (!SHA256_Final(hash, &sha256)) {
        throw std::runtime_error("SHA256_Final failed");
    }

    return ByteArrayToHexString(hash, SHA256_DIGEST_LENGTH);
}

string Encryption::ByteArrayToHexString(const unsigned char* byteArray, int length)
{
    stringstream hexStream;
    for (size_t i = 0; i < length; ++i) {
        hexStream << hex << setw(2) << setfill('0') << static_cast<int>(byteArray[i]);
    }
    return hexStream.str();

}

Encryption encryption;