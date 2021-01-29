/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptType.h"
namespace jokescript {

    enum class JokeVariable {
        unset,
        scope,
        member_pub,
        member_pri
    };

    struct JokeVariableInfo {
        char* name = nullptr;
        JokeTypeInfo* type = nullptr;
        bool intype = false;
        union {
            void* pointer = nullptr;
            JokeBlock* block;
            JokeTypeInfo* type;
        }depends;
    };

    JokeVariableInfo* CCNV CreateJokeVariableInfo(char* name,JokeDefinitionList* list);

    JokeVariableInfo* CCNV ParseVardef(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block,JokeLogger* log);

    JokeVariableInfo* CCNV SearchVarOnBlock(const char* name,JokeBlockList* block);
    JokeVariableInfo* CCNV SearchVarOnChild();
}