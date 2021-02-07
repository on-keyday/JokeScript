/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"compiler_filereader.h"

namespace jokescript {
	namespace ctype{
		bool is_first_of_identifier(char c);
		bool is_usable_for_identifier(char c);
		bool is_number(char c);
		bool is_hexnumber(char c);
		bool is_octnumber(char c);
		bool is_binnumber(char c);
		bool is_numbersuffix(char c);
		bool is_first_of_string(char c);
		bool is_unnamed(const char* c);
		namespace reader {
			bool Number(const char* s,compiler::ReadStatus* status);
			bool Identifier(const char* s,compiler::ReadStatus* status);
		}
	}
}