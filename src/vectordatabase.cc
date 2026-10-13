#include "VectorDatabase.h"
#include <fstream>
#include <iostream>
#include <print>
#include <cmath>
#include <queue>
#include <functional>
//单数据插入
void VectorDatabase::insert(const Chunk& chunk) {
    storage_.push_back(chunk);
}
//批量数据插入
void VectorDatabase::insert(const std::vector<Chunk>& chunks) {
    storage_.reserve(storage_.size() + chunks.size());
    storage_.insert(storage_.end(), chunks.begin(), chunks.end());
}
//计算余弦相似度
float VectorDatabase::cos_similarity(const std::vector<float>& a, const std::vector<float>& b) const{
    if(a.size()!=b.size()){
        return 0.0f;
    }
    float dotresult=0.0f;
    float norma=0.0f;
    float normb=0.0f;
    for(size_t i=0;i<a.size();i++){
        //先算点积，再算a和b的模长（后面直接返回余弦的公式）
        dotresult+=a[i]*b[i];
        norma+=a[i]*a[i];
        normb+=b[i]*b[i];
    }
    if(norma==0.0f||normb==0.0f){
        return 0.0f;
    }
    return dotresult/(std::sqrt(norma)*std::sqrt(normb));
}
//返回余弦相似度最高的top_k个chunk（用最小堆）
std::vector<Chunk> VectorDatabase::search(const std::vector<float>& query_emb,size_t top_k)const{
    std::vector<Chunk> results;
    if(storage_.empty()||top_k<=0){
        return results;
    }
    std::priority_queue<searchResult,std::vector<searchResult>,std::greater<searchResult>> minheap;
    //这里开始用最小堆的方法实现要求
    for(const auto& chunk:storage_){
        if(chunk.embedding.empty()) continue;
        float similarity=cos_similarity(query_emb,chunk.embedding);
        //如果堆还没装满的话就先插入chunk
        if(minheap.size()<static_cast<size_t>(top_k)){
            minheap.push({similarity,chunk});
        }
        //但如果装满了就把相似度最小的那个chunk pop出去
        else if(similarity>minheap.top().similarity){
            minheap.pop();
            minheap.push({similarity,chunk});
        }
    }
    results.resize(minheap.size());
    for(int i=static_cast<int>(minheap.size())-1;i>=0;--i){
        results[i]=minheap.top().chunk;
        minheap.pop();
    }
    return results;
}
//这个是二进制序列化写盘，文件头包含魔数0x4D524147、版本号
bool VectorDatabase::saveToDisk(const std::string& path) const{
    std::ofstream out(path,std::ios::out|std::ios::binary);
    if(!out.is_open()){
        std::print(std::cerr,"Failed to open file:{}\n", path);
        return false;
    }
    uint32_t magic=0x4D524147;
    out.write(reinterpret_cast<char*>(&magic),sizeof(magic));
    uint32_t version=1;
    out.write(reinterpret_cast<const char*>(&version),sizeof(version));
    uint64_t chunk_count=static_cast<uint64_t>(storage_.size());
    out.write(reinterpret_cast<const char*>(&chunk_count),sizeof(chunk_count));
    for(const auto& chunk:storage_){
        chunk.serialize(out);
    }
    out.close();
    return true;
}
//按要求包括了读取并校验⽂件头，失败时返回false并输出错误信息
bool VectorDatabase::loadFromDisk(const std::string& path){
    std::ifstream in(path,std::ios::in|std::ios::binary);
    if(!in.is_open()){
        std::print(std::cerr,"Failed to open file:{}\n", path);
        return false;
    }
    uint32_t magic=0;
    in.read(reinterpret_cast<char*>(&magic),sizeof(magic));
    if(magic != 0x4D524147){ 
        std::print(std::cerr,"Error: Invalid magic number:{}\n", magic); 
        return false;
    }
    uint32_t version=0;
    in.read(reinterpret_cast<char*>(&version),sizeof(version));
    if(version!=1){
        std::print(std::cerr,"Error: Unsupported database file version:{}\n", version); //
        return false;
    }
    uint64_t chunk_count=0;
    in.read(reinterpret_cast<char*>(&chunk_count),sizeof(chunk_count));
    storage_.clear();
    storage_.reserve(chunk_count);
    for(uint64_t i=0;i<chunk_count;++i){
        Chunk chunk=Chunk::deserialize(in);
        if(!in){
            std::print(std::cerr, "Error,Database corrupted{}\n", i);
            return false;
        }
        storage_.push_back(std::move(chunk));
    }
    in.close();
    return true;
}