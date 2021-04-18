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


// 输入：生成项数num 输出：数据库消息msg
void ServiceImpl::setupDB(int &num, std::string &msg){
    rocksdb::DB* db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    options.create_if_missing = true;
    options.error_if_exists = true;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    // 不存在数据库则建立新的并添加键值对，存在则不做改变
    if (r_status.ok())
    {
        rocksdb::Slice key("01");
        rocksdb::Slice value("success");
        r_status = db->Put(rocksdb::WriteOptions(), key, value);
        if (r_status.ok())
        {
            msg="Setup Successfully! ";
        }
        else
        {
            msg="Setup Failed! ";
        }  
    }
    else
    {
        msg="DB Exists! If you want to update, you can use 'add' or 'delete'. ";
    }

    delete db;
}

// 输入：关键词key 输出：数据库消息msg 关键词对应的值data
void ServiceImpl::searchDB(const bool &iskey, const std::string &key, std::string &data, std::vector<std::string> &re){
    rocksdb::DB* db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);
    re.clear();
    if (r_status.ok())
    {
        if (iskey)
        {
            r_status = db->Get(rocksdb::ReadOptions(), key, &data);
            
            if(r_status.ok()){
                re.push_back("[Key:Value] = [ "+key+" : "+data+" ]");
            }else{
                re.push_back("Search Failed! No Key is equal to \'"+key+"\' ");
            }
        }
        else
        {
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
            bool isfound=false;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                if (key==it->value().ToString())
                {
                    isfound=true;
                    re.push_back("[Key:Value] = [ "+it->key().ToString()+" : "+key+" ]");
                }
            }
            if (!isfound)
            {
                re.push_back("Search Failed! No Value is equal to \'"+key+"\' ");
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

// 输入：键值对<add_key,add_value> 输出：数据库消息msg
void ServiceImpl::addDB(const std::string &add_key, const std::string &add_value, std::string &msg){
    rocksdb::DB* db;
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
            msg="Add Successfully! ";
        }
        else
        {
            msg="Add Failed! ";
        }  
    }
    else
    {
        msg="No DB! Please setup a DB first! ";
    }
    
    delete db;
}

void ServiceImpl::deleteDB(const bool &iskey, const std::string &key, std::vector<std::string> &re){
    rocksdb::DB* db;
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
                if(r_status.ok()){
                    re.push_back("[Key:Value] = [ "+key+" : "+data+" ] was deleted! ");
                    re.push_back("Delete Successfully! DeletedItems_num:1. ");
                }else{
                    re.push_back("Delete Failed! Some Error in DB.");
                }
            }
            else
            {
                re.push_back("Delete Failed! No Key is equal to \'"+key+"\' ");
            }
        }
        else
        {
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
            int foundnum=0;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                if (key==it->value().ToString())
                {
                    foundnum++;
                    re.push_back("[Key:Value] = [ "+it->key().ToString()+" : "+key+" ] was deleted! ");
                    db->Delete(rocksdb::WriteOptions(), it->key());
                }
            }
            if (!foundnum)
            {
                re.push_back("Delete Failed! No Value is equal to \'"+key+"\' ");
            }
            else
            {
                re.push_back("Delete Successfully! DeletedItems_num:"+std::to_string(foundnum)+". ");
            }
            
            assert(it->status().ok()); // Check for any errors found during the scan
            delete it;
        }
        
    }
    else
    {
        msg="No DB! Please setup a DB first! ";
    }
    delete db;
}

void ServiceImpl::showDB(std::vector<std::string> &re){
    rocksdb::DB* db;
    rocksdb::Options options;
    rocksdb::Status r_status;
    r_status = rocksdb::DB::Open(options, DBPATH, &db);

    re.clear();

    if (r_status.ok())
    { 
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) 
        {
            re.push_back("[Key:Value] = [ "+it->key().ToString() + " : " +it->value().ToString()+" ]");
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
 
void RunServer() {
    std::string server_address("0.0.0.0:50051");
    ServiceImpl service;
    // SetupRequestImpl setup_service;
  
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    // builder.RegisterService(&setup_service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    
    server->Wait();
}
 
int main(int argc, char** argv) {
    RunServer();
  
    return 0;
}
