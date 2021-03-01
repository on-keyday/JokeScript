/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"common_structs.h"
#include"log.h"
#include"compiler_identifier_holder.h"
#ifdef __linux__
#define _EXIST_LLVM 1
#else
#define _EXIST_LLVM 0
#endif

#if _EXIST_LLVM
#include<llvm/Support/ErrorHandling.h>
#include<llvm/IR/Type.h>
#include<llvm/IR/Constant.h>
#include<llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include<llvm/IR/Constant.h>
#include<llvm/IR/DerivedTypes.h>
#endif


namespace PROJECT_NAME {
	namespace llvm_wrap {
#if _EXIST_LLVM
		using llvm_error_handler_t = llvm::fatal_error_handler_t;
		using llvm_LLVMContext = llvm::LLVMContext;
		using llvm_IntType = llvm::IntegerType;
		using llvm_Type = llvm::Type;
		using llvm_ArrayType = llvm::ArrayType;
		using llvm_VectorType = llvm::VectorType;
		using llvm_FuncType = llvm::FunctionType;
		using llvm_Function = llvm::Function;
		using llvm_AllocaInst = llvm::AllocaInst;
		using llvm_Constant = llvm::Constant;
#else
		using llvm_LLVMContext = void*;
		using llvm_Type = void;
		using llvm_FuncType = void;
		using llvm_Function = void;
		using llvm_AllocaInst = void;
		using llvm_Constant = void;
		using llvm_FuncType = void;
		using llvm_IntEx = void;
#endif
		

		struct llvm_Err {
			llvm_Err() = delete;
			llvm_Err(llvm_Err&) = delete;
			llvm_Err(llvm_Err&&) = default;
			llvm_Err(void* data,const std::string& reason, bool crash) :data(data), reason(reason), crash(crash) {};
			void* data;
			const std::string reason;
			bool crash;
		};
		void _when_err(void* user_data,const std::string& reason,bool gen_crash_diag);
		struct LLVM_Converter {
		private:
			compiler::IdHolder* holder=nullptr;
			llvm_LLVMContext context;
			std::map<compiler::Type*, llvm_Type*> type_convert;
		public:
			llvm_Type* make_type(compiler::Type* type);
			llvm_FuncType* make_functype(compiler::Type* type);
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
