#pragma once
#include "config.h"
#include "chunk.h"
#include <string>
#include <vector>
class DocumentProcessor{
    public:
    explicit DocumentProcessor(const AppConfig& config);
    ~DocumentProcessor()=default;
    std::vector<Chunk> processNovel(const std::string& filepath) const;
    private:
    const AppConfig& config_;
    size_t nextUtf8Boundary(const std::string& text, size_t index) const;
    size_t findSplitPoint(const std::string& text, size_t start, size_t end) const;
};