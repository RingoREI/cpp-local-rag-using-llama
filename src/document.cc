#include "document.h"
#include <fstream>
#include <iostream>
#include <codecvt>
#include <print>
#include <algorithm>
#include <regex>
DocumentProcessor::DocumentProcessor(const AppConfig& config):config_(config){}
//实现保证UTF8合法性的辅助工具函数
size_t DocumentProcessor::nextUtf8Boundary(const std::string& text, size_t index) const{
   if(index>=text.size()) return text.size();
    while(index<text.size()&&(text[index]&0xC0)==0x80){
     index++;
    }
    return index;
}
//寻找最佳的切分点(考虑中英文标点)
size_t DocumentProcessor::findSplitPoint(const std::string& text, size_t start, size_t end) const {
    if (end>=text.size()) return text.size();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide_text=converter.from_bytes(text.substr(start, end-start));
    size_t current_position=wide_text.size();
    size_t search_limit=current_position;
    if(current_position>100){
        search_limit=current_position-100;
    }
    else{
        search_limit=0;
    }
    for (size_t i=current_position;i>search_limit;--i) {
        wchar_t ch=wide_text[i-1];
        if (ch == L'\n' || ch == L'.' || ch == L'!' || ch == L'?' || 
            ch == L'，' || ch == L'。' || ch == L'！' || ch == L'？') {
            std::string before=converter.to_bytes(wide_text.substr(0, i));
            return start+before.size();
        }
    }
    while(end<text.size()&&(text[end]&0xC0)==0x80) end++;
    return end;
}
//文本处理函数
std::vector<Chunk> DocumentProcessor::processNovel(const std::string& filepath) const{
    std::vector<Chunk> chunks;
    std::ifstream file(filepath,std::ios::in|std::ios::binary);
    if(!file.is_open()){
        std::print(std::cerr,"Failed to open file: {}\n",filepath);
        return chunks;
    }
    std::string full_text;
    std::string line;
    std::vector<std::pair<size_t, std::string>> chapters;
    //这里用了文档给的正则表达式
    std::regex chapter_regex(R"(^(第.*[章节回卷]|Chapter\s+\d+))");
    while(std::getline(file,line)){
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        std::smatch match;
        if(std::regex_search(line, match, chapter_regex)){
            chapters.push_back({full_text.size(), line});
        }
        full_text+=line+"\n";
    }
    file.close();
    size_t chunk_size=static_cast<size_t>(config_.doc().chunk_size);
    size_t overlap_size=static_cast<size_t>(config_.doc().overlap_size);
    //这里是通过滑动窗口法切割文本实现overlap机制（搜了一下我发现一般overlap机制就是通过这个方法实现）
    //先定义窗口的大小chunk_size和重叠的大小overlap_size
    if(chunk_size == 0) chunk_size=500;
    if(overlap_size >= chunk_size) overlap_size=chunk_size/2;
    //先初始化滑动窗口的起点、当前章节、chunkID
    size_t start=0;
    unsigned int chunkID=0;
    std::string current_chapter="";
    while(start<full_text.size()){
        for(const auto& chapter:chapters){
            if(start>=chapter.first){
                current_chapter=chapter.second;
            }
            else{
                break;
            }
        }
        //再定义滑动窗口的终点，调用寻找最佳切分点的函数从右向左寻找标点符号
        size_t end=start+chunk_size;
        if(end<full_text.size()){
            end=findSplitPoint(full_text,start,end);
        }
        Chunk chunk;
        chunk.id=chunkID++;
        chunk.text=full_text.substr(start,end-start);
        chunk.embedding={};
        if(current_chapter.empty()){
            chunk.metadata="Before Text";
        }
        else{
            chunk.metadata=current_chapter;
        }
        chunks.push_back(chunk);
        if(end==full_text.size()) break;
        //确定下一个chunk的起点，先减去重叠大小保留上一块末尾的文本，然后再调用nextUTF8Boundary函数避免切在多个汉字中间的情况
        size_t next_start;
        if(end>overlap_size){
            next_start=end-overlap_size;
        }
        else{
            next_start=start;
        }
        start=nextUtf8Boundary(full_text,next_start);
        if(start<=next_start&&start<end){
            start=end;
        }
    }
    return chunks;
}
