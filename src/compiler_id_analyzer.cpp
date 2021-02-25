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
	if (status->failed)return nullptr;
	if (!reader->expect_or_err("?"))return nullptr;
	if (ctype::is_unnamed(status->buf.get_const())) {
		holder->hash.unname_hash(status->buf);
	}
	else {
		int i = 0;
		if (search_type_on_block(status->buf.get_const(), holder, i)) {
			holder->logger->semerr_val("*:name conflict.",status->buf.get_const());
		}
	}
	common::EasyVector<char> hold(nullptr);
	hold = std::move(status->buf);
	ret = type_detail(hold.get_const(),holder,reader,false);
	if (!ret) return nullptr;
	if (!ttype::is_naming(ret->type)||strcmp(hold.get_const(),ret->name)!=0) {
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
		return get_common_derived(TypeType::pointer_t, 0, holder, reader, "pointer to");
	}
	else if (reader->expect("&")) {
		return get_common_derived(TypeType::referrence_t, 0, holder, reader, "referece of");
	}
	else if (reader->ahead("(")||reader->ahead("<")) {
		return get_functypes(holder, reader);
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
		/*if (ret->type == TypeType::simple_alias_t) {
			ret = ret->root;
		}*/
		if (reader->expect("<")) {
			holder->logger->unimplemented("template instance");
			/*ret = resolve_template(ret, holder, reader);
			if (!ret)return nullptr;
			if (!reader->expect_or_err(">"))*/
			return nullptr;
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
	return get_derived(ttype, size, ret, holder);
}

Type* compiler::get_has_size(IdHolder* holder, Reader* reader) {
	reader->abyte();
	auto status = holder->get_status();
	auto ar_size = 0ull;
	const char* err = "vector of";
	if (!reader->ahead("]")) {
		reader->readwhile(status, ctype::reader::Number);
		if (status->failed)return nullptr;
		auto num_type = get_number_type(status->buf.get_const(), holder);
		if (!num_type)return nullptr;
		if (num_type->root == holder->get_float_bit_t()) {
			holder->logger->semerr("float value is not usable for array's size.");
			return nullptr;
		}
		ar_size = common::strtoull_ex(status->buf.get_const(), nullptr);
		if (ar_size == 0) {
			holder->logger->semerr("0 size array is not usable");
			return nullptr;
		}
		err = "array of";
	}
	if (!reader->expect_or_err("]"))return nullptr;

	//ar_size = common::strtoull_ex(status->buf.get_const(), nullptr);

	return get_common_derived(TypeType::has_size_t, ar_size, holder, reader, err);
}

Type* compiler::get_functypes(IdHolder* holder, Reader* reader) {
	common::EasyVector<Type*> types;
	Type* ret = nullptr;
	if (reader->expect("<")) {
		holder->logger->unimplemented("function template.");
		return nullptr;
	}
	if (!reader->expect_or_err("("))return nullptr;
	while (!reader->eof()) {
		if (reader->ahead(")")) {
			break;
		}
		else {
			auto hold = type_detail(nullptr, holder, reader, false);
			if (!hold)return nullptr;
			types.add(hold);
		}
		if (!reader->expect(","))break;
	}
	if (!reader->expect_or_err(")"))return nullptr;
	return get_optandreturns(holder,reader,types);
}

Type* compiler::get_optandreturns(IdHolder* holder, Reader* reader,common::EasyVector<Type*>& types) {
	common::EasyVector<Option*> opts = get_options(holder, reader);
	if (opts == nullptr)return nullptr;
	Type* rettype = nullptr;
	if (reader->expect("->")) {
		rettype = type_detail(nullptr, holder, reader, false);
		if (!rettype)return nullptr;
	}
	else {
		rettype = holder->get_void();
	}
	if (!rettype)return nullptr;
	return get_function(TypeType::function_t, rettype, types, opts, holder);
}

common::EasyVector<Option*> compiler::get_options(IdHolder* holder, Reader* reader) {
	common::EasyVector<Option*> ret;
	if (reader->expect("ccnv")) {
		if (!reader->expect_or_err("("))return nullptr;
		reader->abyte();
		auto s = holder->get_status();
		reader->readwhile(s, ctype::reader::Identifier);
		if (s->failed)return nullptr;
		auto opt = holder->get_opt("ccnv", s->buf.get_const());
		if (!opt)return nullptr;
		ret.add(opt);
	}
	else if (reader->expect("capt")) {
		if (!reader->expect_or_err("("))return nullptr;
		common::EasyVector<Identifier*> ids;
		while (!reader->eof()) {
			if (reader->ahead(")"))break;
			auto s = holder->get_status();
			reader->readwhile(s, ctype::reader::Identifier);
			if (s->failed)return nullptr;
			auto id = search_id_on_block(s->buf.get_const(), holder);
			if (!id) {
				holder->logger->semerr_val("* is not id.",s->buf.get_const());
				return nullptr;
			}
			ids.add(id);
		}
		if (!reader->expect_or_err(")"))return nullptr;
		auto opt = holder->get_opt("capt", ids);
		if (!opt)return nullptr;
		ret.add(opt);
	}
	else if (reader->expect("va_args")) {
		const char* boolean = "true";
		if (reader->expect("(")) {
			if (reader->expect("true")) {
			}
			else if(reader->expect_or_err_pe("false","true or false")){
				boolean = "false";
			}
		}
		auto opt= holder->get_opt("va_args",boolean);
		if (!opt)return nullptr;
		ret.add(opt);
	}
	ret.pack();
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
		else {
			ret->root = holder->get_void();
		}
		if (!reader->expect_or_err("{"))return nullptr;
		auto current = holder->get_current();
		current->types.add(ret);
		while (!reader->eof()) {
			if (reader->ahead("}")) {
				break;
			}
			else if (reader->ahead("!")) {
				auto hold = type_analyze(holder, reader);
				if (!hold)return nullptr;
				hold->type_on = ret;
				ret->types.add(hold);
			}
			else if (reader->ahead("$") || reader->ahead("@")) {
				auto hold = id_analyze(holder, reader);
				if (!hold)return nullptr;
				hold->type_on = ret;
				ret->ids.add(hold);
			}
			else {
				holder->logger->unexpected_token("}!$@", reader->abyte());
				return nullptr;
			}
			if (!check_semicolon(holder, reader))return nullptr;
		}
		if (current->types.remove_end() != ret) {
			holder->logger->syserr("system is broken.");
			return nullptr;
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
		if (ret->type == TypeType::simple_alias_t) {
			ret = ret->root;
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
		//if (current)break;
		search = search->prev;
	}
	return nullptr;
}

Type* compiler::search_type_on_type(const char* name, Type* base, int& isvar) {
	Type* search = base;
	while (search) {
		if (base->type != TypeType::struct_t /*&&base->type != TypeType::instance_s_t*/)return nullptr;
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

/*
Type* compiler::resolve_template(Type* base, IdHolder* holder, Reader* reader) {
	holder->logger->unimplemented("template instance");
	return nullptr;
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
			if (depth->derived[i]->depends.type == arg) {
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
			param->depends.type = arg;
			param->types.pack();
			param->ids.pack();
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
		ret->root = base;
		ret->ids.add_copy(base->ids.get_const(), base->ids.get_size());
		ret->types.add_copy(base->types.get_const(),base->types.get_size());
		ret->derived = std::move(params);
		//if (!instantiate(ret,base,params,holder))return nullptr;
	}
	return ret;
}*/


/*bool compiler::instantiate(Type* instance, Type* base, common::EasyVector<Type*>& types, IdHolder* holder) {
	auto i = 0ull;
	return false;
}*/

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
	if (!ttype::is_derivedtype(ttype))return nullptr;
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
		i++;
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
	ret->ids.unuse();
	ret->types.unuse();
	ret->derived.pack();
	base->derived.add(ret);
	return ret;
}

Type* compiler::get_function(TypeType ttype, Type* rettype, common::EasyVector<Type*>& args,common::EasyVector<Option*>& opts,IdHolder* holder) {
	auto i = 0ull;
	while (rettype->function[i]) {
		if (rettype->function[i]->types.get_size() == args.get_size()&&rettype->function[i]->type==ttype) {
			if(rettype->function[i]->types==args&&rettype->function[i]->opts==opts)return rettype->function[i];
		}
		i++;
	}
	auto ret = holder->make_type(common::StringFilter() = "()");
	if (!ret)return nullptr;
	ret->type = TypeType::function_t;
	ret->root = rettype;
	rettype->function.add(ret);
	i = 0ull;
	ret->types = std::move(args);
	ret->opts = std::move(opts);
	ret->opts.pack_f();
	ret->types.pack_f();
	ret->ids.unuse();
	return ret;
}

bool compiler::ttype::is_templatetype(TypeType ttype) {
	return false;//ttype == TypeType::template_f_t || ttype == TypeType::template_s_t;
}

bool compiler::ttype::is_derivedtype(TypeType ttype) {
	return ttype == TypeType::has_size_t || ttype == TypeType::pointer_t || ttype == TypeType::referrence_t;
}

bool compiler::ttype::is_naming(TypeType ttype) {
	return /*ttype == TypeType::function_t||*/ttype==TypeType::struct_t||ttype==TypeType::enum_t||ttype==TypeType::interface_t||is_templatetype(ttype);
}

bool compiler::ttype::is_sets(TypeType ttype) {
	return ttype == TypeType::enum_t || ttype == TypeType::struct_t /*|| ttype == TypeType::instance_s_t*/||ttype==TypeType::interface_t;
}

Type* compiler::typecmp(Type* t1, Type* t2,IdHolder* holder) {
	if (!t1 || !t2)return nullptr;
	Type* c1 = t1, * c2 = t2;
	if (c1 == c2)return c1;
	if (c1->type == TypeType::simple_alias_t)c1 = c1->root;
	if (c2->type == TypeType::simple_alias_t)c2 = c2->root;
	if (c1->type == TypeType::referrence_t)c1 = c1->root;
	if (c2->type == TypeType::referrence_t)c2 = c2->root;
	if (c1 == c2)return t1;
	if (t1->type == t2->type&&t1->type==TypeType::has_size_t) {
		if (is_bit_t(t1->root,holder)||is_bit_t(t2->root,holder)) {
			if (!is_bit_t(t1->root, holder) || !is_bit_t(t2->root, holder)) {
				return nullptr;
			}
			if (t1->root == holder->get_float_bit_t() || t2->root == holder->get_float_bit_t()) {
				if (t1->root == t2->root) {
					return t1->size > t2->size ? t1 : t2;
				}
				else {
					return t1->root == holder->get_float_bit_t() ? t1 : t2;
				}
			}
			if (t1->size == t2->size) {
				if (t1->root == holder->get_bit_t(false) || t2->root == holder->get_bit_t(false)) {
					return t1->root == holder->get_bit_t(false) ? t1 : t2;
				}
				return t1;
			}
			return t1->size > t2->size ? t1 : t2;
		}
	}
	return nullptr;
}

bool compiler::is_bit_t(Type* type, IdHolder* holder) {
	return type == holder->get_bit_t(false) || type == holder->get_bit_t(true) || type == holder->get_float_bit_t();
}


Identifier* compiler::id_analyze(IdHolder* holder, Reader* reader) {
	bool is_const = false;
	if (reader->expect("@")) {
		is_const = true;
	}
	else if (!reader->expect_or_err("$")) {
		return nullptr;
	}
	Identifier* ret = nullptr;
	ReadStatus* status = holder->get_status();
	reader->readwhile(status, ctype::reader::Identifier);
	if (status->failed)return nullptr;
	if (ctype::is_unnamed(status->buf.get_const())) {
		holder->hash.unname_hash(status->buf);
	}
	else {
		if (search_id_on_block(status->buf.get_const(), holder)) {
			holder->logger->semerr_val("*:name conflict.", status->buf.get_const());
		}
	}
	ret = holder->make_id(status->buf.get_raw_z());
	if (!ret)return nullptr;
	ret->is_const = is_const;
	if (reader->expect("?")) {
		ret->type=type_detail(nullptr, holder, reader,true);
		if (!ret->type)return nullptr;
		if (ret->type->type==TypeType::function_t) {
			ret->params = std::move(ret->type->ids);
		}
	}
	if (reader->expect("=")) {
		ret->init = assign(holder,reader);
		if (!ret->init)return nullptr;
		if (!ret->type) {
			ret->type = ret->init->type;
		}
		else {
			if (!typecmp(ret->type, ret->init->type,holder)) {
				holder->logger->semerr("type can not assignable.");
				return nullptr;
			}
		}
	}
	else if (!ret->type&&reader->expect("(")) {
		if (!get_func_instance(ret, holder, reader))return nullptr;
		auto current=holder->get_current();
		current->ids.add(ret);
		ret->init=make_block_tree_pair(holder, "{}");
		if (!ret->init)return nullptr;
		ret->init->rel = ret;
		ret->init->relblock->id = ret;
		ret->init->relblock->ids.add_copy(ret->params.get_const(), ret->params.get_size());
		if (!reader->expect_or_err("{"))return nullptr;
		if (!block_detail(holder, reader))return nullptr;
		if (!reader->expect_or_err("}"))return nullptr;
		if (!holder->to_parent_block())return nullptr;
		if (current->ids.remove_end() != ret) {
			holder->logger->syserr("system is broken.");
			return nullptr;
		}
	}
	if (!ret->type) {
		holder->logger->semerr("type can not be identified.");
		return nullptr;
	}
	if (is_const&&!ret->init) {
		holder->logger->semerr("constant value have to initialize.");
		return nullptr;
	}
	return ret;
}

bool compiler::get_func_instance(Identifier* func,IdHolder* holder, Reader* reader) {
	common::EasyVector<Type*> types;
	common::EasyVector<Identifier*> ids;
	while (!reader->eof()) {
		if (reader->ahead(")")) {
			break;
		}
		else {
			auto hold = id_analyze(holder, reader);
			if (!hold)return nullptr;
			ids.add(hold);
			types.add_nz(hold->type);
		}
		reader->expect(",");
	}
	if (!reader->expect_or_err(")"))return false;
	auto type = get_optandreturns(holder, reader, types);
	if (!type)return false;
	func->type = type;
	func->params = std::move(ids);
	return true;
}

Identifier* compiler::search_id_on_block(const char* name,IdHolder* holder) {
	Block* search = holder->get_current();
	while (search) {
		auto i = 0ull;
		while (search->ids[i]) {
			if (strcmp(search->ids[i]->name, name) == 0) {
				return search->ids[i];
			}
			i++;
		}
		//if (current)break;
		search = search->prev;
	}
	return nullptr;
}

Identifier* compiler::search_id_on_id(const char* name, Identifier* id) {
	if (!id)return nullptr;
	return search_id_on_type(name, id->type);
}

Identifier* compiler::search_id_on_type(const char* name, Type* type) {
	if (!name)return nullptr;
	Type* search = type;
	while (search) {
		if (!ttype::is_sets(search->type))return nullptr;
		auto i = 0ull;
		while (search->ids[i]) {
			if (strcmp(search->ids[i]->name, name) == 0) {
				return search->ids[i];
			}
		}
		search = search->root;
	}
	return nullptr;
}