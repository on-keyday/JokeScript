/*license*/
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
		};

		template<class T>
		T translate_byte_as_is(const char* s) {
			T res = 0;
			char* res_p = (char*)&res;
			for (auto i = 0u; i < sizeof(T); i++) {
				res_p[i] = s[i];
			}
			return res;
		}

		template<class T>
		T translate_byte_reverse(const char* s) {
			T res = 0;
			char* res_p = (char*)&res;
			auto k = 0ull;
			for (auto i = sizeof(T)-1;;i--) {
				res_p[i] = s[k];
				if (i == 0)break;
				k++;
			}
			return res;
		}

		struct Reader {
		private:
			common::io_base::Input input;
			log::Log* logger=nullptr;
			size_t readpos = 0;
			bool iseof = false;
			const char* prev_expect = nullptr;
			typedef bool(*IgnoreHandler)(common::String& buf, size_t& readpos);
			IgnoreHandler ignore_handler;
			bool ignore();
			bool ignore_default();
		public:
			Reader()=delete;
			Reader(Reader&) = delete;
			Reader(Reader&&) = delete;
			Reader(const char* filename,bool is_bin=false,log::Log* logger=nullptr,IgnoreHandler handler=nullptr);
			Reader(size_t size,const char* base,log::Log* logger=nullptr,IgnoreHandler handler = nullptr);
			Reader(common::String& buf, log::Log* logger = nullptr, IgnoreHandler handler = nullptr);
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
			bool seek(size_t pos);
			bool block(const char* start,const char* end);
			char offset(long long ofs);
			size_t get_readpos() const;
			char get_const_char() const;
			bool add_str(const char* str);
			bool add(const char* buf,size_t size);
			IgnoreHandler set_ignore(IgnoreHandler handler);
			template<class T>
			size_t read_byte(T* res=nullptr,T (*endian_handler)(const char*)=translate_byte_as_is,size_t size=sizeof(T)) {
				if (size % sizeof(T) != 0)return 0;
				size_t pos = 0;
				while (!eof()&&input.buf.get_size()-readpos>=sizeof(T)&&pos < size / sizeof(T)) {
					if (res) {
						if (!endian_handler)return false;
						auto s = input.buf.get_const();
						res[pos] = endian_handler(&s[readpos]);
					}
					readpos += sizeof(T);
					pos++;
				}
				return pos;
			}
			common::String& buf_ref() { return input.buf; }
			bool release_eof();
			size_t readable_size();
		};

		bool not_ignore(common::String& buf, size_t& readpos);
		bool ignore_space(common::String& buf, size_t& readpos);
		bool ignore_space_and_line(common::String& buf, size_t& readpos);
	}
}