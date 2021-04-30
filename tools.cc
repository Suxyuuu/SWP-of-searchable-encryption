#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>

// #include "tools.h"

// Wi 每个单词最长为20字符 160位
void random_generate_DB(int &num, std::vector<std::string> &value)
{
    static std::default_random_engine e(time(0));
    static std::uniform_int_distribution<unsigned> u(0, 25);  // 生成随机字母
    static std::uniform_int_distribution<unsigned> u1(1, 20); // 生成单个单词的随机长度
    static std::uniform_int_distribution<unsigned> u2(5, 15); // 生成单个消息value的随机单词个数
    std::string str;
    int word_num;
    for (int i = 0; i < num; i++)
    {
        word_num = u2(e);
        for (int j = 0; j < word_num; j++)
        {
            for (unsigned m = 0; m < u1(e); m++)
            {
                str += (char)(u(e) + 97);
            }
            if (j != word_num - 1)
            {
                str += " ";
            }
        }
        value.push_back(str);
        str.clear();
    }
}

void random_generate_DB(int &num, std::vector<std::vector<std::string>> &value)
{
    static std::default_random_engine e(time(0));
    static std::uniform_int_distribution<unsigned> u(0, 25);  // 生成随机字母
    static std::uniform_int_distribution<unsigned> u1(1, 20); // 生成单个单词的随机长度
    static std::uniform_int_distribution<unsigned> u2(5, 15); // 生成单个消息value的随机单词个数
    std::string word;
    std::vector<std::string> one_cf;
    int word_num;
    for (int i = 0; i < num; i++)
    {
        word_num = u2(e);
        for (int j = 0; j < word_num; j++)
        {
            for (unsigned m = 0; m < u1(e); m++)
            {
                word += (char)(u(e) + 'a');
            }
            one_cf.push_back(word);
            word.clear();
        }
        value.push_back(one_cf);
        one_cf.clear();
    }
}

void random_sequence(int seed, unsigned char (*a)[8])
{
    std::default_random_engine e(seed);
    std::uniform_int_distribution<unsigned> u(0, 255);
    for (int i = 0; i < 8; i++)
    {
        (*a)[i] = u(e);
    }
    // printf("a:");
    // for (int i = 0; i != 8; i++)
    //     printf("%02x", a[i]);
    // printf("\n");
}

void split(const std::string &s, std::vector<std::string> &sv, const char flag)
{
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (getline(iss, temp, flag))
    {
        sv.push_back(temp);
    }
    return;
}

void key_string2unisignedchar(std::string &key, unsigned char (*output)[24])
{
    for (int i = 0; i < 24; i++)
    {
        if (i < key.size())
        {
            (*output)[i] = key[i];
        }
        else
        {
            (*output)[i] = 0;
        }
    }
}

void word_string2unisignedchar(std::string &word, unsigned char (*output)[20])
{

    for (int i = 0; i < 20; i++)
    {
        if (i < word.size())
        {
            (*output)[i] = word[i];
        }
        else
        {
            (*output)[i] = 0;
        }
    }
}

// int main()
// {
//     int num = 10;
//     std::vector<std::vector<std::string>> value;
//     random_generate_DB(num, value);
//     for (auto &&i : value)
//     {
//         for (auto &&word : i)
//         {
//             std::cout << word << " ";
//         }
//         std::cout << std::endl;
//     }

//     return 0;
// }
