/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once 
#include"JokeScriptMacro.h"
#include"JokeScriptCommon.h"
#include"JokeScriptInfoLists.h"
#include<string.h>

namespace jokescript {
	enum class JokeSymbol {
		unset,
		bin,
		unary,
		integer,
		real,
		string,
		null,
		boolean,
		array,
		var,
		func,
		control
	};

	enum class TreeRel {
		unset,
		block,
		var,
		func
	};

	struct JokeTree {
		char* symbol=nullptr;
		JokeSymbol symtype=JokeSymbol::unset;
		JokeTree* left=nullptr;
		JokeTree* right=nullptr;
		JokeBlock* depends=nullptr;
		JokeTypeInfo* type=nullptr;
		EasyVector<JokeTree*> params=nullptr;
		TreeRel reltype=TreeRel::unset;
		union {
			JokeBlock* block = nullptr;
			JokeVariableInfo* var;
			JokeFunctionInfo* func;
		}rel;
	};

	JokeTree* CCNV CreateJokeTree(char* symbol,JokeDefinitionList* list);

	bool CCNV SpecifyType(JokeTree* tree, unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTree* CCNV Control(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTree* CCNV Expr(bool& res,unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTree* CCNV Comma(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect);

	JokeTree* CCNV Assigns(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* CCNV BinaryOps(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	template<class Now,class... After>
	JokeTree* CCNV BinaryOps(unsigned long long& i,unsigned long long& u,JokeDefinitionList* list,JokeBlockList* block,JokeLogger* log,bool& expect,Now&& now,After&&... afters) {
		JokeTree* ret = BinaryOps(i, u, list, block, log,expect, afters...),*tmptree=nullptr;
		if (!ret)return nullptr;
		const char* nowline = list->file->loglines[i];
		bool ok = false;
		if (!nowline) {
			return ret;
		}
		while (nowline[u]) {
			for (auto symbol : now) {
				auto leng = strlen(symbol);
				if (strncmp(&nowline[u], symbol, leng)==0) {
					if (leng==1) {
						if (nowline[u + 1] == symbol[0]) {
							continue;
						}
					}
					u += leng;
					tmptree = CreateJokeTree(StringFilter() = symbol, list);
					if (!tmptree) {
						return nullptr;
					}
					tmptree->symtype = JokeSymbol::bin;
					tmptree->params.unuse();
					tmptree->left = ret;
					ret = tmptree;
					tmptree = nullptr;
					tmptree = BinaryOps(i, u, list, block, log,expect,afters...);
					if (!tmptree) {
						return nullptr;
					}
					ret->right = tmptree;
					tmptree = nullptr;
					if (!SpecifyType(ret,i,u,list,block,log)) {
						return nullptr;
					}
					ok = true;
					break;
				}
			}
			if (!ok)break;
			ok = false;
		}
		return ret;
	}


	JokeTree* CCNV UnaryOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* CCNV SingleOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* CCNV NumberDec(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTypeInfo* CCNV SuffixToType(JokeSymbol& willtype,EasyVector<char>& id, unsigned long long& i, unsigned long long& u,const char* nowline, JokeDefinitionList* list, JokeLogger* log);

	JokeTree* CCNV Match(unsigned long long& i, unsigned long long& u, const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

}