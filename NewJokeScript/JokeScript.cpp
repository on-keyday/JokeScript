#include"JokeScript.h"
#include"compiler_identifier_holder.h"
#include"compiler_syntax_analyzer.h"
#include"compiler_tree_to_llvm.h"
#include"compiler_filereader.h"

using namespace jokescript;

struct Instance {
	compiler::Reader* s;
	compiler::IdHolder* holder;
};

int ccnv compiler_main(int argc, char** argv) {
	compiler::Reader reader("joke.jok");
	
	return 0;
}