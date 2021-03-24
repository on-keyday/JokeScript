/*license*/
#pragma once
#include"common/structs.h"
#include"common/filereader.h"

namespace PROJECT_NAME {
	namespace user_tools {
		enum class JSONType {
			null,
			number,
			boolean,
			string,
			array,
			object,
			pair
		};

		struct JSON;

		struct JSONNode {
		private:
			common::EasyVectorP<JSONNode*> pair;
			char* _name = nullptr;
			JSONType _type = JSONType::null;
			JSON* _on=nullptr;
		public:
			JSON* parent() const;
			const char* name() const;
			JSONType type() const;
			JSONNode() = delete;
			JSONNode(const char* name, JSONType type,JSON* on);
			JSONNode* idx(uint64_t n)const;
			uint64_t len()const;
			uint64_t cap()const;
			bool unuse();
			bool pack();
			JSONNode& add(JSONNode* node);
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
				auto str = std::to_string(num);
				auto ret = common::create<JSONNode>(str.c_str(), JSONType::number,this);
				if (!ret)return nullptr;
				ret->unuse();
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
			uint64_t this_size() const;
			~JSON();
		};

		
		/*
		JSONNode* print_types(JSON* json,compiler::IdHolder* holder, std::map<compiler::Type*, uint64_t>& idmap);

		JSONNode* print_ids(JSON* json, compiler::IdHolder* holder, std::map<compiler::Type*, uint64_t>& idmap);

		JSONNode* print_trees(JSON* json, compiler::IdHolder* holder, std::map<compiler::Type*, uint64_t>& idmap);



		JSONNode* print_Type(JSON* json,compiler::Type* type,std::map<compiler::Type*,uint64_t>& idmap);
		JSONNode* print_TypeType(JSON* json,compiler::TypeType ttype);

		JSONNode* print_Identifier(JSON* json, compiler::Identifier* id, std::map<compiler::Type*, uint64_t>& idmap);

		JSONNode* print_SyntaxTree(JSON* json,compiler::SyntaxTree* tree, std::map<compiler::Type*, uint64_t>& idmap);
		JSONNode* print_TreeType(JSON* json,compiler::TreeType ttype);
		*/
	}
}