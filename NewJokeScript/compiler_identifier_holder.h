/*license*/
#pragma once
#include"log.h"
#include"compiler_filereader.h"
#include"common_tools.h"

namespace jokescript {
	namespace compiler {
		struct Identifier;
		struct SyntaxTree;
		struct Type;

		struct Block {
			Block* parent=nullptr;
			common::EasyVector<Type*> types;
			common::EasyVector<Identifier*> ids;
		};

		struct SyntaxTree {
			char* symbol=nullptr;
			SyntaxTree* left=nullptr;
			SyntaxTree* right=nullptr;
			common::EasyVector<SyntaxTree*> children;
			Identifier* rel = nullptr;
			Block* depends = nullptr;
		};

		enum class TypeType {
			unset,

			//base types
			bit_t,
		
			//derived types
			has_size_t, //for string and arrays
			pointer_t,
			referrence_t,
			struct_t,

			//function types
			function_t,

			//function class types
			interface_t,

			//template types
			template_f_t,
			template_s_t,
			template_p_t,
			instance_t,

			//other types
			enum_t,
			simple_alias_t
		};


		struct Type {
			char* name=nullptr;
			TypeType type=TypeType::unset;
			common::EasyVector<Identifier*> ids;
			common::EasyVector<Type*> types;
			Type* root = nullptr;
			bool self_pointer = false;
			bool on_block = false;
			union {
				unsigned long long size = 0;
				unsigned long long len;
			};
			union {
				Block* block=nullptr;
				Type* type;
			}depends;
			common::EasyVector<Type*> derived;
		};
		
		struct Identifier {
			char* name = nullptr;
			bool is_const=false;
			bool on_block = false;
			Type* type=nullptr;
			union {
				Block* block = nullptr;
				Type* type;
			}depends;
		};

		struct IdHolder {
		private:
			ReadStatus status = {0};
			common::EasyVector<Type*> types;
			Block* current = nullptr;
		public:
			log::Log* logger=nullptr;
			Block* make_block();
			bool to_parent_block();
			Block* make_tmpblock();
			bool break_tmpblock();
			Block* get_current();
			SyntaxTree* make_tree(char* symbol);
			Type* make_type(char* symbol);
			Type* get_bit_t(bool signeds);
			ReadStatus* get_status();
			common::tools::Hash hash;
		};

		template<class T>
		T* create() {
			T* ret = nullptr;
			try {
				ret = new T;
			}
			catch (...) {
				return nullptr;
			}
			return ret;
		}
	}
}