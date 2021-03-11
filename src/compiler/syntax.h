/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"identifier.h"

namespace PROJECT_NAME {
	namespace syntax {
		identifier::SyntaxTree* parse_syntax(io::Reader* reader,identifier::Maker* maker);
	}
}