/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#define dll_property __declspec(dllexport)
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
};

//std::ofstream file("Type.json");

void print(const char* s) {
	std::cout << s;
}

int ccnv jokescript::compiler_main(int argc, char** argv) {
	log::Log logger;
	compiler::Reader reader("joke.jok",&logger);
	compiler::IdHolder holder;
	holder.logger = &logger;
	holder.make_block();
	if (!compiler::program(&holder, &reader))return -1;
	Instance inst;
	inst.holder = &holder;
	inst.reader = &reader;
	//print_types(&inst, print);
	//print("\n");
	print_trees(&inst, print);
	return 0;
}



int ccnv jokescript::print_types(Instance* ins, void(*printer)(const char*)) {
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
	//common::EasyVector<char> hold;
	json.print_node(result, printer, 0, 2);
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
}