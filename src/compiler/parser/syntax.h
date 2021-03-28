/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#include"../data/identifier.h"
#include"../../common/structs.h"
#include"../../common/filereader.h"
#include<initializer_list>

namespace PROJECT_NAME {
	namespace syntax {

		struct Expect {
			common::EasyVectorP<const char*> chars;
			Expect(std::initializer_list<const char*> init) {
				chars.add_copy(init.begin(), init.size());
			}
			Expect& operator=(const Expect& e) {
				this->chars.add_copy(e.chars.get_const(), e.chars.get_size());
				return *this;
			}

			Expect& operator=(std::nullptr_t) {
				this->chars.unuse();
				return *this;
			}
		};

		struct Expects {
			common::EasyVectorP<Expect> expects;
			Expects(std::initializer_list<Expect> init) {
				expects.add_copy_ref(init.begin(), init.size());
			}
			Expect* begin(){
				return expects.begin();
			}
			Expect* end() {
				return expects.end();
			}
		};

		
		template<class Char,class Reader,class TreeMaker,class TreeType,class ExpectList>
		struct Parser {
		private:
			TreeMaker* maker=nullptr;
			Reader* reader = nullptr;
			ExpectList* begin = nullptr;
			ExpectList* end = nullptr;
			ExpectList* current = nullptr;
			
			bool move_list(bool forward=true) {
				if (forward) {
					if (current == end)return false;
					current = current+1;
				}
				else {
					if (current == begin)return false;
					current = curren - 1;
				}
				return true;
			}

			TreeType* invoke_bin() {
				if (move_list()) {
					ret = binary();
				}
				else {
					ret = unary()
				}
			}

			TreeType* binary() {
				TreeType* ret = invoke_bin();
				if (!ret)return nullptr;
				bool ok = false;
				while (true) {
					if (current) {
						auto& expects = *current;
						for (auto now : expects) {
							if (reader->expect(now)) {
								TreeType* tree = invoke_bin();
								if (!tree)return nullptr;
								ret = maker->binary(now, ret, tree);
								if (!ret)return nullptr;
								ok = true;
								break;
							}
						}
					}
					if (ok) {
						ok = false;
						continue;
					}
					break;
				}
				move_list(false);
				return ret;
			}

			TreeType* unary() {

			}
		public:
			bool set(TreeMaker* maker, Reader* reader, ExpectList* begin, ExpectList* end) {
				this->maker = maker;
				this->reader = reader;
				this->begin = begin;
				this->end = end;
				return true;
			}

			bool parse() {
				current = begin;
			}
		};

		struct ReaderOverWrap {
			io::Reader* reader;
			bool expect();
			bool identifier();
			bool brackets(bool end=false);
		};


		identifier::SyntaxTree* parse_syntax(io::Reader*,identifier::Maker*);

		using BasicParser = Parser<char,ReaderOverWrap,identifier::Maker,identifier::SyntaxTree,Expects>;

		BasicParser* make_parser();
	}
}