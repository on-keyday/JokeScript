/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#ifndef dll_property
#ifdef _WIN32
#define dll_property __declspec(dllimport)
#else
#define dll_property
#endif
#endif

#ifndef ccnv
#ifdef _WIN32
#define ccnv __stdcall
#else
#define ccnv
#endif
#endif

#ifdef __cplusplus
namespace jokescript {
	extern "C" {
#endif
		struct Instance;
		
		dll_property Instance* ccnv make_instance();
		dll_property void ccnv delete_instance(Instance* ins);

		dll_property int ccnv set_callback(Instance* ins, void(*msgh)(const char*));
		dll_property int ccnv set_option(Instance* ins,const char* opt,const char* value);

		dll_property int ccnv get_option(Instance* ins,const char* opt);

		dll_property int ccnv output(const char* outname,unsigned int type);

		dll_property int ccnv compile(Instance* ins);

		dll_property int ccnv jit_setup(Instance *ins);

		typedef int (*main_proc)();

		dll_property main_proc ccnv get_program_main(Instance* ins);

		dll_property int ccnv compiler_main(int argc,char** argv);

		dll_property int ccnv print_types(Instance* ins,void(*printer)(const char*));

		dll_property int ccnv print_trees(Instance* ins, void(*printer)(const char*));
#ifdef __cplusplus
	}
}
#endif