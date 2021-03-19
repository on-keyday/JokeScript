/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"json_tools.h"
#include"../common/ctype.h"
#include"../common/tools.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::io;
using namespace PROJECT_NAME::json_tools;
namespace PROJECT_NAME {
	namespace json_tools {

		struct JSONNamed {
			char* name = nullptr;
			JSON* obj = nullptr;
			~JSONNamed() {
				common::free(name);
				common::kill(obj);
			}
		};

		struct JSON_detail {
			JSON* parent;
			JSONType type;
			union {
				common::EasyVectorP<JSONNamed*> object = nullptr;
				common::EasyVectorP<JSON*> array;
				double num_f;
				int64_t num_i;
				char* str;
				bool flag;
			};
			JSON_detail(JSON* p) {
				type = JSONType::null;
				parent = p;
			}
			JSON_detail(bool f, JSON* p) {
				type = JSONType::boolean;
				flag = f;
				parent = p;
			}
			JSON_detail(double num, JSON* p) {
				type = JSONType::number_f;
				parent = p;
			}
			template<class T>
			JSON_detail(T num, JSON* p) {
				if (num <= 0x7F'FF'FF'FF'FF'FF'FF'FF) {
					num_i = num;
					type = JSONType::number;
				}
				else {
					type = JSONType::null;
				}
				parent = p;
			}
			JSON_detail(const char* strin, JSON* p) {
				if (!strin) {
					type = JSONType::null;
				}
				else {
					str = common::StringFilter() = strin;
					if (!str) {
						type = JSONType::string;
					}
					else {
						type = JSONType::string;
					}
				}
				parent = p;
			}
			~JSON_detail()
			{
				if (type == JSONType::string) {
					common::free(str);
				}
				else if (type == JSONType::object) {
					object.remove_each(common::kill);
					object.unuse();
				}
				else if (type == JSONType::array) {
					array.remove_each(common::kill);
					array.unuse();
				}
			}
		};
		/*struct JSONMaker{
			static JSON_detail* make_null(JSON*);
			static JSON_detail* make_string(const char* str, JSON*);
			static JSON_detail* make_bool(bool f, JSON*);
			static JSON_detail* make_num(double num, JSON*);
			static JSON_detail* make_num(int64_t num, JSON*);
			static JSON_detail* make_num(const char* num, JSON*);
			static JSON_detail* make_obj();
			static JSON_detail* make_array();

			static bool add(JSON_detail * base, const char* name, JSON_detail * value);
			static bool add(JSON_detail * base, JSON_detail * value);
		};*/
	}
}

JSON JSON::invalid;

JSON_detail* make_null(JSON* p) {
	return common::create<JSON_detail>(p);
}

JSON_detail* make_string(const char* str, JSON* p) {
	return common::create<JSON_detail>(str,p);
}

JSON_detail* make_bool(bool f, JSON* p) {
	return common::create<JSON_detail>(f,p);
}

JSON_detail* make_num(double num, JSON* p) {
	return common::create<JSON_detail>(num,p);
}

JSON_detail* make_num(int64_t num, JSON* p) {
	return common::create<JSON_detail>(num, p);
}

JSON_detail* make_num(const char* num, JSON* p) {
	if (!num)return nullptr;
	int ofs = 0;
	if (num[0] == '-') {
		ofs = 1;
	}
	uint64_t n;
	int bit = 0;
	bool f = false, u = false;
	auto res=ctype::get_number_type(&num[1],n,bit,f,u);
	if (!res||f||(u&&bit>=8)) {
		char* end = nullptr;
		auto fs=strtod(num, &end);
		if (*end != '\0')return nullptr;
		return make_num(fs,p);
	}
	if (ofs) {
		return make_num(-(int64_t)n,p);
	}
	else {
		return make_num((int64_t)n,p);
	}
}

JSON_detail* make_obj(JSON* p) {
	auto ret = make_null(p);
	if (ret) {
		ret->type = JSONType::object;
	}
	return ret;
}

JSON_detail* make_array(JSON* p) {
	auto ret = make_null(p);
	if (ret) {
		ret->type = JSONType::array;
	}
	return ret;
}

bool add(JSON_detail* base, const char* name, JSON* value) {
	if (!base || !name || !value)return false;
	if (base->type != JSONType::object)return false;
	if(!common::check_name_conflict(name, base->object))return false;
	auto ad = common::create<JSONNamed>();
	if (!ad)return false;
	auto s = common::StringFilter() = name;
	if (!s) {
		common::kill(ad);
		return false;
	}
	ad->name = s;
	ad->obj = value;
	base->object.add(ad);
	return true;
}

bool add(JSON_detail* base, JSON* value) {
	if (!base || !value)return false;
	if (base->type != JSONType::array)return false;
	base->array.add(value);
	return true;
}

JSON& JSON::operator[](const char* name) {
	if (this == &invalid)return invalid;
	if (!p||p->type != JSONType::object) {
		common::kill(p);
		p = make_obj(this);
		if (!p)return invalid;
	}
	else{		
		auto s = common::search_T(name, p->object);
		if (s)return *s->obj;
	}
	auto ad = common::create<JSON>();
	if (!ad)return invalid;
	if (!add(p, name, ad)) {
		common::kill(ad);
		return invalid;
	}
	return *ad;
}

JSON& JSON::operator[](uint64_t i) {
	if (this == &invalid)return invalid;
	if (!p || p->type != JSONType::array) {
		common::kill(p);
		p = make_obj(this);
		if (!p)return invalid;
	}
	
	auto ad = common::create<JSON>();
	if (!ad)return invalid;
	if (!add(p,ad)) {
		common::kill(ad);
		return invalid;
	}
	return *ad;
}

JSON::operator const char* () {
	if (!p)return "null";
	if (p->type == JSONType::null) {
		return "null";
	}
	else if (p->type == JSONType::string) {
		return p->str;
	}
	else if (p->type==JSONType::boolean) {
		if (p->flag) {
			return "true";
		}
		else {
			return "false";
		}
	}
	else if (p->type == JSONType::number) {
		common::free(this->hold);
		this->hold = common::StringFilter() = std::to_string(p->num_i).c_str();
		return this->hold;
	}
	else if (p->type == JSONType::number_f) {
		common::free(this->hold);
		this->hold = common::StringFilter() = std::to_string(p->num_f).c_str();
		return this->hold;
	}
	else if (p->type==JSONType::object) {
		common::String s;
		s.add('{');
		for (auto e:p->object) {
			s.add_copy(e->name, ctype::strlen(e->name));
			s.add(':');
			auto child = (const char*)*e->obj;
			s.add_copy(child,ctype::strlen(child));
		}
		s.add('}');
		common::free(this->hold);
		this->hold = s.get_raw_z();
		return this->hold;
	}
	else if (p->type==JSONType::array) {
		common::String s;
		s.add('[');
		for (auto e : p->array) {
			auto child = (const char*)*e;
			s.add_copy(child, ctype::strlen(child));
		}
		s.add(']');
		common::free(this->hold);
		this->hold = s.get_raw_z();
		return this->hold;
	}
	return nullptr;
}

/*
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
	common::String str;
	get_nodestr(node, str,has_line,ofs);
	printer(str.get_const());
	return true;
}

bool  user_tools::JSON::get_nodestr(JSONNode* node, common::String& str,int has_line,int ofs) const{
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

bool user_tools::JSON::line(int has_line,common::String& str) const{
	if (has_line > 0) {
		for (int i = 0; i < has_line; i++)str.add(' ');
	}
	return true;
}
*/

/*
JSONNode* user_tools::JSON::make_JSON(const char* jsonstr) {
	if (!jsonstr)return nullptr;
	io::Reader reader(jsonstr, strlen(jsonstr));
	return make_JSON_detail(&reader);
}

JSONNode* user_tools::JSON::make_JSON_file(const char* filename) {
	if (!filename)return nullptr;
	io::Reader reader(filename);
	if (reader.eof())return nullptr;
	return make_JSON_detail(&reader);
}

JSONNode* user_tools::JSON::make_JSON_detail(io::Reader* reader) {
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
*/



