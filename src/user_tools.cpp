/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"user_tools.h"
#include"compiler_ctype.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;
using namespace PROJECT_NAME::user_tools;

user_tools::JSONNode::JSONNode(const char* name,JSONType type,JSON* on) {
	this->_name = common::StringFilter() = name;
	this->_type = type;
	this->_on = on;
}

JSON* user_tools::JSONNode::parent() const {
	return _on;
}

const char* user_tools::JSONNode::name() const {
	return _name;
}

JSONType user_tools::JSONNode::type() const {
	return _type;
}

JSONNode* user_tools::JSONNode::idx(uint64_t n) const{
	return this->pair[n];
}

bool user_tools::JSONNode::unuse() {
	return this->pair.unuse();
}

bool user_tools::JSONNode::pack() {
	return this->pair.pack();
}

user_tools::JSONNode::~JSONNode() {
	free(_name);
}

JSONNode& user_tools::JSONNode::add(JSONNode* node) {
	if (!node)return *this;
	if (node->parent() != this->parent())return *this;
	if (this->type() == JSONType::object) {
		if (node->type() != JSONType::pair)return *this;
		this->pair.add(node);
	}
	else if(this->type()==JSONType::array){
		if (node->type() == JSONType::pair)return *this;
		this->pair.add(node);
	}
	else if (this->type()==JSONType::pair) {
		if (node->type() == JSONType::pair)return *this;
		if (this->idx(0))return *this;
		this->pair.add(node);
	}
	else {
		return *this;
	}
	return *this;
}

uint64_t user_tools::JSONNode::len()const {
	if (this->type() == JSONType::object|| this->type() == JSONType::array) {
		return pair.get_size();
	}
	else {
		return 1;
	}
}

uint64_t user_tools::JSONNode::cap() const {
	return pair.get_cap();
}

user_tools::JSON::~JSON() {
	nodes.remove_each(common::kill);
}


bool user_tools::JSON::init() {
	nodes.remove_each(common::kill);
	return true;
}
/*
bool user_tools::JSON::add_root(JSONNode* node) {
	return root.add_nz(node);
}*/

JSONNode* user_tools::JSON::make_null() {
	auto ret = common::create<JSONNode>("null",JSONType::null,this);
	if (!ret)return nullptr;
	ret->unuse();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_bool(bool b) {
	JSONNode* ret = nullptr;
	if (b) {
		ret = common::create<JSONNode>("true", JSONType::boolean,this);
	}
	else {
		ret = common::create<JSONNode>("false",JSONType::boolean,this);
	}
	if (!ret)return nullptr;
	ret->unuse();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_number(const char* num) {
	if (!num)return nullptr;
	auto ret= common::create<JSONNode>(num, JSONType::number,this);
	if (!ret)return nullptr;
	ret->unuse();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_string(const char* str) {
	if (!str) {
		return make_null();
	}
	auto ret = common::create<JSONNode>(str, JSONType::string,this);
	if (!ret)return nullptr;
	ret->unuse();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_obj() {
	auto ret = common::create<JSONNode>("{}", JSONType::object,this);
	if (!ret)return nullptr;
	ret->pack();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_array() {
	auto ret = common::create<JSONNode>("[]", JSONType::array,this);
	if (!ret)return nullptr;
	ret->pack();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::make_pair(const char* name, JSONNode* node) {
	if (!name || !node)return nullptr;
	if (node->parent() != this)return nullptr;
	if (node->type() == JSONType::pair)return nullptr;
	auto ret = common::create<JSONNode>(name, JSONType::pair,this);
	if (!ret)return nullptr;
	ret->add(node);
	ret->pack();
	this->nodes.add(ret);
	return ret;
}

JSONNode* user_tools::JSON::get_value(JSONNode* obj, const char* name) const{
	if (!obj || !name)return nullptr;
	if (obj->type() != JSONType::object)return nullptr;
	auto i = 0ull;
	while (obj->idx(i)) {
		if (strcmp(obj->idx(i)->name(),name) == 0) {
			return obj->idx(i)->idx(0);
		}
		i++;
	}
	return nullptr;
}

JSONNode* user_tools::JSON::get_value(JSONNode* array, uint64_t index) const{
	if (!array)return nullptr;
	if (array->type() != JSONType::array)return nullptr;
	return array->idx(index);
}

JSONNode* user_tools::JSON::get_nodes(JSONNode* base, const char* path) const{
	if (!base || !path)return nullptr;
	bool prevarray = false;
	bool arraytype = true;
	Reader reader(path, strlen(path));
	JSONNode* ret = base;
	if (reader.ahead("/")) {
		arraytype = false;
	}
	while (!reader.eof()) {
		if (arraytype) {
			if (reader.expect("[")) {
				prevarray = true;
			}
			else if(reader.expect(".")){
				prevarray = false;
			}
			else {
				return nullptr;
			}
		}
		else {
			if (!reader.expect("/"))return nullptr;
			if (reader.eof())break;
		}

		if (reader.ahead("\"")||reader.ahead("'")) {
			auto buf = reader.string(false);
			buf.remove_top();
			buf.remove_end();
			ret = get_value(ret, buf.get_const());
			if (!ret)return nullptr;
		}
		else if (ctype::is_number(reader.abyte())) {
			ReadStatus status = {0};
			reader.readwhile(&status, ctype::reader::DigitNumber);
			if (status.failed)return nullptr;
			char* endp = nullptr;
			auto index = strtoull(status.buf.get_const(), &endp, 10);
			if (*endp != '\0')return nullptr;
			ret = get_value(ret, index);
			if (!ret)return nullptr;
		}
		else if (arraytype&&!prevarray) {
			ReadStatus status = { 0 };
			reader.readwhile(&status, ctype::reader::Identifier);
			if (status.failed)return nullptr;
			if (ctype::is_unnamed(status.buf.get_const()))return nullptr;
			ret = get_value(ret, status.buf.get_const());
			if (!ret)return nullptr;
		}
		else if (!arraytype) {
			ReadStatus status = { 0 };
			status.num = '/';
			reader.readwhile(&status, ctype::reader::Until);
			ret = get_value(ret, status.buf.get_const());
			if (!ret)return nullptr;
		}
		else {
			return nullptr;
		}
		if (prevarray) {
			if (!reader.expect("]"))return nullptr;
		}
	}
	return ret;
}

bool user_tools::JSON::print_node(JSONNode* node,void(*printer)(const char*), int has_line, int ofs) const{
	if (!node||!printer)return false;
	common::EasyVector<char> str;
	get_nodestr(node, str,has_line,ofs);
	printer(str.get_const());
	return true;
}

bool  user_tools::JSON::get_nodestr(JSONNode* node, common::EasyVector<char>& str,int has_line,int ofs) const{
	if (!node)return false;
	if (node->type() == JSONType::string) {
		str.add_copy("\"", 1);
		for (auto c = node->name(); *c;c++) {
			if (*c == '"'||*c=='\\'||*c=='/'||ctype::is_escapable_control(*c)) {
				str.add('\\');
			}
			str.add(*c);
		}
		str.add_copy("\"", 1);
	}
	else if (node->type()==JSONType::object) {
		//line(has_line, str);
		str.add_copy("{", 1);
		auto i = 0ull;
		while (node->idx(i)) {
			if (i != 0) {
				str.add_copy(",", 1);
				
			}
			if (has_line >= 0) {
				str.add('\n');
			}
			line(has_line+ofs, str);
			get_nodestr(node->idx(i),str,has_line<0?-1:has_line+ofs,ofs);
			i++;
		}
		if (i&&has_line >= 0) {
			str.add('\n');
			line(has_line, str);
		}
		str.add_copy("}", 1);
	}
	else if (node->type() == JSONType::array) {
		//line(has_line, str);
		str.add_copy("[", 1);
		auto i = 0ull;
		while (node->idx(i)) {
			if (i != 0) {
				str.add_copy(",", 1);
			}
			if (has_line >= 0)str.add('\n');
			line(has_line+ofs,str);
			get_nodestr(node->idx(i), str, has_line < 0 ? -1 : has_line + ofs,ofs);
			i++;
		}
		if (i&&has_line >= 0) {
			str.add('\n');
			line(has_line, str);
		}
		str.add_copy("]", 1);
	}
	else if (node->type()==JSONType::pair) {
		str.add_copy("\"", 1);
		str.add_copy(node->name(), strlen(node->name()));
		str.add_copy("\":",2);
		get_nodestr(node->idx(0),str, has_line < 0 ? -1 : has_line,ofs);
	}
	else {
		str.add_copy(node->name(), strlen(node->name()));
	}
	return true;
}

bool user_tools::JSON::line(int has_line,common::EasyVector<char>& str) const{
	if (has_line > 0) {
		for (int i = 0; i < has_line; i++)str.add(' ');
	}
	return true;
}

JSONNode* user_tools::JSON::make_JSON(const char* jsonstr) {
	if (!jsonstr)return nullptr;
	compiler::Reader reader(jsonstr, strlen(jsonstr));
	return make_JSON_detail(&reader);
}

JSONNode* user_tools::JSON::make_JSON_file(const char* filename) {
	if (!filename)return nullptr;
	compiler::Reader reader(filename);
	if (reader.eof())return nullptr;
	return make_JSON_detail(&reader);
}

JSONNode* user_tools::JSON::make_JSON_detail(Reader* reader) {
	JSONNode* ret = nullptr;
	if (reader->expect("{")) {
		ret = make_obj();
		if (!ret)return nullptr;
		while (!reader->eof()) {
			if (reader->ahead("\"")) {
				auto buf=reader->string(false);
				if (!reader->expect(":"))return nullptr;
				buf.remove_top();
				buf.remove_end();
				auto obj = make_JSON_detail(reader);
				if (!obj)return nullptr;
				ret->add(make_pair(buf.get_const(),obj));
				if (!reader->expect(",") && !reader->ahead("}"))return nullptr;
			}
			else if (reader->ahead("}")) {
				break;
			}
			else {
				return nullptr;
			}
		}
		if (!reader->expect("}"))return nullptr;
	}
	else if (reader->expect("[")) {
		ret = make_array();
		if (!ret)return nullptr;
		while (!reader->eof()) {
			if (reader->ahead("]")) {
				break;
			}
			else {
				auto hold = make_JSON_detail(reader);
				if (!hold)return nullptr;
				ret->add(hold);
				if (!reader->expect(",") && !reader->ahead("]"))return nullptr;
			}
		}
		if (!reader->expect("]"))return nullptr;
	}
	else if (reader->ahead("\"")) {
		auto buf = reader->string(false);
		if (buf == nullptr)return nullptr;
		buf.remove_top();
		buf.remove_end();
		return make_string(buf.get_const());
	}
	else if (ctype::is_number(reader->abyte())) {
		ReadStatus status = { 0 };
		reader->readwhile(&status, ctype::reader::DigitNumber);
		if (status.failed)return nullptr;
		return make_number(status.buf.get_const());
	}
	else if (reader->expect("null")) {
		return make_null();
	}
	else if (reader->expect("true")) {
		return make_bool(true);
	}
	else if (reader->expect("false")) {
		return make_bool(false);
	}
	return ret;
}

void user_tools::JSON::remove(JSONNode* node) {
	if (!node)return;
	if (node->parent() != this)return;
	for (auto i = 0ull; node->idx(i);i++) {
		remove(node->idx(i));
	}
	common::kill(this->nodes.remove_if(node));
	this->nodes.pack();
	return;
}

uint64_t user_tools::JSON::this_size() const{
	uint64_t ret = sizeof(JSON);
	auto i = 0ull;
	while (nodes[i]) {
		ret += sizeof(JSONNode);
		ret += strlen(nodes[i]->name()) + 1;
		ret += sizeof(JSONNode*) * nodes[i]->cap();
		i++;
	}
	ret += this->nodes.get_cap()*sizeof(JSONNode*);
	return ret;
}

JSONNode* user_tools::print_Type(JSON* json,Type* type, std::map<Type*, uint64_t>& idmap) {
	if (!type)return 0;
	const char* root_s=nullptr;
	if (type->root)root_s = type->root->name;
	auto ret=json->make_obj();
	auto rootinfo = json->make_obj();
	auto types = json->make_array();
	if (!ret || !rootinfo||!types)return nullptr;
	auto i = 0ull;
	while (type->types[i]) {
		auto tmp = json->make_obj();
		if (!tmp)return nullptr;
		tmp->add(json->make_pair("name", json->make_string(type->types[i]->name)))
			.add(json->make_pair("id", json->make_number(idmap[type->types[i]])));
		types->add(tmp);
		i++;
	}
	auto children = json->make_obj();
	if (!children)return nullptr;
	children->add(json->make_pair("types", types));
	rootinfo->add(json->make_pair("name", json->make_string(root_s)))
		.add(json->make_pair("id", json->make_number(idmap[type->root])));
	ret->add(json->make_pair("name", json->make_string(type->name)))
		.add(json->make_pair("id",json->make_number(idmap[type])))
	    .add(json->make_pair("root",rootinfo))
		.add(json->make_pair("children",children))
		.add(json->make_pair("size",json->make_number(type->size)))
		.add(json->make_pair("kind",print_TypeType(json,type->type)));
	return ret;
}

JSONNode* user_tools::print_TypeType(JSON* json,TypeType ttype) {
	if (ttype == TypeType::base_t) {
		return json->make_string("base_t");
	}
	else if(ttype==TypeType::has_size_t){
		return json->make_string("has_size_t");
	}
	else if (ttype==TypeType::pointer_t) {
		return json->make_string("pointer_t");
	}
	else if (ttype==TypeType::referrence_t) {
		return json->make_string("reference_t");
	}
	else if (ttype==TypeType::function_t) {
		return json->make_string("function_t");
	}
	else if (ttype==TypeType::struct_t) {
		return json->make_string("struct_t");
	}
	else if (ttype==TypeType::interface_t) {
		return json->make_string("interface_t");
	}
	else if (ttype == TypeType::enum_t) {
		return json->make_string("enum_t");
	}
	else if (ttype==TypeType::simple_alias_t) {
		return json->make_string("simple_alias_t");
	}
	/*else if (ttype==TypeType::option_t) {
		str.add_copy("option_t",8);
	}*/
	else {
		return json->make_string("unset");
	}
}

JSONNode* user_tools::print_SyntaxTree(JSON* json, compiler::SyntaxTree* tree) {
	if (!tree)return json->make_null();
	JSONNode* ret = json->make_obj();
	if (!ret)return nullptr;
	ret->add(json->make_pair("symbol", json->make_string(tree->symbol)));
	ret->add(json->make_pair("kind", print_TreeType(json, tree->ttype)));
	ret->add(json->make_pair("left", print_SyntaxTree(json, tree->left)));
	ret->add(json->make_pair("right", print_SyntaxTree(json, tree->right)));
	auto arr = json->make_array();
	if (!arr)return nullptr;
	auto i = 0ull;
	while (tree->children[i]) {
		arr->add(print_SyntaxTree(json, tree->children[i]));
		i++;
	}
	ret->add(json->make_pair("children", arr));
	if (tree->ttype==TreeType::ctrl&&tree->depends) {
		auto block = json->make_array();
		if (!block)return nullptr;
		i = 0;
		while (tree->depends->trees[i]) {
			block->add(print_SyntaxTree(json, tree->depends->trees[i]));
			i++;
		}
		ret->add(json->make_pair("block",block));
	}
	return ret;
}

JSONNode* user_tools::print_TreeType(JSON* json, compiler::TreeType ttype) {
	if (ttype == TreeType::bin) {
		return json->make_string("binary");
	}
	else if (ttype==TreeType::unary) {
		return json->make_string("unary");
	}
	else if (ttype==TreeType::literal) {
		return json->make_string("literal");
	}
	else if (ttype==TreeType::defined) {
		return json->make_string("defined");
	}
	else if (ttype==TreeType::ctrl) {
		return json->make_string("control");
	}
	else {
		return json->make_string("unset");
	}
}
