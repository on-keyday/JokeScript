/*license*/
#include"compiler_syntax_analyzer.h"
#include"compiler_ctype.h"
using namespace jokescript;
using namespace jokescript::compiler;

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
	if (reader->expect("(")) {
		ret = comma(holder, reader);
		if (!ret)return nullptr;
		if (!reader->expect_or_err(")")) {
			return nullptr;
		}
	}
	else if (reader->expect_pf("true", ctype::is_usable_for_identifier)||reader->expect_pf("false",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = reader->prev());
		if (!ret)return nullptr;
	}
	else if (reader->expect_pf("null",ctype::is_usable_for_identifier)) {
		ret = holder->make_tree(common::StringFilter() = reader->prev());
		if (!ret)return nullptr;
	}
	else if (ctype::is_number(reader->abyte())) {
		auto status = holder->get_status();
		reader->readwhile(status, ctype::reader::Number);
		if (status->failed)return nullptr;
		ret = holder->make_tree(status->buf.get_raw_z());
		if (!ret)return nullptr;
	}
	else if (ctype::is_first_of_string(reader->abyte())) {
		auto res = reader->string();
		if (!res.is_enable())return false;
		ret = holder->make_tree(res.get_raw_z());
		if (!ret)return nullptr;
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
	}
	else if (ctype::is_first_of_identifier(reader->abyte())) {

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