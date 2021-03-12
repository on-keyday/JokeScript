/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"identifier.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;

#define SEARCH_T(arg,search) if(arg&&!*arg){*arg=search_T(name,scope->memb.get_base()->search);ok=false;}

bool identifier::is_derived(TypeKind kind) {
	return false;
}

bool identifier::search_name(const char* name,Maker* maker, Type** type, Variable** var, Template** temp, Macro** macro) {
	if (!name)return false;
	auto scope = maker->scope;
	while (scope) {
		bool ok = false;
		if (scope->memb.get_base()) {
			ok = true;
			SEARCH_T(type, types);
			SEARCH_T(var, vars);
			SEARCH_T(temp, templates);
			SEARCH_T(macro, macros);
		}
		if (ok)break;
		scope = scope->prev;
	}
	return true;
}