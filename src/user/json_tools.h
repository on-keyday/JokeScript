/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once
#include"../common/stdcpps.h"
#include"../common/structs.h"
#include"../common/filereader.h"
#include<vector>
//#include"compiler_identifier_holder.h"

namespace PROJECT_NAME {
	namespace json_tools {
		enum class JSONType {
			null,
			number,
			number_f,
			boolean,
			string,
			array,
			object
		};

		struct JSON_detail;

		struct JSONMaker;

		struct JSON {
		private:
			JSON_detail* p=nullptr;
			char* hold=nullptr;
			static JSON invalid;
		public:
			JSON() {}

			JSON(JSON&& from) {
				if (from.p != this->p) {
					this->p = from.p;
					from.p = nullptr;
				}
			}
			JSON& operator[](const char* name);
			JSON& operator[](uint64_t i);
			operator const char* ();
		};
		
		/*
		struct JSONNode {
		private:
			char* _name = nullptr;
			union {
				void* null = nullptr;
				common::EasyVectorP<JSONNode*> set;
				bool flag;
				int64_t num;
				double num_f;
				char* str;
			};
			JSONType _type = JSONType::null;
			JSON* _on=nullptr;
		public:
			JSON* parent() const;
			const char* name() const;
			JSONType type() const;
			JSONNode() = delete;
			JSONNode(JSONType type, const char* name=nullptr);
			JSONNode(const char* str,JSON* on,const char* name=nullptr);
			JSONNode(bool flag,JSON* on, const char* name = nullptr);
			JSONNode(int64_t num, JSON* on, const char* name = nullptr);
			JSONNode(double num, JSON* on, const char* name = nullptr);
			JSONNode* idx(uint64_t n)const;
			uint64_t len()const;
			//uint64_t cap()const;
			//bool unuse();
			//bool pack();
			JSONNode* add(JSONNode* node);
			~JSONNode();
		};

		struct JSON {
		private:
			common::EasyVector<JSONNode*> nodes;
			//common::EasyVector<JSONNode*> root;

			JSONNode* make_JSON_detail(io::Reader* reader);
			bool line(int has_line, common::String& str) const;

			
		public:
			bool init();
			//bool add_root(JSONNode* node);
			JSONNode* make_null();
			JSONNode* make_bool(bool b);
			template<class T>
			JSONNode* make_number(T num) {
				if (num > 0x7F'FF'FF'FF'FF'FF'FF'FF)return nullptr;
				auto ret = common::create<JSONNode>(num,this);
				if (!ret)return nullptr;
				this->nodes.add(ret);
				return ret;
			}
			JSONNode* make_number(const char* num);
			JSONNode* make_string(const char* str);
			JSONNode* make_obj();
			JSONNode* make_array();
			JSONNode* make_pair(const char* name,JSONNode* node);

			JSONNode* get_value(JSONNode* obj,const char* name) const;
			JSONNode* get_value(JSONNode* array,uint64_t index) const;

			JSONNode* get_nodes(JSONNode* base,const char* path) const;

			//bool print_nodes(void(*printer)(const char*));
			//bool get_nodesstr(common::EasyVector<char>& str);
			bool print_node(JSONNode* node,void(*printer)(const char*), int has_line, int ofs) const;
			bool get_nodestr(JSONNode* node,common::String& str,int has_line,int ofs) const;

			JSONNode* make_JSON(const char* jsonstr);
			JSONNode* make_JSON_file(const char* filename);

			void remove(JSONNode* node);//dangerous
			//uint64_t this_size() const;
			~JSON();
		};*/

		
	}
}