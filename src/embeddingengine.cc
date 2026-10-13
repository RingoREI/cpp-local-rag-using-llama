#include "embeddingengine.h"
#include <llama.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <print> 
#include <cstring>
EmbeddingEngine::EmbeddingEngine(const std::string& model_path, const AppConfig& config)
:LlamaModelBase(model_path,ModelMode::Embedding,config){}
//通过sanitizeUtf8函数实现清理残缺的utf8序列
std::string EmbeddingEngine::sanitizeUtf8(const std::string& text) const{
    std::string cleanstr;
    cleanstr.reserve(text.size());
    for(size_t i=0;i<text.size();){
        unsigned char c=text[i];
        size_t len=1;
        if(c>=0x80){
            if((c & 0xE0)==0xC0) len=2;
            else if((c&0xF0)==0xE0) len=3;
            else if((c&0xF8)==0xF0) len=4;
            else{ i++; continue; }
        }
        if(i+len>text.size()){
            break; 
        }
        bool valid=true;
        for(size_t j=1;j<len;++j){
            if(((unsigned char)text[i+j]&0xC0)!=0x80){
                valid=false;
                break;
            }
        }
        if(valid){
            cleanstr.append(text,i,len);
            i+=len;
        } 
        else{
            i++; 
        }
    }
    return cleanstr;
}
std::vector<float> EmbeddingEngine::generateEmbedding(const std::string& text){
    //第一次运行的时候文档程序遇到了没有办法建库的问题，为了排除空文本的影响写了这个判断（虽然后面发现确实是我找到的小说没有正常复制到txt文件里）
    if(!model_||!ctx_){
        throw std::runtime_error("Model or context is null in EmbeddingEngine");
    }
    //调用前面写好的函数清理utf8序列
    std::string clean_text=sanitizeUtf8(text);
    //按文档要求调用tokenize
    std::vector<llama_token> tokens=tokenize(clean_text,true);
    int n_ctx=llama_n_ctx(ctx_);
    size_t max_tokens = static_cast<size_t>(n_ctx);
    if (tokens.size() > max_tokens) {
        tokens.resize(max_tokens);
    }
    int embd_dim = llama_model_n_embd(model_);
    if (tokens.empty()) {
        return std::vector<float>(embd_dim, 0.0f);
    }
    //清空kv cache
    llama_memory_t mem = llama_get_memory(ctx_);
    llama_memory_clear(mem, 0);
    //手动构造llama——batch
    llama_batch batch=llama_batch_init(tokens.size(),0,1);
    batch.n_tokens=tokens.size();
    for(size_t i=0;i<tokens.size();++i){
        batch.token[i]=tokens[i];
        batch.pos[i]=i;
        batch.n_seq_id[i]=1;
        batch.seq_id[i][0]=0;
        batch.logits[i]=(i==tokens.size()-1)?1:0;
    }
    //调用llama——decode推理
    int decode_result=llama_decode(ctx_,batch);
    if(decode_result!=0){
        llama_batch_free(batch);
        throw std::runtime_error("llama_decode failed in EmbeddingEngine");
    }
    // 这里按pj文档要求优先通过llama_get_embeddings_seq  获取序列向量，失败时回退到llama_get_embeddings（运行的时候发现其实只有llama_get_embeddings_seq也可以
    float* emb_result=llama_get_embeddings_seq(ctx_,0);
    if(!emb_result) emb_result=llama_get_embeddings(ctx_);
    //返回float向量
    std::vector<float> embedding(embd_dim,0.0f);
    if(emb_result){
        std::copy(emb_result,emb_result+embd_dim,embedding.begin());
    } 
    else{
        llama_batch_free(batch);
        throw std::runtime_error("Failed to retrieve embeddings from llama.cpp context.");
    }
    llama_batch_free(batch);
    return embedding;
}
