/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"filereader.h"
#include"ctype.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::io;


Reader::Reader(const char* filename,bool is_bin,log::Log* logger,IgnoreHandler handler) {
	this->logger = logger;
	this->ignore_handler = handler;
	const char* mode = "r";
	if (is_bin)mode = "rb";
	if (!input.readall(filename,mode))iseof = true;
}

Reader::Reader(uint64_t s,const char* base,log::Log* logger, IgnoreHandler handler) {
	this->logger = logger;
	this->ignore_handler = handler;
	if (base) {
		input.buf.add_copy(base, s);
	}
	else {
		iseof = true;
	}
	input.name.unuse();
}

Reader::Reader(common::String& buf, log::Log* logger, IgnoreHandler handler) {
	this->logger = logger;
	this->ignore_handler = handler;
	input.buf = std::move(buf);
	if (!input.buf.get_size())iseof = true;
	input.name.unuse();
}

bool Reader::ignore() {
	if (!ignore_handler)return ignore_default();
	auto& buf = input.buf;
	auto res=ignore_handler(buf, readpos);
	if (!res)iseof = true;
	return res;
}

bool Reader::ignore_default() {
	common::String& str = this->input.buf;
	while (1) {
		if (str[readpos] == ' ' || str[readpos] == '\n' || str[readpos] == '\t' || str[readpos] == '\v' || str[readpos] == '\r') {
			readpos++;
			continue;
		}
		else if (str[readpos] == '/' && str[readpos + 1] == '*') {
			readpos += 2;
			auto to = strstr(&str.get_const()[readpos], "*/");
			if (!to) {
				iseof = true;
				return false;
			}
			readpos += (to - &str.get_const()[readpos]) + 2;
			continue;
		}
		else if (str[readpos] == '/' && str[readpos + 1] == '/') {
			auto to = strstr(&str.get_const()[readpos], "\n");
			if (!to) {
				iseof = true;
				return false;
			}
			readpos += (to - &str.get_const()[readpos]) + 1;
			continue;
		}
		else if (str[readpos] == '\0') {
			iseof = true;
			return false;
		}
		break;
	}
	return true;
}

bool Reader::expect(const char* symbol) {
	if (!ahead(symbol))return false;
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool Reader::expect_p1(const char* symbol, char judge) {
	if (!ahead(symbol))return false;
	if (input.buf[readpos+strlen(symbol)]==judge) {
		return false;
	}
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool Reader::expect_pf(const char* symbol, bool (*but)(char)) {
	if (!but||!ahead(symbol))return false;
	if (but(input.buf[readpos + strlen(symbol)])) {
		return false;
	}
	readpos += strlen(symbol);
	prev_expect = symbol;
	return true;
}

bool Reader::expect_or_err(const char* symbol) {
	if (!expect(symbol)) {
		if (logger) {
			common::String s;
			const char* msg = "unexpected token. expected \"",
				* but = "\", but \'";
			s.add_copy(msg, strlen(msg));
			s.add_copy(symbol, strlen(symbol));
			s.add_copy(but, strlen(but));
			s.add(input.buf[readpos]);
			s.add_copy("\'.", 2);
			logger->synerr(s.get_const());
		}
		return false;
	}
	return true;
}

bool Reader::expect_or_err_pe(const char* symbol, const char* expected) {
	if (!expect(symbol)) {
		if (logger) {
			common::String s;
			const char* msg = "unexpected token. expected \"",
				* but = "\", but \'";
			s.add_copy(msg, strlen(msg));
			s.add_copy(expected, strlen(symbol));
			s.add_copy(but, strlen(but));
			s.add(input.buf[readpos]);
			s.add_copy("\'.", 2);
			logger->synerr(s.get_const());
		}
		return false;
	}
	return true;
}

const char* Reader::prev() {
	return prev_expect;
}

bool Reader::ahead(const char* symbol) {
	if (!symbol)return false;
	if (!ignore())return false;
	return ctype::strneaq(&input.buf.get_const()[readpos], symbol,strlen(symbol));
}

common::String Reader::string(bool raw) {
	if (input.buf[readpos] != '\"' && input.buf[readpos] != '\'')return nullptr;
	common::String ret,& str = input.buf;
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

bool Reader::readwhile(ReadStatus* status, bool (*judge)(const char*, ReadStatus*)) {
	if (!judge)return false;
	while (judge(&input.buf.get_const()[readpos], status)) {
		readpos++;
	}
	return true;
}

char Reader::abyte() {
	ignore();
	return input.buf[readpos];
}

bool Reader::eof() {
	ignore();
	return iseof;
}

bool Reader::seek(uint64_t pos) {
	if (pos > input.buf.get_size())return false;
	readpos = pos;
	if(input.buf.get_size()!=pos)iseof = false;
	return true;
}

bool Reader::block(const char* start, const char* end) {
	if (!end||!expect(start))return false;
	uint64_t dp = 0;
	auto& str = input.buf;
	while (!eof()) {
		ignore();
		if (eof())return false;
		if (str[readpos]==start[0]) {
			if (expect(start)) {
				dp++;
			}
		}
		else if (str[readpos]==end[0]) {
			if (expect(end)) {
				if (dp == 0)break;
				dp--;
			}
		}
		readpos++;
	}
	return true;
}

char Reader::offset(long long ofs) {
	return input.buf[readpos+ofs];
}

uint64_t Reader::get_readpos() const {
	return readpos;
}

char Reader::get_const_char() const {
	return input.buf[readpos];
}


bool Reader::add_str(const char* str) {
	if (!str)return false;
	auto len = ctype::strlen(str);
	if (!len)return false;
	input.buf.add_copy(str, len);
	iseof = false;
	return true;
}

bool Reader::add(const char* buf, uint64_t size) {
	if (!buf||!size)return false;
	input.buf.add_copy(buf, size);
	iseof = false;
	return true;
}

Reader::IgnoreHandler Reader::set_ignore(IgnoreHandler handler) {
	auto ret = ignore_handler;
	ignore_handler = handler;
	return ret;
}

bool io::not_ignore(common::String& buf, uint64_t& readpos) {
	if (buf.get_size() <= readpos)return false;
	return true;
}