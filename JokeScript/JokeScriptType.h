/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptCommon.h"
#include"JokeScriptInfoLists.h"
#include"JokeScriptLog.h"
#ifndef CCNV
#define CCNV __stdcall
#endif

#define is_first_of_identitier(x) (isalpha((unsigned char)(x))||(x)=='_')

#define JTYPE_null_t 0
#define JTYPE_s8 1
#define JTYPE_u8 2
#define JTYPE_s16 3
#define JTYPE_u16 4
#define JTYPE_s32 5
#define JTYPE_u32 6
#define JTYPE_s64 7
#define JTYPE_u64 8
#define JTYPE_f32 9
#define JTYPE_f64 10
#define JTYPE_string 11
#define JTYPE_bool 12

namespace jokescript{

    enum class JokeType{
        unset,
        builtin,
        type_alias,
        function,
        //function_template,
        function_options,
        operators,
        structure,
        structure_template,
        template_param,
        //template_param_va,
        template_instance,
        pointer,
        reference,
        array,
        vector,
        some_returns,
        not_defined
    };
    
    struct JokeTypeInfo {
        char* name=nullptr;
        bool intype = false;
        union {
            union {
                void* pointer;
                JokeBlock* block;
                JokeTypeInfo* type;
            }depends;
            char* optval=nullptr;
        };
        JokeTypeInfo* root=nullptr;
        unsigned long long size=0;
        JokeType type=JokeType::unset;
        EasyVector<JokeTypeInfo*> ch_types;
        EasyVector<JokeVariableInfo*> ch_vars;
        ~JokeTypeInfo();
    };

    void DeleteJokeTypeInfo(JokeTypeInfo* info);

    JokeTypeInfo* CCNV CreateJokeTypeInfo(char* name, JokeDefinitionList* list);

    JokeTypeInfo* CCNV ParseTypedef(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block,JokeLogger* log);

    JokeTypeInfo* CCNV ParseTypedef_detail(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV GetIdSemantics(unsigned long long& i, unsigned long long& u,const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetRootInfo(JokeTypeInfo* info,unsigned long long& i, unsigned long long& u, const char*& nowline,JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetFuncOrTemplateInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetFuncOpts(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetArrayOrPointerInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    

    JokeTypeInfo* CCNV SetReturnInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetStructureInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SetTemplateInfo(JokeTypeInfo* info, unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

    EasyVector<char> CCNV CollectId(unsigned long long& u, const char* nowline,JokeLogger* log);
    JokeTypeInfo* CCNV GetTypebyName(unsigned long long& i,unsigned long long& u, const char* nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);
    JokeTypeInfo* CCNV SearchTypeOnUndefined(const char* type, JokeBlockList* block);
    JokeTypeInfo* CCNV SearchTypeOnBlock(const char* type, JokeBlockList* block);
    JokeTypeInfo* CCNV SearchTypeOnChild(const char* type, JokeTypeInfo* info);

    bool CCNV TypeCmp(JokeTypeInfo* type1, JokeTypeInfo* type2);
    bool CCNV TypeCmp_detail(JokeTypeInfo* type1,JokeTypeInfo* type2,unsigned long long depth);
    bool CCNV AreAutoCastable(JokeTypeInfo* type1, JokeTypeInfo* type2);
}