/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"compiler_syntax_analyzer.h"
#include"compiler_ctype.h"
#include"compiler_id_analyzer.h"
using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;

SyntaxTree* compiler::comma(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = assign(holder, reader), * tmptree = nullptr;
	if (!ret)return nullptr;
	if (reader->eof()) {
		return ret;
	}
	while (1) {
		if (reader->expect(",")) {
			tmptree = holder->make_tree(common::StringFilter() = ",");
			if (!tmptree)return nullptr;
			tmptree->left = ret;
			ret = tmptree;
			tmptree = assign(holder, reader);
			if (!tmptree) return nullptr;
			ret->right = tmptree;
			tmptree = nullptr;

			continue;
		}
		break;
	}
	return ret;
}

const char* As[] = {
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"|=",
	"^=",
	"**="
};

const char* Or[] = {
	"||"
};

const char* And[] = {
	"&&"
};

const char* Eaq[] = {
	"==",
	"!="
};

const char* Rel[] = {
	"<",
	"<=",
	">",
	">="
};

const char* Shift[] = {
	"<<",
	">>"
};

const char* Bor[] = {
	"|"
};

const char* Bxor[] = {
	"^"
};

const char* Band[] = {
	"&"
};

const char* Add[] = {
	"+",
	"-"
};

const char* Mul[] = {
	"*",
	"/",
	"%"
};

const char* Pow[] = {
	"**"
};

#define fbinary(...) binary(__VA_ARGS__,Or,And,Eaq,Rel,Shift,Bor,Bxor,Band,Add,Mul,Pow)

SyntaxTree* compiler::assign(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = fbinary(holder,reader), * tmptree = nullptr;
	if (!ret)return nullptr;
	if (reader->eof()) {
		return ret;
	}
	bool ok = false;
	while (1) {
		for (auto symbol : As) {
			if (reader->expect(symbol)) {
				tmptree = holder->make_tree(common::StringFilter()=symbol);
				if (!tmptree)return nullptr;
				tmptree->left = ret;
				ret = tmptree;
				tmptree = assign(holder,reader);
				if (!tmptree) return nullptr;
				ret->right = tmptree;
				tmptree = nullptr;
				ok = true;
				break;
			}
		}
		if (!ok)break;
		ok = false;
	}
	return ret;
}

inline SyntaxTree* compiler::binary(IdHolder* holder, Reader* reader) {
	return unary(holder, reader);
}

SyntaxTree* compiler::unary(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = nullptr;
	if (reader->expect_p1("+",'+')) {
		ret=single(holder, reader);
	}
	else if (reader->expect_p1("-",'-')||reader->expect("++")||reader->expect("--")||reader->expect("*")) {
		ret = holder->make_tree(common::StringFilter() = reader->prev());
		if (!ret) return nullptr;
		ret->right = single(holder, reader);
		if (!ret->right) {
			return nullptr;
		}
	}
	else {
		ret = single(holder,reader);
	}
	return ret;
}

SyntaxTree* compiler::single(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = nullptr;
	if (reader->expect_pf("true", ctype::is_usable_for_identifier)||reader->expect_pf("false",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = reader->prev());
		if (!ret)return nullptr;
		ret->ttype = TreeType::literal;
		ret->type = get_derived(TypeType::has_size_t, 1, holder->get_bit_t(false), holder);
		if (!ret->type)return nullptr;
	}
	else if (reader->expect_pf("null",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = reader->prev());
		if (!ret)return nullptr;
		ret->ttype = TreeType::literal;
		ret->type = get_derived(TypeType::has_size_t, 1, holder->get_bit_t(false), holder);
		if (!ret->type)return nullptr;
	}
	else if (ctype::is_number(reader->abyte())) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Number);
		if (status->failed)return nullptr;
		Type* thold = get_number_type(status->buf.get_const(),holder);
		if (!thold)return nullptr;
		ret = holder->make_tree(status->buf.get_raw_z());
		if (!ret)return nullptr;
		ret->type = thold;
		ret->ttype = TreeType::literal;
	}
	else if (ctype::is_first_of_string(reader->abyte())) {
		auto res = reader->string();
		if (!res.is_enable())return nullptr;
		ret = holder->make_tree(res.get_raw_z());
		if (!ret)return nullptr;
		ret->type = holder->get_string();
		if (!ret->type)return nullptr;
		ret->ttype = TreeType::literal;
	}
	else if (reader->expect_pf("match",ctype::is_usable_for_identifier)) {
		ret = match(holder, reader);
		if (!ret)return nullptr;
	}
	else if (reader->expect_pf("co",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = "co");
		if (!ret)return nullptr;
		ret->right = assign(holder, reader);
		if (!ret->right)return nullptr;
		ret->ttype = TreeType::ctrl;
		ret->type = ret->right->type;
	}
	else if (ctype::is_first_of_identifier(reader->abyte())) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Identifier);
		if (status->failed)return nullptr;
		auto hold = search_id_on_block(status->buf.get_const(), holder);
		if (!hold)return nullptr;
	}
	else if (reader->expect_or_err("(")) {
		ret = comma(holder, reader);
		if (!ret)return nullptr;
		if (!reader->expect_or_err(")")) {
			return nullptr;
		}
	}
	
	if(!ret){
		return nullptr;
	}

	ret->children.unuse();

	bool ok = false;
	while (true) {
		if (reader->expect("(")){
			if (ret->type->type != TypeType::function_t) {
				holder->logger->semerr_val("\"*\" is not callable.",ret->symbol);
			}
			auto tmp = holder->make_tree(common::StringFilter() = "(call)");
			if (!tmp)return nullptr;
			tmp->ttype = TreeType::ctrl;
			tmp->left = ret;
			tmp->type = ret->type->root;
			auto i = 0ull;
			while (ret->type->ids[i]) {
				if (i!=0) {
					if (!reader->expect_or_err(","))return nullptr;
				}
				auto hold = assign(holder, reader);
				if (!hold)return nullptr;
				if (!typecmp(ret->type, hold->type)) {
					holder->logger->semerr("argument type and parameter type are not same.");
					return nullptr;
				}
				tmp->children.add(hold);
				i++;
			}

			if (!reader->expect(")")) {
				if (ret->type->types[0]) {
					if (strcmp(ret->type->types[0]->name, "va_args")==0) {
						while (!reader->eof()) {
							if (reader->ahead(")")) {
								break;
							}
							else if (!reader->expect_or_err(",")) {
								return nullptr;
							}
							auto hold = assign(holder, reader);
							if (!hold)return nullptr;
							tmp->children.add(hold);
							i++;
						}
						if (!reader->expect_or_err(")"))return nullptr;
						ok = true;
					}
					else {
						holder->logger->semerr_val("\"*\" is not va_args function.", ret->symbol);
					}
				}
				else {
					holder->logger->semerr_val("\"*\" is not va_args function.", ret->symbol);
				}
			}
			else {
				ok = true;
			}
			if (!ok)return nullptr;
			ret = tmp;
		}
		else if (reader->expect("->")) {
			if (ret->type->type != TypeType::pointer_t) {
				holder->logger->semerr_val("\"*\" is not pointer. not dereferencable.", ret->symbol);
				return nullptr;
			}
			
			auto hold = holder->make_tree(common::StringFilter() = "->");
		}
		else if (reader->expect(".")) {

		}
		if (ok) {
			ok = false;
			continue;
		}
		break;
	}

	return ret;
}

SyntaxTree* compiler::match(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = holder->make_tree(common::StringFilter() = "match");
	if (!ret)return nullptr;
	SyntaxTree* tmptree = assign(holder, reader), * tmptree2 = nullptr;;
	if (!tmptree)return nullptr;
	ret->right = tmptree;
	if (reader->expect_or_err("{"))return nullptr;
	while (1) {
		if (reader->expect("(")) {
			tmptree2 = holder->make_tree(common::StringFilter() = "?=");
			if (!tmptree2)return nullptr;
			tmptree = assign(holder, reader);
			if (!tmptree)return nullptr;
			tmptree2->left = tmptree;
			if (!reader->expect_or_err(")"))return nullptr;
			if (!reader->expect_or_err("?="))return nullptr;
			tmptree = assign(holder, reader);
			if (!tmptree)return nullptr;
			tmptree2->right = tmptree;
			continue;
		}
		else if (!reader->expect_or_err("}")) {
			return nullptr;
		}
		break;
	}
	return ret;
}

bool compiler::check_semicolon(IdHolder* holder, Reader* reader) {
	if (!reader->expect(";")) {
		if (strcmp(reader->prev(), "}") != 0) {
			holder->logger->unexpected_token("};", reader->abyte());
			return false;
		}
	}
	return true;
}