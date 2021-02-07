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

JokeTree* CCNV jokescript::CreateJokeTree(char* symbol, JokeDefinitionList* list) {
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

bool CCNV jokescript::SpecifyType(JokeTree* tree, unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
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

JokeTree* CCNV jokescript::Expr(bool& res,unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
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
			return nullptr;
		}
		if (nowline[u] != ';') {
			AddJokeUnexpectedTokenErr(log, ";", nowline[u], i, u);
			return nullptr;
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

#define FirstBinOpts(...) BinaryOps(__VA_ARGS__,Or,And,Eaq,Rel,Shift,Bor,Bxor,Band,Add,Mul,Pow)

JokeTree* CCNV jokescript::Comma(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	JokeTree* ret = Assigns(i,u,list,block,log,expect),*tmptree=nullptr;
	if (!ret)return nullptr;
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		return ret;
	}
	while (1) {
		if (nowline[u] == ',') {
			u++;
			tmptree = CreateJokeTree(StringFilter() = ",", list);
			if (!tmptree) {
				AddJokeMemoryFullErr(log);
				return nullptr;
			}
			tmptree->symtype = JokeSymbol::bin;
			tmptree->left = ret;
			tmptree->params.unuse();
			ret = tmptree;
			tmptree = Assigns(i, u, list, block, log, expect);
			if (!tmptree) {
				return nullptr;
			}
			ret->right = tmptree;
			ret->type=tmptree->type;
			tmptree = nullptr;
			continue;
		}
		break;
	}
	return ret;
}

JokeTree* CCNV jokescript::Assigns(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect) {
	JokeTree* ret = FirstBinOpts(i, u, list, block, log,expect), * tmptree = nullptr;
	if (!ret)return nullptr;
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		return ret;
	}
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
		if (!ok)break;
	}
	return ret;
}

JokeTree* CCNV jokescript::BinaryOps(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	return UnaryOpts(i, u, list, block,log,expect);
}

JokeTree* CCNV jokescript::UnaryOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	
	return SingleOpts(i, u, list, block, log, expect);
}

JokeTree* CCNV jokescript::SingleOpts(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log, bool& expect) {
	JokeTree* ret = nullptr,*tmptree=nullptr;
	JokeVariableInfo* var = nullptr;
	JokeFunctionInfo* func = nullptr;
	EasyVector<char> id(nullptr);
	const char* nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	if (nowline[u] == '(') {
		ret = Comma(i, u, list, block, log, expect);
		if (!ret) {
			return nullptr;
		}
		nowline = list->file->loglines[i];
		if (!nowline) {
			AddJokeUnexpectedEOFErr(log, i, u);
			return nullptr;
		}
		if (nowline[u]!=')') {
			AddJokeUnexpectedTokenErr(log, ")", nowline[u], i, u);
			return nullptr;
		}
		expect = true;
		u++;
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
	else if(strncmp(&nowline[u], "true", 4) && !is_first_of_identitier(nowline[u + 4])){
		ret= CreateJokeTree(StringFilter() = "true", list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		ret->symtype = JokeSymbol::boolean;
		ret->type = list->types[JTYPE_bool];
		ret->params.unuse();
	}
	else if (strncmp(&nowline[u], "false", 5) && !is_first_of_identitier(nowline[u + 5])) {
		ret = CreateJokeTree(StringFilter() = "false", list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		ret->symtype = JokeSymbol::boolean;
		ret->type = list->types[JTYPE_bool];
		ret->params.unuse();
	}
	else if (strncmp(&nowline[u], "null", 4)&&!is_first_of_identitier(nowline[u+4])) {
		ret = CreateJokeTree(StringFilter() = "null", list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		ret->symtype = JokeSymbol::null;
		ret->type = list->types[JTYPE_null_t];
		ret->params.unuse();
	}
	else if (strncmp(&nowline[u],"import ",7)) {
		AddJokeSysErr(log, "on this compiler, \"import\" is not supported.", nullptr);
		return nullptr;
	}
	else if (strncmp(&nowline[u], "match", 5)&&!is_first_of_identitier(nowline[u+5])) {
		ret = Match(i, u, nowline,list, block, log,expect);
		if (!ret)return nullptr;
	}
	else if (is_first_of_identitier(nowline[u])) {
		id = CollectId(u, nowline, log);
		if (nowline[u] == '(') {
			func = SearchFuncOnBlock(id.get_const(),block);
			if (!func) {
				var = SearchVarOnBlock(id.get_const(), block);
			}
		}
		else {
			var = SearchVarOnBlock(id.get_const(), block);
			if (!var) {
				func = SearchFuncOnBlock(id.get_const(), block);
			}
		}
		if (!func && !var) {
			AddJokeSemErr(log, "\"*\" is not defined.",id.get_const(),i,u);
			return nullptr;
		}
		ret = CreateJokeTree(id.get_raw_z(), list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		ret->params.unuse();
		if (func) {
			ret->symtype = JokeSymbol::func;
			ret->type = func->type;
			ret->reltype = TreeRel::func;
			ret->rel.func = func;
		}
		else {
			ret->symtype = JokeSymbol::var;
			ret->type = var->type;
			ret->reltype = TreeRel::var;
			ret->rel.var = var;
		}
	}
	else if (nowline[u]=='@') {
		func = ParseFuncdef(i, u, list, block, log);
		if (!func) {
			return nullptr;
		}
		ret= CreateJokeTree(StringFilter()=func->name, list);
		if (!ret) {
			AddJokeMemoryFullErr(log);
			return nullptr;
		}
		expect = false;
		ret->params.unuse();
		ret->symtype = JokeSymbol::func;
		ret->type = func->type;
		ret->rel.func = func;
		ret->reltype = TreeRel::func;
		i++;
		nowline = list->file->loglines[i];
	}
	else if (nowline[u]=='\0'&&list->file->lines[i+1]) {
		if (list->file->lines[i + 1][0]=='{') {
			i+=2;
			u = 0;
			nowline = list->file->lines[i];
			if (!nowline) {
				AddJokeUnexpectedEOFErr(log, i, u);
				return nullptr;
			}
			ret = CreateJokeTree(StringFilter() = "{}", list);
			ret->symtype = JokeSymbol::array;
			JokeTypeInfo* type=CreateJokeTypeInfo(StringFilter() = "[]", list);
			if (!type) {
				AddJokeMemoryFullErr(log);
				return nullptr;
			}
			type->type = JokeType::vector;
			type->ch_types.unuse();
			type->ch_types.unuse();
			while (nowline[u]) {
				tmptree = Assigns(i, u, list, block, log, expect);
				if (!tmptree) {
					return nullptr;
				}
				type->size++;
				if (!type->root) {
					type->root = tmptree->type;
				}
				ret->params.add(tmptree);
				nowline = list->file->loglines[i];
				if (!nowline) {
					AddJokeUnexpectedEOFErr(log, i, u);
					return nullptr;
				}
				if (nowline[u] == ',') {
					u++;
					continue;
				}
				else if (nowline[u]=='\0') {
					if (!type->root) {
						AddJokeSemErr(log,"0 length array is invalid",nullptr,i,u);
						return nullptr;
					}
					i++;
					u = 0;
					nowline = list->file->loglines[i];
					if (!nowline) {
						AddJokeUnexpectedEOFErr(log, i, u);
						return nullptr;
					}
					if (nowline[u] != '}') {
						AddJokeUnexpectedTokenErr(log, "}", nowline[u], i, u);
						return nullptr;
					}
					i++;
					nowline = list->file->loglines[i];
					if (!nowline) {
						AddJokeUnexpectedEOFErr(log, i, u);
						return nullptr;
					}
				}
			}
		}
	}

	if (nowline) {
		while (1) {
			if (nowline[u] == '(') {
				expect = true;
			}
			else if (nowline[u] == '.') {
				u++;
			}
			else if (nowline[u] == '-' && nowline[u + 1] == '>') {
				u += 2;
			}
			else {
				break;
			}
		}
	}
	return ret;
}

JokeTree* CCNV jokescript::NumberDec(unsigned long long& i, unsigned long long& u, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log) {
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
	ret->params.unuse();
	return ret;
}

JokeTypeInfo* CCNV jokescript::SuffixToType(JokeSymbol& willtype,EasyVector<char>& id, unsigned long long& i, unsigned long long& u,const char* nowline, JokeDefinitionList* list, JokeLogger* log) {
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
			ret = list->types[JTYPE_f64];//f64
		}
		else {
			if (u_suffix) {
				if (l_suffix) {
					ret = list->types[JTYPE_u64];//u64 
				}
				else if (h_suffix == 1) {
					ret = list->types[JTYPE_u16];//u16
				}
				else if (h_suffix == 2) {
					ret = list->types[JTYPE_u8];//u8
				}
				else {
					ret = list->types[JTYPE_u32];//u32
				}
			}
			else {
				if (l_suffix) {
					ret = list->types[JTYPE_s64];//s64 
				}
				else if (h_suffix == 1) {
					ret = list->types[JTYPE_s16];//s16
				}
				else if (h_suffix == 2) {
					ret = list->types[JTYPE_s8];//s8
				}
				else {
					ret = list->types[JTYPE_s32];//s32
				}
			}
		}
	}
	else {
		ret = list->types[JTYPE_f32];//f32
		willtype = JokeSymbol::real;
	}

	return ret;
}

JokeTree* CCNV jokescript::Match(unsigned long long& i, unsigned long long& u,const char*& nowline, JokeDefinitionList* list, JokeBlockList* block, JokeLogger* log,bool& expect) {
	u += 5;
	if (nowline[u]==' ') {
		u++;
	}
	JokeTree* judge = Assigns(i, u, list, block, log, expect),*tmptree=nullptr;
	if (!judge) {
		return nullptr;
	}
	JokeTypeInfo* cmp_type = judge->type,*result_type=nullptr;
	JokeTree* ret = CreateJokeTree(StringFilter() = "match", list);
	if (!ret) {
		AddJokeMemoryFullErr(log);
		return nullptr;
	}
	ret->symtype = JokeSymbol::control;
	ret->right = judge;
	judge = nullptr;
	nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	u = 0;
	if (nowline[u] != '{') {
		AddJokeUnexpectedTokenErr(log, "{", nowline[u], i, u);
		return nullptr;
	}
	i++;
	nowline = list->file->loglines[i];
	if (!nowline) {
		AddJokeUnexpectedEOFErr(log, i, u);
		return nullptr;
	}
	while (nowline[u]) {
		if (nowline[u] == '(') {
			tmptree = CreateJokeTree(StringFilter() = "?=", list);
			if (!tmptree) {
				AddJokeMemoryFullErr(log);
				return nullptr;
			}
			tmptree->symtype = JokeSymbol::bin;
			judge = Assigns(i, u, list, block, log,expect);
			if (!judge) {
				return nullptr;
			}
			tmptree->left = judge;
			nowline = list->file->loglines[i];
			if (!nowline) {
				AddJokeUnexpectedEOFErr(log, i, u);
				return nullptr;
			}
			if (nowline[u] != '?') {
				AddJokeUnexpectedTokenErr(log, "?", nowline[u], i, u);
				return nullptr;
			}
			u++;
			if (nowline[u] != '=') {
				AddJokeUnexpectedTokenErr(log, "=", nowline[u], i, u);
				return nullptr;
			}
			u++;
			judge = Assigns(i, u, list, block, log, expect);
			if (!judge) {
				return nullptr;
			}
			tmptree->right = judge;
			ret->params.add(tmptree);
			if (!AreAutoCastable(tmptree->left->type,cmp_type)) {
				AddJokeSemErr(log, "on match expression, types are not auto-castable.", nullptr, i, u);
				return nullptr;
			}
			if (!result_type) {
				result_type = tmptree->right->type;
			}
			else {
				if (!AreAutoCastable(tmptree->right->type, cmp_type)) {
					AddJokeSemErr(log, "on match expression, types are not auto-castable.", nullptr, i, u);
					return nullptr;
				}
			}
			nowline = list->file->loglines[i];
			if (!nowline) {
				AddJokeUnexpectedEOFErr(log, i, u);
				return nullptr;
			}
			if (nowline[u] != ';') {
				AddJokeUnexpectedTokenErr(log, ";", nowline[u], i, u);
				return nullptr;
			}
		}
		else if (nowline[u]=='}') {
			i++;
			u = 0;
			break;
		}
		else {
			AddJokeUnexpectedTokenErr(log, "(}", nowline[u], i, u);
			return nullptr;
		}
		i++;
		nowline = list->file->loglines[i];
		if (!nowline) {
			AddJokeUnexpectedEOFErr(log, i, u);
			return nullptr;
		}
		u = 0;
	}
	if (result_type) {
		ret->type = result_type;
	}
	else {
		ret->type = list->types[JTYPE_null_t];//null_t
	}
	expect = false;
	nowline = list->file->loglines[i];
	return ret;
}