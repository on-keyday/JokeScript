/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once 
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
		var,
		func,
		control
	};

	struct JokeTree {
		char* symbol=nullptr;
		JokeSymbol symtype=JokeSymbol::unset;
		JokeTree* left=nullptr;
		JokeTree* right=nullptr;
		JokeBlock* depends=nullptr;
		JokeTypeInfo* type=nullptr;
		EasyVector<JokeTree*> params=nullptr;
		JokeBlock* relblock = nullptr;
	};

	JokeTree* CreateJokeTree(char* symbol,JokeDefinitionList* list);

	bool SpecifyType(JokeTree* tree, unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTree* Expr(bool& res,unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTree* Comma(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect);

	JokeTree* Assigns(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* BinaryOps(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	template<class Now,class... After>
	JokeTree* BinaryOps(unsigned long long& i,unsigned long long& u,JokeDefinitionList* list,JokeBlockList* block,JokeLogger* log,bool& expect,Now&& now,After&&... afters) {
		JokeTree* ret = BinaryOps(i, u, list, block, log,expect, afters...),*tmptree=nullptr;
		if (!ret)return nullptr;
		const char* nowline = list->file->loglines[i];
		bool ok = false;
		if (!nowline) {
			return nullptr;
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


	JokeTree* UnaryOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* SingleOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect);

	JokeTree* NumberDec(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log);

	JokeTypeInfo* SuffixToType(JokeSymbol& willtype,EasyVector<char>& id, unsigned long long& i, unsigned long long& u,const char* nowline, JokeDefinitionList* list, JokeLogger* log);
}