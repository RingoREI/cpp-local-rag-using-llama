#include "llamamodel.h"
#include <llama.h>
#include <stdexcept>
#include <iostream>
//构造函数，这里先加载embedding再加载generation
LlamaModelBase::LlamaModelBase(const std::string& model_path, ModelMode mode, const AppConfig& config){
    auto model_para=llama_model_default_params();
    model_para.n_gpu_layers=config.model().n_gpu_layers;
    model_=llama_model_load_from_file(model_path.c_str(), model_para);
    if(!model_){
        throw std::runtime_error("Failed to load model from path: "+model_path);
    }
    auto ctx_para=llama_context_default_params();
    if(mode==ModelMode::Embedding){
        ctx_para.n_ctx=config.context().emb_n_ctx;
        ctx_para.n_batch=config.context().emb_n_batch;
        ctx_para.embeddings=true;
    }
    else{
        ctx_para.n_ctx=config.gen_model().gen_n_ctx;
        ctx_para.n_batch=config.gen_model().gen_n_batch;
        ctx_para.embeddings=false;
    }
    ctx_=llama_init_from_model(model_,ctx_para);
    if(!ctx_){
        llama_model_free(model_);
        throw std::runtime_error("Failed to initialize llama_context for " + model_path);
    }
}
//手动析构，先释放ctx再释放model
LlamaModelBase::~LlamaModelBase(){
    if(ctx_){
        llama_free(ctx_);
    }
    if(model_){
        llama_model_free(model_);
    }
}
//这里是实现了protected里面的tokenize函数
std::vector<llama_token> LlamaModelBase::tokenize(const std::string& text, bool add_special) const{
    const struct llama_vocab* vocab=llama_model_get_vocab(model_);
    std::vector<llama_token> result(text.size()+4);
    int n_tokens=llama_tokenize(vocab,text.c_str(),text.size(),result.data(),result.size(),add_special,true);
    if(n_tokens<0){
        result.resize(-n_tokens);
        n_tokens=llama_tokenize(vocab,text.c_str(),text.size(),result.data(),result.size(),add_special,true);
    }
    result.resize(n_tokens);
    return result;
}