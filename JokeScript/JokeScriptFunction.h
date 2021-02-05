/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptMacro.h"
#include"JokeScriptInfoLists.h"


namespace jokescript {
	struct JokeFunctionInfo {
		char* name;
		JokeTypeInfo* type;
		EasyVector<JokeVariableInfo*> params;
		JokeBlock* depends;
		JokeBlock* relblock;
	};

	JokeFunctionInfo* CCNV ParseFuncdef(unsigned long long& i, unsigned long long& u,JokeDefinitionList* list,JokeBlockList* block,JokeLogger* log);

	JokeFunctionInfo* CCNV SearchFuncOnBlock(const char* name,JokeBlockList* block);
}