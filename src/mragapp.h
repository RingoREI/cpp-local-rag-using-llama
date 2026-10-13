#pragma once
#include "config.h"
#include "document.h"
#include "chunk.h"
#include "vectordatabase.h"
#include "embeddingengine.h"
#include "generationengine.h"
#include <llama.h>
#include <string>
#include <memory>
#include <vector>
struct BackendGuard {
    BackendGuard() { llama_backend_init(); }
    ~BackendGuard() { llama_backend_free(); }
};
class MragAPP{
    public:
    explicit MragAPP(const std::string& config_path);
    ~MragAPP()=default;
    bool reload();
    const std::string& getConfigPath() const {return config_path_;}
    void buildKnowledgeBase(const std::string& txt,const std::string& db);
    void chatloop(const std::string& db);
    private:
    BackendGuard guard_; 
    std::string config_path_; 
    AppConfig config_;
    bool loadFromFile(const std::string& filepath);
    std::unique_ptr<EmbeddingEngine> embedding_engine_;
    std::unique_ptr<GenerationEngine> generation_engine_;
};

