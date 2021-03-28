/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"syntax.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::syntax;

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

/*
void syntax::init_expects() {
	As.next = &Or;
	Or.prev = &As;
	Or.next = &And;
	And.prev = &Or;
	And.next = &Eaq;
	Eaq.prev = &And;
	Eaq.next = &Rel;
	Rel.prev = &Eaq;
	Rel.next = &Shift;
	Shift.prev = &Rel;
	Shift.next = &Bor;
	Bor.prev = &Shift;
	Bor.next = &Band;
	Band.prev = &Bor;
	Band.next = &Add;
	Add.prev = &Band;
	Add.next = &Mul;
	Mul.prev = &Add;
	Mul.next = &Pow;
	Pow.prev=&Mul;
}
*/

identifier::SyntaxTree* syntax::parse_syntax(io::Reader*, identifier::Maker*) {
	return nullptr;
}

BasicParser* syntax::make_parser() {
	return nullptr;
	auto ret = common::create<BasicParser>();
	if (!ret)return nullptr;
}