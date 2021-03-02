/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"common_structs.h"
using namespace PROJECT_NAME;
#if OutDebugInfo
#include<vector>
#endif

OutDebugMemoryInfo(
std::map<void*, size_t> sizeinfo;
std::vector<uint64_t> graph;
void Record() {
    uint64_t memo=0;
    for (auto s : sizeinfo) {
        memo += s.second;
    }
    graph.push_back(memo);
}
void ShowGraph() {
    std::cout << "\n";
    for (auto i : graph) {
        std::cout << i << "\n";
    }
}
)

void* common::calloc(size_t elm, size_t obj) {
    auto ret=std::calloc(elm, obj);
    OutDebugMemoryInfo(
    std::cout << "calloc:" << ret << ":" << elm * obj << "\n";
    if (ret) {
        sizeinfo[ret] = elm * obj;
        Record();
    })
    return ret;
}

void* common::realloc(void* p, size_t size) {
    auto ret = std::realloc(p, size);
    OutDebugMemoryInfo(
    std::cout << "realloc:" << p << "->" << ret << ":" << size << "\n";
    if (ret) {
        sizeinfo[p] = 0;
        sizeinfo[ret] = size;
        Record();
    })
    return ret;
}

void common::free(void* p) {
    OutDebugMemoryInfo(std::cout << "free:" << p << "\n";sizeinfo[p] = 0; Record();)
    std::free(p);
}

common::StringFilter& common::StringFilter::operator=(const char* str) {
    if (!str)return *this;
    unsigned long long i = 0;
    while (str[i]) {
        s.add(str[i]);
        i++;
    }
    return *this;
}

common::StringFilter::operator char* () {
    return s.get_raw_z();
}