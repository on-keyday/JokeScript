/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#ifdef _WIN32
#define dll_property __declspec(dllexport)
#endif
#include"JokeScript.h"
#include"compiler_identifier_holder.h"
#include"compiler_syntax_analyzer.h"
//#include"compiler_tree_to_llvm.h"
#include"compiler_filereader.h"
#include"user_tools.h"
#include"user_action.h"


using namespace jokescript;

struct jokescript::Instance {
	compiler::Reader* reader;
	compiler::IdHolder* holder;
	char* printed;
};

//std::ofstream file("Type.json");
void print(const char* s) {
	std::cout << s;
}

Instance* ccnv jokescript::make_instance() {
	auto ret = common::create<Instance>();
	if (!ret)return nullptr;
	ret->holder = nullptr;
	ret->reader = nullptr;
	ret->printed = nullptr;
	return ret;
}

void ccnv jokescript::delete_instance(Instance* ins) {
	if (!ins)return;
	free(ins->printed);
	common::kill(ins->holder->logger);
	common::kill(ins->holder);
	common::kill(ins->reader);
	common::kill(ins);
	return;
}

int ccnv jokescript::set_option(Instance* ins, const char* opt, const char* value) {
	if (!ins||!opt||!value)return 0;
	if (strcmp(opt,"-s")==0) {
		if (ins->reader || ins->holder)return 0;
		auto hold = common::create<log::Log>();
		if (!hold)return 0;
		auto reader = common::create<compiler::Reader>(value,hold);
		auto holder = common::create<compiler::IdHolder>();
		if (!reader||!holder)goto Err;
		if (!holder->make_block())goto Err;
		holder->logger = hold;
		ins->reader = reader;
		ins->holder = holder;
		goto Fin;
	Err:
		common::kill(hold);
		common::kill(reader);
		common::kill(holder);
		return 0;
	}
Fin:
	return 1;
}

int ccnv jokescript::parse(Instance* ins) {
	if (!ins)return 0;
	if (!ins->holder || !ins->reader)return 0;
	if (!compiler::program(ins->holder, ins->reader))return 0;
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
	print(to_string(inst));
	delete_instance(inst);
	print("\n");
	return 0;
}

const char* ccnv jokescript::to_string(Instance* ins) {
	if (!ins)return nullptr;
	if (!ins->holder)return nullptr;
	user_tools::JSON json;
	std::map<compiler::Type*,uint64_t> idmap;
	auto types = user_tools::print_types(&json, ins->holder,idmap);
	if (!types)return nullptr;
	auto trees = user_tools::print_trees(&json, ins->holder,idmap);
	if (!trees)return nullptr;
	auto ids = user_tools::print_ids(&json, ins->holder, idmap);
	if (!ids)return nullptr;
	auto res = json.make_obj();
	if (!res)return nullptr;
	res->add(json.make_pair("types", types)).add(json.make_pair("ids",ids)).add(json.make_pair("trees", trees));
	common::String retstr;
	json.get_nodestr(res, retstr, 0, 2);
	auto ret = retstr.get_raw_z();
	if (!ret)return nullptr;
	free(ins->printed);
	ins->printed = ret;
	OutDebugMemoryInfo(ShowGraph();)
	return ret;
}


/*int ccnv jokescript::print_types(Instance* ins, void(*printer)(const char*)) {
	if (!ins || !printer)return 0;
	auto& types = ins->holder->get_types();
	user_tools::JSON json;
	auto nodes = json.make_array();
	if (!nodes)return 0;
	auto i = 0ull;
	std::map<compiler::Type*,uint64_t> idmap;
	idmap.emplace(nullptr,0);
	while (types[i]) {
		idmap.emplace(types[i],i+1);
		i++;
	}
	i = 0;
	while (types[i]) {
		auto hold = user_tools::print_Type(&json, types[i], idmap);
		if (!hold)return 0;
		nodes->add(hold);
		i++;
	}
	auto result=json.make_obj();
	if (!result)return 0;
	result->add(json.make_pair("types",nodes));
	result->add(json.make_pair("count", json.make_number(nodes->len())));
	common::EasyVector<char> hold;
	json.get_nodestr(result,hold, 0, 2);
	return 1;
}

int ccnv jokescript::print_trees(Instance* ins, void(*printer)(const char*)) {
	if (!ins || !printer)return 0;
	auto trees = ins->holder->get_current();
	if (!trees)return 0;
	user_tools::JSON json;
	auto nodes = json.make_array();
	if (!nodes)return 0;
	auto i = 0ull;
	while (trees->trees[i]) {
		auto hold = user_tools::print_SyntaxTree(&json, trees->trees[i]);
		if (!hold)return 0;
		nodes->add(hold);
		i++;
	}
	auto result = json.make_obj();
	if (!result)return 0;
	result->add(json.make_pair("trees", nodes));
	result->add(json.make_pair("count", json.make_number(nodes->len())));
	json.print_node(result, printer, 0, 2);
	return 1;
}*/