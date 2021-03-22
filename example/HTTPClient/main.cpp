#include<chrono>
#include"common/io.h"
#include"HTTPClient.h"
using namespace network;
using namespace ctype;

void location(HTTPClient& client) {
	if (client.statuscode() >= 301 && client.statuscode() <= 308) {
		auto local = client.header("location");
		if (local) {
			std::cout << "location is '" << local << "'\n";
		}
	}
}

void responseinfo(HTTPClient& client) {
	auto type = client.header("content-type");
	if (type) {
		std::cout << "content-type is '" << type << "'\n";
	}
	std::cout << "content length is " << client.len() << " byte\n";
}

bool run_method(HTTPClient& client,const char* method,const char* uri,const char* filename,bool raw,bool redirect,BodyFlag effort) {
	if (streaq(method,"help")) {
		std::cout << "methods list\n";
		std::cout << "get <url> [<filename>]:GET method\n";
		std::cout << "head <url> :HEAD method\n";
		std::cout << "put <url> <filename>:PUT method\n";
		std::cout << "post <url> <filename>:POST method\n";
		std::cout << "options <url>:OPTION method\n";
	}
	else if (streaq(method, "get")) {
		auto start = std::chrono::system_clock::now();
		if (!client.get(uri,redirect,255,false,effort)) {
			std::cout << "method 'GET' is faild!\n";
			return false;
		}
		auto end = std::chrono::system_clock::now();
		std::cout << "accessed address: " << client.address() << "\n";
		if(raw)std::cout << client.raw() << "\n";
		std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "msec\n";
		std::cout << "status code: " << client.statuscode() << "\n";
		if (!redirect) location(client);
		if(client.statuscode()==200||client.statuscode()==418)responseinfo(client);
		if (filename) {
			common::io_base::OutPut out;
			if (out.writeall(filename, true, client.body(), client.len())) {
				std::cout << "content was written to '" << filename << "'\n";
			}
			else {
				std::cout << "failed to write content to '" << filename<< "'\n";
			}
		}
	}
	else if (streaq(method,"head")) {
		auto start = std::chrono::system_clock::now();
		if (!client.get(uri, redirect,255,true)) {
			std::cout << "method 'HEAD' is faild!\n";
			return false;
		}
		auto end = std::chrono::system_clock::now();
		if (client.address())std::cout << "accessed address: " << client.address() << "\n";
		if (raw)std::cout << client.raw() << "\n";
		std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "msec\n";
		std::cout << "status code: " << client.statuscode() << "\n";
		if (!redirect)location(client);
	}
	else if (streaq(method,"options")) {
		auto start = std::chrono::system_clock::now();
		if (!client.options(uri)) {
			std::cout << "method 'OPTIONS' is faild!\n";
			return false;
		}
		auto end = std::chrono::system_clock::now();
		if (client.address())std::cout << "accessed address: " << client.address() << "\n";
		if (raw)std::cout << client.raw() << "\n";
		std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "msec\n";
		std::cout << "status code: " << client.statuscode() << "\n";
		if (!redirect)location(client);
		auto allow = client.header("allow");
		if (allow) {
			std::cout << "allow methods are '" << allow << "'\n";
		}
	}
	else if (streaq(method, "put")) {
		common::io_base::Input input;
		if (!input.readall(filename, "rb")) {
			std::cout << "file '" << filename << "' is unloadable!\n";
			return false;
		}
		auto start = std::chrono::system_clock::now();
		if (!client.put(uri,input.buf.get_const(),input.buf.get_size())) {
			std::cout << "method 'PUT' is faild!\n";
			return false;
		}
		auto end = std::chrono::system_clock::now();
		if(client.address())std::cout << "accessed address: " << client.address() << "\n";
		if (raw)std::cout << client.raw() << "\n";
		std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "msec\n";
		std::cout << "status code: " << client.statuscode() << "\n";
	}
	else if (streaq(method,"post")) {
		common::io_base::Input input;
		if (!input.readall(filename, "rb")) {
			std::cout << "file '" << filename << "' is unloadable!\n";
			return false;
		}
		auto start = std::chrono::system_clock::now();
		if (!client.post(uri, input.buf.get_const(), input.buf.get_size())) {
			std::cout << "method 'POST' is faild!\n";
			return false;
		}
		auto end = std::chrono::system_clock::now();
		if (client.address())std::cout << "accessed address: " << client.address() << "\n";
		if (raw)std::cout << client.raw() << "\n";
		std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "msec\n";
		std::cout << "status code: " << client.statuscode() << "\n";
	}
	else {
		std::cout << "method '" << method << "' is not allowed.\n";
		return false;
	}
	return true;
}

bool read_cmdline(io::Reader& cmdline, common::String& res) {
	if (cmdline.ahead("\"") || cmdline.ahead("\'")) {
		res = cmdline.string(false);
		res.remove_top();
		res.remove_end();
	}
	else if (!cmdline.eof()) {
		io::ReadStatus rs{ 0 };
		rs.num = ' ';
		cmdline.readwhile(&rs, ctype::reader::Until);
		res = std::move(rs.buf);
	}
	else {
		return false;
	}
	return true;
}


void info_cb(const SSL* ssl,int type,int val) {
	std::cout << "ssl_info: type=" << type << ",value=" << val << "\n";
}

int err_cb(const char* str,size_t len,void*) {
	std::cout << str << "\n";
	return 1;
}

template<class T>
bool set_trueorfalse(io::Reader& cmdline, T& res,T t,T f ) {
	if (cmdline.expect("true")) {
		res = t;
	}
	else if (cmdline.expect("false")) {
		res = f;
	}
	else {
		std::cout << "'true' or 'false' is required for argument\n";
		return false;
	}
	return true;
}

void set_ssl_cb(HTTPClient& client,bool flag) {
	if (flag) {
		client.set_infocb(info_cb);
		client.set_errcb(err_cb);
	}
	else{
		client.set_infocb(nullptr);
		client.set_errcb(nullptr);
	}
}

int interactive_mode(HTTPClient& client,bool inraw,bool inredirect,BodyFlag ineffort) {
	std::cout << "interactive mode\n";
	std::cout << "openssl version:" << OpenSSL_version(OPENSSL_VERSION) << "\n";
	std::cout << "type 'help' for more infomation.\n";
	bool raw = inraw;
	bool redirect = inredirect;
	BodyFlag effort = ineffort;
	bool (*j)(char)=ctype::is_usable_for_identifier;
	while (true) {
		std::cout << ">>";
		std::string input;
		std::getline(std::cin, input);
		io::Reader cmdline(input.size(), input.c_str(), nullptr, io::ignore_space);
		if (cmdline.expect_pf("exit",j)) {
			break;
		}
		else if (cmdline.expect_pf("help",j)) {
			run_method(client, "help", nullptr, nullptr, raw,redirect,effort);
			std::cout << "other command\n";
			std::cout << "exit:finish interactive mode\n";
			std::cout << "help:show this help\n";
			std::cout << "sslinfo:show OpenSSL info\n";
			std::cout << "raw <true|false>:set flag of visibility of HTTP Headers\n";
			std::cout << "redirect <true|false>:set flag to redirect request\n";
			std::cout << "effort <true|false>:set flag of behavior when 'GET' method response has no 'content-length' or 'transfer-encoding'\n";
			std::cout << "sslmsg <true|false>:set flag of OpenSSL callback\n";
			std::cout << "all <true|false>:set all flag\n";
		}
		else if (cmdline.expect_pf("sslinfo",j)) {
			std::cout << "openssl version:" << OpenSSL_version(OPENSSL_VERSION) << "\n";
		}
		else if (cmdline.expect_pf("clear",j)) {
			::system("cls");
		}
		else if (cmdline.expect_pf("get",j)) {
			common::String url = nullptr,file=nullptr;
			if(!read_cmdline(cmdline,url)) {
				std::cout << "one argument is required.\n";
				continue;
			}
			if (!cmdline.eof()) {
				read_cmdline(cmdline, file);
			}
			run_method(client, "get", url.get_const(), file.get_const(), raw,redirect,effort);
		}
		else if (cmdline.expect_pf("head",j)|| cmdline.expect_pf("options", j)) {
			common::String url = nullptr;
			if(!read_cmdline(cmdline, url)){
				std::cout << "one argument is required.\n";
				continue;
			}
			run_method(client, cmdline.prev(), url.get_const(), nullptr, raw,redirect,effort);
		}
		else if (cmdline.expect_pf("put",j)||cmdline.expect_pf("post",j)) {
			common::String url = nullptr, file = nullptr;
			if(!read_cmdline(cmdline, url)) {
				std::cout << "two argument is required.\n";
				continue;
			}
			if(!read_cmdline(cmdline, file)) {
				std::cout << "two argument is required.\n";
				continue;
			}
			run_method(client, cmdline.prev(), url.get_const(), file.get_const(), raw,redirect,effort);
		}
		else if (cmdline.expect_pf("raw",j)) {
			set_trueorfalse(cmdline, raw, true, false);
		}
		else if (cmdline.expect_pf("redirect",j)) {
			set_trueorfalse(cmdline, redirect, true, false);
		}
		else if (cmdline.expect_pf("effort",j)) {
			set_trueorfalse(cmdline, effort, BodyFlag::effort, BodyFlag::onheader);
		}
		else if (cmdline.expect_pf("sslmsg",j)) {
			bool ssl_f = false;
			if (set_trueorfalse(cmdline, ssl_f, true, false)) {
				set_ssl_cb(client, ssl_f);
			}
		}
		else if (cmdline.expect_pf("all",j)) {
			bool all = false;
			if (set_trueorfalse(cmdline, all, true, false)) {
				raw = all;
				effort = all?BodyFlag::effort:BodyFlag::onheader;
				redirect = all;
				set_ssl_cb(client, all);
			}
		}
		else {
			io::ReadStatus rs{ 0 };
			rs.num = ' ';
			cmdline.readwhile(&rs, ctype::reader::Until);
			auto cmd = rs.buf.get_const();
			if (ctype::is_unnamed(cmd)) {
				std::cout << "input command!\n";
			}
			else{
				std::cout << "unknown command:" << cmd << "\n";
			}
		}
	}
	return 0;
}

int main(int argc, char** argv){
	if (argc <= 1) {
		std::cout << "input args!\n";
		return -1;
	}
	common::EasyVector<char*> arg;
	arg.add_copy(argv, argc);
	uint64_t ofs = 1;
	bool interactive = false;
	bool rawdata = false;
	bool redirect = false;
	bool ssl_cb = false;
	BodyFlag effort = BodyFlag::onheader;
	const char* cacert = "cacert.pem";
	while (arg[ofs]) {
		if (streaq(arg[ofs], "-i")) {
			interactive = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs],"-r")) {
			rawdata = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs], "-a")) {
			redirect = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs],"-c")) {
			ssl_cb = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs], "-e")) {
			effort=BodyFlag::effort;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs], "-s")) {
			ofs++;
			auto tmp = arg[ofs];
			if (!tmp) {
				std::cout << "syntax of option -s is invalid\n";
				return -1;
			}
			cacert = tmp;
			ofs++;
			continue;
		}
		break;
	}
	if (!arg[ofs]&&!interactive) {
		std::cout << "no method is in command line.\n";
		return -1;
	}
	if (streaq(arg[ofs], "get")|| streaq(arg[ofs], "head")) {
		if (!arg[ofs + 1]) {
			std::cout << "one argument is required.\n";
			return -1;
		}
	}
	else if (streaq(arg[ofs], "put")) {
		if (!arg[ofs + 2]) {
			std::cout << "two argument is required\n";
			return -1;
		}
	}
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data)) {
		std::cout << "failed to initialize WinSock.\n";
		return -1;
	}
	HTTPClient client(cacert);
	if (ssl_cb) {
		client.set_infocb(info_cb);
		client.set_errcb(err_cb);
	}
	int res = 0;
	if (arg[ofs]) {
		res=run_method(client,arg[ofs], arg[ofs + 1], arg[ofs + 2], rawdata,redirect,effort)?0:-1;
	}
	if (interactive) {
		res=interactive_mode(client,rawdata,redirect,effort);
	}
	WSACleanup();
	return 0;
}
