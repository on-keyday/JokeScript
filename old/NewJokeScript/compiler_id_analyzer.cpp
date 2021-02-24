/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"stdcpps.h"
#include"compiler_id_analyzer.h"
#include"compiler_identifier_holder.h"
#include"compiler_ctype.h"
#include"common_tools.h"
#include"compiler_syntax_analyzer.h"


using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;

Type* compiler::get_named_type(const char* name, bool unname, IdHolder* holder) {
	if (!name) {
		if (!unname) {
			holder->logger->semerr("unexpected unnamed set types.");
			return nullptr;
		}
		return holder->make_type(holder->hash.get_hash());
	}
	else {
		return holder->make_type(common::StringFilter() = name);
	}
}

Type* compiler::type_analyze(IdHolder* holder, Reader* reader) {
	if (!reader->expect_or_err("!"))return nullptr;
	Type* ret = nullptr;
	ReadStatus* status = holder->get_status();
	reader->readwhile(status, ctype::reader::Identifier);
	if (status->failed)return false;
	if (reader->expect_or_err("?"))return nullptr;
	if (ctype::is_unnamed(status->buf.get_const())) {
		holder->hash.unname_hash(status->buf);
	}
	common::EasyVector<char> hold(nullptr);
	hold = std::move(status->buf);
	ret = type_detail(hold.get_const(),holder,reader,false);
	if (!ret) return nullptr;
	if (!ttype::is_naming(ret->type)||strcmp(hold.get_const(),ret->name)==0) {
		auto tmp=holder->make_type(hold.get_raw_z());
		if (!tmp)return nullptr;
		tmp->derived.unuse();
		tmp->types.unuse();
		tmp->ids.unuse();
		tmp->type = TypeType::simple_alias_t;
		tmp->root = ret;
		ret = tmp;
	}
	return ret;
}

Type* compiler::type_detail(const char* name,IdHolder* holder, Reader* reader,bool unname) {
	if (ctype::is_first_of_identifier(reader->abyte())) {
		return get_ids(holder,reader);
	}
	else if (reader->expect("|")) {
		return get_sets(name, holder, reader, unname);
	}
	else if (reader->expect("*")) {
		return get_common_derived(TypeType::referrence_t, 0, holder, reader, "pointer to");
	}
	else if (reader->expect("&")) {
		return get_common_derived(TypeType::referrence_t, 0, holder, reader, "referece of");
	}
	else if (reader->ahead("(")||reader->ahead("<")) {
		return get_funcs(name, holder, reader, unname);
	}
	else if (reader->expect_or_err_pe("[", "|*&[\" or alphabet or \"_")) {
		return get_has_size(holder,reader);
	}
	else {
		return nullptr;
	}
}

Type* compiler::get_ids(IdHolder* holder, Reader* reader) {
	if (reader->expect_pf("void", ctype::is_usable_for_identifier)) {
		return holder->get_void();
	}
	else if (reader->expect_pf("bit_t", ctype::is_usable_for_identifier)) {
		bool sig = true;
		bool fl = false;
		if (reader->expect("(")) {
			if (reader->expect("unsigned")) {
				sig = false;
			}
			else if (reader->expect("float")) {
				fl = true;
			}
			else if (reader->expect_or_err("signed")) {
				sig = true;
			}
			else {
				return nullptr;
			}
			if (!reader->expect_or_err(")"))return nullptr;
		}
		if (fl) {
			return holder->get_float_bit_t();
		}
		else {
			return holder->get_bit_t(sig);
		}
	}
	else {
		Type* ret = resolve_type_by_name(holder, reader);
		if (!ret)return nullptr;
		if (ret->type == TypeType::simple_alias_t) {
			ret = ret->root;
		}
		else if (reader->expect("<")) {
			ret = resolve_template(ret, holder, reader);
			if (!ret)return nullptr;
			if (!reader->expect_or_err(">"))return nullptr;
		}
		return ret;
	}
}

Type* compiler::get_common_derived(TypeType ttype, uint64_t size, IdHolder* holder, Reader* reader,const char* err) {
	Type* ret = type_detail(nullptr, holder, reader, true);
	if (!ret)return nullptr;
	if (ret->type == TypeType::referrence_t) {
		holder->logger->synerr_val("* reference is unusable.",err);
		return nullptr;
	}
	else if (ttype::is_templatetype(ret->type)) {
		holder->logger->semerr_val("* template is unusable.", err);
		return nullptr;
	}
	return get_derived(TypeType::pointer_t, 0, ret, holder);
}

Type* compiler::get_has_size(IdHolder* holder, Reader* reader) {
	reader->abyte();
	auto status = holder->get_status();
	reader->readwhile(status, ctype::reader::Number);
	if (status->failed)return nullptr;
	auto num_type = get_number_type(status->buf.get_const(), holder);
	if (!num_type)return nullptr;
	if (num_type->root == holder->get_float_bit_t()) {
		holder->logger->semerr("float value is not usable for array's size.");
		return nullptr;
	}
	if (!reader->expect_or_err("]"))return nullptr;

	auto ar_size = common::strtoull_ex(status->buf.get_const(), nullptr);

	return get_common_derived(TypeType::has_size_t, ar_size, holder, reader, "array of");
}

Type* compiler::get_funcs(const char* name, IdHolder* holder, Reader* reader, bool unname) {
	Type* ret = get_named_type(name, unname, holder);
	if (!ret)return nullptr;
	if (reader->expect("<")) {
		holder->logger->unimplemented("function template.");
		return nullptr;
	}
	if (!reader->expect_or_err("("))return nullptr;
	while (!reader->eof()) {
		if (reader->ahead(")")) {
			break;
		}
		else if (reader->ahead("$") || reader->ahead("@")) {
			auto hold = id_analyze(holder, reader);
			if (!hold)return nullptr;
			ret->ids.add(hold);
		}
		else {
			holder->logger->unexpected_token("}$@", reader->abyte());
			return nullptr;
		}
		if (!reader->expect(","))break;
	}
	if (!reader->expect_or_err(")"))return nullptr;
	return ret;
}

Type* compiler::get_sets(const char* name, IdHolder* holder, Reader* reader, bool unname) {
	Type* ret = get_named_type(name, unname, holder);
	if (!ret)return nullptr;
	if (reader->expect("enum")) {
		holder->logger->unimplemented("enum");
		return nullptr;
	}
	else if (reader->expect("interface")) {
		holder->logger->unimplemented("interface");
		return nullptr;
	}
	else {
		reader->expect("struct");
		ret->type = TypeType::struct_t;
		if (reader->expect("<")) {
			holder->logger->unimplemented("struct template");
			return nullptr;
		}
		if (reader->expect("(")) {
			holder->logger->unimplemented("base structure");
			return nullptr;
		}
		if (!reader->expect_or_err("{"))return nullptr;
		while (!reader->eof()) {
			if (reader->ahead("}")) {
				break;
			}
			else if (reader->ahead("!")) {
				auto hold = type_analyze(holder, reader);
				if (!hold)return nullptr;
				hold->depends.type = ret;
				ret->types.add(hold);
			}
			else if (reader->ahead("$") || reader->ahead("@")) {
				auto hold = id_analyze(holder, reader);
				if (!hold)return nullptr;
				hold->depends.type = ret;
				ret->ids.add(hold);
			}
			else {
				holder->logger->unexpected_token("}!$@", reader->abyte());
				return nullptr;
			}
			if (!check_semicolon(holder, reader))return nullptr;
		}
		if (!reader->expect_or_err("}"))return nullptr;
	}
	return ret;
}

Type* compiler::resolve_type_by_name(IdHolder* holder, Reader* reader) {
	Type* ret = nullptr;
	common::EasyVector<char> id;
	int isvar = false;
	while (1) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Identifier);
		if (ctype::is_unnamed(status->buf.get_const())) {
			holder->logger->unexpected_token("alphabet or number",reader->abyte());
			return nullptr;
		}
		id.add_copy(status->buf.get_const(),status->buf.get_size());
		if (!ret) {
			ret = search_type_on_block(status->buf.get_const(), holder,isvar);
		}
		else {
			ret = search_type_on_type(status->buf.get_const(), ret,isvar);
		}
		if (!ret) {
			holder->logger->semerr_val("\"*\" is not defined.", id.get_const());
			return nullptr;
		}
		if (reader->expect(".")) {
			id.add('.');
			continue;
		}
		break;
	}
	if (isvar) {
		if (isvar == 1) {
			holder->logger->semerr_val("\"*\" is not type, is variable.", id.get_const());
		}
		else {
			holder->logger->semerr_val("\"*\" is not type, is constant value.", id.get_const());
		}
	}
	return ret;
}

Type* compiler::search_type_on_block(const char* name, IdHolder* holder, int& isvar) {
	Block* search = holder->get_current();
	while (search) {
		auto i=0ull;
		while (search->types[i]) {
			if (strcmp(search->types[i]->name,name)==0) {
				isvar = 0;
				return search->types[i];
			}
			i++;
		}
		i = 0;
		while (search->ids[i]) {
			if (strcmp(search->ids[i]->name, name) == 0) {
				if (search->ids[i]->is_const) {
					isvar = -1;
				}
				else {
					isvar = 1;
				}
				return search->ids[i]->type;
			}
			i++;
		}
		search = search->parent;
	}
	return nullptr;
}

Type* compiler::search_type_on_type(const char* name, Type* base, int& isvar) {
	Type* search = base;
	if (base->type != TypeType::struct_t && base->type != TypeType::template_s_t)return nullptr;
	while (search) {
		auto i = 0ull;
		while (search->types[i]) {
			if (strcmp(search->types[i]->name, name) == 0) {
				isvar = 0;
				return search->types[i];
			}
			i++;
		}
		i = 0;
		while (search->ids[i]) {
			if (strcmp(search->ids[i]->type->name, name) == 0) {
				if (search->ids[i]->is_const) {
					isvar = -1;
				}
				else {
					isvar = 1;
				}
				return search->ids[i]->type;
			}
			i++;
		}
		search = search->root;
	}
	return nullptr;
}

Type* compiler::resolve_template(Type* base, IdHolder* holder, Reader* reader) {
	if (!base)return nullptr;
	if (!ttype::is_templatetype(base->type)) {
		holder->logger->semerr_val("unexpandable. \"*\" is not template.",base->name);
		return nullptr;
	}
	common::EasyVector<Type*> params;
	Type* ret = nullptr;
	Type* arg = nullptr;
	Type* depth = base;
	uint64_t count = 0;
	bool ok = false;
	bool new_make = false;
	while (!reader->eof()) {
		count++;
		if (count > base->len) {
			holder->logger->semerr("parameter count is unmatched.");
			return nullptr;
		}
		arg = type_detail(nullptr,holder, reader,false);
		if (!arg)return nullptr;
		for (auto i = 0ull; depth->derived[i]; i++) {
			if (depth->derived[i]->types[0] == arg) {
				ret = depth->derived[i];
				depth = depth->derived[i];
				ok = true;
				break;
			}
		}
		if (!ok) {
			Type* param = holder->make_type(holder->hash.get_hash());
			if (!param)return nullptr;
			param->type = TypeType::template_a_t;
			param->types.add(arg);
			param->types.pack();
			param->ids.unuse();
			param->root = depth;
			depth->derived.add(param);
			depth = param;
			ret = depth;
			new_make = true;
		}
		params.add(ret);
		if (reader->expect(",")) {
			ok = false;
			continue;
		}
		break;
	}
	if (count != base->len) {
		holder->logger->semerr("parameter count is unmatched.");
		return nullptr;
	}
	if (new_make) {
		if (base->type == TypeType::template_f_t) {
			ret->type = TypeType::instance_f_t;
		}
		else {
			ret->type = TypeType::instance_s_t;
		}
		ret->derived = std::move(params);
	}
	return ret;
}

Type* compiler::get_number_type(const char* num,IdHolder* holder) {
	if (!num)return nullptr;
	auto size = strlen(num);
	if (!size)return nullptr;
	const char* check = "18446744073709551615";
	bool uf = false;
	bool lf = false;
	int base = 10;
	bool(*judge)(char) = nullptr;
	int ofs = 0;
	char flags = 0;
	uint64_t criteria = 0;
	uint64_t bit_size = 0;

	if (strstr(num, ".")||strstr(num,"p")||strstr(num,"+")||strstr(num,"-")) {
		if (num[size - 1] == 'f' || num[size - 1] == 'F') {
			return get_derived(TypeType::has_size_t, 32, holder->get_float_bit_t(), holder);
		}
		else {
			return get_derived(TypeType::has_size_t, 64, holder->get_float_bit_t(), holder);
		}
	}
	if (num[0]!='0'&&(strstr(num,"f")||strstr(num,"F"))) {
		return get_derived(TypeType::has_size_t, 32, holder->get_float_bit_t(), holder);
	}
	
	if (strstr(num,"u")||strstr(num,"U")) {
		uf = true;
	}
	if (strstr(num, "l") || strstr(num, "L")) {
		lf = true;
	}

	if (num[0]=='0') {
		if (num[1]=='x') {
			base = 16;
			ofs = 2;
			judge = ctype::is_hexnumber;
			criteria = 2;
			flags = '8';
		}
		else if (num[1] == 'b') {
			base = 2;
			ofs = 2;
			judge = ctype::is_binnumber;
			criteria = 8;
			flags = '1';
		}
		else if (ctype::is_octnumber(num[1])) {
			base = 8;
			ofs = 1;
			judge = ctype::is_octnumber;
			check = "1777777777777777777777";
		}
	}

	if (base==10||base==8) {
		const auto max_v = 0xFF'FF'FF'FF'FF'FF'FF'FF;
		char* end = nullptr;
		auto res=strtoull(&num[ofs], &end, base);
		if (*end != '\0' && ctype::is_numbersuffix(*end)) {
			holder->logger->synerr("unknown number like.");
			return nullptr;
		}
		if (res == max_v) {
			auto len = strlen(check);
			if (strncmp(&num[ofs], check, len) == 0&& num[ofs+len]!='\0'&&ctype::is_numbersuffix(num[ofs+len])) {
				holder->logger->semerr("too large number not to be compilable.");
				return nullptr;
			}
		}
		else if (res == 0) {
			if (num[ofs] != '0'){
				holder->logger->synerr("unknown number like.");
				return nullptr;
			}
		}
		if (res <= 0xFF) {
			bit_size = 8;
			if (res > 0x7F) {
				uf = true;
			}
		}
		else if(res<= 0xFFFF){
			bit_size = 16;
			if (res > 0x7FFF) {
				uf = true;
			}
		}
		else if (res <= 0xFFFFFFFF) {
			bit_size = 32;
			if (res > 0x7FFFFFFF) {
				uf = true;
			}
		}
		else {
			bit_size = 64;
			if (res > 0x7FFFFFFFFFFFFFFF) {
				uf = true;
			}
		}
	}
	else {
		auto len = common::count_while_f(&num[ofs],judge);
		if (len == 0) {
			holder->logger->synerr("unknown number like.");
			return nullptr;
		}
		if (num[ofs+len - 1] != '\0'&&!ctype::is_numbersuffix(num[ofs+len-1])) {
			holder->logger->synerr("unknown number like.");
			return nullptr;
		}
		if (len <= criteria) {
			bit_size = 8;
			if (len==criteria&&num[ofs]>=flags) {
				uf = 1;
			}
		}
		else if (len <= criteria * 2) {
			bit_size = 16;
			if (len == criteria*2 && num[ofs] >= flags) {
				uf = 1;
			}
		}
		else if (len <= criteria * 4) {
			bit_size = 32;
			if (len == criteria * 4 && num[ofs] >= flags) {
				uf = 1;
			}
		}
		else if (len <= criteria * 8) {
			bit_size = 64;
			if (len == criteria * 8 && num[ofs] >= flags) {
				uf = 1;
			}
		}
		else {
			holder->logger->semerr("too large number not to be compilable.");
			return nullptr;
		}
		if (lf)bit_size = 64;
	}

	return get_derived(TypeType::has_size_t, bit_size, holder->get_bit_t(!uf), holder);
}

Type* compiler::get_derived(TypeType ttype,uint64_t size, Type* base, IdHolder* holder) {
	if (!base)return nullptr;
	auto i = 0ull;	
	if (ttype::is_derivedtype(ttype))return nullptr;
	while (base->derived[i]) {
		if (base->derived[i]->type == ttype) {
			if (ttype == TypeType::has_size_t) {
				if (base->derived[i]->size == size) {
					return base->derived[i];
				}
			}
			else if (ttype == TypeType::pointer_t) {
				return base->derived[i];
			}
			else if (ttype == TypeType::referrence_t) {
				return base->derived[i];
			}
		}
	}
	const char* s = nullptr;
	if (ttype == TypeType::has_size_t) {
		s = "[]";
	}
	else if (ttype == TypeType::pointer_t) {
		s = "*";
	}
	else {
		s = "&";
	}
	auto ret = holder->make_type(common::StringFilter() = s);
	if (!ret)return nullptr;
	ret->type = ttype;
	ret->size = size;
	ret->root = base;
	base->derived.add(ret);
	return ret;
}

bool compiler::ttype::is_templatetype(TypeType ttype) {
	return ttype == TypeType::template_f_t || ttype == TypeType::template_s_t;
}

bool compiler::ttype::is_derivedtype(TypeType ttype) {
	return ttype == TypeType::has_size_t || ttype == TypeType::pointer_t || ttype == TypeType::referrence_t;
}

bool compiler::ttype::is_naming(TypeType ttype) {
	return ttype == TypeType::function_t||ttype==TypeType::struct_t||ttype==TypeType::enum_t||ttype==TypeType::interface_t||is_templatetype(ttype);
}

bool compiler::typecmp(Type* t1, Type* t2) {
	return false;
}

Identifier* compiler::id_analyze(IdHolder* holder, Reader* reader) {
	holder->logger->unimplemented("identifer analyzer");
	return nullptr;
}

Identifier* compiler::search_id_on_block(const char* name, IdHolder* holder) {
	Block* search = holder->get_current();
	while (search) {
		auto i = 0ull;
		while (search->ids[i]) {
			if (strcmp(search->ids[i]->name, name) == 0) {
				return search->ids[i];
			}
			i++;
		}
		search = search->parent;
	}
	return nullptr;
}