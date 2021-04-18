#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
 
#include "rpc.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

// grpc通信 
class Client {
    public:
        explicit Client(std::shared_ptr<Channel> channel):stub_(RPC::NewStub(channel)) {}
        
        std::string Setup(const std::string& num);

        std::string Search(const bool &iskey, const std::string& key, std::vector<std::string>&re);

        std::string Add_Data(const std::string& add_key, const std::string& add_value);

        std::string Delete_Data(const bool& iskey, const std::string& del_value, std::vector<std::string>&re);

        std::string Show_All(const std::string& op, std::vector<std::string>&re);

    private:
        std::unique_ptr<RPC::Stub>stub_;
};

int client_operate(std::string &op);

void split(const std::string& s, std::vector<std::string>& sv, const char flag);
