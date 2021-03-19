/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"identifier.h"
#include"../../common/tools.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;

#define SEARCH_T(arg,search) if(arg&&!*arg){*arg=common::search_T(name,scope->memb.get_base()->search);ok=false;}

template<>
bool MembersP::name_conflict<Type>(const char* name) { if (!p)return true; return common::check_name_conflict(name, p->types); }
template<>
bool MembersP::name_conflict<Variable>(const char* name) { if (!p)return true; return common::check_name_conflict(name, p->vars); }
template<>
bool MembersP::name_conflict<Macro>(const char* name) { if (!p)return true; return common::check_name_conflict(name, p->macros); }
template<>
bool MembersP::name_conflict<Template>(const char* name) { if (!p)return true; return common::check_name_conflict(name, p->templates); }

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