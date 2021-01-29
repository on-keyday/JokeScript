/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#ifndef CCNV
#define CCNV __stdcall
#endif

namespace jokescript {
	bool CCNV IsJokeReserved(const char* name);
	bool CCNV IsJokeKeyWord(const char* name);
	bool CCNV IsJokeFuncOpt(const char* name);
}