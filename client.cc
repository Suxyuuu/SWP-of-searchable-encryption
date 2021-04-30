#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include "rpc.grpc.pb.h"
#include "client.h"
#include "encryption.h"
#include "tools.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

std::string Client::Setup()
{
    SetupRequestMessage request;
    request.set_setup_request("setup");
    SetupResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<SetupResponseMessage>> rpc(stub_->Asyncsetup(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        return reply.setup_response();
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Search(std::string &search_word)
{
    SearchRequestMessage request;
    unsigned char Wi[20];
    word_string2unisignedchar(search_word, &Wi);
    unsigned char Xi[24];
    std::string const_ecb_key = "ecb_key";
    std::string const_hash_key = "hash_key";
    unsigned char ecb_key[24];
    key_string2unisignedchar(const_ecb_key, &ecb_key);
    unsigned char hash_key[24];
    key_string2unisignedchar(const_hash_key, &hash_key);

    des_ecb_encryption(ecb_key, Wi, &Xi);
    unsigned char Li[8];
    for (size_t i = 0; i < 8; i++)
    {
        Li[i] = Xi[i];
    }
    unsigned char Ki[16];
    HMAC(hash_key, 24, Li, 8, &Ki);
    for (auto &&i : Ki)
    {
        request.add_ki(i);
    }
    // std::cout << request.ki_size() << std::endl;
    for (auto &&i : Xi)
    {
        request.add_xi(i);
    }
    // std::cout << request.xi_size() << std::endl;

    SearchResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<SearchResponseMessage>> rpc(stub_->Asyncsearch(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        std::cout << reply.search_response() << std::endl;
        if (reply.search_cf_size() == 0)
        {
            return "Search Successfully! But no hitted.";
        }
        else
        {
            std::vector<std::string> cf;
            for (auto &&i : reply.search_cf())
            {
                cf.push_back(i);
            }

            std::vector<int> kv_num;
            int kv_all = 0;
            for (auto &&i : reply.search_kv_num())
            {
                kv_all += i;
                kv_num.push_back(i);
            }

            std::vector<int> key;
            for (auto &&i : reply.search_kv())
            {
                key.push_back(i.key());
            }

            std::vector<std::vector<unsigned char>> value;
            std::vector<unsigned char> en_word;
            // unsigned char en_word[24];
            for (auto &&item : reply.search_kv())
            {
                for (size_t i = 0; i < item.value_size(); i++)
                {
                    en_word.push_back(item.value()[i]);
                }
                value.push_back(en_word);
                en_word.clear();
            }

            int num = 0;
            int count = 0;
            for (size_t i = 0; i < cf.size(); i++)
            {
                std::cout << "COLUMNFAMILY NAME: " << cf[i] << ":" << std::endl;
                num = kv_num[i];
                for (size_t j = 0; j < num; j++)
                {
                    std::cout << key[j + count] << " : ";
                    for (size_t m = 0; m < 24; m++)
                    {
                        printf("%02x", value[j + count][m]);
                    }
                    std::cout << std::endl;

                    // jiemi
                    std::string const_ecb_key = "ecb_key";
                    std::string const_hash_key = "hash_key";
                    unsigned char word[20];
                    unsigned char encode[24];
                    std::cout << "Encode the value: ";
                    for (size_t a = 0; a < 24; a++)
                    {
                        encode[a] = value[j + count][a];
                    }
                    decode_swp(encode, const_ecb_key, const_hash_key, key[j + count], &word);
                    printf("%s\n", word);
                }
                count += num;
            }
            return "Search successfully!";
        }
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

bool Client::Add_Data(unsigned int &add_key, unsigned char (*add_value)[24], std::string &columnfamily)
{
    AddRequestMessage request;

    request.set_add_key(add_key);
    for (int i = 0; i < 24; i++)
    {
        request.add_add_value((*add_value)[i]);
    }
    request.set_add_columnfamily(columnfamily);

    AddResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<AddResponseMessage>> rpc(stub_->Asyncadd_data(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        return reply.add_response();
    }
    else
    {
        // return "Failed: Can't connect to server.";
        return false;
    }
}

std::string Client::Delete_Data(const std::string &del_cf)
{
    DeleteRequestMessage request;
    request.set_delete_columnfamily(del_cf);
    DeleteResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<DeleteResponseMessage>> rpc(stub_->Asyncdelete_data(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        return reply.delete_response();
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Show_All()
{
    ShowAllRequestMessage request;
    request.set_showall_request("show");
    ShowAllResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<ShowAllResponseMessage>> rpc(stub_->Asyncshow_all(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        std::vector<std::string> cf;
        for (auto &&i : reply.showall_cf())
        {
            cf.push_back(i);
        }

        std::vector<int> kv_num;
        int kv_all = 0;
        for (auto &&i : reply.kv_num())
        {
            kv_all += i;
            kv_num.push_back(i);
        }

        std::vector<int> key;
        for (auto &&i : reply.showall_kv())
        {
            key.push_back(i.key());
        }

        std::vector<std::vector<unsigned char>> value;
        std::vector<unsigned char> en_word;
        // unsigned char en_word[24];
        for (auto &&item : reply.showall_kv())
        {
            for (size_t i = 0; i < item.value_size(); i++)
            {
                en_word.push_back(item.value()[i]);
            }
            value.push_back(en_word);
            en_word.clear();
        }

        bool flag = false;
        flag = reply.showall_response();

        if (!flag)
        {
            return "Failed: Can't fetch data.";
        }
        else if (kv_all == 0)
        {
            return "null";
        }
        else
        {
            int num = 0;
            int count = 0;
            for (size_t i = 0; i < cf.size(); i++)
            {
                std::cout << "COLUMNFAMILY NAME: " << cf[i] << ":" << std::endl;
                num = kv_num[i];
                for (size_t j = 0; j < num; j++)
                {
                    std::cout << key[j + count] << " : ";
                    for (size_t m = 0; m < 24; m++)
                    {
                        printf("%02x", value[j + count][m]);
                    }
                    std::cout << std::endl;

                    // jiemi
                    std::string const_ecb_key = "ecb_key";
                    std::string const_hash_key = "hash_key";
                    unsigned char word[20];
                    unsigned char encode[24];
                    std::cout << "Encode the value: ";
                    for (size_t a = 0; a < 24; a++)
                    {
                        encode[a] = value[j + count][a];
                    }
                    decode_swp(encode, const_ecb_key, const_hash_key, key[j + count], &word);
                    printf("%s\n", word);
                }
                count += num;
            }
        }
        return "Show all data successfully!";
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Random_Gen_DB(int &num)
{
    std::vector<std::vector<std::string>> value;
    random_generate_DB(num, value);
    RandomGenerateDBRequestMessage request;

    for (size_t i = 0; i < num; i++)
    {
        request.add_cf_name("cf_" + std::to_string(i + 1));
    }

    std::string const_ecb_key = "ecb_key";
    std::string const_hash_key = "hash_key";
    for (auto &&onecf : value)
    {
        int key = 0; // 记录每个cf的键值对数量 同时作为键值 1-value1
        for (auto &&word : onecf)
        {
            unsigned char word_uchar[20];
            unsigned char encode_word[24];
            kv *k_v;
            k_v = request.add_gen_kv();
            word_string2unisignedchar(word, &word_uchar);
            encode_swp(word_uchar, const_ecb_key, const_hash_key, key + 1, &encode_word);
            k_v->set_key(key + 1);
            for (size_t j = 0; j < 24; j++)
            {
                k_v->add_value(encode_word[j]);
            }
            key++;
        }
        request.add_gen_kv_num(key);
    }

    RandomGenerateDBResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<RandomGenerateDBResponseMessage>> rpc(stub_->AsyncRanGenDB(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        return reply.gen_response();
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Destroy()
{
    DestroyRequestMessage request;
    request.set_destroy_request("clear");
    DestroyResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<DestroyResponseMessage>> rpc(stub_->AsyncDestroyDB(&context, request, &cq));
    rpc->Finish(&reply, &status, (void *)1);
    void *got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void *)1);
    GPR_ASSERT(ok);
    if (status.ok())
    {
        return reply.destroy_response();
    }
    else
    {
        return "Failed: Can't connect to server.";
    }
}

// 客户端操作指令处理 0--quit 1--setup 2--search 3--add 4--delete 5--showall 负值 error
int client_operate(std::string &op)
{
    if (op == "quit")
    {
        std::cout << "Client Shutdown." << std::endl;
        return 0;
    }
    else if (op == "setup")
    {
        std::cout << "Setup-ing..." << std::endl;
        return 1;
    }
    else if (op == "search")
    {
        std::cout << "Search-ing..." << std::endl;
        return 2;
    }
    else if (op == "add")
    {
        std::cout << "Add-ing..." << std::endl;
        return 3;
    }
    else if (op == "delete")
    {
        std::cout << "Delete-ing..." << std::endl;
        return 4;
    }
    else if (op == "show")
    {
        std::cout << "Get_data-ing..." << std::endl;
        return 5;
    }
    else if (op == "rangen")
    {
        std::cout << "Random_Generate_DB-ing..." << std::endl;
        return 6;
    }
    else if (op == "destroy")
    {
        std::cout << "Destroy_DB-ing..." << std::endl;
        return 7;
    }

    else
    {
        std::cout << "Input ERROR! ";
        return -1;
    }
}

int main(int argc, char **argv)
{
    Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::string operation;
    std::vector<std::string> operation_splited;
    int operation_code = 0;
    std::string reply;

    std::vector<std::string> re;

    while (std::cout << "> ", getline(std::cin, operation))
    {

        split(operation, operation_splited, ' ');

        operation_code = client_operate(operation_splited.front());
        if (operation_code < 0)
        {
            std::cout << "Please input your operation again: " << std::endl;
        }
        else if (operation_code == 0)
        {
            break;
        }
        else if (operation_code == 1)
        {
            reply = client.Setup();
            std::cout << reply << std::endl;
        }
        else if (operation_code == 2)
        {
            reply = client.Search(operation_splited[1]);
            std::cout << reply << std::endl;
        }
        else if (operation_code == 3)
        {
            unsigned int add_key = 0;
            unsigned char add_value[24];
            std::string columnfamily;
            if (operation_splited.size() < 4)
            {
                columnfamily = "default";
            }
            else
            {
                columnfamily = operation_splited[3];
            }

            add_key = stoi(operation_splited[1]);

            std::string const_ecb_key = "ecb_key";
            std::string const_hash_key = "hash_key";
            unsigned char word[20];
            word_string2unisignedchar(operation_splited[2], &word);

            encode_swp(word, const_ecb_key, const_hash_key, add_key, &add_value);

            bool flag = false;
            flag = client.Add_Data(add_key, &add_value, columnfamily);

            if (flag)
            {
                std::cout << "Add Successfully!" << std::endl;
            }
            else
            {
                std::cout << "Add Failed!" << std::endl;
            }
        }
        else if (operation_code == 4)
        {
            reply = client.Delete_Data(operation_splited[1]);
            std::cout << reply << std::endl;
        }
        else if (operation_code == 5)
        {

            reply = client.Show_All();
            if (reply == "null")
            {
                std::cout << "DB exists, but no data!" << std::endl;
            }
            else
            {
                std::cout << reply << std::endl;
            }
        }
        else if (operation_code == 6)
        {
            int num = stoi(operation_splited[1]);
            reply = client.Random_Gen_DB(num);
            std::cout << reply << std::endl;
        }
        else if (operation_code == 7)
        {
            reply = client.Destroy();
            std::cout << reply << std::endl;
        }
        else
        {
            ;
        }
    }

    return 0;
}