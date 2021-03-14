/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"structs.h"
#include"io.h"
#include"log.h"

namespace PROJECT_NAME {
	namespace io {
		struct ReadStatus {
			log::Log* logger;
			common::String buf;
			const char* begin=nullptr;
			unsigned int num=0;
			bool flag1 = false;
			bool flag2 = false;
			bool flag3 = false;
			bool flag4 = false;
			bool flag5 = false;
			bool failed = false;
			//Type* option = nullptr;
		};

		struct Reader {
		private:
			common::io_base::Input input;
			log::Log* logger=nullptr;
			uint64_t readpos = 0;
			bool iseof = false;
			const char* prev_expect = nullptr;
			typedef bool(*IgnoreHandler)(common::String& buf, uint64_t& readpos);
			IgnoreHandler ignore_handler;
			bool ignore();
			bool ignore_default();
		public:
			Reader()=delete;
			Reader(const char* filename,bool is_bin=false,log::Log* logger=nullptr);
			Reader(const char* base,uint64_t s,log::Log* logger=nullptr);
			bool expect(const char* symbol);
			bool expect_p1(const char* symbol,char judge);
			bool expect_pf(const char* symbol,bool (*judge)(char));
			bool expect_or_err(const char* symbol);
			bool expect_or_err_pe(const char* symbol,const char* expected);
			const char* prev();
			bool ahead(const char* symbol);
			common::String string(bool raw);
			bool readwhile(ReadStatus* status,bool (*judge)(const char*,ReadStatus*));
			char abyte();
			bool eof();
			bool seek(uint64_t pos);
			bool block(const char* start,const char* end);
			char offset(long long ofs);
			uint64_t get_readpos() const;
			char get_const_char() const;
			bool add_str(const char* str);
			IgnoreHandler set_ignore(IgnoreHandler handler);
		};

		bool not_ignore(common::String& buf, uint64_t& readpos);
	}
}