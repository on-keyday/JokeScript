/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"structs.h"

namespace PROJECT_NAME {
	namespace common {
		struct Hash {
		private:
			uint64_t hash_count = 0;
			StringP tmpc = nullptr;
		public:
			bool name_hash(String& buf,const char* name);
			bool unname_hash(String& buf);
			uint64_t make_hash(const char* str);
			char* get_hash();
			char* get_hash_with_name(const char* s);
			const char* get_hash_const();
		};

		uint64_t count_while_f(const char* p,bool(*judge)(char));
		uint64_t strtoull_ex(const char* str,char** p);


		StringP to_utf8(char32_t c);
		String16P to_utf16(char32_t c);
		char32_t to_utf32(const char* c,bool& suc);
		char32_t to_utf32(const char16_t* c, bool& suc);
		int get_utf8bytesize(char c);
		
	}
}