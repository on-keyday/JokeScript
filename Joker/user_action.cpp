#include"user_action.h"
#include"compiler_filereader.h"
#include"compiler_ctype.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::compiler;


bool interactive::json_assign(std::map<std::string, user_tools::JSONNode*>& idmap,user_tools::JSON& json,compiler::Reader& reader) {
	ReadStatus status = { 0 };
	reader.readwhile(&status, ctype::reader::Identifier);
	if (status.failed)return false;
	if (ctype::is_unnamed(status.buf.get_const()))return false;
	if (!reader.expect("="))return false;
	user_tools::JSONNode* obj = nullptr;
	if (reader.abyte() == '\'') {
		auto buf = reader.string(false);
		buf.remove_top();
		buf.remove_end();
		obj = json.make_JSON(buf.get_const());
	}
	else if (reader.abyte() == '\"') {
		auto buf = reader.string(false);
		buf.remove_top();
		buf.remove_end();
		obj = json.make_JSON_file(buf.get_const());
	}
	else {
		common::EasyVector<char> buf;
		if (!json_str(idmap, json, buf, reader, true))return false;
		obj = json.make_JSON(buf.get_const());
	}
	if (!obj)return false;
	if (idmap[status.buf.get_const()]) {
		json.remove(idmap[status.buf.get_const()]);
	}
	idmap[status.buf.get_const()] = obj;
	return true;
}

bool interactive::json_str(std::map<std::string, user_tools::JSONNode*>& idmap, user_tools::JSON& json, common::EasyVector<char>& buf, compiler::Reader& reader,bool end) {
	reader.abyte();
	ReadStatus ch = { 0 };
	reader.readwhile(&ch, ctype::reader::Identifier);
	if (ch.failed)return false;
	if (ctype::is_unnamed(ch.buf.get_const()))return false;;
	try {
		auto node = idmap.at(ch.buf.get_const());
		if (reader.abyte() == '.' || reader.abyte() == '[' || reader.abyte() == '/') {
			ReadStatus ch = { 0 };
			if (end) {
				reader.readwhile(&ch, ctype::reader::End);
			}
			else {
				ch.num = ' ';
				reader.readwhile(&ch,ctype::reader::Until);
			}
			if (ctype::is_unnamed(ch.buf.get_const()))return false;
			json.get_nodestr(json.get_nodes(node, ch.buf.get_const()), buf, 0, 2);
		}
		else {
			json.get_nodestr(node, buf, 0, 2);
		}
	}
	catch (...) {
		return false;
	}
	if (ctype::is_unnamed(buf.get_const())) {
		return false;
	}
	return true;
}

const char* interactive::json_help() {
	return
		R"(usage:
  memsize:show memory size of JSON structure.
  ids:show JSON ids.
  [id]=['JSON'|"filename"|cid]:make JSON id.
  print [cid]:show value of id.
  delete [cid]:delete id.
  write [cid] [filename]:write id's value to file.
  exit:finish this prompt.
  help:show this help.

id's children are constant values.
id is reassinable.
you can use for 'cid',
  id.child
  id["child"].child
  id[0]["child"]
  id/child/child/0/child
)";
}

int interactive::json_reader() {
	auto& out = std::cout;
	auto& in = std::cin;
	user_tools::JSON json;
	std::map<std::string, user_tools::JSONNode*> idmap;
	bool ok = false;
	bool exit = false;
	while (true) {
		Reader reader(nullptr, 0ull);
		std::string input;
		out << ">>";
		std::getline(in, input);
		in.clear();
		reader.add_str(input.c_str());
		while (true) {
			if (reader.expect_pf("exit", ctype::is_usable_for_identifier)) {
				exit = true;
			}
			else if (reader.expect_pf("memsize", ctype::is_usable_for_identifier)) {
				out << json.this_size() << "\n";
				ok = true;
			}
			else if (reader.expect_pf("delete", ctype::is_usable_for_identifier)) {
				ok = true;
				reader.abyte();
				ReadStatus status = { 0 };
				reader.readwhile(&status, ctype::reader::Identifier);
				if (status.failed)break;
				if (ctype::is_unnamed(status.buf.get_const()))break;
				if (idmap[status.buf.get_const()]) {
					json.remove(idmap[status.buf.get_const()]);
					idmap[status.buf.get_const()] = nullptr;
				}
			}
			else if (reader.expect_pf("ids", ctype::is_usable_for_identifier)) {
				for (auto s : idmap) {
					if (s.second) {
						out << s.first << "\n";
					}
				}
				ok = true;
			}
			else if (reader.expect_pf("write", ctype::is_usable_for_identifier)) {
				common::EasyVector<char> got;
				if (!json_str(idmap, json, got, reader, false))break;
				reader.abyte();
				ReadStatus status = { 0 };
				reader.readwhile(&status, ctype::reader::End);
				if (ctype::is_unnamed(status.buf.get_const()))break;
				common::io::OutPut file;
				if (file.writeall(status.buf.get_const(), false, got.get_const(), got.get_size())) {
					ok = true;
				}
			}
			else if (reader.expect_pf("print", ctype::is_usable_for_identifier)) {
				common::EasyVector<char> got;
				if (!json_str(idmap, json, got, reader,true))break;
				out << got.get_const() << "\n";
				ok = true;
			}
			else if (reader.expect_pf("help", ctype::is_usable_for_identifier)) {
				out << json_help();
				ok = true;
			}
			else if (ctype::is_first_of_identifier(reader.abyte())) {
				ok = json_assign(idmap, json, reader);
			}
			break;
		}
		if (exit)break;
		if (!ok) {
			out << "invalid expression!\n";
		}
		ok = false;
	}
	return 0;
}