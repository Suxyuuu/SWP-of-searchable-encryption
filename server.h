#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "rpc.grpc.pb.h"

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

const std::string DBPATH = "/home/suu/Documents/rocksdbtemp"; //rocksDB的数据存储目录绝对路径

class ServiceImpl final : public RPC::Service
{
    std::string msg;
    std::vector<std::string> re;

    grpc::Status setup(ServerContext *context, const SetupRequestMessage *request, SetupResponseMessage *reply)
        override
    {
        setupDB(msg);
        reply->set_setup_response(msg);
        return grpc::Status::OK;
    }

    grpc::Status search(ServerContext *context, const SearchRequestMessage *request, SearchResponseMessage *reply)
        override
    {
        std::string data;
        searchDB(request->is_key(), request->search_request(), data, re);
        for (auto &&item : re)
        {
            reply->add_search_response(item);
        }
        return grpc::Status::OK;
    }

    grpc::Status add_data(ServerContext *context, const AddRequestMessage *request, AddResponseMessage *reply)
        override
    {
        addDB(request->add_request_key(), request->add_request_value(), msg);
        reply->set_add_response(msg);
        return grpc::Status::OK;
    }

    grpc::Status delete_data(ServerContext *context, const DeleteRequestMessage *request, DeleteResponseMessage *reply)
        override
    {
        std::string data;
        deleteDB(request->is_key(), request->delete_request(), re);
        for (auto &&item : re)
        {
            reply->add_delete_response(item);
        }
        return grpc::Status::OK;
    }

    grpc::Status show_all(ServerContext *context, const ShowAllRequestMessage *request, ShowAllResponseMessage *reply)
        override
    {

        showDB(re);
        for (auto &&item : re)
        {
            reply->add_showall_response(item);
        }
        return grpc::Status::OK;
    }

    grpc::Status RanGenDB(ServerContext *context, const RandomGenerateDBRequestMessage *request, RandomGenerateDBResponseMessage *reply)
        override
    {
        std::vector<std::string> keyv;
        std::vector<std::string> valuev;
        for (auto &&item : request->gen_request_key())
        {
            keyv.push_back(item);
        }
        for (auto &&item : request->gen_request_value())
        {
            valuev.push_back(item);
        }
        rangenDB(keyv, valuev, msg);
        reply->set_gen_response(msg);
        return grpc::Status::OK;
    }

    grpc::Status ClearDB(ServerContext *context, const ClearRequestMessage *request, ClearResponseMessage *reply)
        override
    {
        clearDB(msg);
        reply->set_clear_response(msg);
        return grpc::Status::OK;
    }

    grpc::Status DestroyDB(ServerContext *context, const DestroyRequestMessage *request, DestroyResponseMessage *reply)
        override
    {
        destroyDB(msg);
        reply->set_destroy_response(msg);
        return grpc::Status::OK;
    }

    void setupDB(std::string &msg);
    void searchDB(const bool &iskey, const std::string &key, std::string &data, std::vector<std::string> &re);
    void addDB(const std::string &key, const std::string &value, std::string &msg);
    void deleteDB(const bool &iskey, const std::string &key, std::vector<std::string> &re);
    void showDB(std::vector<std::string> &re);
    void rangenDB(std::vector<std::string> &keyv, std::vector<std::string> &valuev, std::string &msg);
    void clearDB(std::string &msg);
    void destroyDB(std::string &msg);
};