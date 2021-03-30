/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"../../common/structs.h"
#include"../../common/filereader.h"
#include"parser.h"

namespace PROJECT_NAME {
	namespace syntax {
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

			~Expect() {
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
			Expect* begin() {
				return expects.begin();
			}
			Expect* end() {
				return expects.end();
			}

			Expect& operator[](size_t pos) {
				return expects.idx_ref(pos);
			}
		};

		identifier::SyntaxTree* parse_syntax(io::Reader*, identifier::Maker*);

		struct ReaderOverWrap : parser::IReader<char>{
			io::Reader* reader=nullptr;
			bool expect(const char* str);
			bool unary(bool& err);
			bool brackets(bool begin);
			bool keyword();
			bool number();
			bool string();
			bool identifier();
			bool after();
		};

		struct MakerOverWrap : parser::IMaker<identifier::SyntaxTree, char, Expect, ReaderOverWrap> {
			identifier::Maker* maker=nullptr;
			identifier::SyntaxTree* binary(const char* symbol, identifier::SyntaxTree* left, identifier::SyntaxTree* right, Expect* flags);
			identifier::SyntaxTree* unary(ReaderOverWrap* reader, identifier::SyntaxTree* tree);
			identifier::SyntaxTree* keyword(ReaderOverWrap* reader);
			identifier::SyntaxTree* number(ReaderOverWrap* reader);
			identifier::SyntaxTree* string(ReaderOverWrap* reader);
			identifier::SyntaxTree* identifier(ReaderOverWrap* reader);
			identifier::SyntaxTree* after(ReaderOverWrap* reader);
			

			bool assignable(identifier::SyntaxTree* tree);
		};

		using BasicParser = parser::Parser<char, ReaderOverWrap, MakerOverWrap, identifier::SyntaxTree, Expect>;

		BasicParser* make_parser(io::Reader* reader, identifier::Maker* maker);
	}
}

