/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"log.h"
using namespace PROJECT_NAME;

bool log::Log::info(const char* message) {
	return false;
}

bool log::Log::syserr(const char* message) {
	return false;
}

bool log::Log::synerr(const char* message) {
	return false;
}

bool log::Log::synerr_val(const char* message, const char* val) {
	return false;
}


bool log::Log::semerr(const char* message) {
	return false;
}

bool log::Log::semerr_val(const char* message, const char* val) {
	return false;
}

bool log::Log::unexpected_token(const char* expected, char but) {
	"unexpected token. expected 'x' or 'X','b' or 'B',or octnumber";
	return false;
}

bool log::Log::unimplemented(const char* message) {
	return false;
}