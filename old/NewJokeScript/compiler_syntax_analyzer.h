/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"compiler_filereader.h"
#include"compiler_identifier_holder.h"

namespace PROJECT_NAME {
	namespace compiler {

		SyntaxTree* comma(IdHolder* holder, Reader* reader);

		SyntaxTree* assign(IdHolder* holder,Reader* reader);

		inline SyntaxTree* binary(IdHolder* holder,Reader* reader);

		template<class Now, class... After>
		SyntaxTree* binary(IdHolder* holder,Reader* reader,Now&& now, After&&... afters) {
			SyntaxTree* ret = binary(holder,reader, afters...), * tmptree = nullptr;
			bool ok = false;
			if (!ret)return nullptr;
			if (reader->eof()) {
				return ret;
			}
			while (1) {
				for (auto symbol : now) {
					if (reader->ahead(symbol)) {
						if (strlen(symbol) == 1) {
							if (!reader->expect_p1(symbol, symbol[0])) {
								continue;
							}
						}
						else {
							reader->expect(symbol);
						}
						tmptree = holder->make_tree(common::StringFilter()=symbol);
						if (!tmptree)return nullptr;
						tmptree->left = ret;
						ret = tmptree;
						tmptree = binary(holder,reader, afters...);
						if (!tmptree) {
							return nullptr;
						}
						ret->right = tmptree;
						tmptree = nullptr;
						ok = true;
						break;
					}
				}
				if (!ok)break;
				ok = false;
			}
			return ret;
		}

		SyntaxTree* unary(IdHolder* holder,Reader* reader);
		SyntaxTree* single(IdHolder* holder,Reader* reader);
		SyntaxTree* match(IdHolder* holder, Reader* reader);

		bool check_semicolon(IdHolder* holder, Reader* reader);
	}
}