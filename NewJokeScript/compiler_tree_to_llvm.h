/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"common_structs.h"
#include"compiler_identifier_holder.h"
#include"llvms.h"


namespace PROJECT_NAME {
	namespace compiler {
		struct LLVM {
			llvm::LLVMContext ctx;
			llvm::Module* _module=nullptr;
			llvm::IRBuilder<llvm::LLVMContext>* builder;
			~LLVM();
		};

		LLVM* make_llvm(const char* name);

		LLVM* analyze_tree(IdHolder* holder);
	}
}