/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptMacro.h"
#include"JokeScriptLog.h"
#ifdef _WIN32
#define fopen_s fopen_s
#else
#define fopen_s(pfp,filename,mode) ((*pfp)=fopen(filename,mode)) 
#endif
namespace jokescript{
    struct JokeFile {
        char* filename = nullptr;
        EasyVector<char*> lines;
        EasyVector<char*> loglines;
        EasyVector<unsigned long long> logtoreal_line;
        EasyVector<unsigned long long> logtoreal_pos;
        ~JokeFile();
    };
    JokeFile* CCNV LoadJoke(const char* filename, JokeLogger* log);
    bool CCNV ParseJoke(JokeFile* file, JokeLogger* log);
}