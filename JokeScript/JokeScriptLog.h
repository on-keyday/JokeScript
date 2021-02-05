/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptMacro.h"
#include"JokeScriptCommon.h"
namespace jokescript {
    struct JokeFile;
    typedef void (*LogCallBack)(const char*);

    enum class JokeMsgType {
        info,
        log,
        system_err,
        syntax_err,
        semantics_err
    };

    struct JokeMessage {
        char* msg;
        JokeMsgType type;
    };


    struct JokeLogger {
        LogCallBack cb;
        EasyVector<JokeMessage*> msgs;
        const JokeFile* file=nullptr;
    };


    bool CCNV AddJokeMsg(JokeLogger* log, const char* msg, JokeMsgType type);


    bool CCNV CommonAdd(EasyVector<char>& to,const char* type, const char* msg, const char* value);
    bool CCNV ResolveFileLine(EasyVector<char>& to,JokeLogger* log,unsigned long long line, unsigned long long pos);
    bool CCNV AddJokeInfo(JokeLogger* log, const char* msg, const char* value);
    bool CCNV AddJokeLog(JokeLogger* log, const char* msg, const char* value);
    bool CCNV AddJokeSysErr(JokeLogger* log, const char* msg,const char* value);
    bool CCNV AddJokeSynErr(JokeLogger* log, const char* msg,const char* value, unsigned long long line, unsigned long long pos);
    bool CCNV AddJokeSemErr(JokeLogger* log, const char* msg, const char* value,unsigned long long line, unsigned long long pos);

    bool CCNV AddJokeUnexpectedTokenErr(JokeLogger* log,const char* expected,char but,unsigned long long line, unsigned long long pos);
    bool CCNV AddJokeUnexpectedEOFErr(JokeLogger* log,unsigned long long line,unsigned long long pos);
    bool CCNV AddJokeMemoryFullErr(JokeLogger* log);
    bool CCNV AddJokeCompilerBrokenErr(JokeLogger* log, const char* msg);
}