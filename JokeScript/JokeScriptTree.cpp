/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptTree.h"
#include"JokeScriptType.h"
#include "JokeScriptVariable.h"
#include "JokeScriptFunction.h"
#include <ctype.h>

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
	list->trees.add(tree);
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
				AddJokeCompilerBrokenErr(log, "unexpected nullptr on \'tree->right->type\'.");
				return false;
			}
			if (tmp->type != JokeType::pointer) {
				AddJokeSemErr(log, "dereference is unspecifiable",nullptr,i,u);
				return false;
			}
			tree->type = tmp->root;
			if (!tree->type) {
				AddJokeCompilerBrokenErr(log, "unexpected nullptr on \'tree->type\'.");
				return false;
			}
		}
	}
	return true;
}

JokeTree* jokescript::Expr(bool& res,unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	res = false;
	JokeTree* ret = nullptr;
	bool expect = true;
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	if (nowline[0] == '!') {
		auto type = ParseTypedef(i, u, list, block, log);
		if (!type) {
			return nullptr;
		}
	}
	else if (nowline[0] == '$') {
		auto var = ParseVardef(i, u, list, block, log);
		if (!var) {
			return nullptr;
		}
	}
	else if (nowline[0] == '@') {
		auto func = ParseFuncdef(i, u, list, block, log);
		if (!func) {
			return nullptr;
		}
		expect = false;
	}
	else {
		ret = Assigns(i,u,list,block,log,expect);
		if (!ret) {
			return nullptr;
		}
	}

	if (expect) {
		const char* nowline = list->file->loglines[i];
		if (!nowline) {
			AddJokeUnexpectedEOFErr(log, i, u);
			return false;
		}
		if (nowline[u] != ';') {
			AddJokeUnexpectedTokenErr(log, ";", nowline[u], i, u);
			return false;
		}
	}

	res = true;
	return ret;
}

const char* As[] = {
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"|=",
	"^=",
	"**="
};

const char* Or[] = {
	"||"
};

const char* And[] = {
	"&&"
};

const char* Eaq[] = {
	"==",
	"!="
};

const char* Rel[] = {
	"<",
	"<=",
	">",
	">="
};

const char* Shift[] = {
	"<<",
	">>"
};

const char* Bor[] = {
	"|"
};

const char* Bxor[] = {
	"^"
};

const char* Band[] = {
	"&"
};

const char* Add[] = {
	"+",
	"-"
};

const char* Mul[] = {
	"*",
	"/",
	"%"
};

const char* Pow[] = {
	"**"
};

#define FirstBinOpts(...) BinaryOps(__VA_ARGS__,Or,And,Eaq,Rel,Shift,Bor,Bxor,Band,And,Mul,Pow)

JokeTree* jokescript::Assigns(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect) {
	JokeTree* ret = FirstBinOpts(i, u, list, block, log,expect), * tmptree = nullptr;
	if (!ret)return nullptr;
	const char* nowline = list->file->loglines[i];
	bool ok = false;
	while (1) {
		for (auto s : As) {
			auto len = strlen(s);
			if (strncmp(&nowline[u],s,len)==0) {
				u += len;
				tmptree = CreateJokeTree(StringFilter() = s, list);
				if (!tmptree) {
					AddJokeMemoryFullErr(log);
					return nullptr;
				}
				tmptree->symtype = JokeSymbol::bin;
				tmptree->params.unuse();
				tmptree->left = ret;
				ret = tmptree;
				tmptree = nullptr;
				tmptree = Assigns(i, u, list, block, log,expect);
				if (!tmptree) {
					return nullptr;
				}
				ret->right = tmptree;
				tmptree = nullptr;
				if (!SpecifyType(ret,i,u,list, block, log)) {
					return nullptr;
				}
				ok = true;
				break;
			}
		}
	}
	return ret;
}

JokeTree* jokescript::BinaryOps(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	return UnaryOpts(i, u, list, block,log,expect);
}

JokeTree* jokescript::UnaryOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	return SingleOpts(i, u, list, block, log, expect);
}

JokeTree*jokescript::SingleOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	JokeTree* ret = nullptr,*tmptree=nullptr;
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	if (nowline[u] == '(') {
		while (1) {
			u++;
			ret = Assigns(i, u, list, block, log, expect);
			if (!ret) {
				return nullptr;
			}
			nowline = list->file->loglines[i];
			if (!nowline) {
				AddJokeUnexpectedEOFErr(log, i, u);
				return nullptr;
			}
			expect = true;
			if (nowline[u] == ',') {
				if (nowline[u+1]==')') {
					u++;
					break;
				}
				tmptree = CreateJokeTree(StringFilter() = ",", list);
				if (!tmptree) {
					AddJokeMemoryFullErr(log);
					return nullptr;
				}
				tmptree->symtype = JokeSymbol::bin;
				tmptree->left = ret;
			}
			else if (nowline[u]==')') {
				break;
			}
			else {
				AddJokeUnexpectedEOFErr(log, i, u);
				return nullptr;
			}
		}
	}
	else if (isdigit((unsigned char)nowline[u])) {
		/*
		int base = 10;
		int ofs = 0;
		if (nowline[u]=='0') {
			if (nowline[u+1]=='x'||nowline[u+1]=='X') {
				base = 16;
				ofs = 2;
			}
			else if (nowline[u+1] == 'b'||nowline[u+1]=='B') {
				base = 2;
				ofs = 2;
			}
			else if (nowline[u+1]>='0'&&nowline[u+1]<='7') {
				base = 8;
				ofs = 1;
			}
			else if (isalnum((unsigned char)nowline[u+1])) {
				AddJokeUnexpectedTokenErr(log,"xXbB01234567",nowline[u+1],i,u);
				return nullptr;
			}
		}
		char* check = nullptr;
		unsigned long long res=strtoull(&nowline[u+ofs], &check, base);
		if (!res && check==&nowline[u+ofs]) {
			AddJokeSynErr(log, "unknown number like",nullptr, i,u);
			return nullptr;
		}
		EasyVector<char> id;
		while (&nowline[u] != check) {
			id.add(nowline[u]);
		}
		ret=CreateJokeTree(id.get_raw_z(), list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		ret->symtype = JokeSymbol::integer;*/
		ret = NumberDec(i, u, list, block, log);
	}
	return ret;
}

JokeTree* jokescript::NumberDec(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
	JokeTree* ret = nullptr;
	EasyVector<char> id;
	JokeSymbol willtype = JokeSymbol::integer;
	int base = 10;
	int ofs = 0;
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	while (true) {
		if (nowline[u] == '0') {
			if (nowline[u + 1] == 'x' || nowline[u + 1] == 'X') {
				base = 16;
				ofs = 2;
			}
			else if (nowline[u + 1] == 'b' || nowline[u + 1] == 'B') {
				base = 2;
				ofs = 2;
			}
			else if (nowline[u + 1] >= '0' && nowline[u + 1] <= '7') {
				base = 8;
				ofs = 1;
			}
			else if (!isalnum((unsigned char)nowline[u + 1])) {
				AddJokeUnexpectedTokenErr(log, "xXbB01234567", nowline[u + 1], i, u);
				return nullptr;
			}
		}
		char* check = nullptr;
		unsigned long long res = strtoull(&nowline[u + ofs], &check, base);
		if (!res && check == &nowline[u + ofs]) {
			AddJokeSynErr(log, "unknown number like", nullptr, i, u);
			return nullptr;
		}
		if (base == 10 && (*check == '.' || *check == 'e'||*check=='E')) {
			willtype = JokeSymbol::real;
			break;
		}
		else if (base==16&&(*check=='.'||*check=='p'||*check=='P')) {
			willtype = JokeSymbol::real;
			break;
		}
		while (&nowline[u] != check) {
			id.add(nowline[u]);
			u++;
		}
		break;
	}

	if (willtype==JokeSymbol::real) {
		bool doted = false;
		bool eorp = false;
		bool ok = true;
		if (base == 16) {
			id.add_copy(&nowline[u], 2);
			u += 2;
		}
		while (true) {
			if (base == 10) {
				if (isdigit((unsigned char)nowline[u])) {
					id.add(nowline[u]);
					if (doted||eorp) {
						ok = true;
					}
				}
				else if(nowline[u]=='.'){
					if (doted||eorp) {
						AddJokeUnexpectedTokenErr(log, "0123456789", '.', i, u);
						return nullptr;
					}
					id.add('.');
					doted = true;
				}
				else if (!eorp && (nowline[u] == 'e' || nowline[u] == 'E')) {
					id.add(nowline[u]);
					u++;
					if (nowline[u] != '+' && nowline[u] != '-') {
						AddJokeUnexpectedTokenErr(log,"+-",nowline[u], i, u);
						return nullptr;
					}
					id.add(nowline[u]);
					ok = false;
					eorp = true;
				}
				else {
					break;
				}
			}
			else {
				if (!eorp&&(isdigit((unsigned char)nowline[u])||(nowline[u] >= 'a' && nowline[u] <= 'f'|| nowline[u] >= 'A' && nowline[u] <= 'F'))) {
					id.add(nowline[u]);
				}
				else if (isdigit((unsigned char)nowline[u])) {
					id.add(nowline[u]);
					ok = true;
				}
				else if (nowline[u]=='.') {
					if (doted||eorp) {
						AddJokeUnexpectedTokenErr(log, "0123456789",'.', i, u);
						return nullptr;
					}
					id.add('.');
					doted = true;
				}
				else if (!eorp&&(nowline[u]=='p'||nowline[u]=='P')) {
					id.add(nowline[u]);
					u++;
					if (nowline[u]!='+'&&nowline[u]!='-') {
						AddJokeUnexpectedTokenErr(log,"+-",nowline[u], i, u);
						return nullptr;
					}
					id.add(nowline[u]);
					eorp = true;
				}
				else {
					break;
				}
			}
			u++;
		}
		if (!ok) {
			return nullptr;
		}
	}

	auto type = SuffixToType(willtype, id, i, u, nowline, list, log);

	ret = CreateJokeTree(id.get_raw_z(), list);
	if (!ret) {
		AddJokeMemoryFullErr(log);
		return nullptr;
	}
	ret->symtype = willtype;
	ret->type = type;

	return ret;
}

JokeTypeInfo* jokescript::SuffixToType(JokeSymbol& willtype,EasyVector<char>& id, unsigned long long& i, unsigned long long& u,const char* nowline, JokeDefinitionList* list, JokeLogger* log) {
	JokeTypeInfo* ret=nullptr;
	bool u_suffix = false, f_suffix = false, l_suffix = false;
	int h_suffix = 0;

	if (nowline[u] == 'u' || nowline[u] == 'U') {
		u_suffix = true;
	}
	else if (nowline[u] == 'f' || nowline[u] == 'F') {
		f_suffix = true;
	}
	else if (nowline[u] == 'l' || nowline[u] == 'L') {
		l_suffix = true;
	}
	else if (nowline[u] == 'h' || nowline[u] == 'H') {
		h_suffix++;
	}

	if (u_suffix || f_suffix || l_suffix || h_suffix) {
		id.add(nowline[u]);
		u++;
	}

	if (nowline[u] == 'u' || nowline[u] == 'U') {
		if (f_suffix || u_suffix) {
			AddJokeSynErr(log, "unexpected suffix \"U\".", nullptr, i, u);
			return nullptr;
		}
		u_suffix = true;
		id.add(nowline[u]);
		u++;
	}
	else if (nowline[u] == 'l' || nowline[u] == 'L') {
		if (f_suffix || h_suffix) {
			AddJokeSynErr(log, "unexpected suffix \"L\".", nullptr, i, u);
			return nullptr;
		}
		l_suffix = true;
		id.add(nowline[u]);
		u++;
	}
	else if (nowline[u] == 'h' || nowline[u] == 'H') {
		if (f_suffix || l_suffix) {
			AddJokeSynErr(log, "unexpected suffix \"L\".", nullptr, i, u);
			return nullptr;
		}
		h_suffix++;
		id.add(nowline[u]);
		u++;
	}


	if (!f_suffix) {
		if (willtype == JokeSymbol::real) {
			ret = list->types[10];//f64
		}
		else {
			if (u_suffix) {
				if (l_suffix) {
					ret = list->types[8];//u64 
				}
				else if (h_suffix == 1) {
					ret = list->types[4];//u16
				}
				else if (h_suffix == 2) {
					ret = list->types[2];//u8
				}
				else {
					ret = list->types[6];//u32
				}
			}
			else {
				if (l_suffix) {
					ret = list->types[7];//s64 
				}
				else if (h_suffix == 1) {
					ret = list->types[5];//s16
				}
				else if (h_suffix == 2) {
					ret = list->types[1];//s8
				}
				else {
					ret = list->types[5];//s32
				}
			}
		}
	}
	else {
		ret = list->types[9];//f32
		willtype = JokeSymbol::real;
	}

	return ret;
}