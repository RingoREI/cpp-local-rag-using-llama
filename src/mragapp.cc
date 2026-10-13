#include "mragapp.h"
#include <iostream>
#include <print>
#include <stdexcept>
#include "vectordatabase.h"
bool MragAPP::reload(){
    if (!config_.reload()) {
        std::println(std::cerr, "Config reload failed");
        return false;
    }
    std::string new_emb = config_.model().emb_model_path;
    if (!new_emb.empty()) {
        embedding_engine_.reset(); 
        embedding_engine_ = std::make_unique<EmbeddingEngine>(new_emb, config_);
    } else {
        embedding_engine_.reset();
    }
    std::string new_gen = config_.model().gen_model_path;
    if (!new_gen.empty()) {
        generation_engine_.reset();
        generation_engine_ = std::make_unique<GenerationEngine>(new_gen, config_);
    } else {
        generation_engine_.reset();
    }
    std::println(std::cout, "Config reloaded successfully!");
    return true;
}
MragAPP::MragAPP(const std::string& config_path) 
:config_path_(config_path),config_(config_path){
    std::string emb_path=config_.model().emb_model_path;
    std::string gen_path=config_.model().gen_model_path;
    if(!emb_path.empty()){
        embedding_engine_=std::make_unique<EmbeddingEngine>(emb_path, config_);
    }
    if(!gen_path.empty()){
        generation_engine_=std::make_unique<GenerationEngine>(gen_path, config_);
    }
}
void MragAPP::buildKnowledgeBase(const std::string& txt,const std::string& db) {
    if(!embedding_engine_){
        throw std::runtime_error("Embedding Engine Initialize Error");
    }
    std::println(std::cout,"reading txt");
    DocumentProcessor processor(config_);
    std::vector<Chunk> chunks=processor.processNovel(txt);
    if(chunks.empty()){
        std::println(std::cerr,"empty chunk");
        return;
    }
    std::println(std::cout,"chunk finished,embedding");
    for(size_t i=0;i<chunks.size();++i){
        chunks[i].embedding=embedding_engine_->generateEmbedding(chunks[i].text);
        if((i+1)%10==0||i==chunks.size()-1){
            std::println(std::cout,"embedding finished");
        }
    }
    VectorDatabase vectordb;
    vectordb.insert(chunks);
    std::println(std::cout,"serializing");
    if(vectordb.saveToDisk(db)){
        std::println("serialize finished");
    }
    else{
        std::println(std::cerr,"serialize failed");
    }
}
void MragAPP::chatloop(const std::string& db){
    if(!embedding_engine_){
        throw std::runtime_error("embedding engine failed");
    }
    if(!generation_engine_){
        throw std::runtime_error("generation engine failed");
    }
    std::println(std::cout, "Loading database from:{}",db);
    VectorDatabase vectordb;
    if(!vectordb.loadFromDisk(db)){
        std::println(std::cerr,"load failed");
        return;
    }
    int top_k=config_.retrieval().top_k;
    std::string question;
    while(true){
        std::print(std::cout, "\n提问 >>> ");
        if(!std::getline(std::cin, question)){
            break; 
        }
        if (question == "exit" || question == "quit") {
            break;
        }
        if(question=="/reload"){
            reload();
            continue;
        }
        if (question.empty()) {
            continue;
        }
        std::vector<float> query_emb=embedding_engine_->generateEmbedding(question);
        std::vector<Chunk> results=vectordb.search(query_emb,static_cast<size_t>(top_k));
        std::println(std::cout,"Retrieved {} relevant chunks.",results.size());
        std::string answer = generation_engine_->generateStream(question, results);
        std::println(std::cout);
    }
}