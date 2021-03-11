/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"common_structs.h"
#include"compiler_identifier_holder.h"

namespace PROJECT_NAME {
	enum class OpeCode {
		nop,
		mv,
		add,
		sub,
		mul,
		div,
		mod,
		ld
	};

	struct Opeland {

	};
	
	unsigned char convert_op_to_bin(OpeCode code,common::EasyVectorP<Opeland> oplands,compiler::Type* type);
}