/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"identifier.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;

io::ReadStatus* Maker::get_read_status() {
	status.logger = logger;
	status.num = 0;
	status.begin = nullptr;
	status.buf.clear();
	status.failed = false;
	status.flag1 = false;
	status.flag2 = false;
	status.flag3 = false;
	status.flag4 = false;
	status.flag5 = false;
	return &status;
}

Type* Maker::make_type(char* name, TypeKind kind, bool name_must,bool hold) {
	if (name_must&&!name) {
		logger->memoryfull();
		return nullptr;
	}
	auto ret = common::create<Type>();
	if (!ret) {
		common::free(name);
		logger->memoryfull();
		return nullptr;
	}
	ret->name = name;
	ret->kind = kind;
	if (hold)types.add(ret);
	return ret;
}

Type* Maker::get_derived(Type* base, uint64_t size,TypeKind kind) {
	if (!base)return nullptr;
	if (!is_derived(kind))return nullptr;
	for (auto it:base->derived) {
		if (it->kind==kind) {
			if (kind == TypeKind::has_size_t) {
				if (it->size==size) {
					return it;
				}
			}
			else {
				return it;
			}
		}
	}	
	auto ret = make_type(nullptr, kind, false);
	if (!ret)return nullptr;
	ret->base.type = base;
	if (kind == TypeKind::has_size_t)ret->size = size;
	return ret;
}

Type* Maker::get_function(Type* rettype, common::EasyVectorP<Type*>& params,common::EasyVectorP<identifier::TypeOption*>& opts) {
	if (!rettype)return nullptr;
	for (auto it : rettype->function) {
		if (it->params == params && it->options == opts)return it;
	}
	auto ret = make_type(nullptr,TypeKind::function_t,false);
	if (!ret)return nullptr;
	ret->params = std::move(params);
	ret->options = std::move(opts);
	ret->base.type = rettype;
	return ret;
}

Type* Maker::get_void() {
	if (!void_t) {
		void_t = make_type(nullptr, TypeKind::void_t, false);
	}
	return void_t;
}

Type* Maker::get_bit_t(bool is_unsigned, bool is_float) {
	if (is_float) {
		if (!bit_t_float) {
			bit_t_float = make_type(nullptr, TypeKind::bit_t, false,false);
		}
		return bit_t_float;
	}
	else if(is_unsigned){
		if (!bit_t_unsigned) {
			bit_t_unsigned = make_type(nullptr, TypeKind::bit_t, false,false);
		}
		return bit_t_unsigned;
	}
	else {
		if (!bit_t_signed) {
			bit_t_signed = make_type(nullptr, TypeKind::bit_t, false,false);
		}
		return bit_t_signed;
	}
}

Type* Maker::get_bool() {
	return get_derived(get_bit_t(true), 1, TypeKind::has_size_t);
}

Variable* Maker::make_variable(char* name) {
	if(!name) {
		logger->memoryfull();
		return nullptr;
	}
	auto ret = common::create<Variable>();
	if (!ret) {
		common::free(name);
		logger->memoryfull();
		return nullptr;
	}
	ret->name = name;
	vars.add(ret);
	return ret;
}

SyntaxTree* Maker::make_tree(char* symbol,TreeKind kind) {
	if (!symbol) {
		logger->memoryfull();
		return nullptr;
	}
	auto ret = common::create<SyntaxTree>();
	if (!ret) {
		common::free(symbol);
		logger->memoryfull();
		return nullptr;
	}
	ret->symbol = symbol;
	ret->kind = kind;
	return ret;
}