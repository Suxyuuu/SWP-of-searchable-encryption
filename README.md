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

**其余第三方库空闲时间再写**

### 3. 目录结构

**有空再写**