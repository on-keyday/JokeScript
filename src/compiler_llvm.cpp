/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"compiler_llvm.h"
#include"compiler_id_analyzer.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;

void llvm_wrap::_when_err(void* user_data, const std::string& reason, bool gen_crash_diag) {
	throw llvm_Err(user_data,reason,gen_crash_diag);
}



llvm_wrap::llvm_Type* llvm_wrap::LLVM_Converter::make_type(compiler::Type* type) {
#if !_EXIST_LLVM
	return nullptr;
#else
	if (!type)return nullptr;
	if (auto p=type_convert[type])return p;
	if (type->type==TypeType::base_t) {
		if (type->root == holder->get_void())return llvm_Type::getVoidTy(context);
		holder->logger->semerr("'bit_t' has to be used with '[(number)]'.");
		return nullptr;
	}
	else if (type->type == TypeType::has_size_t) {
		if (is_bit_t(type->root,holder)) {
			if (type->root==holder->get_float_bit_t()) {
				if (type->size == 32) {
					return llvm_Type::getFloatTy(context);
				}
				else if(type->size==64){
					return llvm_Type::getDoubleTy(context);
				}
				else {
					holder->logger->semerr("On LLVM API, float value has to have 32 or 64 bit.");
					return nullptr;
				}
			}
			if (llvm_IntType::MAX_INT_BITS < type->size) {
				holder->logger->semerr("On LLVM API,max integer bit is 16777215, but larger number was passed.");
				return nullptr;
			}
			if (type->size == 0) {
				holder->logger->semerr("variable length bit field is not exist on this language.");
				return nullptr;
			}
			return llvm_Type::getIntNTy(context, (unsigned int)type->size);
		}
		else {
			llvm_Type* ret = nullptr;
			auto elm = make_type(type->root);
			if (!elm)return nullptr;
			if (type->size == 0) {
				ret =llvm_VectorType::get(elm,1,true);
			}
			else {
				ret = llvm_ArrayType::get(elm,type->size);
			}
			return ret;
		}
	}
	else if (type->type==TypeType::pointer_t||type->type==TypeType::reference_t) {//TODO:search how to use reference type like C++
		auto base = make_type(type->root);
		if (!base)return nullptr;
		return base->getPointerTo();
	}
	else if(type->type==TypeType::function_t){
		return make_functype(type);
	}
	else if (type->type==TypeType::simple_alias_t) {
		return make_type(type->root);
	}
#endif
}

llvm_wrap::llvm_FuncType* llvm_wrap::LLVM_Converter::make_functype(compiler::Type* type) {
#if !_EXIST_LLVM
	return nullptr;
#else
	if (!type)return nullptr;
	if (type->type != TypeType::function_t)return nullptr;
	auto rettype = make_type(type->root);
	if (!rettype)return nullptr;
	
#endif
}

llvm_wrap::llvm_Function* llvm_wrap::LLVM_Converter::make_function(Identifier* id) {
#if !_EXIST_LLVM
	return nullptr;
#else
	
	
#endif
}

llvm_wrap::llvm_AllocaInst* llvm_wrap::LLVM_Converter::make_local_variable(Identifier* id) {
#if !_EXIST_LLVM
	return nullptr;
#else

#endif
}

llvm_wrap::llvm_Constant* llvm_wrap::LLVM_Converter::make_constant_value(compiler::SyntaxTree* tree) {
#if !_EXIST_LLVM
	return nullptr;
#else

#endif
}

LLVM* compiler::convert_to_llvm(IdHolder* holder) {
#if !_EXIST_LLVM
	return nullptr;
#else
	llvm::ScopedFatalErrorHandler scope(llvm_wrap::_when_err);
	try {

	}
	catch (llvm_wrap::llvm_Err& e) {
		holder->logger->semerr_val("LLVM error:*",e.reason.c_str());
		return nullptr;
	}
	catch (...) {
		holder->logger->syserr("LLVM system is broken.");
		return nullptr;
	}
#endif
}