/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptCommon.h"
namespace jokescript {
    struct JokeFile;

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
        EasyVector<JokeMessage*> msgs;
        const JokeFile* file=nullptr;
    };


    bool AddJokeMsg(JokeLogger* log, const char* msg, JokeMsgType type);

    bool AddJokeInfo(JokeLogger* log, const char* msg, const char* value);
    bool AddJokeLog(JokeLogger* log, const char* msg, const char* value);
    bool AddJokeSysErr(JokeLogger* log, const char* msg,const char* value);
    bool AddJokeSynErr(JokeLogger* log, const char* msg,const char* value, unsigned long long line, unsigned long long pos);
    bool AddJokeSemErr(JokeLogger* log, const char* msg, const char* value,unsigned long long line, unsigned long long pos);

    bool AddJokeUnexpectedTokenErr(JokeLogger* log,const char* expected,char but,unsigned long long line, unsigned long long pos);
    bool AddJokeUnexpectedEOFErr(JokeLogger* log,unsigned long long line,unsigned long long pos);
    bool AddJokeMemoryFullErr(JokeLogger* log);
    bool AddJokeCompilerBrokenErr(JokeLogger* log, const char* msg);
}