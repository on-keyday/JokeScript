/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"log.h"
#include"compiler_filereader.h"
#include"common_tools.h"

namespace PROJECT_NAME {
	namespace compiler {
		struct Identifier;
		struct SyntaxTree;
		struct Type;
		//using Option = Type;


		struct Block {
			Block* prev=nullptr;
			SyntaxTree* reltree = nullptr;
			Identifier* id = nullptr;
			bool has_this=false;
			common::EasyVectorP<Type*> types;
			common::EasyVectorP<Identifier*> ids;
			common::EasyVectorP<SyntaxTree*> trees;
		};


		enum class TreeType{
			unset,
			bin,
			unary,
			literal,
			defined,
			ctrl
		};

		struct SyntaxTree {
			char* symbol=nullptr;
			SyntaxTree* left=nullptr;
			SyntaxTree* right=nullptr;
			common::EasyVectorP<SyntaxTree*> children;
			Identifier* rel = nullptr;
			Block* relblock = nullptr;
			Type* type=nullptr;
			TreeType ttype=TreeType::unset;
			~SyntaxTree();
		};

		enum class TypeType {
			unset,

			//base types
			base_t,
		
			//derived types
			has_size_t, //for string and arrays
			pointer_t,
			reference_t,

			//set types
			struct_t,
			enum_t,
			interface_t,

			//function types
			function_t,
			
			/*/template types
			template_f_t,
			template_s_t,
			template_p_t,
			template_a_t,
			instance_f_t,
			instance_s_t,*/

			//other types
			simple_alias_t,
		};

		enum class OptionType {
			unset,
			vec,
			str,
			boolean,
			type
		};

		struct Option {
			char* name=nullptr;
			OptionType type=OptionType::unset;
			union {
				common::EasyVectorP<Identifier*> ids=nullptr;
				char* value;
				bool flag;
				compiler::Type* this_p;
			};
			~Option();
		};


		struct Type {
			char* name=nullptr;
			TypeType type=TypeType::unset;
			common::EasyVectorP<Identifier*> ids;
			common::EasyVectorP<Type*> types;
			common::EasyVectorP<Option*> opts;
			Type* root = nullptr;
			//bool on_block = false;
			//union {
				uint64_t size = 0;
				//uint64_t len;
			//};
			//union {
				//Block* block=nullptr;
				//Type* type_on=nullptr;
			//};
			common::EasyVectorP<Type*> derived;
			common::EasyVectorP<Type*> function;
			~Type();
		};
		
		struct Identifier {
			char* name = nullptr;
			bool is_const=false;
			bool is_global = false;
			//bool on_block = false;
			Type* type=nullptr;
			//union {
				//Block* block = nullptr;
				//Type* type_on=nullptr;
			//};
			SyntaxTree* init=nullptr;
			common::EasyVectorP<Identifier*> params=nullptr;
			~Identifier();
		};

		struct IdHolder {
		private:
			ReadStatus status = {0};
			common::EasyVectorP<Block*> blocks;
			common::EasyVectorP<Type*> types;
			common::EasyVectorP<Option*> options;
			common::EasyVectorP<Identifier*> ids;
			common::EasyVectorP<SyntaxTree*> trees;
			Block* current = nullptr;
		public:
			IdHolder();
			Type* _this = nullptr;
			log::Log* logger=nullptr;
			Block* make_block();
			bool to_parent_block();
			Block* make_tmpblock();
			bool break_tmpblock();
			Block* get_current();
			SyntaxTree* make_tree(char* symbol,TreeType ttype,Type* type);
			Type* make_type(char* symbol);
			Option* make_option(char* name,char* value,OptionType type);
			Identifier* make_id(char* symbol);
			Type* get_float_bit_t();
			Type* get_bit_t(bool signeds);
			Type* get_void();
			Type* get_string();
			ReadStatus* get_status();
			Option* get_opt(const char* name,const char* value);
			Option* get_opt(const char* name,common::EasyVectorP<Identifier*>& ids);
			Option* get_opt(const char* name, bool flag);
			Option* get_opt(const char* name,Type* this_p);
			common::Hash hash;

			common::EasyVectorP<Type*>& get_types();
			common::EasyVectorP<Identifier*>& get_ids();
			common::EasyVectorP<SyntaxTree*>& get_trees();
			~IdHolder();
		};

		
	}
}