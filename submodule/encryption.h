#include <iostream>
#include <vector>
#include <string>

void des_ecb_encryption(unsigned char *key, unsigned char *in, unsigned char (*out)[24]);

void des_ecb_decryption(unsigned char *key, unsigned char *in, unsigned char (*out)[20]);

void HMAC(unsigned char *key, int key_len, unsigned char *in, int in_len, unsigned char (*out)[16]);

void encode_swp(unsigned char *Wi,
                std::string &ecb_key,
                std::string &hash_key,
                int random_sequence_seed,
                unsigned char (*Ci)[24]);

void decode_swp(unsigned char *Ci,
                std::string &ecb_key,
                std::string &hash_key,
                int random_sequence_seed,
                unsigned char (*Wi)[20]);

bool search_swp(std::vector<unsigned char> &Ki_vector,
                std::vector<unsigned char> &Xi_vector,
                std::vector<unsigned char> &Ci_vector);
