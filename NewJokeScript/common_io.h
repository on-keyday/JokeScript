/*license*/
#pragma once
#include"common_structs.h"


namespace jokescript {
	namespace common {
		namespace io {
			struct Input {
				EasyVector<char> name;
				EasyVector<char> buf;
				bool readall(const char* filename);
				bool readfromfp(FILE* fp);
			};
		}
	}
}