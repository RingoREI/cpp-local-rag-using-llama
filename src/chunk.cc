#include "chunk.h"
#include <fstream>
//二进制序列化，接下来会按照文档要求标出函数涉及的不同功能（接下来的成员函数实现里也会把文档里要求的每一步标注出来）
void Chunk::serialize(std::ofstream &out) const{
    //先写入id，在写入text长度+内容，再写入metadata长度+内容，再写入embedding维度+浮点数组（size_t统一为uint64)
    out.write(reinterpret_cast<const char*>(&id), sizeof(id));
    uint64_t text_len=static_cast<uint64_t>(text.size());
    out.write(reinterpret_cast<const char*>(&text_len), sizeof(text_len));
    if(text_len>0) out.write(text.data(), text_len);
    uint64_t metadata_len=static_cast<uint64_t>(metadata.size());
    out.write(reinterpret_cast<const char*>(&metadata_len), sizeof(metadata_len));
    if(metadata_len>0) out.write(metadata.data(), metadata_len);
    uint64_t embedding_dim=static_cast<uint64_t>(embedding.size());
    out.write(reinterpret_cast<const char*>(&embedding_dim), sizeof(embedding_dim));
    if(embedding_dim>0) out.write(reinterpret_cast<const char*>(embedding.data()),embedding_dim*sizeof(float));
}
Chunk Chunk::deserialize(std::ifstream &in){
    Chunk chunk;
    //读id，再读text长度内容，再读metadata长度内容，再读embedding向量
    in.read(reinterpret_cast<char*>(&chunk.id), sizeof(chunk.id));
    uint64_t text_len=0;
    in.read(reinterpret_cast<char*>(&text_len),sizeof(text_len));
    if(text_len>0){
        chunk.text.resize(text_len);
        in.read(&chunk.text[0],text_len);
    }
    uint64_t metadata_len=0;
    in.read(reinterpret_cast<char*>(&metadata_len),sizeof(metadata_len));
    if(metadata_len>0){
        chunk.metadata.resize(metadata_len);
        in.read(&chunk.metadata[0],metadata_len);
    }
    uint64_t embedding_dim=0;
    in.read(reinterpret_cast<char*>(&embedding_dim),sizeof(embedding_dim));
    if(embedding_dim>0){
        chunk.embedding.resize(embedding_dim);
        in.read(reinterpret_cast<char*>(chunk.embedding.data()),embedding_dim*sizeof(float));
    }
    if(!in) throw std::runtime_error("Deserialization failed");
    return chunk;
}