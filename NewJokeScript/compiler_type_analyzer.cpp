/*license*/
#include"compiler_type_analyzer.h"
#include"compiler_identifier_holder.h"
#include"compiler_ctype.h"
#include"common_tools.h"

using namespace jokescript;
using namespace jokescript::compiler;

Type* compiler::type_analyze(IdHolder* holder, Reader* reader) {
	if (!reader->expect_or_err("!"))return nullptr;
	Type* ret = nullptr;
	TypeType ttype = TypeType::unset;
	bool will_delete = false;
	ReadStatus* status = holder->get_status();
	reader->readwhile(status, ctype::reader::Identifier);
	if (status->failed)return false;
	
	if (reader->expect_or_err("?"))return nullptr;

	if (ctype::is_unnamed(status->buf.get_const())) {
		holder->hash.name_hash(status->buf);
	}

	
}

Type* compiler::type_detail(IdHolder* holder, Reader* reader) {
	Type* ret = nullptr;
	TypeType ttype = TypeType::unset;
	if (ctype::is_first_of_identifier(reader->abyte())) {
		if (reader->expect("bit_t")) {
			ttype = TypeType::bit_t;
			if (reader->expect("(")) {
				if (!reader->expect_or_err("unsigned"))return nullptr;
				if (!reader->expect_or_err(")"))return nullptr;
				ret = holder->get_bit_t(false);
			}
			else {
				ret = holder->get_bit_t(true);
			}
		}
		else {
			ret = resolve_type_by_name(holder, reader);
		}
	}
	else if (reader->expect("|")) {

	}
	else if (reader->expect("*")) {

	}
	else if (reader->expect("&")) {

	}
	else if (reader->ahead("(")||reader->ahead("<")) {

	}
	else if (reader->expect_or_err_pe("[","|*&[\" or alphabet or \"_")) {

	}
	else {
		return nullptr;
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
			holder->logger->symerr_val("\"*\" is not defined.", id.get_const());
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
			holder->logger->symerr_val("\"*\" is not type, is variable.", id.get_const());
		}
		else {
			holder->logger->symerr_val("\"*\" is not type, is constant value.", id.get_const());
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

Type* get_derived(TypeType ttype,uint64_t size, Type* base, IdHolder* holder) {
	auto i = 0ull;	
	if (ttype != TypeType::has_size_t && ttype != TypeType::pointer_t && ttype != TypeType::referrence_t)return nullptr;
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