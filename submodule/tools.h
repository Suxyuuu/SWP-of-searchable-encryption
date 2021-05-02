#include <iostream>
#include <vector>
#include <string>

void random_generate_DB(int &num, std::vector<std::string> &value);
void random_generate_DB(int &num, std::vector<std::vector<std::string>> &value);

void split(const std::string &s, std::vector<std::string> &sv, const char flag);

void random_sequence(int seed, unsigned char (*a)[8]);

void key_string2unisignedchar(std::string &key, unsigned char (*output)[24]);

void word_string2unisignedchar(std::string &word, unsigned char (*output)[20]);