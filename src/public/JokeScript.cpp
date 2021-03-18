/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#ifdef _WIN32
#define dll_property __declspec(dllexport)
#endif
#include"../common/stdcpps.h"
#include"JokeScript.h"
#include"../common/filereader.h"
#include"../user/json_tools.h"
#include"../user/user_action.h"

using namespace jokescript;

struct CompilerOpts {
	bool use_llvm;
};

struct jokescript::Instance {
	CompilerOpts opts;
	io::Reader* reader;
	//compiler::IdHolder* holder;
	bool parsed;
	char* printed;
	union {
		//compiler::LLVM* llvm;

	}result;
};


void print() {}

template<class T,class... Other>
void print(T s,Other... other) {
	std::cout << s;
	print(other...);
}

Instance* ccnv jokescript::make_instance() {
	auto ret = common::create<Instance>();
	if (!ret)return nullptr;
	ret->opts = { 0 };
	ret->reader = nullptr;
	ret->printed = nullptr;
	ret->parsed = false;
	return ret;
}

void ccnv jokescript::delete_instance(Instance* ins) {
	if (!ins)return;
	common::free(ins->printed);
	common::kill(ins->reader);
	common::kill(ins);
	return;
}

int ccnv jokescript::set_option(Instance* ins, const char* opt, const char* value) {
	if (!ins||!opt||!value)return 0;
	if (strcmp(opt,"-s")==0) {
		auto hold = common::create<log::Log>();
		if (!hold)return 0;
		auto reader = common::create<io::Reader>(value,hold);
		ins->reader = reader;
		goto Fin;
		common::kill(hold);
		common::kill(reader);
		return 0;
	}
Fin:
	return 1;
}

int ccnv jokescript::parse(Instance* ins) {
	if (!ins)return 0;
	return 1;
}

int ccnv jokescript::compile(Instance* ins) {
	if (!ins)return 0;
	if (!ins->parsed)return 0;
	return 1;
}

int ccnv jokescript::compiler_main(int argc, char** argv) {
	const char* name = nullptr;
	if (argc >= 2) {
		name = argv[1];
	}
	else {
		name = "joke.jok";
	}
	auto inst = make_instance();
	if (!inst)return -1;
	if(!set_option(inst, "-s", name))return -2;
	if (!parse(inst))return -3;
	delete_instance(inst);
	print("\n");
	
	OutDebugMemoryInfo(ShowGraph();)
	return 0;
}

