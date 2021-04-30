#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cstring>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "encryption.h"
#include "tools.h"

// ecb模式进行des加密 以单词Wi为单位
// 加密前：Wi 20字符 160位长
// 加密后：Xi 24字符 192位长
void des_ecb_encryption(unsigned char *key, unsigned char *in, unsigned char (*out)[24])
{
    int ret = 1;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER *cipher;

    unsigned char iv[8];

    for (int i = 0; i < 8; i++)
    {

        memset(&iv[i], 0, 1);
    }

    EVP_CIPHER_CTX_init(ctx);
    cipher = EVP_des_ecb();
    ret = EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);
    if (ret != 1)
    {

        printf("EVP_EncryptInit_ex err1!\n");
    }

    int len = 0;
    int outlen = 0;

    EVP_EncryptUpdate(ctx, *out + len, &outlen, in, 20);

    len += outlen;

    EVP_EncryptFinal_ex(ctx, *out + len, &outlen);
    len += outlen;

    EVP_CIPHER_CTX_cleanup(ctx);

    // unsigned char[] 转为string
}

// ecb模式进行des解密
void des_ecb_decryption(unsigned char *key, unsigned char *in, unsigned char (*out)[20])
{
    int ret = 1;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER *cipher;

    unsigned char iv[8];

    for (int i = 0; i < 8; i++)
    {

        memset(&iv[i], 0, 1);
    }

    EVP_CIPHER_CTX_init(ctx);
    cipher = EVP_des_ecb();

    ret = EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);

    if (ret != 1)
    {

        printf("EVP_DecryptInit_ex err1!\n");
    }

    int total = 0;
    int outlen = 0;

    ret = EVP_DecryptUpdate(ctx, *out, &outlen, in, 24);

    total += outlen;

    EVP_DecryptFinal_ex(ctx, *out + total, &outlen);

    EVP_CIPHER_CTX_cleanup(ctx);
}

// 带密钥的hash函数
void HMAC(unsigned char *key, int key_len, unsigned char *in, int in_len, unsigned char (*out)[16])
{

    unsigned int outlen = 0;
    HMAC_CTX *c = HMAC_CTX_new();
    HMAC_CTX_reset(c);
    HMAC_Init_ex(c, key, key_len, EVP_md5(), NULL);
    HMAC_Update(c, in, in_len);
    HMAC_Final(c, *out, &outlen);
    HMAC_CTX_free(c);
}

void encode_swp(unsigned char *Wi,
                std::string &ecb_key,
                std::string &hash_key,
                int random_sequence_seed,
                unsigned char (*Ci)[24])
{
    unsigned char ecb_k[24];
    key_string2unisignedchar(ecb_key, &ecb_k);
    unsigned char Xi[24];
    des_ecb_encryption(ecb_k, Wi, &Xi);

    unsigned char Li[8];
    unsigned char Ri[16];
    for (int i = 0; i < 8; i++)
    {
        Li[i] = Xi[i];
    }
    for (int i = 0; i < 16; i++)
    {
        Ri[i] = Xi[i + 8];
    }

    unsigned char hash_k[24];
    key_string2unisignedchar(hash_key, &hash_k);
    unsigned char Ki[16];
    HMAC(hash_k, 24, Li, 8, &Ki);

    unsigned char Si[8];
    random_sequence(random_sequence_seed, &Si);

    unsigned char Fki[16];
    HMAC(Ki, 16, Si, 8, &Fki);

    unsigned char Ti[24];
    for (int i = 0; i < 8; i++)
    {
        Ti[i] = Si[i];
    }
    for (int i = 0; i < 16; i++)
    {
        Ti[i + 8] = Fki[i];
    }

    for (int i = 0; i < 24; i++)
    {
        (*Ci)[i] = Ti[i] ^ Xi[i];
    }
}

void decode_swp(unsigned char *Ci,
                std::string &ecb_key,
                std::string &hash_key,
                int random_sequence_seed,
                unsigned char (*Wi)[20])
{
    unsigned char ecb_k[24];
    key_string2unisignedchar(ecb_key, &ecb_k);
    unsigned char hash_k[24];
    key_string2unisignedchar(hash_key, &hash_k);
    unsigned char Si[8];
    random_sequence(random_sequence_seed, &Si);

    unsigned char Li[8];
    unsigned char Ri[16];
    unsigned char Ki[16];
    unsigned char Fki[16];
    unsigned char Xi[24];

    //jiemi
    for (int i = 0; i < 8; i++)
    {
        Li[i] = Si[i] ^ Ci[i];
    }
    HMAC(hash_k, 24, Li, 8, &Ki);
    HMAC(Ki, 16, Si, 8, &Fki);
    for (int i = 0; i < 16; i++)
    {
        Ri[i] = Ci[i + 8] ^ Fki[i];
    }
    for (int i = 0; i < 8; i++)
    {
        Xi[i] = Li[i];
    }
    for (int i = 0; i < 16; i++)
    {
        Xi[i + 8] = Ri[i];
    }

    des_ecb_decryption(ecb_k, Xi, Wi);
}

bool search_swp(std::vector<unsigned char> &Ki_vector,
                std::vector<unsigned char> &Xi_vector,
                std::vector<unsigned char> &Ci_vector)
{
    unsigned char Ki[16];
    unsigned char Xi[24];
    unsigned char Ci[24];
    unsigned char Ti[24];
    unsigned char TiL[8];
    unsigned char TiR[16];
    for (size_t i = 0; i < 24; i++)
    {
        Xi[i] = Xi_vector[i];
        Ci[i] = Ci_vector[i];
        Ti[i] = Xi[i] ^ Ci[i];
    }
    for (size_t i = 0; i < 8; i++)
    {
        TiL[i] = Ti[i];
    }
    for (size_t i = 0; i < 16; i++)
    {
        Ki[i] = Ki_vector[i];
        TiR[i] = Ti[i + 8];
    }
    unsigned char out[16];
    HMAC(Ki, 16, TiL, 8, &out);
    for (size_t i = 0; i < 16; i++)
    {
        if (TiR[i] != out[i])
        {
            return false;
        }
    }
    return true;
}
// int main()
// {
//     unsigned int add_key = 1;
//     unsigned char add_value[24];
//     std::string const_ecb_key = "ecb_key";
//     std::string const_hash_key = "hash_key";
//     unsigned char word[20];
//     std::string mingwen = "mingwen";
//     word_string2unisignedchar(mingwen, &word);

//     encode_swp(word, const_ecb_key, const_hash_key, add_key, &add_value);

//     std::vector<unsigned char> Ci_vector;
//     for (size_t i = 0; i < 24; i++)
//     {
//         Ci_vector.push_back(add_value[i]);
//     }
//     std::vector<unsigned char> Xi_vector;
//     std::vector<unsigned char> Ki_vector;

//     unsigned char ecb_key[24];
//     key_string2unisignedchar(const_ecb_key, &ecb_key);
//     unsigned char hash_key[24];
//     key_string2unisignedchar(const_hash_key, &hash_key);
//     unsigned char Xi[24];
//     des_ecb_encryption(ecb_key, word, &Xi);
//     for (size_t i = 0; i < 24; i++)
//     {
//         Xi_vector.push_back(Xi[i]);
//     }

//     unsigned char Li[8];
//     for (size_t i = 0; i < 8; i++)
//     {
//         Li[i] = Xi[i];
//     }
//     unsigned char Ki[16];
//     HMAC(hash_key, 24, Li, 8, &Ki);
//     for (size_t i = 0; i < 16; i++)
//     {
//         Ki_vector.push_back(Ki[i]);
//     }

//     if (search_swp(Ki_vector, Xi_vector, Ci_vector))
//     {
//         printf("true\n");
//     }
//     else
//     {
//         printf("false\n");
//     }

//     // decode_swp(add_value, const_ecb_key, const_hash_key, add_key, &word);
//     // printf("%s\n", word);
//     return 0;
// }

// int main()

// {
//     // jiami
//     std::string word = "input1";
//     unsigned char Wi[20];
//     for (int i = 0; i < 20; i++)
//     {
//         if (i < word.size())
//         {
//             Wi[i] = word[i];
//         }
//         else
//         {
//             Wi[i] = 0;
//         }
//     }

//     std::string key = "ecb"; // ecb加密的key
//     unsigned char key1[24];
//     for (int i = 0; i < 24; i++)
//     {
//         if (i < key.size())
//         {
//             key1[i] = key[i];
//         }
//         else
//         {
//             key1[i] = 0;
//         }
//     }

// unsigned char Xi[24];
// des_ecb_encryption(key1, Wi, &Xi);

// unsigned char Li[8];
// unsigned char Ri[16];
// for (int i = 0; i < 8; i++)
// {
//     Li[i] = Xi[i];
// }
// for (int i = 0; i < 16; i++)
// {
//     Ri[i] = Xi[i + 8];
// }

//     key = "hash";
//     unsigned char key2[24];
//     for (int i = 0; i < 24; i++)
//     {
//         if (i < key.size())
//         {
//             key2[i] = key[i];
//         }
//         else
//         {
//             key2[i] = 0;
//         }
//     }

//     unsigned char key3[16];
//     HMAC(key2, 24, Li, 8, &key3);

//     unsigned char Si[8];
//     random_sequence(6, &Si);

//     unsigned char Fki[16];
//     HMAC(key3, 16, Si, 8, &Fki);

//     unsigned char Ti[24];
//     for (int i = 0; i < 8; i++)
//     {
//         Ti[i] = Si[i];
//     }
//     for (int i = 0; i < 16; i++)
//     {
//         Ti[i + 8] = Fki[i];
//     }

//     unsigned char Ci[24];
//     for (int i = 0; i < 24; i++)
//     {
//         Ci[i] = Ti[i] ^ Xi[i];
//     }

//     // jiansuo
//     for (int i = 0; i < 24; i++)
//     {
//         Ti[i] = Ci[i] ^ Xi[i];
//     }

//     unsigned char TiL[8];
//     unsigned char TiR[16];
//     for (int i = 0; i < 8; i++)
//     {
//         TiL[i] = Ti[i];
//     }
//     for (int i = 0; i < 16; i++)
//     {
//         TiR[i] = Ti[i + 8];
//     }

//     unsigned char TiL_hmac[16];
//     HMAC(key3, 16, TiL, 8, &TiL_hmac);
//     int flag = 1;
//     for (int i = 0; i < 16; i++)
//     {
//         if (TiL_hmac[i] != TiR[i])
//         {
//             flag = 0;
//         }
//     }
//     if (flag == 1)
//     {
//         printf("Search Successful!\n");
//     }
//     else
//     {
//         printf("Search Failed!\n");
//     }

//     //jiemi
//     for (int i = 0; i < 8; i++)
//     {
//         Li[i] = Si[i] ^ Ci[i];
//     }
//     HMAC(key2, 24, Li, 8, &key3);
//     HMAC(key3, 16, Si, 8, &Fki);
//     for (int i = 0; i < 16; i++)
//     {
//         Ri[i] = Ci[i + 8] ^ Fki[i];
//     }
//     for (int i = 0; i < 8; i++)
//     {
//         Xi[i] = Li[i];
//     }
//     for (int i = 0; i < 16; i++)
//     {
//         Xi[i + 8] = Ri[i];
//     }

//     des_ecb_decryption(key1, Xi, &Wi);
//     printf("%s\n", Wi);

//     return 0;
// }