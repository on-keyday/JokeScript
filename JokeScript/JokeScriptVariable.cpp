/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptVariable.h"
#include"JokeScriptFunction.h"
#include"JokeScriptTree.h"
#include <iostream>
using namespace jokescript;

JokeVariableInfo* CCNV jokescript::CreateJokeVariableInfo(char* name, JokeDefinitionList* list) {
    if (!name || !list)return nullptr;
    JokeVariableInfo* info = nullptr;
    try {
        info = new JokeVariableInfo;
    }
    catch (...) {
        return nullptr;
    }
    info->name = name;
    list->vars.add(info);
    return nullptr;
}

JokeVariableInfo* CCNV jokescript::ParseVardef(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block,JokeLogger* log) {
    if (!list)return nullptr;
    if (!list->file->loglines[i])return nullptr;
    if (list->file->loglines[i][u] != '$')return nullptr;
    u++;
    bool ok = false;
    const char* nowline = list->file->loglines[i];
    EasyVector<char> id(nullptr);
    JokeVariableInfo* info = nullptr, * tmpinfo = nullptr;
    JokeTypeInfo* type = nullptr;
    JokeFunctionInfo* func = nullptr;

    id = CollectId(u, nowline, log);

    if (nowline[u]!='?') {
        return nullptr;
    }
    u++;

    info = CreateJokeVariableInfo(id.get_raw_z(), list);

    if (!info) {
        return nullptr;
    }

    if (nowline[u]=='!') {
        type = ParseTypedef(i, u, list, block, log);
        nowline = list->file->loglines[i];
    }
    else if (isalpha((unsigned char)nowline[u])||nowline[u]=='_'){
        type = GetTypebyName(i, u, nowline, list, block, log);
    }

    if (nowline[u]=='=') {
        u++;
        if (nowline[u] == '@') {
            JokeFunctionInfo* func = ParseFuncdef(i, u, list, block, log);
            if (!func) {
                return nullptr;
            }
            if (!type) {
                type = func->type;
            }
        }
        else {
            JokeTree* tree = Expr(i, u, list, block, log);
            if (!tree) {
                return nullptr;
            }
            if (!type) {
                type = tree->type;
            }
        }
    }

    if (nowline[u] != ';') {
        return nullptr;
    }

    return info;
}

JokeVariableInfo* CCNV jokescript::SearchVarOnBlock(const char* name, JokeBlockList* block) {
    if (!name || !block)return nullptr;
    JokeBlock* current = block->current;
    if (!current)return nullptr;
    while (1) {
        for (auto i = 0ull; current->vars[i]; i++) {
            if (strcmp(name, current->vars[i]->name) == 0) {
                return current->vars[i];
            }
        }
        if (!current->parent)break;
        current = current->parent;
    }
    return nullptr;
}