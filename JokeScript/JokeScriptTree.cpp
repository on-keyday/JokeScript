/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptTree.h"
#include"JokeScriptType.h"

using namespace jokescript;

JokeTree* jokescript::CreateJokeTree(char* symbol, JokeDefinitionList* list) {
	if (!symbol || !list)return nullptr;
	
	JokeTree* tree = nullptr;
	try {
		tree = new JokeTree;
	}
	catch (...) {
		return nullptr;
	}
	tree->symbol = symbol;
	return tree;
}

bool jokescript::SpecifyType(JokeTree* tree, unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	if (!tree)return false;
	if (!tree->right)return false;
	if (tree->symtype == JokeSymbol::unary) {
		if (strcmp(tree->symbol,"*")==0) {
			auto tmp = tree->right->type;
			if (!tmp) {
				AddJokeSysErr(log, "unexpected nullptr on \'tree->right->type\'. this compiler is broken", nullptr);
				return false;
			}
			if (tmp->type != JokeType::pointer) {
				AddJokeSemErr(log, "dereference is unspecifiable",nullptr,i,u);
				return false;
			}
			tree->type = tmp->root;
			if (!tree->type) {
				AddJokeSysErr(log, "unexpected nullptr on \'tree->type\'. this compiler is broken", nullptr);
				return false;
			}
		}
	}
	return true;
}

JokeTree* jokescript::Expr(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	JokeTree* ret = Assigns(i, u, list, block, log);
	if (!list->file->loglines[i]) {
		return nullptr;
	}
	if (list->file->loglines[i][u] != ';') {
		return nullptr;
	}
	return ret;
}

JokeTree* jokescript::Assigns(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	return nullptr;
}