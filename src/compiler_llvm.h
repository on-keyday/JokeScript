/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"common_structs.h"
#include"compiler_identifier_holder.h"
#ifdef __linux__
#define _EXIST_LLVM 
#endif

#ifdef _EXIST_LLVM
#include"llvm/IR/Constant.h"
#include"llvm/IR/Function.h"
#include"llvm/IR/Instruction.h"
#include"llvm/IR/Constant.h"
#endif

namespace jokescript {
	namespace llvm_wrap {
#ifdef _EXIST_LLVM
		using llvm_Function = llvm::Function;
		using llvm_AllocaInst = llvm::AllocaInst;
		using llvm_Constant = llvm::Constant;
#else
		using llvm_Function = void;
		using llvm_AllocaInst = void;
		using llvm_Constant = void;
#endif
		struct LLVM_Converter {
			llvm_Function* make_function(compiler::Identifier* id);
			llvm_AllocaInst* make_local_variable(compiler::Identifier* id);
			llvm_Constant* make_constant_value(compiler::SyntaxTree* tree);
		};
	}
	namespace compiler {
		struct LLVM {
			char a_object;
		};
		LLVM* convert_to_llvm(IdHolder* holder);
	}
}
