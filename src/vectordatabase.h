#pragma once
#include "chunk.h"
#include <vector>
#include <string>
#include <queue>
class VectorDatabase {
    public:
    VectorDatabase()=default;
    ~VectorDatabase()=default;
    void insert(const Chunk& chunk);
    void insert(const std::vector<Chunk>& chunks);
    std::vector<Chunk> search(const std::vector<float>& query_emb,size_t top_k) const;
    bool saveToDisk(const std::string& filename) const;
    bool loadFromDisk(const std::string& filename);
    size_t size() const{ return storage_.size();}
    private:
    std::vector<Chunk> storage_;
    float cos_similarity(const std::vector<float>& a, const std::vector<float>& b) const;
    struct searchResult{
        float similarity;
        Chunk chunk;
        bool operator>(const searchResult& other) const{
        return similarity > other.similarity;
        }
    };
};