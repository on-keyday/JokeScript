/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"syntax.h"
#include"type.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::syntax;
using namespace PROJECT_NAME::identifier;

Expects Operators = {
 {
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
},
{
	"||"
},
{
	"&&"
},
{
	"==",
	"!="
},
{
	"<",
	"<=",
	">",
	">="
},
{
	"<<",
	">>"
},

{
	"|"
},

{
	"^"
},
{
	"&"
},
{
	"+",
	"-"
},

{
	"*",
	"/",
	"%"
},

{
	"**"
}
};


bool init_operators() {
	try {
		Operators[0].assign = true;
		Operators[1].boolean = true;
		Operators[2].boolean = true;
		Operators[3].boolean = true;
		Operators[4].boolean = true;
	}
	catch (...) {
		return false;
	}
	return true;
}

identifier::SyntaxTree* syntax::parse_syntax(io::Reader*, identifier::Maker*) {
	return nullptr;
}

BasicParser* syntax::make_parser(io::Reader* reader, identifier::Maker* maker) {
	if (!init_operators())return nullptr;
	auto ret = common::create<BasicParser>();
	if (!ret)return nullptr;
	auto makerover = common::create<syntax::MakerOverWrap>();
	if (!makerover) {
		common::kill(ret);
		return nullptr;
	}
	auto readerover = common::create<syntax::ReaderOverWrap>();
	if (!readerover) {
		common::kill(ret);
		common::kill(readerover);
		return nullptr;
	}
	makerover->maker = maker;
	readerover->reader = reader;
	ret->set(makerover, readerover, Operators.begin(), Operators.end());
	return ret;
}

bool ReaderOverWrap::expect(const char* str) {
	if (ctype::strlen(str) == 1) {
		return reader->expect_p1(str, str[0]);
	}
	else {
		return reader->expect(str);
	}
}

bool ReaderOverWrap::unary(bool& err) {
	if (expect("+") || expect("++") || expect("-") || expect("--") || expect("*") || expect("&")||expect("~")) {
		return true;
	}
	else if (reader->expect_pf("cast",ctype::is_usable_for_identifier)) {
		err = true;
	}
	return false;
}

bool ReaderOverWrap::brackets(bool begin) {
	if (begin) {
		return reader->expect("(");
	}
	else {
		return reader->expect_or_err(")");
	}
}

bool ReaderOverWrap::keyword() {
	if (reader->expect_pf("import",ctype::is_usable_for_identifier))return true;
	return false;
}

bool ReaderOverWrap::number() {
	return ctype::is_number(reader->abyte());
}

bool ReaderOverWrap::string() {
	return ctype::is_first_of_string(reader->abyte());
}

bool ReaderOverWrap::identifier() {
	return ctype::is_first_of_identifier(reader->abyte());
}

bool ReaderOverWrap::after() {
	return expect("[") || expect("(")||expect("++")||expect("--");
}

SyntaxTree* MakerOverWrap::binary(const char* symbol, SyntaxTree* left, SyntaxTree* right, Expect* flags) {
	auto ret=maker->make_tree(common::StringFilter() = symbol, TreeKind::binary);
	if (!ret)return nullptr;
	if (flags->assign) {
		if (!assignable(left)) {
			maker->logger->semerr("not assignable.");
			return nullptr;
		}
		ret->type = maker->type_decider(left->type, right->type, maker);
	}
	else if (flags->boolean) {
		ret->type = maker->get_bool();
	}
	else {
		ret->type = maker->type_decider(left->type, right->type, maker);
	}
	if (!ret->type)return nullptr;
	return ret;
}

SyntaxTree* MakerOverWrap::unary(ReaderOverWrap* reader, SyntaxTree* tree) {
	auto cmp = reader->reader->prev();
	SyntaxTree* ret = nullptr;
	if (ctype::streaq("+",cmp)||ctype::streaq("-",cmp)) {
		tree->type=tree->type;
	}
	else if (ctype::streaq("++",cmp)||ctype::streaq("--",cmp)||ctype::streaq("~",cmp)) {
		
	}
	return nullptr;
}
SyntaxTree* MakerOverWrap::keyword(ReaderOverWrap* reader) {
	return nullptr;
}

SyntaxTree* MakerOverWrap::number(ReaderOverWrap* reader) {
	return nullptr;
}
SyntaxTree* MakerOverWrap::string(ReaderOverWrap* reader) {
	return nullptr;
}

SyntaxTree* MakerOverWrap::identifier(ReaderOverWrap* reader) {
	return nullptr;
}

SyntaxTree* MakerOverWrap::after(ReaderOverWrap* reader) {
	return nullptr;
}

bool MakerOverWrap::assignable(SyntaxTree* tree) {
	return true;
}