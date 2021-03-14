/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"variable.h"
#include"syntax.h"
#include"type.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;
using namespace PROJECT_NAME::io;
using namespace PROJECT_NAME::variable;

bool variable::parse_variable_set(Reader* reader, Maker* maker) {
	return false;
}

bool variable::parse_variable(io::Reader* reader, identifier::Maker* maker) {
	common::EasyVector<Variable*> vars;
	auto t=parse_variable_detail(reader, maker,true,false);
	if (!t)return false;

	return false;
}

Variable* variable::parse_variable_detail(Reader* reader, Maker* maker, bool may_comma, bool type_must) {
	auto s = maker->get_read_status();
	reader->readwhile(s, ctype::reader::Identifier);
	if (s->failed)return nullptr;
	if (ctype::is_unnamed(s->buf.get_const())) {
		maker->logger->semerr("on this context, unnamed variable is not definable.");
		return nullptr;
	}
	auto ret = maker->make_variable(s->buf.get_raw_z());
	if (!ret)return nullptr;
	if (!reader->ahead(",") && !reader->ahead("=")) {
		auto type = type::parse_type(reader, maker);
		if (!type)return nullptr;
		ret->type=type;
	}
	if (reader->expect("=")) {
		auto init = syntax::parse_syntax(reader, maker);
		if (!init)return nullptr;
		ret->init = init;
	}
	if (may_comma && !ret->type) {
		if (!reader->ahead(",")) {
			maker->logger->unexpected_token(",", reader->abyte());
			return nullptr;
		}
	}
	if (type_must && !ret->type) {
		maker->logger->synerr("on this context,type has to be explicitly specified,but is not.");
		return nullptr;
	}
	return ret;
}