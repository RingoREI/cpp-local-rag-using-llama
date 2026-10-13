#pragma once
#include "config.h"
#include <string>
#include <vector>
#include <llama.h>
struct llama_model;
struct llama_context;
enum class ModelMode{
    Embedding,
    Generation
};
class LlamaModelBase{
    public:
    LlamaModelBase(const std::string& model_path, ModelMode mode, const AppConfig& config);
    virtual ~LlamaModelBase();
    //按照要求禁用了拷贝构造和赋值运算符
    LlamaModelBase(const LlamaModelBase&)=delete;
    LlamaModelBase& operator=(const LlamaModelBase&)=delete;
    LlamaModelBase(LlamaModelBase&&) noexcept=default;
    LlamaModelBase& operator=(LlamaModelBase&&) noexcept=default;
    protected:
    std::vector<llama_token> tokenize(const std::string& text, bool add_special) const;
    struct llama_model* model_ = nullptr;
    struct llama_context* ctx_ = nullptr;
};