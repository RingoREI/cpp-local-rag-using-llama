cmake_minimum_required(VERSION 3.22)
project(LocalRAGSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json_include)
add_subdirectory(llama_src)
add_executable(main_rag
    src/main.cc                 
    src/config.cc              
    src/chunk.cc               
    src/document.cc           
    src/vectordatabase.cc      
    src/llamamodel.cc          
    src/embeddingengine.cc
    src/generationengine.cc
    src/mragapp.cc
)

target_include_directories(main_rag PRIVATE 
    src
    ${llama_SOURCE_DIR}          
    ${llama_SOURCE_DIR}/include  
    ${llama_SOURCE_DIR}/ggml/include 
)

target_link_libraries(main_rag PRIVATE 
    llama
    ggml
    nlohmann_json
)