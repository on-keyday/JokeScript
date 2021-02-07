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
    return info;
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
    JokeTree* tree = nullptr;

    id = CollectId(u, nowline, log);

    if (nowline[u]!='?') {
        return nullptr;
    }

    info = CreateJokeVariableInfo(id.get_raw_z(), list);

    if (!info) {
        return nullptr;
    }

    info->depends.block = block->current;
    block->current->vars.add(info);

    if (nowline[u + 1] != '=' ||(nowline[u+1]=='='&&(nowline[u + 2] == '>' || nowline[u + 2] == '<'))) {
        type = ParseTypedef_detail(i, u, list, block, log);
        nowline = list->file->loglines[i];
    }
    else {
        u++;
    }
    

    if (nowline[u]=='=') {
        u++;
        if (nowline[u] == '@') {
            func = ParseFuncdef(i, u, list, block, log);
            if (!func) {
                return nullptr;
            }
            if (!type) {
                type = func->type;
            }
        }
        else {
            bool res = false;
            tree = Assigns(i, u, list, block, log,res);
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