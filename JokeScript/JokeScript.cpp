/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#define DLL __declspec(dllexport)
#include"JokeScript.h"
#include"JokeScriptLoader.h"
#include"JokeScriptType.h"
#include<iostream>

using namespace jokescript;

struct jokescript::JokeCompiler {
	JokeLogger* logger;
	JokeFile* file;
	JokeDefinitionList* defs;
	JokeBlockList* blocks;
};

JokeCompiler* CCNV jokescript::CreateJokeCompiler() {
	JokeCompiler* ret = nullptr;
	try {
		ret = new JokeCompiler;
	}
	catch (...) {
		return nullptr;
	}
	ret->blocks = nullptr;
	ret->defs = nullptr;
	ret->file = nullptr;
	ret->logger = nullptr;
	return ret;
}

int CCNV jokescript::DeleteJokeCompiler(JokeCompiler* joke) {
	if (!joke)return 0;
	delete joke->blocks;
	delete joke->defs;
	delete joke->file;
	delete joke->logger;
	delete joke;
	return 1;
}

int CCNV jokescript::JokeCompiler_Load(JokeCompiler* joke, const char* filename) {
	if (!joke || !filename)return 0;
	if (!joke->logger) {
		try {
			joke->logger = new JokeLogger;
		}
		catch (...) {
			return 0;
		}
	}
	auto loaded = LoadJoke(filename,joke->logger);
	if (!loaded) {
		return 0;
	}
	joke->file = loaded;
	joke->logger->file = loaded;
	return ParseJoke(loaded,joke->logger);
}

int CCNV jokescript::JokeCompiler_Compile(JokeCompiler* joke) {
	if (!joke)return 0;
	joke->defs = CreateJokeDefinitionList(joke->file, joke->logger);
	joke->blocks = SetBuiltInType(joke->defs, joke->logger);
	unsigned long long i = 0, j = 0;
	auto info = ParseTypedef(i, j, joke->defs,joke->blocks,joke->logger);
	i = 0;
	while (joke->defs->types[i]) {
		auto f = joke->defs->types[i];
		std::cout << f->name << "\n";
		i++;
	}
	std::cout << "\n";
	i = 0;
	while (joke->blocks->current->types[i]) {
		auto f = joke->blocks->current->types[i];
		std::cout << f->name << "\n";
		i++;
	}
	return 1;
}