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

bool compiler::program(IdHolder* holder, Reader* reader) {
	if (!block_detail(holder, reader))return false;
	if (!reader->eof()) {
		if (strcmp(reader->prev(), "}") == 0) {
			holder->logger->unexpected_token("EOF", '}');
		}
		else {
			holder->logger->syserr("by unknown reason, compilation is failed.");
		}
		return false;
	}
	return true;
}

SyntaxTree* compiler::block(IdHolder* holder, Reader* reader) {
	auto ret = make_block_tree_pair(holder,"{}");
	if (!ret)return nullptr;
	if (!block_detail(holder,reader))return nullptr;
	if (!reader->expect_or_err("}"))return nullptr;
	if (!holder->to_parent_block())return nullptr;;
	return ret;
}

bool compiler::block_detail(IdHolder* holder, Reader* reader) {
	while (!reader->eof()) {
		if (reader->ahead(";")) {
		}
		else if (reader->ahead("!")) {
			auto type = type_analyze(holder, reader);
			if (!type)return false;
			holder->get_current()->types.add(type);
		}
		else if (reader->ahead("$") || reader->ahead("@")) {
			auto id = id_analyze(holder, reader);
			if (!id)return false;
			holder->get_current()->ids.add(id);
		}
		else if (reader->expect("{")) {
			auto tree = block(holder, reader);
			if (!tree)return false;
			holder->get_current()->trees.add(tree);
		}
		else if (reader->ahead("}")) {
			break;
		}
		else if (reader->expect_pf("break", ctype::is_usable_for_identifier) || reader->expect_pf("continue", ctype::is_usable_for_identifier)) {
			if (!break_usable(holder)) {
				holder->logger->semerr_val("in this scope, '*' is not usable.", reader->prev());
				return false;
			}
			auto hold = holder->make_tree(common::StringFilter() = reader->prev(), TreeType::ctrl, nullptr);
			if (!hold)return false;
			holder->get_current()->trees.add(hold);
		}
		else if (reader->expect_pf("return", ctype::is_usable_for_identifier)) {
			if (!return_usable(holder)) {
				holder->logger->semerr_val("in this scope, 'return' is not usable.", reader->prev());
				return false;
			}
			auto hold = holder->make_tree(common::StringFilter() = "return", TreeType::ctrl, nullptr);
			if (!hold)return false;
			if (!reader->ahead(";")) {
				hold->right = assign(holder, reader);
				if (!hold->right)return false;
				if (!hold->right->type) {
					holder->logger->semerr("+not usable for return statment.");
					return false;
				}
			}
			holder->get_current()->trees.add(hold);
		}
		else if (reader->expect_pf("loop",ctype::is_usable_for_identifier)) {
			auto hold = loop(holder, reader);
			if (!hold)return false;
			holder->get_current()->trees.add(hold);
		}
		else if (reader->expect_pf("if", ctype::is_usable_for_identifier)) {
			auto hold = ifs(holder, reader);
			if (!hold)return false;
			holder->get_current()->trees.add(hold);
		}
		else {
			auto tree = comma(holder, reader);
			if (!tree)return false;
			holder->get_current()->trees.add(tree);
		}
		if (!check_semicolon(holder, reader))return false;
	}
	return true;
}

SyntaxTree* compiler::comma(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = assign(holder, reader), * tmptree = nullptr;
	if (!ret)return nullptr;
	if (reader->eof()) {
		return ret;
	}
	while (!reader->eof()) {
		if (reader->expect(",")) {
			tmptree = holder->make_tree(common::StringFilter() = ",",TreeType::bin,nullptr);
			if (!tmptree)return nullptr;
			tmptree->left = ret;
			ret = tmptree;
			tmptree = assign(holder, reader);
			if (!tmptree) return nullptr;
			ret->right = tmptree;
			ret->type = ret->right->type;
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

StrSet Or[] = {
	{"||",true}
};

StrSet And[] = {
	{"&&",true}
};

StrSet Eaq[] = {
	{"==",true},
	{"!=",true}
};

StrSet Rel[] = {
	{"<",true},
	{"<=",true},
	{">",true},
	{">=",true}
};

StrSet Shift[] = {
	{"<<",false},
	{">>",false}
};

StrSet Bor[] = {
	{"|",false}
};

StrSet Bxor[] = {
	{"^",false}
};

StrSet Band[] = {
	{"&",false}
};

StrSet Add[] = {
	{"+",false},
	{"-",false}
};

StrSet Mul[] = {
	{"*",false},
	{"/",false},
	{"%",false}
};

StrSet Pow[] = {
	{"**",false}
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
				tmptree = holder->make_tree(common::StringFilter()=symbol,TreeType::bin,nullptr);
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
		ret = holder->make_tree(common::StringFilter() = reader->prev(),TreeType::unary,nullptr);
		if (!ret) return nullptr;
		ret->right = single(holder, reader);
		if (!ret->right) {
			return nullptr;
		}
		ret->type = ret->right->type;
	}
	else {
		ret = single(holder,reader);
	}
	return ret;
}

SyntaxTree* compiler::single(IdHolder* holder, Reader* reader) {
	SyntaxTree* ret = nullptr;
	if (reader->expect_pf("true", ctype::is_usable_for_identifier)||reader->expect_pf("false",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = reader->prev(),TreeType::literal,nullptr);
		if (!ret)return nullptr;
		ret->type=get_derived(TypeType::has_size_t, 1, holder->get_bit_t(false), holder);
		if (!ret->type)return nullptr;
	}
	else if (reader->expect_pf("null",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = "null",TreeType::literal,nullptr);
		if (!ret)return nullptr;
		ret->type = get_derived(TypeType::has_size_t, 1, holder->get_bit_t(false), holder);
		if (!ret->type)return nullptr;
	}
	else if (ctype::is_number(reader->abyte())) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Number);
		if (status->failed)return nullptr;
		Type* thold = get_number_type(status->buf.get_const(),holder);
		if (!thold)return nullptr;
		ret = holder->make_tree(status->buf.get_raw_z(),TreeType::literal,thold);
		if (!ret)return nullptr;
	}
	else if (ctype::is_first_of_string(reader->abyte())) {
		auto res = reader->string(false);
		if (res==nullptr)return nullptr;
		ret = holder->make_tree(res.get_raw_z(),TreeType::literal,holder->get_string());
		if (!ret)return nullptr;
	}
	else if (reader->expect_pf("match",ctype::is_usable_for_identifier)) {
		ret = match(holder, reader);
		if (!ret)return nullptr;
	}
	else if (reader->expect_pf("co",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = "co",TreeType::ctrl,nullptr);
		if (!ret)return nullptr;
		ret->right = assign(holder, reader);
		if (!ret->right)return nullptr;
		ret->type = ret->right->type;
	}
	else if (reader->expect_pf("break",ctype::is_usable_for_identifier)||reader->expect_pf("continue",ctype::is_usable_for_identifier)) {
		if (!break_usable(holder)) {
			holder->logger->semerr_val("in this scope, '*' is not usable.",reader->prev());
			return nullptr;
		}
		ret = holder->make_tree(common::StringFilter() = reader->prev(), TreeType::ctrl, nullptr);
	}
	else if (reader->expect_pf("return",ctype::is_usable_for_identifier)) {
		if (!return_usable(holder)) {
			holder->logger->semerr_val("in this scope, 'return' is not usable.", reader->prev());
			return nullptr;
		}
		ret = holder->make_tree(common::StringFilter()="return",TreeType::ctrl,nullptr);
		if (!ret)return nullptr;
		if (!reader->ahead(";")) {
			ret->right = assign(holder, reader);
			if (!ret->right)return nullptr;
			if (!ret->right->type) {
				holder->logger->semerr("+not usable for return statment.");
				return nullptr;
			}
		}
	}
	else if (ctype::is_first_of_identifier(reader->abyte())) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Identifier);
		if (status->failed)return nullptr;
		auto hold = search_id_on_block(status->buf.get_const(), holder);
		if (!hold)return nullptr;
		ret = holder->make_tree(status->buf.get_raw_z(), TreeType::defined, hold->type);
	}
	else if (reader->ahead("@(")) {
		auto id = id_analyze(holder, reader);
		if (!id)return nullptr;
		if (id->type->type != TypeType::function_t) {
			holder->logger->semerr("on this context,non function is not definable.");
			return nullptr;
		}
		ret = holder->make_tree(common::StringFilter()="(anonymous function)",TreeType::defined,id->type);
		if (!ret)return nullptr;
		ret->rel = id;
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
			/*if (ret->type->type != TypeType::function_t) {
				holder->logger->semerr_val("\"*\" is not callable.",ret->symbol);
			}
			auto tmp = holder->make_tree(common::StringFilter() = "(call)");
			if (!tmp)return nullptr;
			tmp->ttype = TreeType::ctrl;
			tmp->left = ret;
			tmp->type = ret->type->root;
			auto i = 0ull;
			while (ret->type->types[i]) {
				if (i!=0) {
					if (!reader->expect_or_err(","))return nullptr;
				}
				auto hold = assign(holder, reader);
				if (!hold)return nullptr;
				if (!typecmp(ret->type->types[i], hold->type,holder)) {
					holder->logger->semerr("argument type and parameter type are not same.");
					return nullptr;
				}
				tmp->children.add(hold);
				i++;
			}

			if (!reader->expect(")")) {
				if (ret->type->opts.get_if(holder->get_opt("va_args", "true"))) {
					while (!reader->eof()) {
						if (!reader->expect_or_err(","))return nullptr;
						auto hold = assign(holder, reader);
						if (!hold)return nullptr;
						if (!typecmp(ret->type->types[i], hold->type, holder)) {
							holder->logger->semerr("argument type and parameter type are not same.");
							return nullptr;
						}
						tmp->children.add(hold);
						if (reader->ahead(")"))break;
					}
					if (!reader->expect_or_err(")"))return nullptr;
					ok = true;
				}
			}
			else {
				ok = true;
			}
			if (!ok)return nullptr;
			ret = tmp;*/
			ret = call(ret, holder, reader);
			if (!ret)return nullptr;
			ok = true;
		}
		else if (reader->expect("->")) {
			if (ret->type->type != TypeType::pointer_t) {
				holder->logger->semerr_val("\"*\" is not pointer. not dereferencable.", ret->symbol);
				return nullptr;
			}
			auto hold = holder->make_tree(common::StringFilter() = "*",TreeType::unary,ret->type->root);
			if (!hold)return nullptr;
			hold->right = ret;
			ret = hold;
			ret=member(ret, holder, reader);
			if (!ret)return nullptr;
		}
		else if (reader->expect(".")) {
			ret = member(ret, holder, reader);
			if (!ret)return nullptr;
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
	SyntaxTree* ret = holder->make_tree(common::StringFilter() = "match",TreeType::ctrl,nullptr);
	if (!ret)return nullptr;
	SyntaxTree* expr = assign(holder, reader), * result = nullptr;
	if (!expr)return nullptr;
	Type* restype = nullptr;
	ret->right = expr;
	if (!reader->expect_or_err("{"))return nullptr;
	while (1) {
		if (reader->expect("(")) {
			result = holder->make_tree(common::StringFilter() = "?",TreeType::ctrl,nullptr);
			if (!result)return nullptr;
			expr = assign(holder, reader);
			if (!expr)return nullptr;
			result->left = expr;
			if (!reader->expect_or_err(")"))return nullptr;
			if (!reader->expect_or_err("?"))return nullptr;
			expr = assign(holder, reader);
			if (!expr)return nullptr;
			result->right = expr;
			ret->children.add(result);
			if (!restype) {
				restype = result->right->type;
			}
			else if (!typecmp(restype, result->right->type, holder)) {
				holder->logger->semerr("result type are not constant.");
				return nullptr;
			}
			if (!check_semicolon(holder, reader))return nullptr;
			continue;
		}
		else if (!reader->expect_or_err("}")) {
			return nullptr;
		}
		break;
	}
	if (!restype)restype = holder->get_void();
	ret->type=restype;
	return ret;
}


SyntaxTree* compiler::call(SyntaxTree* func, IdHolder* holder, Reader* reader) {
	bool ok = false;
	if (func->type->type != TypeType::function_t) {
		holder->logger->semerr_val("\"*\" is not callable.", func->symbol);
	}
	auto ret = holder->make_tree(common::StringFilter() = "(call)", TreeType::ctrl,func->type->root);
	if (!ret)return nullptr;
	ret->left = func;
	auto i = 0ull;
	while (func->type->types[i]) {
		if (i != 0) {
			if (!reader->expect_or_err(","))return nullptr;
		}
		auto hold = assign(holder, reader);
		if (!hold)return nullptr;
		if (!typecmp(func->type->types[i], hold->type, holder)) {
			holder->logger->semerr("argument type and parameter type are not same.");
			return nullptr;
		}
		ret->children.add(hold);
		i++;
	}

	if (!reader->expect(")")) {
		if (func->type->opts.get_if(holder->get_opt("va_args", "true"))) {
			while (!reader->eof()) {
				if (!reader->expect_or_err(","))return nullptr;
				auto hold = assign(holder, reader);
				if (!hold)return nullptr;
				if (!typecmp(func->type->types[i], hold->type, holder)) {
					holder->logger->semerr("argument type and parameter type are not same.");
					return nullptr;
				}
				ret->children.add(hold);
				if (reader->ahead(")"))break;
			}
			if (!reader->expect_or_err(")"))return nullptr;
			ok = true;
		}
	}
	else {
		ok = true;
	}
	if (!ok)return nullptr;
	return ret;
}

SyntaxTree* compiler::member(SyntaxTree* base, IdHolder* holder, Reader* reader) {
	reader->abyte();
	auto status = holder->get_status();
	reader->readwhile(status, ctype::reader::Identifier);
	if (status->failed)return nullptr;
	auto id = search_id_on_type(status->buf.get_const(), base->type);
	if (!id) {
		holder->logger->semerr("not dereferencable.");
		return nullptr;
	}
	auto ret = holder->make_tree(common::StringFilter()=".",TreeType::bin,id->type);
	if (!ret)return nullptr;
	ret->left = base;
	ret->right = holder->make_tree(status->buf.get_raw_z(),TreeType::defined,id->type);
	if (!ret->right)return nullptr;
	ret->right->rel = id;
	return ret;
}

SyntaxTree* compiler::loop(IdHolder* holder, Reader* reader) {
	auto ret = make_block_tree_pair(holder, "loop");
	if (!ret)return nullptr;
	if (reader->expect_pf("first",ctype::is_usable_for_identifier)) {
		auto got = holder->make_tree(common::StringFilter() = "first", TreeType::ctrl,nullptr);
		if (!got)return nullptr;
		got->right = assign(holder, reader);
		if (!got->right)return nullptr;
		ret->children.add(got);
	}
	if (reader->expect_pf("if",ctype::is_usable_for_identifier)) {
		auto got = holder->make_tree(common::StringFilter() = "if", TreeType::ctrl, nullptr);
		if (!got)return nullptr;
		got->right = assign(holder, reader);
		if (!got->right)return nullptr;
		ret->children.add(got);
	}
	if (reader->expect_pf("continue",ctype::is_usable_for_identifier)) {
		auto got = holder->make_tree(common::StringFilter() = "continue", TreeType::ctrl, nullptr);
		if (!got)return nullptr;
		got->right = assign(holder, reader);
		if (!got->right)return nullptr;
		ret->children.add(got);
	}
	if (!reader->expect_or_err("{"))return nullptr;
	if (!block_detail(holder, reader))return nullptr;
	if (!reader->expect_or_err("}"))return nullptr;
	if (!holder->to_parent_block())return nullptr;
	return ret;
}

SyntaxTree* compiler::ifs(IdHolder* holder, Reader* reader){
	auto ret = make_block_tree_pair(holder, "if");
	if (!ret)return nullptr;
	ret->left = assign(holder,reader);
	if (!ret->left)return nullptr;
	if (!reader->expect_or_err("{"))return nullptr;
	if (!block_detail(holder, reader))return nullptr;
	if (!reader->expect_or_err("}"))return nullptr;
	if (!holder->to_parent_block())return nullptr;
	if (reader->expect("else")) {
		if (reader->expect("if")) {
			ret->right = ifs(holder, reader);
			if (!ret->right)return nullptr;
		}
		else if(reader->expect_or_err("{")){
			ret->right = block(holder, reader);
			if (!ret->right)return nullptr;
		}
		else {
			return nullptr;
		}
	}
	return ret;
}

bool compiler::set_relative(Block* block, SyntaxTree* tree) {
	block->reltree = tree;
	tree->relblock = block;
	return true;
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

bool compiler::break_usable(IdHolder* holder) {
	auto search = holder->get_current();
	while (search) {
		if (search->reltree) {
			if (strcmp(search->reltree->symbol, "loop")) {
				return true;
			}
		}
		search = search->prev;
	}
	return false;
}

bool compiler::return_usable(IdHolder* holder) {
	return true;
}

SyntaxTree* compiler::make_block_tree_pair(IdHolder* holder,const char* name) {
	auto current = holder->make_block();
	if (!current)return nullptr;
	auto ret = holder->make_tree(common::StringFilter()=name, TreeType::ctrl, nullptr);
	if (!ret)return nullptr;
	set_relative(current, ret);
	return ret;
}