#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>

#include "encryption.h"

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

// test
// int main()
// {
//     std::vector<std::string> out;
//     int a = 10;
//     random_generate_DB(a, out);
//     for (auto &&item : out)
//     {
//         std::cout << item << std::endl;
//     }
//     return 0;
// }