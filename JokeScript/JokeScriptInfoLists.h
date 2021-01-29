/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptCommon.h"
#include"JokeScriptLoader.h"

namespace jokescript {
    struct JokeTypeInfo;
    struct JokeVariableInfo;
    struct JokeFunctionInfo;


    struct JokeDefinitionList {
        const JokeFile* file=nullptr;
        //EasyVector<const char*> lines;
        EasyVector<JokeTypeInfo*> types;
        EasyVector<JokeVariableInfo*> vars;
        EasyVector<JokeFunctionInfo*> funcs;
        ~JokeDefinitionList();
    };

    struct JokeBlock {
        EasyVector<JokeTypeInfo*> types;
        EasyVector<JokeVariableInfo*> vars;
        EasyVector<JokeBlock*> child;
        JokeFunctionInfo* relfunc = nullptr;
        JokeBlock* parent = nullptr;
        bool istmp = false;
    };

    struct JokeBlockList {
        EasyVector<JokeBlock*> list;
        JokeBlock* current = nullptr;
        JokeBlock* undefinedlist = nullptr;
    };

    JokeDefinitionList* CCNV CreateJokeDefinitionList(JokeFile* file, JokeLogger* log);
    JokeBlockList* CCNV SetBuiltInType(JokeDefinitionList* list, JokeLogger* log);
    //bool CCNV ParseDefinition(JokeDefinitionList* list);
}