/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptLoader.h"
#include "JokeScriptType.h"
#include"JokeScriptVariable.h"
#include"JokeScriptKeyword.h"
//#include<iostream>
#include<ctype.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
using namespace jokescript;




jokescript::JokeTypeInfo::~JokeTypeInfo() {
    free(name);
    if (type == JokeType::function_options) {
        free(optval);
    }
}

void jokescript::DeleteJokeTypeInfo(JokeTypeInfo* info) {
    delete info;
}


JokeTypeInfo* CCNV jokescript::CreateJokeTypeInfo(char* name,JokeDefinitionList* list) {
    if (!list||!name)return nullptr;
    JokeTypeInfo* info = nullptr;
    try {
        info = new JokeTypeInfo;
    }
    catch (...) {
        return nullptr;
    }
    list->types.add(info);
    info->name = name;
    return info;
}

JokeTypeInfo* CCNV jokescript::ParseTypedef(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    if (!list->file->loglines[i])return nullptr;
    if (list->file->loglines[i][u] != '!')return nullptr;
    u++;
    return ParseTypedef_detail(i, u, list, block, log);
}

JokeTypeInfo* CCNV jokescript::ParseTypedef_detail(unsigned long long& i,unsigned long long& u,JokeDefinitionList* list,JokeBlockList* block,JokeLogger* log) {
    const char* nowline = list->file->loglines[i];
    JokeTypeInfo* info = nullptr,*tmpinfo=nullptr;


    info = GetIdSemantics(i, u, nowline, list, block, log);
    if (!info)return nullptr;

    tmpinfo=SetRootInfo(info, i, u, nowline, list, block, log);
    if (!tmpinfo)return nullptr;
    info = tmpinfo;

    tmpinfo=SetFuncOrTemplateInfo(info, i, u, nowline, list, block, log);
    if (!tmpinfo)return nullptr;
    info = tmpinfo;

    
    tmpinfo = SetArrayOrPointerInfo(info, i, u, nowline, list, block, log);
    if (!tmpinfo)return nullptr;
    info = tmpinfo;
    
    if (info->type==JokeType::unset||info->type==JokeType::type_alias) {
        if (info->name[0]=='\0') {
            tmpinfo = info->root;
            list->types.remove_if(info);
            block->current->types.remove_if(info);
            delete info;
            info = tmpinfo;
        }
        else {
            info->type = JokeType::type_alias;
            info->ch_types.unuse();
            info->ch_vars.unuse();
        }
    }

    if (info->type==JokeType::type_alias) {
        info = info->root;
        if (!info) {
            AddJokeCompilerBrokenErr(log, "unexpected nullptr on \'info->root\' while \'info->type==JokeType::type_alias\'.");
            return nullptr;
        }
    }

    return info;
}

JokeTypeInfo* CCNV jokescript::GetIdSemantics(unsigned long long& i,unsigned long long& u,const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* info=nullptr, * tmpinfo=nullptr;
    EasyVector<char> id(nullptr);
    id = CollectId(u, nowline, log);
    if (nowline[u] != '?') {
        AddJokeUnexpectedTokenErr(log,"?",nowline[u],i,u);
        return nullptr;
    }
    auto d = id[0];
    u++;
    if (d) {
        tmpinfo = SearchTypeOnUndefined(id.get_const(), block);
        if (tmpinfo) {
            if (tmpinfo->type == JokeType::not_defined) {
                block->undefinedlist->types.remove_if(tmpinfo);
                info = tmpinfo;
                tmpinfo = nullptr;
                info->type = JokeType::unset;
            }
        }
    }
    if (!info) {
        if (IsJokeReserved(id.get_const())) {
            return nullptr;
        }
        tmpinfo = SearchTypeOnBlock(id.get_const(), block);
        if (tmpinfo) {
            AddJokeSemErr(log, "type \"*\" is already defined", id.get_const(), i, u);
            return nullptr;
        }
        info = CreateJokeTypeInfo(id.get_raw_z(), list);
        if (!info) {
            AddJokeMemoryFullErr(log);
            return nullptr;
        }
    }
    if (d) {
        info->depends.block = block->current;
        block->current->types.add(info);
    }
    return info;
}

JokeTypeInfo* CCNV jokescript::SetRootInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* tmpinfo=nullptr;
    if (nowline[u] == '<') {//on function returning
        /*u++;
        JokeTypeInfo* root = nullptr;
        info->type = JokeType::function;
        root = ParseTypedef(i, u, list, block, log);
        if (!root) {
            return nullptr;
        }
        if (root->type != JokeType::builtin && block->current->types.get_end() == root) {
            block->current->types.remove_if(root);
            root->depends.block = nullptr;
        }
        nowline = list->file->loglines[i];
        if (!nowline) {
            return nullptr;
        }
        if (nowline[u] != '>') {
            return nullptr;
        }
        info->root = root;
        u++;*/
        tmpinfo = SetReturnInfo(info, i, u, nowline, list, block, log);
    }
    else if (nowline[u] == '=') {//on structure
        u++;
        tmpinfo=SetStructureInfo(info, i, u, nowline, list, block, log);
    }
    else if (isalpha((unsigned char)nowline[u]) || nowline[u] == '_') {//on identifier
        tmpinfo = GetTypebyName(i,u,nowline,list,block,log);
    }
    if (!tmpinfo)return nullptr;
    info = tmpinfo;
    //if 'info' have no root (maybe syntax '!id?' or '!id?()' or structure), 'info->root' will be 'null_t' 
    if (!info->root) {
        info->root = list->types[JTYPE_null_t];
    }
    return info;
}

JokeTypeInfo* CCNV jokescript::SetFuncOrTemplateInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* tmpinfo=nullptr;
    if (nowline[u] == '<') {//template arguments
        tmpinfo = SetTemplateInfo(info, i, u, nowline, list, block, log);
        if (!tmpinfo)return nullptr;
        nowline = list->file->lines[i];
        if (!nowline) {
            AddJokeUnexpectedEOFErr(log, i, u);
            return nullptr;
        }
    }
    else if (nowline[u] == '(') {
        if (info->root) {
            if (info->root->type == JokeType::structure_template) {
                return nullptr;
            }
        }
        if (info->type == JokeType::structure || info->type == JokeType::structure_template)return false;
        info->type = JokeType::function;
        while (nowline[u]) {
            u++;
            if (nowline[u] == ')') {
                break;
            }
            JokeVariableInfo* var = ParseVardef(i, u, list, block,log);
            if (!var)return nullptr;
            info->ch_vars.add(var);
            nowline = list->file->loglines[i];
            if (!nowline) {
                AddJokeUnexpectedEOFErr(log, i, u);
                return nullptr;
            }
        }
        if (nowline[u] != ')') {
            return nullptr;
        }
        info->ch_vars.pack();
        info->ch_types.pack();
        u++;
    }
    /*if (nowline[u] == '<') {//template arguments
        if (info->type != JokeType::unset)return nullptr;
        JokeType types = info->root->type;
        unsigned long long size = 0;
        while (types == JokeType::type_alias) {
            tmpinfo = info->root;
            if (!tmpinfo)break;
            types = tmpinfo->type;
        }
        if (types != JokeType::structure_template) {
            AddJokeSemErr(log, "\"*\" is not template", info->name, i, u);
            return nullptr;
        }
        info->type = JokeType::template_instance;
        auto counter = 0ull;
        while (nowline[u]) {
            u++;
            if (nowline[u]=='!') {
                tmpinfo = ParseTypedef(i, u, list, block, log);
            }
            else {
                tmpinfo = GetTypebyName(i,u, nowline, list, block, log);
            }
            if (!tmpinfo) {
                return nullptr;
            }
            info->ch_types.add(tmpinfo);
            counter++;
            nowline = list->file->loglines[i];
            if (!nowline)return nullptr;
            if (nowline[u] == '>')break;
            if (nowline[u] != ',') {
                AddJokeUnexpectedTokenErr(log, ",", nowline[u], i, u);
                return nullptr;
            }
        }
        if (info->root->size != counter) {
            if (!info->root->size) {
                AddJokeSemErr(log, "incomplete type is unsuable", nullptr, i, u);
            }
            else {
                AddJokeSemErr(log, "template parameter count is unmatched.", nullptr, i, u);
            }
            return nullptr;
        }
        info->ch_types.pack();
        info->ch_vars.unuse();
        info->size = counter;
        u++;
    }*/
    return info;
}

JokeTypeInfo* CCNV jokescript::SetFuncOpts(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* tmpinfo=nullptr;
    EasyVector<char> id(nullptr);
    bool capt = false, ccnv = false, co = false,va_args=false;

    if (isalpha((unsigned char)nowline[u])||nowline[u]=='_') {
        if (info->type != JokeType::function)return nullptr;
        while (nowline[u]) {
            id = CollectId(u, nowline, log);
            if (nowline[u] != '(')return nullptr;
            if (!capt && !ccnv && strcmp(id.get_const(), "capt") == 0) {
                if (block->current->istmp)return nullptr;
                tmpinfo = CreateJokeTypeInfo(StringFilter() = "capt", list);
                if (!tmpinfo) {
                    return nullptr;
                }
                tmpinfo->type = JokeType::function_options;
                info->ch_types.add(tmpinfo);
                while (nowline[u]) {
                    u++;
                    if (nowline[u] == ')') {
                        break;
                    }
                    id = CollectId(u, nowline, log);
                    JokeVariableInfo* var = SearchVarOnBlock(id.get_const(), block);
                    if (!var) {
                        return nullptr;
                    }
                    if (var->intype) {
                        return nullptr;
                    }
                    else if (var->depends.block->istmp) {
                        return nullptr;
                    }
                    tmpinfo->ch_vars.add(var);
                    if (nowline[u] != ','&&nowline[u]!=')') {
                        AddJokeUnexpectedTokenErr(log, ",)", nowline[u], i, u);
                        return nullptr;
                    }
                }
                tmpinfo->ch_types.pack_f();
                tmpinfo->ch_vars.pack_f();
            }
            else if (!va_args && strcmp(id.get_const(),"va_args") == 0) {
                tmpinfo = CreateJokeTypeInfo(StringFilter() = "va_args", list);
                if (!tmpinfo) {
                    return nullptr;
                }
                tmpinfo->type = JokeType::function_options;
                info->ch_types.add(tmpinfo);
                while (nowline[u]) {
                    u++;
                    if (nowline[u] == ')') {
                        break;
                    }
                    JokeTypeInfo* type = GetTypebyName(i,u,nowline,list,block,log);
                    if (!type) {
                        return nullptr;
                    }
                    tmpinfo->ch_types.add(type);
                    if (nowline[u] != ',' && nowline[u] != ')') {
                        AddJokeUnexpectedTokenErr(log, ",)", nowline[u], i, u);
                        return nullptr;
                    }
                }
                tmpinfo->ch_types.pack_f();
                tmpinfo->ch_vars.pack_f();
            }
            else if ((!ccnv && !capt && strcmp(id.get_const(), "ccnv") == 0) || (!co && strcmp(id.get_const(), "co") == 0)) {
                tmpinfo = CreateJokeTypeInfo(id.get_raw_z(), list);
                if (!tmpinfo) {
                    return nullptr;
                }
                tmpinfo->type = JokeType::function_options;
                info->ch_types.add(tmpinfo);
                id = CollectId(u, nowline, log);
                tmpinfo->optval = id.get_raw_z();
                if (!tmpinfo->optval) {
                    return nullptr;
                }
                if (nowline[u] != ')') {
                    return nullptr;
                }
            }
            else {
                return nullptr;
            }
            u++;
            if (!(isalpha((unsigned char)nowline[u]) || nowline[u] == '_')) {
                break;
            }
        }
    }
    info->ch_types.pack_f();
    return info;
}

JokeTypeInfo* CCNV jokescript::SetArrayOrPointerInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* tmpinfo = nullptr;
    bool ok = false;
    JokeType willtype = JokeType::unset;
    //if 'info->root' will be pointer or array, 'info->type' will be 'type_alias' 
    if (!(nowline[u] == '>' || nowline[u] == ';' || nowline[u] == ',' || nowline[u] == '=') &&
        (info->type == JokeType::structure_template || info->type == JokeType::function || info->type == JokeType::structure)) {
        if (info->type == JokeType::structure_template) {
            AddJokeSemErr(log, "pointer to template is unspecifiable.", nullptr, i, u);
            return nullptr;
        }
        char* name = info->name;
        info->name = StringFilter() = "";
        if (!info->name) {
            free(name);
            return nullptr;
        }
        tmpinfo = CreateJokeTypeInfo(name, list);
        if (!tmpinfo) {
            AddJokeMemoryFullErr(log);
            return nullptr;
        }
        block->current->types.add(tmpinfo);
        block->current->types.remove_if(info);
        tmpinfo->type = JokeType::type_alias;
        tmpinfo->ch_types.unuse();
        tmpinfo->ch_vars.unuse();
        tmpinfo->root = info;
        info->depends.block = nullptr;
        info = tmpinfo;
        info->depends.block = block->current;
        tmpinfo = nullptr;
    }



    while (nowline[u]) {
        if (nowline[u] == '>' || nowline[u] == ';' || nowline[u] == ',' || nowline[u] == '=') {
            ok = true;
            break;
        }
        if (nowline[u] == '*') {
            if (info->root->type == JokeType::reference) {
                AddJokeSemErr(log, "pointer to reference is unusable.", nullptr, i, u);
                return nullptr;
            }
            tmpinfo = CreateJokeTypeInfo(StringFilter() = "*", list);
            if (!tmpinfo) {
                AddJokeMemoryFullErr(log);
                return nullptr;
            }
            tmpinfo->type = JokeType::pointer;
        }
        else if (nowline[u]=='&') {
            tmpinfo = CreateJokeTypeInfo(StringFilter() = "&", list);
            if (!tmpinfo) {
                AddJokeMemoryFullErr(log);
                return nullptr;
            }
            tmpinfo->type = JokeType::reference;
        }
        else if (nowline[u] == '[') {
            if (info->root->type == JokeType::reference) {
                AddJokeSemErr(log, "array of reference is unusable.", nullptr, i, u);
                return nullptr;
            }
            willtype = JokeType::array;
            unsigned long long size = 0;
            u++;
            if (!isdigit((unsigned char)nowline[u])) {
                if (nowline[u] == ']') {
                    willtype = JokeType::vector;
                }
                else {
                    AddJokeUnexpectedTokenErr(log, "]", nowline[u], i, u);
                    return nullptr;
                }
            }
            else {
                int base = 10;
                if (nowline[u] == '0') {
                    if (nowline[u + 1] == 'x' || nowline[u + 1] == 'X') {
                        base = 16;
                        u += 2;
                    }
                    else if (nowline[u + 1] == 'b' || nowline[u + 1] == 'B') {
                        base = 2;
                        u += 2;
                    }
                    else if (nowline[u + 1] >= '0' && nowline[u + 1] <= '7') {
                        base = 8;
                        u++;
                    }
                }
                if (nowline[u] == ']') {
                    AddJokeSynErr(log, "invalid number", nullptr, i, u);
                    return nullptr;
                }
                char* check = nullptr;
                size = strtoull(&nowline[u], &check, base);
                if (*check != ']') {
                    AddJokeUnexpectedTokenErr(log, "]", *check, i, u);
                    return nullptr;
                }
                if (size == 0) {
                    AddJokeSemErr(log, "0 length array is invalid", nullptr, i, u);
                    return nullptr;
                }
                while (nowline[u] != ']') {
                    u++;
                }
            }
            tmpinfo = CreateJokeTypeInfo(StringFilter() = "[]", list);
            if (!tmpinfo) {
                AddJokeMemoryFullErr(log);
                return nullptr;
            }
            tmpinfo->type = willtype;
            tmpinfo->size = size;
        }
        else {
            AddJokeUnexpectedTokenErr(log, "*&[>;,=", nowline[u], i, u);
            return nullptr;
        }
        tmpinfo->ch_types.unuse();
        tmpinfo->ch_vars.unuse();
        tmpinfo->root = info->root;
        info->root = tmpinfo;
        tmpinfo = nullptr;
        u++;
    }
    if (!ok)return nullptr;
    return info;
}

JokeTypeInfo* CCNV jokescript::SetReturnInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    u++;
    JokeTypeInfo* tmproot = nullptr,*root=nullptr;
    info->type = JokeType::function;
    while (1) {
        tmproot = ParseTypedef(i, u, list, block, log);
        if (!tmproot) {
            return nullptr;
        }
        if (tmproot->type != JokeType::builtin && block->current->types.get_end() == tmproot) {
            block->current->types.remove_if(root);
            tmproot->depends.block = nullptr;
        }
        nowline = list->file->loglines[i];
        if (!nowline) {
            return nullptr;
        }
        if (nowline[u]==',') {
            if (!root) {
                root = CreateJokeTypeInfo(StringFilter() = "", list);
                if (!root) {
                    AddJokeMemoryFullErr(log);
                    return nullptr;
                }
                root->type = JokeType::some_returns;
            }
            root->ch_types.add(tmproot);
            u++;
            continue;
        }
        if (nowline[u] != '>') {
            AddJokeUnexpectedTokenErr(log, ">", nowline[u], i, u);
            return nullptr;
        }
        if (!root) {
            root = tmproot;
        }
        else {
            root->ch_types.add(tmproot);
        }
        break;
    }
    info->root = root;
    u++;
    return info;
}

JokeTypeInfo* CCNV jokescript::SetStructureInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeType willtype=JokeType::structure;
    EasyVector<char> id(nullptr);
    JokeTypeInfo* tmpinfo=nullptr;

    if (nowline[u] == '<') {//template parameter
        u++;
        while (nowline[u]) {
            id = CollectId(u, nowline, log);
            if (nowline[u] == ',' || nowline[u] == '>') {
                if (!id.get_const()[0]) {
                    AddJokeSemErr(log, "no word in template parameter. empty parameter is not allowed.", nullptr, i, u);
                    return nullptr;
                }
                tmpinfo = CreateJokeTypeInfo(id.get_raw_z(), list);
                if (!tmpinfo) {
                    AddJokeMemoryFullErr(log);
                    return nullptr;
                }
                tmpinfo->type = JokeType::template_param;
                tmpinfo->ch_types.unuse();
                tmpinfo->ch_vars.unuse();
                info->ch_types.add(tmpinfo);
                tmpinfo = nullptr;
                if (nowline[u] == '>')break;
            }
            else {
                AddJokeUnexpectedTokenErr(log, ",>", nowline[u], i, u);
                return nullptr;
            }
            u++;
        }
        willtype = JokeType::structure_template;
    }

    if (nowline[u] != '>') {
        return nullptr;
    }
    i++;
    u = 0;
    nowline = list->file->loglines[i];
    if (!nowline) {
        AddJokeUnexpectedEOFErr(log, i, u);
        return nullptr;
    }
    info->type = willtype;
    if (nowline[0] != '{') {
        AddJokeUnexpectedTokenErr(log, "{", nowline[0], i, u);
        return nullptr;
    }

    JokeBlock* tmpblock = nullptr, * holdblock = nullptr;
    tmpblock = CreateJokeBlock();
    if (!tmpblock) {
        AddJokeMemoryFullErr(log);
        return nullptr;
    }
    holdblock = block->current;
    tmpblock->parent = holdblock;
    tmpblock->istmp = true;
    block->current = tmpblock;
    auto tmpc = 0ull;

    while (info->ch_types[tmpc]) {//for template parametor
        tmpblock->types.add(info->ch_types[tmpc]);
        tmpc++;
    }

    i++;
    nowline = list->file->loglines[i];
    while (nowline) {
        u = 0;
        if (nowline[0] == '!') {
            auto ch_type = ParseTypedef(i, u, list, block, log);
            if (!ch_type) {
                delete tmpblock;
                block->current = holdblock;
                return nullptr;
            }
            ch_type->intype = true;
            ch_type->depends.type = info;
            info->ch_types.add(ch_type);
        }
        else if (nowline[0] == '$') {
            auto ch_var = ParseVardef(i, u, list, block,log);
            if (!ch_var) {
                delete tmpblock;
                block->current = holdblock;
                return nullptr;
            }
            ch_var->intype = true;
            ch_var->depends.type = info;
            info->ch_vars.add(ch_var);
        }
        else if (nowline[0] == '}') {
            break;
        }
        else {
            delete tmpblock;
            block->current = holdblock;
            return nullptr;
        }
        nowline=list->file->loglines[i];
        if (nowline[u] != ';') {
            AddJokeUnexpectedTokenErr(log, ";", nowline[u],i,u);
            return nullptr;
        }
        i++;
        nowline = list->file->loglines[i];
    }
    block->current = holdblock;
    delete tmpblock;
    if (!nowline)return nullptr;
    if (nowline[0] != '}') {
        return nullptr;
    }
    info->size = tmpc;
    info->ch_types.pack_f();
    info->ch_vars.pack_f();
    i++;
    u = 0;
    return info;
}

JokeTypeInfo* CCNV jokescript::SetTemplateInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    JokeTypeInfo* tmpinfo = nullptr;
    if (info->type != JokeType::unset) {
        return nullptr;
    }
    JokeType types = info->root->type;
    unsigned long long size = 0;
    while (types == JokeType::type_alias) {
        tmpinfo = info->root;
        if (!tmpinfo)break;
        types = tmpinfo->type;
    }
    if (types != JokeType::structure_template) {
        AddJokeSemErr(log, "\"*\" is not template", info->name, i, u);
        return nullptr;
    }
    info->type = JokeType::template_instance;
    auto counter = 0ull;
    while (nowline[u]) {
        u++;
        if (nowline[u] == '!') {
            tmpinfo = ParseTypedef(i, u, list, block, log);
        }
        else {
            tmpinfo = GetTypebyName(i, u, nowline, list, block, log);
        }
        if (!tmpinfo) {
            return nullptr;
        }
        info->ch_types.add(tmpinfo);
        counter++;
        nowline = list->file->loglines[i];
        if (!nowline)return nullptr;
        if (nowline[u] == '>')break;
        if (nowline[u] != ',') {
            AddJokeUnexpectedTokenErr(log, ",", nowline[u], i, u);
            return nullptr;
        }
    }
    if (info->root->size != counter) {
        if (!info->root->size) {
            AddJokeSemErr(log, "incomplete type is unsuable", nullptr, i, u);
        }
        else {
            AddJokeSemErr(log, "template parameter count is unmatched.", nullptr, i, u);
        }
        return nullptr;
    }
    info->ch_types.pack();
    info->ch_vars.unuse();
    info->size = counter;
    u++;
    return info;
}

EasyVector<char> CCNV jokescript::CollectId(unsigned long long& u,const char* nowline,JokeLogger* log) {
    EasyVector<char> ret;
    if (!nowline) {
        ret.unuse();
        return ret;
    }
    if (!(isalpha(nowline[u]) || nowline[u] == '_')) {
        return ret;
    }
    while (nowline[u]) {
        if (!(isalnum(nowline[u]) || nowline[u] == '_')) {
            break;
        }
        ret.add(nowline[u]);
        u++;
    }
    return ret;
}

JokeTypeInfo* CCNV jokescript::GetTypebyName(unsigned long long& i,unsigned long long& u, const char* nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
    EasyVector<char> id(nullptr),hold;
    JokeTypeInfo* root = nullptr;
    while (nowline[u]) {
        id = CollectId(u, nowline, log);
        hold.add_copy(id.get_const(), id.get_size());
        if (nowline[u] == '.') {
            if (!root) {
                root = SearchTypeOnBlock(id.get_const(), block);
            }
            else {
                root = SearchTypeOnChild(id.get_const(), root);
            }
            if (!root) {
                AddJokeSemErr(log, "type \"*\" is not defined",hold.get_const(),i,u);
                return nullptr;
            }
            hold.add('.');
            u++;
            continue;
        }
        break;
    }
    if (!root) {
        root = SearchTypeOnBlock(id.get_const(), block);
        if (!root&&!IsJokeReserved(id.get_const())) {
            root = CreateJokeTypeInfo(id.get_raw_z(), list);
            if (!root) {
                return nullptr;
            }
            root->type = JokeType::not_defined;
            block->undefinedlist->types.add(root);
        }
    }
    else {
        root = SearchTypeOnChild(id.get_const(), root);
    }
    if (!root) {
        AddJokeSemErr(log, "type \"*\" is not defined", hold.get_const(), i, u);
        return nullptr;
    }
    return root;
}

JokeTypeInfo* CCNV jokescript::SearchTypeOnUndefined(const char* type, JokeBlockList* block) {
    if (!type || !block)return nullptr;
    auto i = 0ull;
    while (block->undefinedlist->types[i]) {
        if (strcmp(type, block->undefinedlist->types[i]->name) == 0) {
            return block->undefinedlist->types[i];
        }
        i++;
    }
    return nullptr;
}

JokeTypeInfo* CCNV jokescript::SearchTypeOnBlock(const char* type, JokeBlockList* block) {
    if (!type || !block)return nullptr;
    JokeBlock* current = block->current;
    JokeTypeInfo* ret = nullptr;
    if (!current)return nullptr;
    while (1) {
        for (auto i = 0ull; current->types[i]; i++) {
            if (strcmp(type,current->types[i]->name)==0) {
                ret = current->types[i];
                if (ret->type == JokeType::type_alias) {
                    ret = ret->root;
                }
                return ret;
            }
        }
        for (auto i = 0ull; current->vars[i]; i++) {
            if (strcmp(type, current->vars[i]->name) == 0) {
                ret = current->vars[i]->type;
                if (ret) {
                    if (ret->type == JokeType::type_alias) {
                        ret = ret->root;
                    }
                }
                return ret;
            }
        }
        if (!current->parent)break;
        current = current->parent;
    }
    return nullptr;
}

JokeTypeInfo* CCNV jokescript::SearchTypeOnChild(const char* type, JokeTypeInfo* info) {
    if (!type || !info)return nullptr;
    JokeTypeInfo* search = info,*ret=nullptr;
    while (search->type == JokeType::type_alias) {
        search = search->root;
        if (!search)return nullptr;
    }
    if (search->type!=JokeType::builtin&&search->type != JokeType::structure&&search->type!=JokeType::template_instance)return nullptr;
    JokeTypeInfo* p=nullptr;
    if (search->type != JokeType::template_instance) {
        p = search;
    }
    else {
        p = search->root;
    }
    if (!p)return nullptr;
    for (auto i = 0ull; p->ch_types[i]; i++) {
        if (strcmp(type, p->ch_types[i]->name) == 0) {
            ret = p->ch_types[i];
            if (p->ch_types[i]->type == JokeType::template_param) {
                ret = search->ch_types[i];
            }
            return ret;
        }
    }
    for (auto k = 0ull; p->ch_vars[k]; k++) {
        if (strcmp(type, p->ch_vars[k]->name) == 0) {
            ret = p->ch_types[k];
            if (p->ch_types[k]->type == JokeType::template_param) {
                ret = search->ch_types[k];
            }
            return p->ch_vars[k]->type;
        }
    }
    return nullptr;
}

bool CCNV jokescript::TypeCmp(JokeTypeInfo* type1, JokeTypeInfo* type2) {
    return TypeCmp_detail(type1, type2, 0);
}

bool CCNV jokescript::TypeCmp_detail(JokeTypeInfo* type1, JokeTypeInfo* type2,unsigned long long depth) {
#define DEPTH_MAX 100
    if (depth > DEPTH_MAX)return false;
    if (type1 == type2)return true;
    if (!type1 || !type2)return false;
    JokeTypeInfo* finalcmp1 = type1,*finalcmp2=type2;

    while (1) {
        if (finalcmp1->type == JokeType::type_alias) {
            while (finalcmp1->type == JokeType::type_alias) {
                finalcmp1 = finalcmp1->root;
                if (!finalcmp1)return false;
            }
        }

        if (finalcmp2->type == JokeType::type_alias) {
            while (finalcmp2->type == JokeType::type_alias) {
               if (!finalcmp2)return false;
            }
        }

        if (finalcmp1 == finalcmp2)return true;
        
        if (finalcmp1->type==finalcmp2->type) {
            if (finalcmp1->type==JokeType::builtin) {
                if (strcmp(finalcmp1->name,finalcmp2->name)==0) {
                    return true;
                }
            }
            else if (finalcmp1->type == JokeType::pointer || finalcmp1->type == JokeType::vector||finalcmp1->type==JokeType::reference) {

            }
            else if (finalcmp1->type==JokeType::template_param) {
                return true;
            }
            else if (finalcmp1->type==JokeType::template_instance) {
                for (auto i = 0u; finalcmp1->ch_vars[i] || finalcmp2->ch_vars[i]; i++) {
                    if (!TypeCmp_detail(finalcmp1->ch_vars[i]->type, finalcmp2->ch_vars[i]->type, depth + 1))return false;
                }
                if (!TypeCmp_detail(finalcmp1->root, finalcmp2->root, depth + 1))return false;
            }
            else if (finalcmp1->type == JokeType::array) {
                if (finalcmp1->size != finalcmp2->size)return false;
            }
            else if (finalcmp1->type == JokeType::structure||finalcmp1->type==JokeType::structure_template) {
                for (auto i = 0u; finalcmp1->ch_vars[i] || finalcmp2->ch_vars[i]; i++) {
                    if (!TypeCmp_detail(finalcmp1->ch_vars[i]->type, finalcmp2->ch_vars[i]->type,depth+1))return false;
                }
                for (auto k = 0ull; finalcmp1->ch_types[k] || finalcmp2->ch_types[k]; k++) {
                    if (!TypeCmp_detail(finalcmp1->ch_types[k], finalcmp2->ch_types[k],depth + 1))return false;
                }
                return true;
            }
            else if (finalcmp1->type == JokeType::function) {
                for (auto i = 0u; finalcmp1->ch_vars[i] || finalcmp2->ch_vars[i]; i++) {
                    if (!TypeCmp_detail(finalcmp1->ch_vars[i]->type, finalcmp2->ch_vars[i]->type,depth+1))return false;
                }
                for (auto k = 0ull; finalcmp1->ch_types[k] || finalcmp2->ch_types[k]; k++) {
                    if (!TypeCmp_detail(finalcmp1->ch_types[k], finalcmp2->ch_types[k],depth+1))return false;
                }
                if (!TypeCmp_detail(finalcmp1->root, finalcmp2->root,depth+1))return false;
                return true;
            }
            finalcmp1 = finalcmp1->root;
            finalcmp2 = finalcmp2->root;
            if (!finalcmp1 || !finalcmp2)return false;
            continue;
        }
        break;
    }

    return false;
}

bool CCNV jokescript::AreAutoCastable(JokeTypeInfo* type1, JokeTypeInfo* type2) {

    return false;
}