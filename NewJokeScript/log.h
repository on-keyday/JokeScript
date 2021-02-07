/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"common_structs.h"
namespace PROJECT_NAME {
	namespace log {
		struct Log {
			bool syserr(const char* message);
			bool synerr(const char* message);
			bool symerr_val(const char* message,const char* val);
			bool unexpected_token(const char* expected,char but);
		};
	}
}