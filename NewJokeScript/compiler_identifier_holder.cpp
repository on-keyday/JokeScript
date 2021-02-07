/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"stdcpps.h"
#include"compiler_identifier_holder.h"

using namespace jokescript;
using namespace jokescript::compiler;

Block* compiler::IdHolder::make_block() {
	return create<Block>();
}

Block* compiler::IdHolder::get_current() {
	return current;
}

SyntaxTree* compiler::IdHolder::make_tree(char* symbol) {
	if (!symbol) {
		logger->syserr("memory is full");
		return nullptr;
	}
	auto ret=create<SyntaxTree>();
	if (!ret) {
		logger->syserr("memory is full");
		free(symbol);
		return nullptr;
	}
	ret->symbol = symbol;
	return ret;
}


Type* compiler::IdHolder::make_type(char* symbol) {
	if (!symbol) {
		logger->syserr("memory is full");
		return nullptr;
	}
	auto ret = create<Type>();
	if (!ret) {
		logger->syserr("memory is full");
		free(symbol);
		return nullptr;
	}
	ret->name = symbol;
	return ret;
}

Type* compiler::IdHolder::get_bit_t(bool signeds) {
	if (signeds) {
		return types[0];
	}
	else {
		return types[1];
	}
}

ReadStatus* compiler::IdHolder::get_status() {
	status.begin = nullptr;
	status.buf.init();
	status.failed = false;
	status.num = 0;
	status.flag1 = false;
	status.flag2 = false;
	status.flag3 = false;
	status.flag4 = false;
	status.flag5 = false;
	status.logger = this->logger;
	return &status;
}