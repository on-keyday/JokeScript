/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptFunction.h"

using namespace jokescript;

JokeFunctionInfo* CCNV jokescript::ParseFuncdef(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	if (!list->file->loglines[i]) {
		return nullptr;
	}
	return nullptr;
}

JokeFunctionInfo* CCNV jokescript::SearchFuncOnBlock(const char* name, JokeBlockList* block) {
	return nullptr;
}