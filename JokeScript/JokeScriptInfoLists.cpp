/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptInfoLists.h"
#include"JokeScriptType.h"
#include"JokeScriptVariable.h"
#include"JokeScriptCommon.h"
#include"JokeScriptFunction.h"
#include"JokeScriptTree.h"
#include<string.h>
#include <map>
using namespace jokescript;


jokescript::JokeDefinitionList::~JokeDefinitionList() {
    this->types.remove_each(DeleteJokeTypeInfo);
}


JokeDefinitionList* CCNV jokescript::CreateJokeDefinitionList(JokeFile* file, JokeLogger* log) {
    if (!file)return nullptr;
    JokeDefinitionList* ret = nullptr;
    try {
        ret = new JokeDefinitionList;
    }
    catch (...) {
        AddJokeMemoryFullErr(log);
        return nullptr;
    }
    ret->file = file;
    /*unsigned long long i = 0;
    bool prevtype = true;
    unsigned int in_block = 0;
    const char* prevp = "", * tmp = nullptr;
    while (file->loglines[i]) {
        auto p = file->loglines[i];
        if (p[0] == '!' || p[0] == '$' || p[0] == '@') {
            ret->lines.add_nz(p);
            prevtype = true;
        }
        else if (tmp = strstr(p, "@")) {
            ret->lines.add_nz(tmp);
            prevtype = true;
        }
        else if (p[0] == '{') {
            ret->lines.add_nz(p);
            in_block++;
        }
        else if (p[0] == '}') {
            if (in_block == 0) {
                delete ret;
                AddJokeSynErr(log, "too many \"}\". \"{\" and \"}\" have to be pair.", nullptr, i, 1);
                return nullptr;
            }
            ret->lines.add_nz(p);
            in_block--;
        }
        else if (prevp[0] == '}' && (p[0] == '>' || p[0] == '[' || p[0] == '*' || p[0] == '=' || p[0] == ';')) {
            ret->lines.add_nz(p);
        }
        else if (!in_block) {
            prevtype = false;
        }
        i++;
        prevp = p;
    }
    if (in_block) {
        delete ret;
        AddJokeSynErr(log, "too many \"{\". \"{\" and \"}\" have to be pair.", nullptr, i - 1, 1);
        return nullptr;
    }*/
    return ret;
}

JokeBlockList* CCNV jokescript::SetBuiltInType(JokeDefinitionList* list, JokeLogger* log) {
    if (!list)return nullptr;
    JokeBlockList* ret = nullptr;
    JokeBlock* root = nullptr, * undef = nullptr;
    try {
        ret = new JokeBlockList;
    }
    catch (...) {
        return nullptr;
    }
    try {
        root = new JokeBlock;
        undef = new JokeBlock;
    }
    catch (...) {
        delete ret;
        delete root;
        AddJokeMemoryFullErr(log);
        return nullptr;
    }
    ret->list.add(root);
    ret->list.add(undef);
    ret->current = root;
    ret->undefinedlist = undef;
    std::map<const char*, unsigned long long> builtin;
    builtin.emplace("null_t", 0);
    builtin.emplace("s8",1);
    builtin.emplace("u8",1);
    builtin.emplace("s16",2);
    builtin.emplace("u16",2);
    builtin.emplace("s32",4);
    builtin.emplace("u32",4);
    builtin.emplace("s64",8);
    builtin.emplace("u64",8);
    //builtin.emplace("s128",16);//for future
    //builtin.emplace("u128",16);
    builtin.emplace("f32",4);
    builtin.emplace("f64",8);
    builtin.emplace("string", 0);
    builtin.emplace("bool", 1);

    for (auto b : builtin) {
        JokeTypeInfo* info = CreateJokeTypeInfo(StringFilter() = b.first, list);
        if (!info) {
            delete ret;
            AddJokeMemoryFullErr(log);
            return nullptr;
        }
        info->type = JokeType::builtin;
        info->size = b.second;
        info->ch_types.unuse();
        info->ch_vars.unuse();
        root->types.add(info);
    }
    return ret;
}

/*
bool CCNV jokescript::ParseDefinition(JokeDefinitionList* list) {
    if (!list)return false;
    unsigned long long i = 0, u = 0;
    while (list->lines[i]) {
        if (list->lines[i][0] == '!') {
            auto check = ParseTypedef(i, u, list, nullptr, nullptr);
        }
    }
    return false;
}*/

JokeBlock* jokescript::CreateJokeBlock() {
    JokeBlock* ret = nullptr;
    try {
        ret = new JokeBlock;
    }
    catch(...){
        return nullptr;
    }
    return ret;
}

bool jokescript::ParseProgram(JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    if (!list || !block || !log)return false;
    unsigned long long line = 0, pos = 0;
    const char* nowline = list->file->loglines[0];
    if (!nowline) {
        AddJokeInfo(log, "\"*\" is only comment file.",list->file->filename);
        return true;
    }
    bool expect = false;
    while (nowline) {
        /*if (nowline[0] == '!') {
            auto type = ParseTypedef(line, pos, list, block, log);
            if (!type) {
                return false;
            }
            expect = true;
        }
        else if (nowline[0]=='$') {
            auto var = ParseVardef(line, pos, list, block, log);
            if (!var) {
                return false;
            }
            expect = true;
        }
        else if (nowline[0]=='@') {
            auto func = ParseFuncdef(line, pos, list, block, log);
            if (!func) {
                return false;
            }
        }
        else*/ 
        if (nowline[0]=='{') {
            auto rel = CreateJokeBlock();
            if (!rel) {
                AddJokeMemoryFullErr(log);
                return false;
            }
            block->list.add(rel);
            rel->parent = block->current;
            block->current->child.add(rel);
            block->current = rel;
        }
        else if (nowline[0]=='}') {
            if (!block->current->parent) {
                AddJokeSynErr(log, "unexpected end of block.\"{\" and \"}\" have to be pair.", nullptr, line, pos);
                return false;
            }
            block->current = block->current->parent;
        }
        else {
            bool res = false;
            Expr(res,line, pos, list, block, log);
            if (!res) {
                return false;
            }
        }
        line++;
        nowline = list->file->loglines[line];
        pos = 0;
    }
    return false;
}