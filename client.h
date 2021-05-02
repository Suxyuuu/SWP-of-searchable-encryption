#include <iostream>
#include <string>
#include <vector>

#include <grpc++/grpc++.h>

#include "protoc/rpc.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

// grpc通信
class Client
{
public:
    explicit Client(std::shared_ptr<Channel> channel) : stub_(RPC::NewStub(channel)) {}

    std::string Setup();

    std::string Search(std::string &search_word);

    bool Add_Data(unsigned int &add_key, unsigned char (*add_value)[24], std::string &columnfamily);

    std::string Delete_Data(const std::string &del_cf);

    std::string Show_All();

    std::string Random_Gen_DB(int &num);

    std::string Destroy();

private:
    std::unique_ptr<RPC::Stub> stub_;
};

int client_operate(std::string &op);
