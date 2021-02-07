/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptKeyword.h"
#include <string.h>

using namespace jokescript;

bool CCNV jokescript::IsJokeReserved(const char* name) {
	return false;
}


bool CCNV jokescript::IsJokeKeyWord(const char* name) {
	if (!name)return false;

	const char* keywords[] = {"if","else","loop","continue","break","first","import","joke","return","co"};
	for (auto key:keywords) {
		if (strcmp(name, key) == 0)return true;
	}
	return false;
}

bool CCNV jokescript::IsJokeFuncOpt(const char* name) {
	if (!name)return false;

	const char* opts[] = {"capt","ccnv","co","gen"};

	for (auto opt : opts) {
		if (strcmp(name, opt) == 0)return true;
	}
	return false;
}