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
		namespace io {
			struct Input {
				String name;
				String buf;
				bool readall(const char* filename);
				bool readfromfp(FILE* fp);
			};

			struct OutPut {
				bool writeall(const char* filename,bool bin,const char* byte, uint64_t size);
				bool writetofp(FILE* fp, const char* byte, uint64_t size);
			};
		}
	}
}