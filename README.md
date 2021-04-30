## SWP of Searchable Encryption

*编程水平有限,代码可能比较丑。*

### 1. 简介

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

+ ### RocksDB

  参考本人写的[Ubuntu/Deepin上安装Rocksdb](https://www.jianshu.com/p/575b2e27b028)

+ ### gRPC和protobuf

  由于`gRPC`的实现依赖`protobuf`，因此这两个库可以同时安装，且建议同时安装，即使用`gRPC`源码中包含的`protobuf`源码来编译安装。

  具体步骤如下：

  #### 下载依赖和源码

  ```shell
  sudo apt-get install build-essential autoconf libtool pkg-config cmake
  git clone -b v1.36.0 https://github.com/grpc/grpc
  cd grpc
  git submodule update --init
  ```

  #### 编译安装

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

  #### 安装结果测试

  ```shell
  cd grpc/examples/cpp/helloworld
  make
  ./greeter_server
  ./greeter_client
  ```

  如果测试失败，查看报错信息，如果是缺少依赖，则多半因为子模块没能成功安装，前往`third_party`目录下手动安装缺少模块，安装方式与`protobuf`类似。

### 3. 目录结构

```
+++
```

