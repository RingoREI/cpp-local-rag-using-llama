#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <print>
AppConfig::AppConfig(const std::string& filepath){
    loadFromFile(filepath);
}
bool AppConfig::loadFromFile(const std::string& filepath){
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false; // 文件不存在不报错，保留默认值
    } 
    std::ifstream file(filepath);
    if(!file.is_open()){
       return;
    }
    try{
    nlohmann::json j;
    file>>j;
    //解析JSON配置
    if(j.contains("models")&&j["models"].is_object()){
        models_.emb_model_path=j["models"].value("embedding", models_.emb_model_path);
        models_.gen_model_path=j["models"].value("generation", models_.gen_model_path);
        models_.n_gpu_layers=j["models"].value("n_gpu_layers", models_.n_gpu_layers);
    }
    if(j.contains("document")&&j["document"].is_object()){
        const auto&d=j["document"];
        doc_.chunk_size=d.value("chunk_size", doc_.chunk_size);
        doc_.overlap_size=d.value("overlap_size", doc_.overlap_size);
    }
    if(j.contains("retrieval")&&j["retrieval"].is_object()){
        const auto&r=j["retrieval"];
        retrieval_.top_k=r.value("top_k", retrieval_.top_k);
    }
    if(j.contains("generation")&&j["generation"].is_object()){
        const auto&g=j["generation"];
        gen_model_.gen_n_ctx=g.value("n_ctx", gen_model_.gen_n_ctx);
        gen_model_.gen_n_batch=g.value("n_batch", gen_model_.gen_n_batch);
        gen_model_.gen_n_ubatch=g.value("n_ubatch", gen_model_.gen_n_ubatch);
        gen_model_.max_output_tokens=g.value("max_output_tokens", gen_model_.max_output_tokens);
        sample_.temperature=g.value("temperature", sample_.temperature);
        sample_.top_k_sampler=g.value("top_k", sample_.top_k_sampler);
        sample_.top_p=g.value("top_p", sample_.top_p);
    }
    
    if(j.contains("embedding")&&j["embedding"].is_object()){
        const auto&c=j["embedding"];
        context_.emb_n_ctx=c.value("n_ctx", context_.emb_n_ctx);
        context_.emb_n_batch=c.value("n_batch", context_.emb_n_batch);
        context_.emb_n_ubatch=c.value("n_ubatch", context_.emb_n_ubatch);
    }
}
catch(const std::exception& e){
    std::print("Error parsing config file: {}", e.what());
    return false;
}
if (doc_.chunk_size<=0) doc_.chunk_size=500;
if (retrieval_.top_k<=0) retrieval_.top_k=3;
return true;
}
bool AppConfig::reload(){
    return loadFromFile(config_path_);
}