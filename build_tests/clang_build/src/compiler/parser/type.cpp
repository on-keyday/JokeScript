/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"type.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;
using namespace PROJECT_NAME::io;
using namespace PROJECT_NAME::type;

bool type::parse_type_set(Reader* reader, Maker* maker) {
	if (!reader->expect_or_err("type"))return false;
	const char* check = nullptr;
	if (reader->expect("{")) {
		check = "}";
	}
	else if (reader->expect("(")) {
		check = ")";
	}
	common::EasyVectorP<Type*> types;
	while (!reader->eof()) {
		bool ok = false;
		while (!reader->eof()) {
			auto s = maker->get_read_status();
			reader->readwhile(s, ctype::reader::Identifier);
			if (s->failed)return false;
			if (ctype::is_unnamed(s->buf.get_const())) {
				maker->logger->semerr("on this context, unnamed type is not definable.");
				return false;
			}
			auto hold = maker->make_type(s->buf.get_raw_z(), TypeKind::named_t);
			if (!hold)return false;
			if (!reader->expect(",")) {
				auto type = parse_type(reader, maker);
				if (!type)return false;
				hold->base.type = type;
				if (!maker->add_member(hold))return false;
				while (auto p = types.remove_end()) {
					p->base.type = type;
					if (!maker->add_member(hold))return false;
				}
				ok = true;
				break;
			}
			else {
				types.add(hold);
			}
		}
		if (!ok) {
			maker->logger->synerr("unexpected end of file.");
			return false;
		}
		if (check) {
			if (!reader->expect(check)) {
				continue;
			}
		}
		break;
	}
	return true;
}

Type* type::parse_type(Reader* reader, Maker* maker) {
	if (reader->expect("*")) {
		return get_derived_common(reader, maker, TypeKind::pointer_t);
	}
	else if (reader->expect("[")) {
		return get_array(reader, maker);
	}
	else if (reader->expect("(")) {
		return get_func(reader, maker);
	}
	else if (reader->expect_pf("void", ctype::is_usable_for_identifier)) {
		return maker->get_void();
	}
	else if (reader->expect_pf("bit_t", ctype::is_usable_for_identifier)) {
		return get_bit_type(reader, maker);
	}
	else if (reader->expect_pf("struct", ctype::is_usable_for_identifier)) {
		return get_struct(reader, maker);
	}
	else {
		return get_named(reader, maker);
	}
}

Type* type::get_array(Reader* reader, Maker* maker) {
	uint64_t size = 0;
	if (!reader->ahead("]")) {
		auto s = maker->get_read_status();
		reader->readwhile(s, ctype::reader::Number);
		if (s->failed)return nullptr;
		int bitsize = 0;
		bool unsignedf = false, floatf = false;
		if (!ctype::get_number_type(s->buf.get_const(), size, bitsize, floatf, unsignedf)) {
			if (size == 0) {
				maker->logger->synerr("unknown number like.");
			}
			else {
				maker->logger->synerr("too large number that not compileable.");
			}
			return nullptr;
		}
		if (floatf) {
			maker->logger->synerr("on this context,float value is not usable.");
		}
	}
	if (!reader->expect_or_err("]"))return nullptr;
	return get_derived_common(reader, maker, TypeKind::has_size_t, size);
}

Type* type::get_derived_common(Reader* reader, Maker* maker, TypeKind kind, uint64_t size) {
	auto base = parse_type(reader, maker);
	if (!base)return nullptr;
	return maker->get_derived(base, size, kind);
}

Type* type::get_func(Reader* reader, Maker* maker) {
	common::EasyVectorP<Type*> params;
	common::EasyVectorP<TypeOption*> opts;
	while (!reader->eof()) {
		if (reader->ahead(")"))break;
		if (ctype::is_first_of_identifier(reader->abyte())) {
			bool ok = false;
			auto nowpos = reader->get_readpos();
			for (auto i = 1ull; reader->offset(i); i++) {
				if (!ctype::is_usable_for_identifier(reader->offset(i))) {
					reader->seek(nowpos + i);
					if (reader->expect(",")) {
						reader->seek(nowpos);
					}
					ok = true;
					break;
				}
			}
			if (!ok) {
				maker->logger->synerr("unexpected end of file.");
				return nullptr;
			}

		}
		auto param = parse_type(reader, maker);
		if (!param)return nullptr;
		params.add(param);
	}
	if (!reader->expect_or_err(")"))return nullptr;
	Type* rettype = nullptr;
	if (reader->expect("->")) {
		rettype = parse_type(reader, maker);
	}
	else {
		rettype = maker->get_void();
	}
	if (!rettype)return nullptr;
	return maker->get_function(rettype, params, opts);
}

Type* type::get_struct(Reader* reader, Maker* maker) {
	if (reader->expect_or_err("{"))return nullptr;
	auto ret = maker->make_type(nullptr, TypeKind::struct_t, false);
	if (!ret)return nullptr;
	ScopeStack stack(ret->members);
	stack.prev = maker->scope->prev;
	maker->scope = &stack;
	while (!reader->eof()) {
		if (reader->ahead("}")) {
			break;
		}
		else if (reader->ahead("type")) {
			if (!parse_type_set(reader, maker))return nullptr;
		}
		else {
			
		}
	}
	maker->scope = maker->scope->prev;
	return ret;
}

Type* type::get_named(Reader* reader, Maker* maker) {
	auto s = maker->get_read_status();
	reader->readwhile(s, ctype::reader::Identifier);
	if (s->failed)return nullptr;
	bool is_var = false;
	Type* ret = nullptr, * type = nullptr;
	Variable* var = nullptr;
	search_name(s->buf.get_const(), maker, &type, &var);
	if (type) {
		ret = type;
	}
	else if (var) {
		ret = var->type;
		is_var = true;
	}
	else {
		maker->logger->semerr_val("name * is not found as type or variable.", s->buf.get_const());
		return nullptr;
	}
	if (!ret)return nullptr;
	if (ret->kind == TypeKind::alias_t) {
		ret = ret->base.type;
	}
	if (reader->expect(".")) {
		ScopeStack* prev = maker->scope;
		while (!reader->eof()) {
			s = maker->get_read_status();
			reader->abyte();
			reader->readwhile(s, ctype::reader::Identifier);
			if (s->failed)return nullptr;
			ScopeStack stack(ret->members);
			maker->scope = &stack;
			search_name(s->buf.get_const(), maker, &type, &var);
			if (type) {
				ret = type;
				is_var = false;
			}
			else if (var) {
				ret = var->type;
				is_var = true;
			}
			else {
				maker->logger->semerr_val("name '*' is not found as type or variable.", s->buf.get_const());
				return nullptr;
			}
			if (ret->kind == TypeKind::alias_t) {
				ret = ret->base.type;
			}
			if (!reader->expect("."))break;
		}
		maker->scope = prev;
	}
	if (is_var) {
		maker->logger->semerr_val("name * is not type, is variable.", s->buf.get_const());
		return nullptr;
	}
	return ret;
}

Type* type::get_bit_type(Reader* reader, Maker* maker) {
	if (reader->expect("(")) {
		bool is_unsigned = false, is_float = false;
		if (reader->expect("unsigned")) {
			is_unsigned = true;
		}
		else if (reader->expect("float")) {
			is_float = true;
		}
		else if (!reader->expect_or_err_pe("signed", "signed, unsigned, or float")) {
			return nullptr;
		}
		if (!reader->expect_or_err(")"))return nullptr;
		return maker->get_bit_t(is_unsigned, is_float);
	}
	return maker->get_bit_t();
}

