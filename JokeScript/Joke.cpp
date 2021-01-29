/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScript.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace jokescript;

int main(int argc, char** argv) {
	auto s = CreateJokeCompiler();
	JokeCompiler_Load(s, "Joke.jok");
	JokeCompiler_Compile(s);
	DeleteJokeCompiler(s);
	return 0;
}