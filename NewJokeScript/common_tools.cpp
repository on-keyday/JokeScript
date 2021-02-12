/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"common_tools.h"
#include"compiler_ctype.h"
#include"stdcpps.h"

using namespace PROJECT_NAME;

bool common::Hash::name_hash(EasyVector<char>& buf) {
	buf.add_copy("%unnamed_",0);
	auto num=std::to_string(unnamed_count);
	unnamed_count++;
	buf.add_copy(num.c_str(), num.length());
	buf.add('_');
	num = std::to_string(make_hash(buf.get_const()));
	buf.add_copy(num.c_str(), num.length());
	return true;
}

uint64_t common::Hash::make_hash(const char* str) {
	if (!str)return 0;
	uint64_t hash = 0xabf28ce5842b3125, i = 0;
	while (str[i]) {
		hash *= 0x100000101c2;
		hash ^= (unsigned char)str[i];
		i++;
	}
	if (i == 0)return 0;
	return hash;
}

char* common::Hash::get_hash() {
	common::EasyVector<char> tmp;
	name_hash(tmp);
	return tmp.get_raw_z();
}

uint64_t common::count_while_f(const char* p, bool(*judge)(char)) {
	if (!p)return 0;
	uint64_t c = 0;
	while (*p) {
		if (judge) {
			if (!judge(*p))return c;
		}
		c++;
	}
	return c;
}

uint64_t common::strtoull_ex(const char* str,char** p) {
	if (!str)return 0;
	int ofs = 0;
	int base = 10;
	if (str[0] == '0') {
		if (str[1] == 'x') {
			ofs = 2;
			base = 16;
		}
		else if (str[1]=='b') {
			ofs = 2;
			base = 2;
		}
		else if(ctype::is_octnumber(str[1])){
			ofs = 1;
			base = 8;
		}
	}
	return strtoull(&str[ofs], p, base);
}