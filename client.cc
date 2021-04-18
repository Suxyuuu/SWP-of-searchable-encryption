#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
 
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
 
#include "rpc.grpc.pb.h"
#include "client.h"
 
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
 

std::string Client::Setup(const std::string& num){
    SetupRequestMessage request;
    request.set_setup_request(num);
    request.set_setup_request2(num);
    SetupResponseMessage reply;
    ClientContext context; 
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<SetupResponseMessage>> rpc(stub_->Asyncsetup(&context, request, &cq));
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok); 
    if (status.ok()){
        return reply.setup_response();
    }
    else {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Search(const bool &iskey, const std::string& key, std::vector<std::string>&re){
    re.clear();
    SearchRequestMessage request;
    request.set_is_key(iskey);
    request.set_search_request(key);
    SearchResponseMessage reply;
    ClientContext context; 
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<SearchResponseMessage> > rpc(stub_->Asyncsearch(&context, request, &cq));
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));   
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok); 
    if (status.ok()){
        for (int i = 0; i < reply.search_response_size(); i++)
        {
            re.push_back(reply.search_response()[i]);
        }
        return "Search results: ";
    }
    else {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Add_Data(const std::string& add_key, const std::string& add_value){
    AddRequestMessage request;
    request.set_add_request_key(add_key);
    request.set_add_request_value(add_value);
    AddResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<AddResponseMessage> > rpc(stub_->Asyncadd_data(&context, request, &cq));
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));   
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok); 
    if (status.ok()){
        return reply.add_response();
    }
    else {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Delete_Data(const bool& iskey, const std::string& del_value, std::vector<std::string>&re){
    DeleteRequestMessage request;
    request.set_is_key(iskey);
    request.set_delete_request(del_value);
    DeleteResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<DeleteResponseMessage> > rpc(stub_->Asyncdelete_data(&context, request, &cq));
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));   
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok); 
    if (status.ok()){
        for (int i = 0; i < reply.delete_response_size(); i++)
        {
            re.push_back(reply.delete_response()[i]);
        }
        return "Delete results: ";
    }
    else {
        return "Failed: Can't connect to server.";
    }
}

std::string Client::Show_All(const std::string& op, std::vector<std::string>&re){
    re.clear();
    ShowAllRequestMessage request;
    request.set_showall_request(op);
    ShowAllResponseMessage reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<ShowAllResponseMessage> > rpc(stub_->Asyncshow_all(&context, request, &cq));
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));   
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok); 
    if (status.ok()){
        for (int i = 0; i < reply.showall_response_size(); i++)
        {
            re.push_back(reply.showall_response()[i]);
        }
        return "Show results:";
    }
    else {
        return "Failed: Can't connect to server.";
    }
}

// 客户端操作指令处理 0--quit 1--setup 2--search 3--add 4--delete 5--showall 负值 error
int client_operate(std::string &op){
    if (op == "quit")
    {
        std::cout<<"Client Shutdown."<<std::endl;
        return 0;
    }
    else if (op=="setup")
    {
        std::cout<<"Setup-ing..."<<std::endl;
        return 1;
    }
    else if (op=="search")
    {
        std::cout<<"Search-ing..."<<std::endl;
        return 2;
    }
    else if (op=="add")
    {
        std::cout<<"Add-ing..."<<std::endl;
        return 3;
    }
    else if (op=="delete")
    {
        std::cout<<"Delete-ing..."<<std::endl;
        return 4;
    }
    else if (op=="show")
    {
        std::cout<<"Get_data-ing..."<<std::endl;
        return 5;
    }
    
    else
    {
        std::cout<<"Input ERROR! ";
        return -1;
    }
    
}

// 划分字符串
void split(const std::string& s, std::vector<std::string>& sv, const char flag) {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return;
}

int main(int argc, char** argv) {
    Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    
    std::string operation;
    std::vector<std::string> operation_splited;
    int operation_code=0;
    std::string reply;

    std::vector<std::string> re;
    
    while (std::cout<<"> ",getline(std::cin,operation))
    {
        
        split(operation, operation_splited, ' ');

        operation_code = client_operate(operation_splited.front());
        if (operation_code<0)
        {
            std::cout << "Please input your operation again: " << std::endl;
        }
        else if (operation_code==0)
        {
            break;
        }
        else if (operation_code==1)
        {
            reply = client.Setup(operation_splited[1]);
            std::cout << reply << std::endl;
        }
        else if (operation_code==2)
        {
            re.clear();
            reply = client.Search(stoi(operation_splited[1]), operation_splited[2], re);
            std::cout << reply << std::endl;
            for (auto &&item : re)
            {
                std::cout<<item<<std::endl;
            }
        }
        else if (operation_code==3)
        {
            reply=client.Add_Data(operation_splited[1],operation_splited[2]);
            std::cout<<reply <<std::endl;
        }
        else if (operation_code==4)
        {
            re.clear();
            reply=client.Delete_Data(stoi(operation_splited[1]), operation_splited[2], re);
            std::cout<<reply <<std::endl;
            for (auto &&item : re)
            {
                std::cout<<item<<std::endl;
            }
        }
        else if (operation_code==5)
        {  
            re.clear();
            reply=client.Show_All(operation_splited[0],re);
            std::cout<<reply <<std::endl;
            if (re.empty())
            {
                std::cout<<"DB exists, but no data!"<<std::endl;
            }
            else
            {
                for (auto &&item : re)
                {
                    std::cout<<item<<std::endl;
                }
            }
        }
        
        else
        {
            ;
        } 
    }
    
    return 0;
}