#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "rpc.grpc.pb.h"
#include "server.h"

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

// 在默认路径创建一个新的数据库
// 输出：执行结果msg
void ServiceImpl::setupDB(std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    options.create_if_missing = true;
    options.error_if_exists = true;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    // 不存在数据库则建立新的并添加键值对，存在则不做改变
    if (r_status.ok())
    {
        msg = "Setup Successfully! ";
    }
    else
    {
        msg = "DB Exists! If you want to update, you can use 'add' or 'delete'. ";
    }

    delete db;
}

// 查询数据库 支持用key查询 同时支持用value查询
// 输入：1-是否是key 2-是key则为key的值 不是key则为value
// 输出：1-是key则输出对应的value 2-不是key则返回所有与value相等的k—v对
void ServiceImpl::searchDB(const bool &iskey, const std::string &key, std::string &data, std::vector<std::string> &re)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    re.clear();
    if (r_status.ok())
    {
        if (iskey)
        {
            r_status = db->Get(rocksdb::ReadOptions(), key, &data);

            if (r_status.ok())
            {
                re.push_back("[Key:Value] = [ " + key + " : " + data + " ]");
            }
            else
            {
                re.push_back("Search Failed! No Key is equal to \'" + key + "\' ");
            }
        }
        else
        {
            rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
            bool isfound = false;
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                if (key == it->value().ToString())
                {
                    isfound = true;
                    re.push_back("[Key:Value] = [ " + it->key().ToString() + " : " + key + " ]");
                }
            }
            if (!isfound)
            {
                re.push_back("Search Failed! No Value is equal to \'" + key + "\' ");
            }
            assert(it->status().ok()); // Check for any errors found during the scan
            delete it;
        }
    }
    else
    {
        re.push_back("No DB! Please setup a DB first! ");
    }
    delete db;
}

// 添加k-v对
// 输入：1-key 2-value
// 输出：执行结果msg
void ServiceImpl::addDB(const std::string &add_key, const std::string &add_value, std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    if (r_status.ok())
    {
        rocksdb::Slice key(add_key);
        rocksdb::Slice value(add_value);
        r_status = db->Put(rocksdb::WriteOptions(), key, value);
        if (r_status.ok())
        {
            msg = "Add Successfully! ";
        }
        else
        {
            msg = "Add Failed! ";
        }
    }
    else
    {
        msg = "No DB! Please setup a DB first! ";
    }

    delete db;
}

// 删除k-v对
// 输入：1-是否是key 2-是key则为key的值 不是key则为value
// 输出：删除结果
void ServiceImpl::deleteDB(const bool &iskey, const std::string &key, std::vector<std::string> &re)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    re.clear();
    if (r_status.ok())
    {
        if (iskey)
        {
            std::string data;
            r_status = db->Get(rocksdb::ReadOptions(), key, &data);
            if (r_status.ok())
            {
                r_status = db->Delete(rocksdb::WriteOptions(), key);
                if (r_status.ok())
                {
                    re.push_back("[Key:Value] = [ " + key + " : " + data + " ] was deleted! ");
                    re.push_back("Delete Successfully! DeletedItems_num:1. ");
                }
                else
                {
                    re.push_back("Delete Failed! Some Error in DB.");
                }
            }
            else
            {
                re.push_back("Delete Failed! No Key is equal to \'" + key + "\' ");
            }
        }
        else
        {
            rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
            int foundnum = 0;
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                if (key == it->value().ToString())
                {
                    foundnum++;
                    re.push_back("[Key:Value] = [ " + it->key().ToString() + " : " + key + " ] was deleted! ");
                    db->Delete(rocksdb::WriteOptions(), it->key());
                }
            }
            if (!foundnum)
            {
                re.push_back("Delete Failed! No Value is equal to \'" + key + "\' ");
            }
            else
            {
                re.push_back("Delete Successfully! DeletedItems_num:" + std::to_string(foundnum) + ". ");
            }

            assert(it->status().ok()); // Check for any errors found during the scan
            delete it;
        }
    }
    else
    {
        msg = "No DB! Please setup a DB first! ";
    }
    delete db;
}

// 展示数据库所有数据 以k-v对的形式展示
void ServiceImpl::showDB(std::vector<std::string> &re)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);

    re.clear();

    if (r_status.ok())
    {
        rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            re.push_back("[Key:Value] = [ " + it->key().ToString() + " : " + it->value().ToString() + " ]");
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
    else
    {
        re.push_back("No DB! Please setup a DB first! ");
    }

    delete db;
}

// 根据client产生的随机数据生成数据库(需要数据库已经存在)
// 输入：1-多个key值 2-多个value值
// 输出：执行结果
void ServiceImpl::rangenDB(std::vector<std::string> &keyv, std::vector<std::string> &valuev, std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    rocksdb::Slice key();
    rocksdb::Slice value();
    // r_status = db->Put(rocksdb::WriteOptions(), key, value);

    if (r_status.ok())
    {
        int success_num;
        for (int i = 0; i < keyv.size(); i++)
        {
            rocksdb::Slice key(keyv[i]);
            rocksdb::Slice value(valuev[i]);
            r_status = db->Put(rocksdb::WriteOptions(), key, value);
            if (r_status.ok())
            {
                msg = "Add Successfully! ";
                success_num++;
            }
            else
            {
                msg = "Add Failed! ";
                success_num = i;
                break;
            }
        }
        if (success_num != keyv.size())
        {
            msg = "Random GenerateDB broke off! Add successfully item_num is " + std::to_string(success_num) + ".";
        }
        else
        {
            msg = "Random GenerateDB successfully!";
        }
    }
    else
    {
        msg = "No DB! Please setup a DB first! ";
    }

    delete db;
}

// 清除数据库所有数据(不删除数据库本身)
// 输出：执行结果
void ServiceImpl::clearDB(std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    r_status = rocksdb::DB::Open(options, DBPATH, &db);

    if (r_status.ok())
    {
        rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
        it->SeekToFirst();
        rocksdb::Slice start(it->key().ToString());
        it->SeekToLast();
        rocksdb::Slice end(it->key().ToString());
        r_status = db->DeleteRange(rocksdb::WriteOptions(), db->DefaultColumnFamily(), start, end);
        if (r_status.ok())
        {
            it->SeekToLast();
            r_status = db->Delete(rocksdb::WriteOptions(), it->key());
            if (r_status.ok())
            {
                msg = "Clear DB Successfully! ";
            }
            else
            {
                msg = "Clear Failed! ";
            }
        }
        else
        {
            msg = "Clear Failed! ";
        }
        delete it;
    }
    else
    {
        msg = "No DB! Please setup a DB first! ";
    }

    delete db;
}

// 删除数据库本身
// 输出：执行结果
void ServiceImpl::destroyDB(std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::DestroyDB(DBPATH, options);

    // options.error_if_exists = true;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);

    if (r_status.ok())
    {
        msg = "Destroy Failed!";
    }
    else
    {
        msg = "Destroy Successfully! ";
    }

    delete db;
}

void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    ServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char **argv)
{
    RunServer();

    return 0;
}
