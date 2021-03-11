/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"identifier.h"
#include"syntax.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::identifier;

bool identifier::parse_variable_set(compiler::Reader* reader, Maker* maker) {
	return false;
}

Variable* identifier::parse_variable(compiler::Reader* reader, Maker* maker, bool on_set, bool type_must) {
	auto s = maker->get_read_status();
	reader->readwhile(s, ctype::reader::Identifier);
	if (s->failed)return nullptr;
	if (ctype::is_unnamed(s->buf.get_const())) {
		maker->logger->semerr("on this context, unnamed variable is not definable.");
		return false;
	}
	auto ret = maker->make_variable(s->buf.get_raw_z());
	if (!ret)return nullptr;
	if (!reader->ahead(",") && !reader->ahead("=")) {
		auto type = parse_type(reader, maker);
		if (!type)return nullptr;
		ret->type=type;
	}
	if (reader->expect("=")) {
		auto init = syntax::parse_syntax(reader, maker);
		if (!init)return nullptr;
		ret->init = false;
	}
	if (on_set && !ret->type) {
		
	}
}