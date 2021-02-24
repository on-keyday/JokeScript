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


compiler::SyntaxTree::~SyntaxTree() {
	free(symbol);
}

compiler::Type::~Type() {
	free(name);
}

compiler::Identifier::~Identifier() {
	free(name);
}

compiler::Option::~Option() {
	free(name);
	if (has_vec)ids.unuse();
}

compiler::IdHolder::IdHolder() {
	const char* base_t[] = {"void","bit_t(signed)","bit_t(unsigned)","bit_t(float)"};
	for (auto s : base_t) {
		auto type=make_type(common::StringFilter() = s);
		if (type) {
			type->type = TypeType::base_t;
		}
	}
}

compiler::IdHolder::~IdHolder() {
	trees.remove_each(common::kill);
	blocks.remove_each(common::kill);
	types.remove_each(common::kill);
	ids.remove_each(common::kill);
	options.remove_each(common::kill);
}

Block* compiler::IdHolder::make_block() {
	auto ret=common::create<Block>();
	if (!ret) {
		logger->syserr("memory is full");
		return nullptr;
	}
	blocks.add(ret);
	ret->prev = current;
	current = ret;
	return ret;
}

bool compiler::IdHolder::to_parent_block() {
	if (!current->prev)return false;
	current = current->prev;
	return true;
}

Block* compiler::IdHolder::get_current() {
	return current;
}

SyntaxTree* compiler::IdHolder::make_tree(char* symbol, TreeType ttype, Type* type) {
	if (!symbol) {
		logger->syserr("memory is full");
		return nullptr;
	}
	auto ret=common::create<SyntaxTree>();
	if (!ret) {
		logger->syserr("memory is full");
		free(symbol);
		return nullptr;
	}
	ret->symbol = symbol;
	ret->ttype = ttype;
	ret->type = type;
	trees.add(ret);
	return ret;
}


Type* compiler::IdHolder::make_type(char* symbol) {
	if (!symbol) {
		logger->syserr("memory is full");
		return nullptr;
	}
	auto ret = common::create<Type>();
	if (!ret) {
		logger->syserr("memory is full");
		free(symbol);
		return nullptr;
	}
	ret->name = symbol;
	types.add(ret);
	return ret;
}

Option* compiler::IdHolder::make_option(char* name, char* value,bool has_vec) {
	if (!name||(!value&&!has_vec)) {
		logger->syserr("memory is full");
		free(name);
		return nullptr;
	}
	auto ret = common::create<Option>();
	if (!ret) {
		logger->syserr("memory is full");
		free(name);
		free(value);
		return nullptr;
	}
	ret->name = name;
	ret->value = value;
	ret->has_vec = has_vec;
	options.add(ret);
	return ret;
}



Identifier* compiler::IdHolder::make_id(char* symbol) {
	if (!symbol) {
		logger->syserr("memory is full");
		return nullptr;
	}
	auto ret = common::create<Identifier>();
	if (!ret) {
		logger->syserr("memory is full");
		free(symbol);
		return nullptr;
	}
	ret->name = symbol;
	ids.add(ret);
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

Option* compiler::IdHolder::get_opt(const char* name, const char* value) {
	auto i = 0ull;
	while (options[i]) {
		if (strcmp(options[i]->name, name) == 0&&!options[i]->has_vec) {
			if (strcmp(options[i]->value,value)==0) {
				return options[i];
			}
		}
		i++;
	}
	return make_option(common::StringFilter()=name,common::StringFilter()=value,false);
}

Option* compiler::IdHolder::get_opt(const char* name, common::EasyVector<Identifier*>& ids) {
	auto i = 0ull;
	while (options[i]) {
		if (strcmp(options[i]->name, name) == 0 && options[i]->has_vec) {
			if (options[i]->ids==ids) {
				return options[i];
			}
		}
		i++;
	}
	auto ret= make_option(common::StringFilter() = name,nullptr, true);
	if (!ret)return nullptr;
	ret->ids = std::move(ids);
	return ret;
}

common::EasyVector<Type*>& compiler::IdHolder::get_types() {
	return types;
}

common::EasyVector<Identifier*>& compiler::IdHolder::get_ids() {
	return ids;
}

common::EasyVector<SyntaxTree*>& compiler::IdHolder::get_trees() {
	return trees;
}