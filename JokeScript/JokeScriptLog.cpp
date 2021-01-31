/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptLog.h"

using namespace jokescript;

bool jokescript::AddJokeMsg(JokeLogger* log, const char* msg, JokeMsgType type) {
	return false;
}

bool jokescript::AddJokeInfo(JokeLogger* log, const char* msg, const char* value) {
	return false;
}

bool jokescript::AddJokeLog(JokeLogger* log, const char* msg, const char* value) {
	return false;
}

bool jokescript::AddJokeSysErr(JokeLogger* log, const char* msg, const char* value) {
	return false;
}

bool jokescript::AddJokeSynErr(JokeLogger* log, const char* msg, const char* value, unsigned long long line, unsigned long long pos) {
	return false;
}

bool jokescript::AddJokeSemErr(JokeLogger* log, const char* msg,const char* value, unsigned long long line, unsigned long long pos) {
	return false;
}

bool jokescript::AddJokeUnexpectedTokenErr(JokeLogger* log, const char* expected, char but, unsigned long long line, unsigned long long pos) {
	return false;
}

bool jokescript::AddJokeUnexpectedEOFErr(JokeLogger* log, unsigned long long line, unsigned long long pos) {
	return false;
}

bool jokescript::AddJokeMemoryFullErr(JokeLogger* log) {
	return false;
}

bool jokescript::AddJokeCompilerBrokenErr(JokeLogger* log, const char* msg) {
	return false;
}