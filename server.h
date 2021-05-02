#include <iostream>
#include <string>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "protoc/rpc.grpc.pb.h"

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

const std::string DBPATH = "rocksdb-data"; //rocksDB的数据存储目录绝对路径

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
        std::vector<unsigned char> Ki;
        std::vector<unsigned char> Xi;
        for (auto &&i : request->ki())
        {
            Ki.push_back(i);
        }
        for (auto &&i : request->xi())
        {
            Xi.push_back(i);
        }
        std::vector<std::string> cf;
        std::vector<int> kv_num;
        std::vector<int> key;
        std::vector<std::vector<unsigned char>> value;

        searchDB(Ki, Xi, cf, kv_num, key, value, msg);

        for (auto &&i : cf)
        {
            reply->add_search_cf(i);
        }
        for (auto &&i : kv_num)
        {
            reply->add_search_kv_num(i);
        }

        kv *k_v;
        for (size_t j = 0; j < key.size(); j++)
        {
            k_v = reply->add_search_kv();
            k_v->set_key(key[j]);
            for (size_t i = 0; i < 24; i++)
            {
                k_v->add_value(value[j][i]);
            }
        }
        reply->set_search_response(msg);

        return grpc::Status::OK;
    }

    grpc::Status add_data(ServerContext *context, const AddRequestMessage *request, AddResponseMessage *reply)
        override
    {
        int add_key;
        std::vector<unsigned char> add_value;
        std::string add_columnfamily;
        add_key = request->add_key();
        for (auto &&item : request->add_value())
        {
            add_value.push_back(item);
        }

        add_columnfamily = request->add_columnfamily();
        reply->set_add_response(addDB(add_key, add_value, add_columnfamily));
        return grpc::Status::OK;
    }

    grpc::Status delete_data(ServerContext *context, const DeleteRequestMessage *request, DeleteResponseMessage *reply)
        override
    {
        std::string data;
        deleteDB(request->delete_columnfamily(), msg);

        reply->set_delete_response(msg);

        return grpc::Status::OK;
    }

    grpc::Status show_all(ServerContext *context, const ShowAllRequestMessage *request, ShowAllResponseMessage *reply)
        override
    {
        std::vector<std::string> cf;
        std::vector<int> kv_num;
        std::vector<int> key;
        std::vector<std::vector<unsigned char>> value;

        reply->set_showall_response(showDB(cf, kv_num, key, value));
        for (auto &&i : cf)
        {
            reply->add_showall_cf(i);
        }
        for (auto &&i : kv_num)
        {
            reply->add_kv_num(i);
        }

        kv *k_v;
        for (size_t j = 0; j < key.size(); j++)
        {
            k_v = reply->add_showall_kv();
            k_v->set_key(key[j]);
            for (size_t i = 0; i < 24; i++)
            {
                k_v->add_value(value[j][i]);
            }
        }

        return grpc::Status::OK;
    }

    grpc::Status RanGenDB(ServerContext *context, const RandomGenerateDBRequestMessage *request, RandomGenerateDBResponseMessage *reply)
        override
    {
        std::vector<std::string> cf_name;
        for (auto &&i : request->cf_name())
        {
            cf_name.push_back(i);
        }
        std::vector<int> every_cf_kv_num;
        for (auto &&i : request->gen_kv_num())
        {
            every_cf_kv_num.push_back(i);
        }
        std::vector<int> key;
        std::vector<std::vector<unsigned char>> value;
        std::vector<unsigned char> one_word;
        for (auto &&k_v : request->gen_kv())
        {
            key.push_back(k_v.key());
            for (auto &&i : k_v.value())
            {
                one_word.push_back(i);
            }
            value.push_back(one_word);
            one_word.clear();
        }

        rangenDB(cf_name, every_cf_kv_num, key, value, msg);
        reply->set_gen_response(msg);
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
    void searchDB(std::vector<unsigned char> &Ki,
                  std::vector<unsigned char> &Xi,
                  std::vector<std::string> &cf,
                  std::vector<int> &kv_num,
                  std::vector<int> &key,
                  std::vector<std::vector<unsigned char>> &value,
                  std::string &msg);
    bool addDB(int add_key, std::vector<unsigned char> &add_value, std::string &add_columnfamily);
    void deleteDB(const std::string &del_cf, std::string &msg);
    bool showDB(std::vector<std::string> &cf,
                std::vector<int> &kv_num,
                std::vector<int> &key,
                std::vector<std::vector<unsigned char>> &value);
    void rangenDB(std::vector<std::string> &cf_name,
                  std::vector<int> &every_cf_kv_num,
                  std::vector<int> &key,
                  std::vector<std::vector<unsigned char>> &value,
                  std::string &msg);
    void destroyDB(std::string &msg);
};