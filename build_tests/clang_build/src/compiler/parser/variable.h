/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"../data/identifier.h"

namespace PROJECT_NAME {
	namespace variable {
		bool parse_variable_set(io::Reader* reader, identifier::Maker* maker);
		identifier::Variable* parse_variable_detail(io::Reader* reader, identifier::Maker* maker, bool on_set = false, bool type_must = false);
	}
}