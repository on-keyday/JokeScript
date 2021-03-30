/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"structs.h"
namespace PROJECT_NAME {
	namespace log {
		struct Log {
			bool info(const char* message);
			bool syserr(const char* message);
			bool synerr(const char* message);
			bool synerr_val(const char* message, const char* val);
			bool semerr(const char* message);
			bool semerr_val(const char* message,const char* val);
			bool unexpected_token(const char* expected,char but);
			bool unimplemented(const char* message);
			bool memoryfull();
		};
	}
}