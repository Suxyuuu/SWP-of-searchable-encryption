[TOC]

## SWP of Searchable Encryption

*编程水平有限,代码可能比较丑。*

## 1. 简介

本项目为毕业设计的一部分，实现一种可搜索加密方案。


选择`SWP`方案为主要思想，通过结合`gRPC`，`ProtocBuf`，`RocksDB`等第三方库来实现。

+ `SWP`方案来自于**D.Song等人的[Practical Techniques for Searches on Encrypted Data](http://ieeexplore.ieee.org/abstract/document/848445/)**，为一种对称可搜索加密方案。

+ `gRPC`为Google开源的一个高性能、开源和通用的RPC框架。
+ `ProtoBuf`为Google开源的一种数据序列化的方案，具有高的转化效率，时间效率和空间效率，远超同类json，xml等。
+ `RocksDB`是一个来自FaceBook的可嵌入式的支持持久化的key-value存储系统，基于`LevelDB`改进而来。

## 2. 配置环境

操作系统版本：Deepin20.2 

内核版本：5.10.18-amd64-desktop

### 第三方库

***注意：第三方库版本尽可能一致***

`rocksdb`版本：6.16.4

`protobuf`版本：3.14.0

`gRPC`版本：1.36.0

其中`protocbuf`和`grpc`版本匹配即可

+ ### 2.1 RocksDB

  参考本人写的[Ubuntu/Deepin上安装Rocksdb](https://www.jianshu.com/p/575b2e27b028)

+ ### 2.2 gRPC和protobuf

  由于`gRPC`的实现依赖`protobuf`，因此这两个库可以同时安装，且建议同时安装，即使用`gRPC`源码中包含的`protobuf`源码来编译安装。

  具体步骤如下：

  #### 2.2.1 下载依赖和源码

  ```shell
  sudo apt-get install build-essential autoconf libtool pkg-config cmake
  git clone -b v1.36.0 https://github.com/grpc/grpc
  cd grpc
  git submodule update --init
  ```

  #### 2.2.2 编译安装

  ```shell
  // 此步骤参考grpc/BUILDING.md
  mkdir -p cmake/build
  cd cmake/build
  cmake ../.. -DBUILD_SHARED_LIBS=ON
  make
  sudo make install
  ```

  正常情况下，`protobuf`作为子模块也已经安装成功，如果没有安装，则需要去`third_party/protobuf`路径下手动安装，安装方式如下：

  ```shell
  ./autogen.sh
  ./configure
  make
  sudo make install
  ```

  #### 2.2.3 安装结果测试

  ```shell
  cd grpc/examples/cpp/helloworld
  make
  ./greeter_server
  ./greeter_client
  ```

  如果测试失败，查看报错信息，如果是缺少依赖，则多半因为子模块没能成功安装，前往`third_party`目录下手动安装缺少模块，安装方式与`protobuf`类似。

## 3. 目录结构

```
SWP-of-searchable-encryption
├── protoc/				# protocbuf相关目录
│	├── rpc.grpc.pb.cc		# 由rpc.protoc生成的grpc通讯接口源文件
│	├── rpc.grpc.pb.h		# 由rpc.protoc生成的grpc通讯接口头文件
│	├── rpc.grpc.pb.o		
│	├── rpc.pb.cc			# 由rpc.protoc生成的基本接口源文件
│	├── rpc.pb.h			# 由rpc.protoc生成的基本接口头文件
│	├── rpc.pb.o			
│	├── rpc.proto			# 使用protocbuf序列化的通讯信息内容
│	└── updateprotoc.sh		# 将rpc.proto生成接口的脚本
├── submodule/				# 工具类文件
│	├── encryption.cc		# 加密解密相关函数源文件
│	├── encryption.h		# 加密解密相关函数头文件
│	├── encryption.o		
│	├── tools.cc			# 工具函数源文件
│	├── tools.h			# 工具函数头文件
│	└── tools.o				
├── rocksdb-data/			# 服务器端数据库目录
├── client				# 客户端可执行文件
├── client.cc				# 客户端源文件
├── client.h				# 客户端头文件
├── client.o					
├── server				# 服务器端可执行文件
├── server.cc				# 服务器端源文件
├── server.h				# 服务器端头文件
├── server.o					
├── Makefile				# 编译server和client
└── README.md				# readme
```

## 4. 实现原理

+ ### 总体思路

  + 将数据划分成一个个的单词，对其进行对称加密并设置陷门后上传。查询时也会对关键词进行加密处理，防止关键词内容的泄露。整个实现过程由几个主要步骤组成（不分先后顺序）：DES加密与解密，伪随机序列的生成，带密钥的Hash函数。
    + DES加密解密：在openssl库函数的基础上实现。
    + 伪随机序列：使用C++标准库中伪随机函数来近似生成。
    + 带密钥的Hash函数：在openssl库函数的基础上实现。

+ ### 加密过程

+ ### 检索过程

+ ### 解密过程