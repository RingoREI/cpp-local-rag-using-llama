#pragma once
#include <string>
#include <vector>
#include <cstdint>
struct Chunk{
    uint64_t id;
    std::string text;
    std::string metadata;
    std::vector<float> embedding;
    void serialize(std::ofstream &out) const;
    static Chunk deserialize(std::ifstream &in);
};