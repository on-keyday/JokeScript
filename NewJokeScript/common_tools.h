/*license*/
#pragma once
#include"common_structs.h"

namespace jokescript {
	namespace common {
		namespace tools {
			struct Hash {
			private:
				uint64_t unnamed_count = 0;
			public:
				bool name_hash(EasyVector<char>& buf);
				uint64_t make_hash(const char* str);
			};
		}
	}
}