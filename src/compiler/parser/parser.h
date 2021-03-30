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
	namespace parser {
		/*/
		struct Expect {
			common::EasyVectorP<const char*> chars;
			bool boolean = false;
			bool assign = false;
			Expect(std::initializer_list<const char*> init) {
				chars.add_copy(init.begin(), init.size());
			}
			Expect& operator=(const Expect& e) {
				this->chars.add_copy(e.chars.get_const(), e.chars.get_size());
				return *this;
			}

			~Expect(){
				chars.unuse();
			}
		};

		struct Expects {
		private:
			common::EasyVectorP<Expect> expects;
		public:
			Expects(std::initializer_list<Expect> init) {
				expects.add_copy_ref(init.begin(), init.size());
			}
			Expect* begin(){
				return expects.begin();
			}
			Expect* end() {
				return expects.end();
			}

			Expect& operator[](size_t pos) {
				return expects.idx_ref(pos);
			}
		};*/
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
					current++;
				}
				else {
					if (current == begin)return false;
					current--;
				}
				return true;
			}

			bool to_begin() {
				current = begin;
				return true;
			}

			bool to_end() {
				current = end;
				return true;
			}

			TreeType* invoke_bin() {
				if (move_list()) {
					 return binary();
				}
				else {
					return  unary();
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
								ret = maker->binary(now, ret, tree,current);
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
				TreeType* ret = nullptr;
				while (true) {
					bool err = false;
					auto res=reader->unary(err);
					if (err)return nullptr;
					if (res) {
						auto tree = unary();
						if (!tree)return nullptr;
						ret = maker->unary(reader,ret);
						if (!ret)return nullptr;
						continue;
					}
					else {
						ret = primary();
					}
					break;
				}
				return ret;
			}

			TreeType* primary() {
				TreeType* ret = nullptr;
				if (reader->brackets(true)) {
					to_begin();
					ret=binary();
					to_end();
					if (!ret)return nullptr;
					if (!reader->brackets(false))return nullptr;
				}
				else if (reader->keyword()) {
					ret = maker->keyword(reader);
				}
				else if (reader->number()) {
					ret = maker->number(reader);
				}
				else if (reader->string()) {
					ret = maker->string(reader);
				}
				else if(reader->identifier()){
					ret = maker->idnentifier(reader);
				}
				else {
					return nullptr;
				}
				if (!ret)return nullptr;
				while (true) {
					if (reader->after()) {
						ret = maker->after(reader);
						if (!ret)return nullptr;
						continue;
					}
					break;
				}
				return ret;
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


		template<class Tree,class Char,class Expects,class Reader>
		struct IMaker {
			virtual Tree* binary(const Char*,Tree*,Tree*,Expects*) = 0;
			virtual Tree* unary(Reader*,Tree*) = 0;
			virtual Tree* keyword(Reader*) = 0;
			virtual Tree* number(Reader*) = 0;
			virtual Tree* string(Reader*) = 0;
			virtual Tree* identifier(Reader*) = 0;
			virtual Tree* after(Reader*) = 0;
		};

		template<class Char>
		struct IReader {
			virtual bool expect(const Char*) = 0;
			virtual bool unary(bool&) = 0;
			virtual bool brackets(bool) = 0;
			virtual bool keyword() = 0;
			virtual bool number() = 0;
			virtual bool string() = 0;
			virtual bool identifier() = 0;
			virtual bool after() = 0;
		};

		/*
		struct ReaderOverWrap {
			io::Reader* reader;
			bool expect();
			bool identifier();
			bool brackets(bool end=false);
		};

		struct MakerOverWrap : IMaker<identifier::SyntaxTree,char,Expect,ReaderOverWrap> {
			identifier::Maker* maker;
			identifier::SyntaxTree* binary(const char* op,identifier::SyntaxTree* left,identifier::SyntaxTree* right,Expect* flags);
		};
		*/
	}
}