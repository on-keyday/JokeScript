/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#ifdef _WIN32
#ifndef CCNV
#define CCNV __stdcall
#endif
#ifndef DLL
#define DLL __declspec(dllimport)
#endif
#else
#define CCNV
#define DLL
#endif

#ifdef __cplusplus
extern "C" {
    namespace jokescript {
#endif
        struct JokeCompiler;

        DLL JokeCompiler* CCNV CreateJokeCompiler();
        DLL int CCNV DeleteJokeCompiler(JokeCompiler* joke);

        DLL int CCNV JokeCompiler_Load(JokeCompiler* joke, const char* filename);
        DLL int CCNV JokeCompiler_Option(JokeCompiler* joke, const char* optname, const char* value);
        DLL int CCNV JokeCompiler_RegisterLogCallback(JokeCompiler* joke, void (*cb)(const char*));
        DLL int CCNV JokeCompiler_Compile(JokeCompiler* joke);
        DLL int CCNV JokeCompiler_OutPut(JokeCompiler* joke,const char* filename);

#ifdef __cplusplus
    }
}
#endif