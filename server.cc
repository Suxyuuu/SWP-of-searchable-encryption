#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "protoc/rpc.grpc.pb.h"
#include "server.h"
#include "submodule/encryption.h"

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
void ServiceImpl::searchDB(std::vector<unsigned char> &Ki,
                           std::vector<unsigned char> &Xi,
                           std::vector<std::string> &hitted_cf,
                           std::vector<int> &kv_num,
                           std::vector<int> &key,
                           std::vector<std::vector<unsigned char>> &value,
                           std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::ColumnFamilyHandle *cf;
    // std::vector<std::string> exist_cf;
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    std::vector<std::string> exist_cf;
    rocksdb::DB::ListColumnFamilies(options, DBPATH, &exist_cf);
    if (exist_cf.size() == 0)
    {
        msg = "There's no data in DB.";
    }
    else
    {
        for (auto &&columnfamily : exist_cf)
        {
            column_families.push_back(rocksdb::ColumnFamilyDescriptor(columnfamily, rocksdb::ColumnFamilyOptions()));
        }
        r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
        if (!r_status.ok())
        {
            msg = "Open DB Failed.";
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
        }
        else
        {
            std::vector<rocksdb::Iterator *> iterators;
            db->NewIterators(rocksdb::ReadOptions(), handles, &iterators);
            for (size_t i = 0; i < iterators.size(); i++)
            {
                for (iterators[i]->SeekToFirst(); iterators[i]->Valid(); iterators[i]->Next())
                {
                    std::vector<unsigned char> Ci;
                    for (size_t j = 0; j < 24; j++)
                    {
                        Ci.push_back(iterators[i]->value().data()[j]);
                    }
                    if (search_swp(Ki, Xi, Ci))
                    {
                        hitted_cf.push_back(handles[i]->GetName());
                        break;
                    }
                }
                assert(iterators[i]->status().ok());
            }
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;

            // Now already got the hitted cf
            if (hitted_cf.size() == 0)
            {
                msg = "No matches!";
            }
            else
            {

                r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
                // std::cerr << r_status.ToString() << std::endl;
                if (!r_status.ok())
                {
                    msg = "Open DB Failed.";
                    for (auto handle : handles)
                    {
                        r_status = db->DestroyColumnFamilyHandle(handle);
                        assert(r_status.ok());
                    }
                    delete db;
                }
                else
                {
                    std::vector<rocksdb::Iterator *> iterators;
                    db->NewIterators(rocksdb::ReadOptions(), handles, &iterators);
                    int kv_count = 0;
                    for (size_t i = 0; i < iterators.size(); i++)
                    {
                        int hit_flag = false;
                        std::string cfname = handles[i]->GetName();
                        for (auto &&hitcf : hitted_cf)
                        {
                            if (hitcf == cfname)
                            {
                                hit_flag = true;
                            }
                        }
                        if (!hit_flag)
                        {
                            continue;
                        }

                        for (iterators[i]->SeekToFirst(); iterators[i]->Valid(); iterators[i]->Next())
                        {
                            std::vector<unsigned char> word;
                            for (size_t j = 0; j < 24; j++)
                            {
                                word.push_back(iterators[i]->value().data()[j]);
                            }
                            value.push_back(word);

                            key.push_back(stoi(iterators[i]->key().ToString()));

                            kv_count++;
                        }
                        assert(iterators[i]->status().ok());
                        kv_num.push_back(kv_count);
                        kv_count = 0;
                    }
                    for (auto handle : handles)
                    {
                        r_status = db->DestroyColumnFamilyHandle(handle);
                        assert(r_status.ok());
                    }
                    delete db;
                    msg = "search successfully!";
                }
            }
        }
    }
}

// 添加k-v对
// 输入：1-key 2-value
// 输出：执行结果msg
bool ServiceImpl::addDB(int add_key, std::vector<unsigned char> &add_value, std::string &add_columnfamily)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::ColumnFamilyHandle *cf;
    std::vector<std::string> exist_cf;
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    bool flag = false;
    int handle_num = 0;

    rocksdb::DB::ListColumnFamilies(options, DBPATH, &exist_cf);
    for (size_t i = 0; i < exist_cf.size(); i++)
    {
        column_families.push_back(rocksdb::ColumnFamilyDescriptor(exist_cf[i], rocksdb::ColumnFamilyOptions()));
        if (exist_cf[i] == add_columnfamily)
        {
            flag = true;
            handle_num = i;
        }
    }

    if (!flag)
    {
        // create a new columnfamily
        r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
        if (r_status.ok())
        {
            r_status = db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), add_columnfamily, &cf);
            assert(r_status.ok());

            column_families.push_back(rocksdb::ColumnFamilyDescriptor(add_columnfamily, rocksdb::ColumnFamilyOptions()));

            r_status = db->DestroyColumnFamilyHandle(cf);
            assert(r_status.ok());
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
        }
        else
        {
            delete db;
            return false;
        }
    }

    r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);

    // add
    if (r_status.ok())
    {
        char v[24];
        for (int i = 0; i < 24; i++)
        {
            v[i] = add_value[i];
        }

        rocksdb::Slice key(std::to_string(add_key));
        rocksdb::Slice value(v, 24);

        if (flag)
        {
            r_status = db->Put(rocksdb::WriteOptions(), handles[handle_num], key, value);
        }
        else
        {
            r_status = db->Put(rocksdb::WriteOptions(), handles[handles.size() - 1], key, value);
        }

        if (r_status.ok())
        {
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
            return true;
        }
        else
        {
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
            return false;
        }
    }
    else
    {
        for (auto handle : handles)
        {
            r_status = db->DestroyColumnFamilyHandle(handle);
            assert(r_status.ok());
        }
        delete db;
        return false;
    }
}

// 删除k-v对
// 输入：1-是否是key 2-是key则为key的值 不是key则为value
// 输出：删除结果
void ServiceImpl::deleteDB(const std::string &del_cf, std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::ColumnFamilyHandle *cf;
    std::vector<std::string> exist_cf;
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    bool flag = false;
    int handle_num = 0;

    rocksdb::DB::ListColumnFamilies(options, DBPATH, &exist_cf);

    for (size_t i = 0; i < exist_cf.size(); i++)
    {
        column_families.push_back(rocksdb::ColumnFamilyDescriptor(exist_cf[i], rocksdb::ColumnFamilyOptions()));
        if (exist_cf[i] == del_cf)
        {
            flag = true;
            handle_num = i;
        }
    }
    if (!flag)
    {
        msg = "Delete Failed! No columnfamily equals " + del_cf + ".";
    }
    else
    {
        r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
        assert(r_status.ok());
        r_status = db->DropColumnFamily(handles[handle_num]);
        assert(r_status.ok());
        msg = "Delete Successfully!";
        for (auto handle : handles)
        {
            r_status = db->DestroyColumnFamilyHandle(handle);
            assert(r_status.ok());
        }
        delete db;
    }
}

// 展示数据库所有数据 以k-v对的形式展示
bool ServiceImpl::showDB(std::vector<std::string> &exist_cf,
                         std::vector<int> &kv_num,
                         std::vector<int> &key,
                         std::vector<std::vector<unsigned char>> &value)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::ColumnFamilyHandle *cf;
    // std::vector<std::string> exist_cf;
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    rocksdb::DB::ListColumnFamilies(options, DBPATH, &exist_cf);

    if (exist_cf.size() == 0)
    {
        return true; // 只是为空 但操作是成功的
    }
    else
    {
        for (auto &&columnfamily : exist_cf)
        {
            // std::cout << columnfamily << std::endl;
            column_families.push_back(rocksdb::ColumnFamilyDescriptor(columnfamily, rocksdb::ColumnFamilyOptions()));
        }
        r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
        if (!r_status.ok())
        {
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
            return false;
        }
        else
        {
            std::vector<rocksdb::Iterator *> iterators;
            db->NewIterators(rocksdb::ReadOptions(), handles, &iterators);
            int kv_count = 0;
            for (size_t i = 0; i < iterators.size(); i++)
            {
                for (iterators[i]->SeekToFirst(); iterators[i]->Valid(); iterators[i]->Next())
                {
                    std::vector<unsigned char> word;
                    for (size_t j = 0; j < 24; j++)
                    {
                        word.push_back(iterators[i]->value().data()[j]);
                    }
                    value.push_back(word);

                    key.push_back(stoi(iterators[i]->key().ToString()));

                    kv_count++;
                }
                assert(iterators[i]->status().ok());
                kv_num.push_back(kv_count);
                kv_count = 0;
            }
            for (auto handle : handles)
            {
                r_status = db->DestroyColumnFamilyHandle(handle);
                assert(r_status.ok());
            }
            delete db;
            return true;
        }
    }
}

// 根据client产生的随机数据生成数据库(需要数据库已经存在且内无数据)
// 输入：1-多个key值 2-多个value值
// 输出：执行结果
void ServiceImpl::rangenDB(std::vector<std::string> &cf_name,
                           std::vector<int> &every_cf_kv_num,
                           std::vector<int> &keyv,
                           std::vector<std::vector<unsigned char>> &valuev,
                           std::string &msg)
{
    rocksdb::DB *db;
    rocksdb::Options options;
    rocksdb::Status r_status;

    rocksdb::ColumnFamilyHandle *cf;
    // std::vector<std::string> exist_cf;
    std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    r_status = rocksdb::DB::Open(options, DBPATH, &db);

    for (auto &&add_cf : cf_name)
    {
        r_status = db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), add_cf, &cf);
        assert(r_status.ok());

        column_families.push_back(rocksdb::ColumnFamilyDescriptor(add_cf, rocksdb::ColumnFamilyOptions()));

        r_status = db->DestroyColumnFamilyHandle(cf);
        assert(r_status.ok());
    }
    column_families.push_back(rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions()));
    delete db;

    r_status = rocksdb::DB::Open(rocksdb::DBOptions(), DBPATH, column_families, &handles, &db);
    // std::cerr << r_status.ToString() << std::endl;
    if (!r_status.ok())
    {
        msg = "Open DB Failed!";
    }
    else
    {
        rocksdb::WriteBatch batch;

        int count = 0;
        for (size_t i = 0; i < cf_name.size(); i++)
        {
            for (size_t j = 0; j < every_cf_kv_num[i]; j++)
            {
                char v[24];
                for (int m = 0; m < 24; m++)
                {
                    v[m] = valuev[j + count][m];
                }

                rocksdb::Slice key(std::to_string(keyv[j + count]));
                rocksdb::Slice value(v, 24);
                batch.Put(handles[i], key, value);
                key.clear();
                value.clear();
            }
            count += every_cf_kv_num[i];
        }

        r_status = db->Write(rocksdb::WriteOptions(), &batch);
        if (!r_status.ok())
        {
            msg = "GenDB Add Failed!";
        }
        else
        {
            msg = "GenDB Successfully!";
        }
        for (auto handle : handles)
        {
            r_status = db->DestroyColumnFamilyHandle(handle);
            assert(r_status.ok());
        }
        delete db;
    }
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
