/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"compiler_identifier_holder.h"
namespace PROJECT_NAME {
	namespace compiler {
		Type* get_named_type(const char* name, bool unnamd, IdHolder* holder);

		Type* type_analyze(IdHolder* holder,Reader* reader);
		Type* type_detail(const char* name,IdHolder* holder, Reader* reader,bool unname);

		Type* get_ids(IdHolder* holder, Reader* reader);
		Type* get_common_derived(TypeType ttype, uint64_t size,IdHolder* holder,Reader* reader,const char* err);
		Type* get_has_size(IdHolder* holder, Reader* reader);
		Type* get_funcs(const char* name, IdHolder* holder, Reader* reader, bool unname);
		Type* get_sets(const char* name, IdHolder* holder, Reader* reader, bool unname);

		Type* resolve_type_by_name(IdHolder* holder, Reader* reader);
		Type* search_type_on_block(const char* name,IdHolder* holder,int& isvar);
		Type* search_type_on_type(const char* name,Type* base,int& isvar);

		Type* resolve_template(Type* base,IdHolder* holder,Reader* reader);

		Type* get_number_type(const char* num,IdHolder* holder);
		Type* get_derived(TypeType ttype,uint64_t size,Type* base,IdHolder* holder);

		namespace ttype {
			bool is_templatetype(TypeType ttype);
			bool is_derivedtype(TypeType ttype);
			bool is_naming(TypeType ttype);
		}
		
		bool typecmp(Type* t1, Type* t2);


		Identifier* id_analyze(IdHolder* holder, Reader* reader);

		Identifier* search_id_on_block(const char* name, IdHolder* holder);
	}
}