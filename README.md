### README
>这是一个以llama.cpp的C API为推理后端构建的本地RAG问答系统。作者：沈曹依
## 1.依赖项安装命令
# apt命令
sudo apt update
sudo apt install -y build-essential cmake git
## 2.模型文件下载地址以及放置路径
# bge-small-zh-v1.5-f16.gguf
下载地址：https://huggingface.co/CompendiumLabs/bge-small-zh-v1.5-gguf/blob/main/bge-small-zh-v1.5-f16.gguf
放置路径： ./models/bge-small-zh-v1.5-f16.gguf
# qwen2.5-1.5b-instruct-q4_k_m.gguf
下载地址：https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct-GGUF/blob/main/qwen2.5-1.5b-instruct-fp16.gguf
放置路径： ./models/qwen2.5-1.5b-instruct-q4_k_m.gguf
## 3.编译命令示例
（如果没有build的话）mkdir build
cd build
cmake ..
make -j4
## 4.运行示例
# 建库
./main_rag --ingest ../novel.txt ../novel.db
>输出示例：
reading txt
chunk finished,embedding
embedding finished
selrializing
serialize finished
# 对话
./main_rag --chat ../novel.db
>输出示例：
Loading database from: ../novel.db
提问>>(这里输入问题，然后模型会输出答案，对于必要的问题会带原文回答)
### 已知问题和局限性说明
1. 目前输入文本必须是UTF8编码，不然会乱码
2. 目前此系统还不支持多个txt输入
3. 经过调试发现现在生成文本的速度很慢，而且受模型限制原因有概率回答不出比较复杂的问题
