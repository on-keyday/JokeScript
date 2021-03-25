/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"structs.h"
using namespace PROJECT_NAME;
#if OutDebugInfo
#include<vector>
#endif

OutDebugMemoryInfo(
std::ofstream rec("memory.log");
std::ofstream out("memory.csv");
std::map<void*, size_t> sizeinfo;
std::vector<uint64_t> graph1;
std::vector<const char*> graph2;
std::vector<void*> graph3;
std::vector<void*> graph4;
void Record(const char* locate,void* p1,void* p2) {
    uint64_t memo=0;
    for (auto s : sizeinfo) {
        memo += s.second;
    }
    graph1.push_back(memo);
    graph2.push_back(locate);
    graph3.push_back(p1);
    graph4.push_back(p2);
}
void ShowGraph() {
    out << "count,loacte,size,pointer1,pointer2,leaks\n";
    for (auto i = 0; i < graph1.size(); i++) {
        out << i << "," << graph2[i] << "," << graph1[i] << "," << graph3[i] << "," << graph4[i] << ",";
        if (strcmp(graph2[i], "realloc")==0) {
            if (sizeinfo[graph4[i]]) {
                out << "true";
            }
            else {
                out << "false";
            }
        }
        else {
            if (sizeinfo[graph3[i]]) {
                out << "true";
            }
            else {
                out << "false";
            }
        }
        out << "\n";
    }
}
)

void* common::calloc(size_t elm, size_t obj) {
    auto ret=std::calloc(elm, obj);
    OutDebugMemoryInfo(
    rec << "calloc:" << ret << ":" << elm * obj << "\n";
    if (ret) {
        sizeinfo[ret] = elm * obj;
        Record("calloc",ret,nullptr);
    })
    return ret;
}

void* common::realloc(void* p, size_t size) {
    auto ret = std::realloc(p, size);
    OutDebugMemoryInfo(
    rec << "realloc:" << p << "->" << ret << ":" << size << "\n";
    if (ret) {
        sizeinfo[p] = 0;
        sizeinfo[ret] = size;
        Record("realloc",p,ret);
    })
    return ret;
}

void common::free(void* p) {
    OutDebugMemoryInfo(rec << "free:" << p << "\n";sizeinfo[p] = 0; Record("free",p,nullptr);)
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