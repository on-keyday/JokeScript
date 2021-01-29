/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"JokeScriptInfoLists.h"


namespace jokescript {
	struct JokeFunctionInfo {
		JokeTypeInfo* type;
		EasyVector<JokeVariableInfo*> params;
	};

	JokeFunctionInfo* ParseFuncdef(unsigned long long& i, unsigned long long& u,JokeDefinitionList* list,JokeBlockList* block,JokeLogger* log);
}