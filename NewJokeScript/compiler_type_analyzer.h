/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"compiler_identifier_holder.h"
namespace jokescript {
	namespace compiler {
		Type* type_analyze(IdHolder* holder,Reader* reader);
		Type* type_detail(IdHolder* holder, Reader* reader);
		Type* resolve_type_by_name(IdHolder* holder, Reader* reader);
		
		Type* search_type_on_block(const char* name,IdHolder* holder,int& isvar);
		Type* search_type_on_type(const char* name,Type* base,int& isvar);

		Type* get_derived(TypeType ttype,Type* base,IdHolder* holder);
	}
}