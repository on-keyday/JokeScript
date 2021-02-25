/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"compiler_filereader.h"
#include"compiler_identifier_holder.h"
#include"compiler_id_analyzer.h"

namespace PROJECT_NAME {
	namespace compiler {
		

		using StrSet = common::SimplePair<const char*,bool>;

		bool program(IdHolder* holder,Reader* reader);

		SyntaxTree* block(IdHolder* holder, Reader* reader);

		bool block_detail(IdHolder* holder,Reader* reader);

		SyntaxTree* comma(IdHolder* holder, Reader* reader);

		SyntaxTree* assign(IdHolder* holder,Reader* reader);

		inline SyntaxTree* binary(IdHolder* holder,Reader* reader);

		template<class Now, class... After>
		SyntaxTree* binary(IdHolder* holder,Reader* reader,Now&& now, After&&... afters) {
			SyntaxTree* ret = binary(holder,reader, afters...), * tmptree = nullptr;
			if (!ret)return nullptr;
			bool ok = false;
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
						tmptree = holder->make_tree(common::StringFilter()=symbol,TreeType::bin,nullptr);
						if (!tmptree)return nullptr;
						tmptree->left = ret;
						ret = tmptree;
						tmptree = binary(holder,reader, afters...);
						if (!tmptree)return nullptr;
						ret->right = tmptree;
						if (symbol == true) {
							ret->type = get_derived(TypeType::has_size_t, 1, holder->get_bit_t(false), holder);
							if (!ret->type)return nullptr;
						}
						else {
							ret->type = typecmp(ret->left->type, ret->right->type, holder);
							if (!ret->type) {
								holder->logger->semerr("types not assignable.");
								return nullptr;
							}
						}
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
		SyntaxTree* call(SyntaxTree* func,IdHolder* holder, Reader* reader);
		SyntaxTree* member(SyntaxTree* base,IdHolder* holder,Reader* reader);

		SyntaxTree* loop(IdHolder* holder,Reader* reader);
		SyntaxTree* ifs(IdHolder* holder,Reader* reader);

		bool set_relative(Block* block,SyntaxTree* tree);
		bool check_semicolon(IdHolder* holder, Reader* reader);
		bool break_usable(IdHolder* holder);
		bool return_usable(IdHolder* holder);
		SyntaxTree* make_block_tree_pair(IdHolder* holder,const char* name);
		bool block_cycle(IdHolder* holder, Reader* reader);
	}
}