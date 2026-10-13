#include "mragapp.h"
#include <iostream>
#include <print>
#include <string>
#include <cstring>
#include <llama.h>
static void llama_log_callback(enum ggml_log_level level,const char* text,void* user_data){
    (void)level;
    (void)text; 
    (void)user_data;
}
int main(int argc,char* argv[]){
    llama_log_set(llama_log_callback, nullptr);
    if(argc<2){
        std::println(std::cerr, "  {} --ingest <小说.txt> <数据库.bin>", argv[0]);
        std::println(std::cerr, "  {} --chat <数据库.bin>", argv[0]);
        return 1;
    }
    std::string config_path="config.json";
    try{
        MragAPP app(config_path);
        if(std::strcmp(argv[1], "--ingest")==0){
            if(argc!=4){
                std::println(std::cerr,"--ingest 需要两个参数：<txt文件> <数据库文件>");
                return 1;
            }
            app.buildKnowledgeBase(argv[2], argv[3]);
        }
        else if(std::strcmp(argv[1], "--chat") == 0){
            if(argc!=3){
                std::println(std::cerr,"--chat 需要一个参数：<数据库文件>");
                return 1;
            }
            app.chatloop(argv[2]);
        }
        else{
            return 1;
        }
    }
    catch(const std::exception& e){
        std::println(std::cerr,"runtime error{}",e.what());
        return 1;
    }
    return 0;
}