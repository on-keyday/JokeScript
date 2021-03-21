/*license*/
#pragma once
#include"structs.h"


namespace PROJECT_NAME {
	namespace common {
		namespace io_base {
			struct Input {
				String name;
				String buf;
				bool readall(const char* filename,const char* mode="r");
				bool readfromfp(FILE* fp);
			};

			struct OutPut {
				bool writeall(const char* filename,bool bin,const char* byte, uint64_t size);
				bool writetofp(FILE* fp, const char* byte, uint64_t size);
			};
		}
	}
}