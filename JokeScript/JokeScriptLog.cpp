/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptLog.h"
#include"JokeScriptLoader.h"
#include <string.h>
#include<string>

using namespace jokescript;

bool jokescript::AddJokeMsg(JokeLogger* log, const char* msg, JokeMsgType type) {
	if (!log || !msg)return false;
	bool res = true;
	JokeMessage* add_msg = nullptr;
	try {
		add_msg = new JokeMessage;
	}
	catch (...) {
		res = false;
	}
	if (add_msg) {
		add_msg->msg = StringFilter() = msg;
		if (!add_msg->msg) {
			res = false;
			delete add_msg;
		}
		else {
			add_msg->type = type;
			log->msgs.add(add_msg);
		}
	}
	if (log->cb) {
		log->cb(msg);
	}
	return res;
}

bool jokescript::CommonAdd(EasyVector<char>& to, const char* type, const char* msg, const char* value) {
	to.add_copy(type, strlen(type));
	bool ved = false;
	auto i = 0ull;
	while (msg[i]) {
		if (value&&!ved && msg[i] == '*' && (!i || msg[i - 1] != '\\')) {
			to.add_copy(value, strlen(value));
			ved = true;
		}
		else if (msg[i]=='\t') {
			to.add_copy("\\t",2);
		}
		else {
			to.add(msg[i]);
		}
		i++;
	}
	return true;
}

bool jokescript::ResolveFileLine(EasyVector<char>& to, JokeLogger* log, unsigned long long line, unsigned long long pos) {
	auto r_line=log->file->logtoreal_line[line];
	auto r_pos = log->file->logtoreal_pos[line]+pos;
	if (!r_line) {
		r_line = log->file->logtoreal_line.get_end();
		r_pos = log->file->logtoreal_pos.get_end();
	}
	/*else {
		auto r_nowline = log->file->lines[r_line];
		auto l_nowline = log->file->loglines[line];
		auto i = 0ull;
		while (i<pos) {
			if (r_nowline[r_pos]!=l_nowline[i]) {
				if (r_nowline[r_pos] == '\0') {
					r_line++;
					r_nowline = log->file->lines[r_line];
					if (!r_nowline) {
						break;
					}
				}
				else if () {

				}
			}
			else {
				r_pos++;
				i++;
			}
		}
	}*/

	std::string hold1=std::to_string(r_line),hold2=std::to_string(r_pos);
	
	to.add('(');
	to.add_copy(hold1.c_str(), hold1.length());
	to.add(',');
	to.add_copy(hold2.c_str(), hold2.length());
	to.add(')');
	return true;
}

bool jokescript::AddJokeInfo(JokeLogger* log, const char* msg, const char* value) {
	if (!log || !msg)return false;
	const char* info = "Info: ";
	EasyVector<char> to;
	CommonAdd(to, info, msg, value);
	to.add('\n');
	return AddJokeMsg(log,to.get_const(),JokeMsgType::info);
}

bool jokescript::AddJokeLog(JokeLogger* log, const char* msg, const char* value) {
	return false;
}

bool jokescript::AddJokeSysErr(JokeLogger* log, const char* msg, const char* value) {
	if (!log || !msg)return false;
	const char* info = "System Error: ";
	EasyVector<char> to;
	CommonAdd(to, info, msg, value);
	to.add('\n');
	const char* give = to.get_const();
	if (!give) {
		give = msg;
	}
	return AddJokeMsg(log, give, JokeMsgType::system_err);
}

bool jokescript::AddJokeSynErr(JokeLogger* log, const char* msg, const char* value, unsigned long long line, unsigned long long pos) {
	if (!log || !msg)return false;
	const char* info = "Syntax Error: ";
	EasyVector<char> to;
	CommonAdd(to, info, msg, value);
	ResolveFileLine(to, log, line, pos);
	to.add('\n');
	return AddJokeMsg(log,to.get_const(), JokeMsgType::syntax_err);
}

bool jokescript::AddJokeSemErr(JokeLogger* log, const char* msg,const char* value, unsigned long long line, unsigned long long pos) {
	if (!log || !msg)return false;
	const char* info = "Semantcs Error: ";
	EasyVector<char> to;
	CommonAdd(to, info, msg, value);
	ResolveFileLine(to, log, line, pos);
	to.add('\n');
	return AddJokeMsg(log, to.get_const(), JokeMsgType::semantics_err);
}

bool jokescript::AddJokeUnexpectedTokenErr(JokeLogger* log, const char* expected, char but, unsigned long long line, unsigned long long pos) {
	if (!log || !expected)return false;
	const char* base = "unexpected token. expected ";
	EasyVector<char> to;
	to.add_copy(base, strlen(base));
	auto i = 0ull;
	if (strlen(expected) <= 5) {
		while (expected[i]) {
			if (i != 0 && expected[i + 1] == '\0') {
				to.add_copy("or ", 3);
			}
			to.add('\'');
			to.add(expected[i]);
			to.add('\'');
			to.add(',');
			to.add(' ');
			i++;
		}
	}
	else {
		to.add('\"');
		to.add_copy(expected, strlen(expected));
		to.add_copy("\", ", 3);
	}
	to.add_copy("but \'", 5);
	to.add(but);
	to.add_copy("\'.", 2);
	return AddJokeSynErr(log,to.get_const(),nullptr,line,pos);
}

bool jokescript::AddJokeUnexpectedEOFErr(JokeLogger* log, unsigned long long line, unsigned long long pos) {
	return AddJokeSynErr(log,"unexpected end of file.",nullptr,line,pos);
}

bool jokescript::AddJokeMemoryFullErr(JokeLogger* log) {
	return AddJokeSysErr(log,"memory is full.",nullptr);
}

bool jokescript::AddJokeCompilerBrokenErr(JokeLogger* log, const char* msg) {
	EasyVector<char> to;
	const char* broken = "this compiler is broken.";
	to.add_copy(msg, strlen(msg));
	to.add(' ');
	to.add_copy(broken, strlen(broken));
	return false;
}