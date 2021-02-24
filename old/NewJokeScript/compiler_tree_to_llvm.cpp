#include"common_structs.h"
#include"compiler_tree_to_llvm.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;


compiler::LLVM::~LLVM() {
	delete _module;
	delete builder;
}

LLVM* compiler::make_llvm(const char* name) {
	if (!name)return nullptr;
	auto ret=common::create<LLVM>();
	if (!ret)return nullptr;
	ret->_module = common::create<llvm::Module>(llvm::StringRef(name),ret->ctx);
	if (!ret->_module) {
		delete ret;
		return nullptr;
	}
	ret->builder = common::create<llvm::IRBuilder<llvm::LLVMContext>>(ret->ctx);
	if (!ret->builder) {
		delete ret;
		return nullptr;
	}
	return ret;
}

LLVM* compiler::analyze_tree(IdHolder* holder) {
	LLVM* ret = nullptr;
	try {
		ret = make_llvm(holder->hash.get_hash_const());
		if (!ret) {
			holder->logger->syserr("memory is full.");
			return nullptr;
		}
		auto functype = llvm::FunctionType::get(ret->builder->getIntNTy(32),false);
		auto mainfunc = llvm::Function::Create(functype, llvm::Function::ExternalLinkage, "main", ret->_module);
		auto entry = llvm::BasicBlock::Create(ret->ctx, "entrypoint", mainfunc);

		ret->builder->SetInsertPoint(entry);
		ret->builder->CreateRet(ret->builder->getInt64(0));
		
		
		ret->_module->print(llvm::outs(),nullptr);
	}
	catch (...) {
		holder->logger->syserr("error ocurred in llvm library.");
		return nullptr;
	}
	return ret;
}

