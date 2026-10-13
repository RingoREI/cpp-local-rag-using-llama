#pragma once
#include "llamamodel.h"
#include "chunk.h"
#include <string>
#include <vector>
class GenerationEngine:public LlamaModelBase{
    public:
    GenerationEngine(const std::string& model_path,const AppConfig& config);
    virtual ~GenerationEngine()=default;
    std::string generateStream(const std::string& query,const std::vector<Chunk>& chunks);
    private:
    std::string makePrompt(const std::string& query,const std::vector<Chunk>& chunks) const;
    const AppConfig& config_;
};