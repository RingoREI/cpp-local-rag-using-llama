#pragma once
#include "llamamodel.h"
#include <vector>
#include <string>
class EmbeddingEngine:public LlamaModelBase{
    public:
    EmbeddingEngine(const std::string& model_path,const AppConfig& config);
    std::vector<float> generateEmbedding(const std::string& text);
    private:
    std::string sanitizeUtf8(const std::string& text) const;
};