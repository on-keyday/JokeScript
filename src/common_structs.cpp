/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"common_structs.h"
using namespace PROJECT_NAME;

void common::free(void* p) {
    std::cout <<"free:"<<p << "\n";
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