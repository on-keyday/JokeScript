/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"common_structs.h"

namespace PROJECT_NAME {
	namespace common {
		struct Hash {
		private:
			uint64_t unnamed_count = 0;
		public:
			bool name_hash(EasyVector<char>& buf);
			uint64_t make_hash(const char* str);
			char* get_hash();
		};

		uint64_t count_while_f(const char* p,bool(*judge)(char));
		uint64_t strtoull_ex(const char* str,char** p);

	}
}