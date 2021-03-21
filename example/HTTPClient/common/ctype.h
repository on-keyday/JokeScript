/*license*/
#pragma once
#include"filereader.h"

namespace PROJECT_NAME {
	namespace ctype{
		bool is_escapable_control(char c);
		bool is_first_of_identifier(char c);
		bool is_usable_for_identifier(char c);
		bool is_number(char c);
		bool is_hexnumber(char c);
		bool is_octnumber(char c);
		bool is_binnumber(char c);
		bool is_numbersuffix(char c);
		bool is_first_of_string(char c);
		bool is_utf8_later(char c);
		bool is_unnamed(const char* c);
		namespace reader {
			bool Number(const char* s,io::ReadStatus* status);
			bool Identifier(const char* s,io::ReadStatus* status);
			bool IdentifierPDot(const char* s, io::ReadStatus* status);
			bool DigitNumber(const char* s, io::ReadStatus* status);
			bool Until(const char* s, io::ReadStatus* status);
			bool End(const char* s, io::ReadStatus* status);
		}

		bool is_utf16_surrogate_high(char16_t c);
		bool is_utf16_surrogate_low(char16_t c);

		bool get_number_type(const char* num, uint64_t& numres,int& bit_size, bool& is_float, bool& is_unsigned);

		template<class Char>
		size_t strlen(const Char* str) {
			if (!str)return 0;
			size_t i = 0;
			for (; str[i]; i++);
			return i;
		}

		template<class Char>
		bool streaq(const Char* str1,const Char* str2) {
			if (strlen<Char>(str1) != strlen<Char>(str2))return false;
			if (!str1 || !str2)return true;
			size_t i = 0;
			while (str1[i]) {
				if (str1[i] != str2[i])return false;
				i++;
			}
			return true;
		}

		template<class Char>
		bool strneaq(const Char* str1, const Char* str2,size_t len) {
			if (!str1 || !str2)return false;
			for (size_t i = 0; i < len;i++) {
				if (str1[i] != str2[i])return false;
				else if (!str1[i] || !str2[i])return false;
			}
			return true;
		}

		//from network
		namespace uri {
			bool is_scheme_ok(char c);
			bool is_authority_ok(char c);
			bool is_path_ok(char c);
			bool is_query_ok(char c);
		}
	}
}