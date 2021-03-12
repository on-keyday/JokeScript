/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"../data/identifier.h"

namespace PROJECT_NAME {
	namespace type {
		bool parse_type_set(io::Reader* reader, identifier::Maker* maker);

		identifier::Type* parse_type(io::Reader* reader, identifier::Maker* maker);
		identifier::Type* get_array(io::Reader* reader, identifier::Maker* maker);
		identifier::Type* get_derived_common(io::Reader* reader, identifier::Maker* maker, identifier::TypeKind kind, uint64_t size = 0);
		identifier::Type* get_func(io::Reader* reader, identifier::Maker* maker);
		identifier::Type* get_struct(io::Reader* reader, identifier::Maker* maker);
		identifier::Type* get_named(io::Reader* reader, identifier::Maker* maker);
		identifier::Type* get_bit_type(io::Reader* reader, identifier::Maker* maker);

		
	}
}