/*license*/
#include"common_tools.h"
#include"stdcpps.h"

using namespace jokescript;

bool common::tools::Hash::name_hash(EasyVector<char>& buf) {
	buf.add_copy("%unnamed_",0);
	auto num=std::to_string(unnamed_count);
	unnamed_count++;
	buf.add_copy(num.c_str(), num.length());
	buf.add('_');
	num = std::to_string(make_hash(buf.get_const()));
	buf.add_copy(num.c_str(), num.length());
	return true;
}

uint64_t common::tools::Hash::make_hash(const char* str) {
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