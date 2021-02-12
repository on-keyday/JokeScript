/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"stdcpps.h"
#include"compiler_identifier_holder.h"
#include"compiler_id_analyzer.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;

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

Type* compiler::IdHolder::get_float_bit_t() {
	return types[3];
}

Type* compiler::IdHolder::get_bit_t(bool signeds) {
	if (signeds) {
		return types[1];
	}
	else {
		return types[2];
	}
}

Type* compiler::IdHolder::get_void() {
	return types[0];
}

Type* compiler::IdHolder::get_string() {
	return get_derived(TypeType::has_size_t, 0, get_derived(TypeType::has_size_t, 8, get_bit_t(false), this), this);
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