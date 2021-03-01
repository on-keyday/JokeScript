/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"compiler_filereader.h"
#include"compiler_ctype.h"

using namespace PROJECT_NAME;

compiler::Reader::Reader(const char* filename,log::Log* logger) {
	this->logger = logger;
	if (!input.readall(filename))iseof = true;;
}

compiler::Reader::Reader(const char* base, uint64_t s) {
	if (base) {
		input.buf.add_copy(base, s);
	}
	else {
		iseof = true;
	}
	input.name.unuse();
}

bool compiler::Reader::expect(const char* symbol) {
	if (!ahead(symbol))return false;
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool compiler::Reader::expect_p1(const char* symbol, char judge) {
	if (!ahead(symbol))return false;
	if (input.buf[readpos+strlen(symbol)]==judge) {
		return false;
	}
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool compiler::Reader::expect_pf(const char* symbol, bool (*but)(char)) {
	if (!but||!ahead(symbol))return false;
	if (but(input.buf[readpos + strlen(symbol)])) {
		return false;
	}
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool compiler::Reader::expect_or_err(const char* symbol) {
	if (!expect(symbol)) {
		common::String s;
		const char* msg = "unexpected token. expected \"",
					*but="\", but \'";
		s.add_copy(msg,strlen(msg));
		s.add_copy(symbol,strlen(symbol));
		s.add_copy(but, strlen(but));
		s.add(input.buf[readpos]);
		s.add_copy("\'.",2);
		if(logger)logger->synerr(s.get_const());
		return false;
	}
	return true;
}

bool compiler::Reader::expect_or_err_pe(const char* symbol, const char* expected) {
	if (!expect(symbol)) {
		common::EasyVector<char> s;
		const char* msg = "unexpected token. expected \"",
			* but = "\", but \'";
		s.add_copy(msg, strlen(msg));
		s.add_copy(expected, strlen(symbol));
		s.add_copy(but, strlen(but));
		s.add(input.buf[readpos]);
		s.add_copy("\'.", 2);
		if (logger)logger->synerr(s.get_const());
		return false;
	}
	return true;
}

const char* compiler::Reader::prev() {
	return prev_expect;
}

bool compiler::Reader::ahead(const char* symbol) {
	if (!symbol)return false;
	common::String& str=this->input.buf;
	while (1) {
		if (str[readpos]==' '||str[readpos]=='\n'||str[readpos]=='\t'||str[readpos]=='\v'||str[readpos]=='\r') {
			readpos++;
			continue;
		}
		else if (str[readpos]=='/'&&str[readpos+1]=='*') {
			readpos += 2;
			auto to=strstr(&str.get_const()[readpos],"*/");
			if (!to) {
				iseof = true;
				return false;
			}
			readpos += (to - &str.get_const()[readpos])+2;
			continue;
		}
		else if (str[readpos]=='/'&&str[readpos+1]=='/') {
			auto to = strstr(&str.get_const()[readpos], "\n");
			if (!to) {
				iseof = true;
				return false;
			}
			readpos += (to - &str.get_const()[readpos]) + 1;
			continue;
		}
		else if (str[readpos]=='\0') {
			iseof = true;
			return false;
		}
		break;
	}
	return strncmp(&str.get_const()[readpos], symbol,strlen(symbol))==0;
}

common::String compiler::Reader::string(bool raw) {
	if (input.buf[readpos] != '\"' && input.buf[readpos] != '\'')return nullptr;
	common::String ret,&str=input.buf;
	char start = str[readpos];
	ret.add(start);
	readpos++;
	while (1) {
		if (!str[readpos]) {
			iseof = true;
			return nullptr;
		}
		else if (str[readpos]=='\\') {
			ret.add('\\');
			readpos++;
			if (!str[readpos]) {
				iseof = true;
				return nullptr;
			}
		}
		else if (ctype::is_escapable_control(str[readpos])&&str[readpos]!='\t') {
			if (!raw) {
				return nullptr;
			}
			ret.add('\\');
		}
		else if (str[readpos]==start) {
			ret.add(start);
			readpos++;
			break;
		}
		ret.add(str[readpos]);
		readpos++;
	}
	return ret;
}

bool compiler::Reader::readwhile(ReadStatus* status, bool (*judge)(const char*, ReadStatus*)) {
	if (!judge)return false;
	while (judge(&input.buf.get_const()[readpos], status)) {
		readpos++;
	}
	return true;
}

char compiler::Reader::abyte() {
	ahead("");
	auto ret = input.buf[readpos];
	return ret;
}

bool compiler::Reader::eof() {
	ahead("");
	return iseof;
}

bool compiler::Reader::seek(bool abs, long long pos) {
	return false;
}

char compiler::Reader::offset(long long ofs) {
	return input.buf[readpos+ofs];
}

uint64_t compiler::Reader::get_readpos() const {
	return readpos;
}

char compiler::Reader::get_const_char() const {
	return input.buf[readpos];
}


bool compiler::Reader::add_str(const char* str) {
	if (!str)return false;
	input.buf.add_copy(str, strlen(str));
	iseof = false;
	return true;
}