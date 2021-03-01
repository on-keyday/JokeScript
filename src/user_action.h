/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/


#include"common_structs.h"
#include"user_tools.h"

namespace PROJECT_NAME {
	namespace interactive {
		bool json_assign(std::map<std::string, user_tools::JSONNode*>& idmap, user_tools::JSON& json, compiler::Reader& reader);
		bool json_str(std::map<std::string, user_tools::JSONNode*>& idmap, user_tools::JSON& json, common::String& buf, compiler::Reader& reader, bool end);
		const char* json_help();
		int json_reader();
	}
	namespace command {

	}
}