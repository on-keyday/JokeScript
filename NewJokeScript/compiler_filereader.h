/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"common_io.h"
#include"log.h"

namespace jokescript {
	namespace compiler {
		struct ReadStatus {
			log::Log* logger;
			common::EasyVector<char> buf;
			const char* begin=nullptr;
			unsigned int num=0;
			bool flag1 = false;
			bool flag2 = false;
			bool flag3 = false;
			bool flag4 = false;
			bool flag5 = false;
			bool failed = false;
		};

		struct Reader {
		private:
			common::io::Input input;
			log::Log* logger=nullptr;
			unsigned long long readpos = 0;
			bool iseof = false;
			const char* prev_expect = nullptr;
		public:
			Reader()=delete;
			Reader(const char* filename);
			bool expect(const char* symbol);
			bool expect_p1(const char* symbol,char not);
			bool expect_pf(const char* symbol,bool (*judge)(char));
			bool expect_or_err(const char* symbol);
			bool expect_or_err_pe(const char* symbol,const char* expected);
			const char* prev();
			bool ahead(const char* symbol);
			common::EasyVector<char> string();
			bool readwhile(ReadStatus* status,bool (*judge)(const char*,ReadStatus*));
			char abyte();
			bool eof();
			bool seek(bool abs,long long pos);
			char offset(long long ofs);
		};
	}
}