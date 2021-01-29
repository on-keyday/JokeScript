/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptCommon.h"
using namespace jokescript;

StringFilter& jokescript::StringFilter::operator=(const char* str) {
    unsigned long long i = 0;
    while (str[i]) {
        s.add(str[i]);
        i++;
    }
    return *this;
}

jokescript::StringFilter::operator char* () {
    auto ret = s.get_raw_z();
    s.init();
    return ret;
}