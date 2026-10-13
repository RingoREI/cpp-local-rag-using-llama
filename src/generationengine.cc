#include "generationengine.h"
#include <llama.h>
#include <iostream>
#include <print> 
#include <stdexcept>
#include <sstream>
#include <algorithm>
GenerationEngine::GenerationEngine(const std::string& model_path,const AppConfig& config)
:LlamaModelBase(model_path,ModelMode::Generation,config),config_(config){}
//这里是我定义的private函数，作用是为了让generatestream能直接调用生成prompt的过程不用另外再写
std::string GenerationEngine::makePrompt(const std::string& query,const std::vector<Chunk>& chunks) const{
    std::stringstream ss;
    ss<<"<|im_start|>system\n你是一个严谨的小说阅读助手。请你【必须并且只能】根据下面提供的【参考上下文】来回答用户的问题。\n如果上下文中有答案，请详细回答并引用处处，如果上下文中没有提到相关信息，请直接回答“根据当前片段无法得出答案”<|im_end|>\n";
    ss<<"<|im_start|>user\n";
    for(size_t i=0;i<chunks.size();++i){
        ss<<"[文章片段 "<<i+1<<" | 章节标签: "<< chunks[i].metadata<<"]: "<< chunks[i].text<<"\n";
    }
    ss<<"<|im_end|>\n";
    ss<<"用户问题: "<<query<<"<|im_end|>\n";
    ss<<"<|im_start|>assistant\n";
    return ss.str();
}
//按照project里要求完成RAG推理过程的函数
std::string GenerationEngine::generateStream(const std::string& query,const std::vector<Chunk>& chunks){
    std::string prompt=makePrompt(query,chunks);
    std::vector<llama_token> tokens=tokenize(prompt,true);
    int n_ctx=llama_n_ctx(ctx_);
    int max_output_tokens=config_.gen_model().max_output_tokens;
    if(tokens.size()+max_output_tokens>static_cast<size_t>(n_ctx)){
        tokens.resize(n_ctx-max_output_tokens-8);
    }
    if(tokens.empty()){
        return "Error:Empty Token";
    }
    //清空kv cache
    llama_memory_t mem = llama_get_memory(ctx_);
    llama_memory_clear(mem,0);
    const struct llama_vocab* vocab=llama_model_get_vocab(model_);
    llama_batch batch=llama_batch_init(std::max(tokens.size(),static_cast<size_t>(max_output_tokens)),0,1);
    batch.n_tokens=tokens.size();
    std::string final_response="";
    //通过一个for循环实现prefill
    for(size_t i=0;i<tokens.size();++i){
        batch.token[i]=tokens[i];
        batch.pos[i]=i;
        batch.n_seq_id[i]=1;
        batch.seq_id[i][0]=0;
        batch.logits[i]=(i==tokens.size()-1)?1:0;
    }
    //这里是一个因为调试的时候发现我的rag系统在回答问题的时候出了点问题所以加上的检查prefill结果并在有问题时抛出异常的判断
    int prefill_result=llama_decode(ctx_, batch);
    if(prefill_result!=0){
        llama_batch_free(batch);
        throw std::runtime_error("Prefill llama_decode failed with code "+std::to_string(prefill_result));
    }
    //初始化采样连
    auto sampler_params=llama_sampler_chain_default_params();
    llama_sampler* sampler=llama_sampler_chain_init(sampler_params);
    llama_sampler_chain_add(sampler,llama_sampler_init_top_k(config_.sample().top_k_sampler));
    llama_sampler_chain_add(sampler,llama_sampler_init_top_p(config_.sample().top_p, 1));
    llama_sampler_chain_add(sampler,llama_sampler_init_temp(config_.sample().temperature));
    llama_sampler_chain_add(sampler,llama_sampler_init_dist(0));
    llama_sampler_accept(sampler,tokens.back());
    int n_past=tokens.size();
    int n_vocab=llama_vocab_n_tokens(llama_model_get_vocab(model_));
    //自回归生成循环
    for(int i=0;i<config_.gen_model().max_output_tokens;++i){
        //先采样
        llama_token new_token=llama_sampler_sample(sampler, ctx_, -1);
        //再检测EOG
        if (llama_vocab_is_eog(vocab, new_token)) break;
        //将token转化为文本并输出
        std::vector<char> piece(128, 0);
        int len=llama_token_to_piece(vocab, new_token, piece.data(), piece.size(), 0, true);
        if (len>0) {
            std::string out(piece.data(), len);
            if(out.find("<|im_end|>")==std::string::npos){
                final_response += out;
                std::cout << out << std::flush;
            } 
            else{
                break;
            }
        }
        //清空batch
        llama_batch_free(batch);
        batch=llama_batch_init(1, 0, 1);
        batch.token[0]=new_token;
        batch.pos[0]=n_past++;
        batch.n_seq_id[0]=1;
        batch.seq_id[0][0]=0;
        batch.logits[0]=1;
        batch.n_tokens=1;
        //decode
        if (llama_decode(ctx_, batch) != 0) {
            std::println(std::cerr,"Decode error at step {}\n", i);
            break;
        }
        llama_sampler_accept(sampler, new_token);
    }
    //释放
    llama_sampler_free(sampler);
    llama_batch_free(batch);
    std::cout << std::endl;
    return final_response;
}