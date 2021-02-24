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

bool common::Hash::name_hash(String& buf, const char* name) {
	buf.add_copy(name, strlen(name));
	auto num = std::to_string(hash_count);
	hash_count++;
	buf.add_copy(num.c_str(), num.length());
	buf.add('_');
	num = std::to_string(make_hash(buf.get_const()));
	buf.add_copy(num.c_str(), num.length());
	return true;
}

bool common::Hash::unname_hash(String& buf) {
	return name_hash(buf,"`unnamed_");
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
	common::String tmp;
	unname_hash(tmp);
	return tmp.get_raw_z();
}

char* common::Hash::get_hash_with_name(const char* s) {
	common::String tmp;
	name_hash(tmp, s);
	return tmp.get_raw_z();
}

const char* common::Hash::get_hash_const() {
	tmpc.init();
	unname_hash(tmpc);
	return tmpc.get_const();
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

common::String common::to_utf8(char32_t c) {
	if (c < 0 || c > 0x10FFFF) {
		return nullptr;
	}
	common::String ret;
	if (c < 128) {
		ret.add((char)c);
	}
	else if (c < 2048) {
		ret.add(0xC0 | char(c >> 6));
		ret.add(0x80 | (char(c & 0x3F)));
	}
	else if (c < 65536) {
		ret.add(0xE0 | char(c >> 12));
		ret.add(0x80 | (char(c >> 6) & 0x3F));
		ret.add(0x80 | (char(c) & 0x3F));
		
	}
	else {
		ret.add(0xF0 | char(c >> 18));
		ret.add(0x80 | (char(c >> 12) & 0x3F));
		ret.add(0x80 | (char(c >> 6) & 0x3F));
		ret.add(0x80 | (char(c) & 0x3F));
	}
	ret.pack_f();
	return ret;
}

common::String16 common::to_utf16(char32_t c) {
	if (c < 0 || c > 0x10FFFF) {
		return nullptr;
	}
	common::String16 ret;
	if (c < 0x10000) {
		ret.add(char16_t(c));
	}
	else {
		ret.add(char16_t((c - 0x10000) / 0x400 + 0xD800));
		ret.add(char16_t((c - 0x10000) % 0x400 + 0xDC00));
	}
	ret.pack_f();
	return ret;
}

char32_t common::to_utf32(const char* s,bool& suc) {
	suc = false;
	if (!s)return 0;
	int size = get_utf8bytesize(s[0]);
	if (!size)return 0;
	char32_t ret = 0;
	if (size == 1) {
		ret = (unsigned char)s[0];
	}
	else if (size==2) {
		if (!ctype::is_utf8_later(s[1])) {
			return 0;
		}
		if ((uint8_t(s[0]) & 0x1E) == 0) {
			return 0;
		}

		ret = char32_t(s[0] & 0x1F) << 6;
		ret |= char32_t(s[1] & 0x3F);
	}
	else if (size==3) {
		if (!ctype::is_utf8_later(s[1])||!ctype::is_utf8_later(s[2])) {
			return 0;
		}
		if ((uint8_t(s[0]) & 0x0F) == 0&&(uint8_t(s[1]) & 0x20) == 0) {
			return 0;
		}

		ret = char32_t(s[0] & 0x0F) << 12;
		ret |= char32_t(s[1] & 0x3F)<<6;
		ret |= char32_t(s[2] & 0x3F);
	}
	else if (size==4) {
		if (!ctype::is_utf8_later(s[1]) || !ctype::is_utf8_later(s[2])||!ctype::is_utf8_later(s[3])) {
			return 0;
		}
		if ((uint8_t(s[0]) & 0x07) == 0 && (uint8_t(s[1]) & 0x30) == 0) {
			return 0;
		}

		ret = char32_t(s[0] & 0x07) << 18;
		ret |= char32_t(s[1] & 0x3F) << 12;
		ret |= char32_t(s[2] & 0x3F) << 6;
		ret |= char32_t(s[3] & 0x3F);
	}
	suc = true;
	return ret;
}

char32_t common::to_utf32(const char16_t* c, bool& suc) {
	suc = false;
	if (!c)return 0;
	char32_t ret = 0;
	if (ctype::is_utf16_surrogate_high(c[0])) {
		if (ctype::is_utf16_surrogate_low(c[1])) {
			ret = 0x10000 + (char32_t(c[0]) - 0xD800) * 0x400 +
				(char32_t(c[1]) - 0xDC00);
		}
		else if (c[1] == 0) {
			ret = c[0];
		}
		else {
			return false;
		}
	}
	else if (ctype::is_utf16_surrogate_low(c[0])) {
		if (c[1] == 0) {
			ret = c[0];
		}
		else {
			return false;
		}
	}
	else {
		ret = c[0];
	}
	return ret;
}

int common::get_utf8bytesize(char c) {
	if (0 <= uint8_t(c) && uint8_t(c) < 0x80) {
		return 1;
	}
	if (0xC2 <= uint8_t(c) && uint8_t(c) < 0xE0) {
		return 2;
	}
	if (0xE0 <= uint8_t(c) && uint8_t(c) < 0xF0) {
		return 3;
	}
	if (0xF0 <= uint8_t(c) && uint8_t(c) < 0xF8) {
		return 4;
	}
	return 0;
}

