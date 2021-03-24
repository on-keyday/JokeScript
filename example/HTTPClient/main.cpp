#include<chrono>
#include"common/io.h"
#include"json_tools.h"
#include"HTTPClient.h"
#include<locale.h>
#include<direct.h>
using namespace network;
using namespace ctype;

bool nomsg = false;
bool (*j)(char) = ctype::is_usable_for_identifier;

#define INPUT_SIZE 3

struct flags_t {
	bool locale = false;
	bool raw = false;
	bool redirect = false;
	bool payload = false;
	BodyFlag effort = BodyFlag::onheader;
	bool addtional = false;
	uint64_t count = 0;
	common::String input[INPUT_SIZE];
};

enum class result_t
{
	succeed_m,
	succeed_f,
	error,
	exit,
	notfound
};

void print() {}

template<class Now,class... Args>
void print(Now now,Args... other) {
	if (nomsg)return;
	std::cout << now;
	print(other...);
}

void error() {}

template<class Now, class... Args>
void error(Now now, Args... other) {
	std::cerr << now;
	error(other...);
}


template<class Time>
void common_info(HTTPClient& client,bool raw,Time time) {
	if(client.address())print("accessed address: ", client.address(), "\n");
	if (raw)print(client.raw(), "\n");
	print("time:", time, "msec\n");
	print("status code: ", client.statuscode(), "\n");
}

void location(HTTPClient& client) {
	if (client.statuscode() >= 301 && client.statuscode() <= 308) {
		auto local = client.header("location");
		if (local) {
			print("location is '" ,local,"'\n");
		}
	}
}

void responseinfo(HTTPClient& client) {
	auto type = client.header("content-type");
	if (type) {
		print("content-type is '", type, "'\n");
	}
	print("content length is ",client.len()," byte\n");
}

bool is_success(unsigned short statuscode) {
	return (statuscode >= 200 && statuscode < 300) || statuscode == 418;
}

bool run_method(HTTPClient& client,const char* method,const char* uri,const char* filename,flags_t& flags) {
	if (streaq(method,"help")) {
		print(
			"methods list\n",
			"get <url> [<filename>]:GET method\n",
			"head <url> :HEAD method\n",
			"put <url> <filename>:PUT method\n",
			"post <url> <filename>:POST method\n",
			"options <url>:OPTION method\n"
		);
		return true;
	}
	else if (streaq(method, "get")) {
		auto start = std::chrono::system_clock::now();
		if (!client.get(uri,flags.redirect,255,false,flags.effort)) {
			error("method 'GET' is faild!\n");
			return false;
		}
		auto end = std::chrono::system_clock::now();
		common_info(client, flags.raw, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		if (!flags.redirect) location(client);
		if(client.statuscode()==200||client.statuscode()==418)responseinfo(client);
		if (filename) {
			common::io_base::OutPut out;
			if (out.writeall(filename, true, client.body(), client.len())) {
				print("content was written to '",filename , "'\n");
			}
			else {
				error("failed to write content to '" ,filename,"'\n");
			}
		}
	}
	else if (streaq(method,"head")) {
		auto start = std::chrono::system_clock::now();
		if (!client.get(uri, flags.redirect,255,true)) {
			error("method 'HEAD' is faild!\n");
			return false;
		}
		auto end = std::chrono::system_clock::now();
		common_info(client, flags.raw, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		if (!flags.redirect)location(client);
	}
	else if (streaq(method,"options")) {
		auto start = std::chrono::system_clock::now();
		if (!client.options(uri)) {
			 error("method 'OPTIONS' is faild!\n");
			return false;
		}
		auto end = std::chrono::system_clock::now();
		common_info(client, flags.raw, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		if (!flags.redirect)location(client);
		auto allow = client.header("allow");
		if (allow) {
			print("allow methods are '",allow ,"'\n");
		}
	}
	else if (streaq(method, "put")) {
		common::io_base::Input input;
		if (flags.payload) {
			input.buf.add_copy(filename, ctype::strlen(filename));
		}
		else {
			if (!input.readall(filename, "rb")) {
				error("file '", filename, "' is unloadable!\n");
				return false;
			}
		}
		auto start = std::chrono::system_clock::now();
		if (!client.put(uri,input.buf.get_const(),input.buf.get_size())) {
			error("method 'PUT' is faild!\n");
			return false;
		}
		auto end = std::chrono::system_clock::now();
		common_info(client, flags.raw, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}
	else if (streaq(method,"post")) {
		common::io_base::Input input;
		if (flags.payload) {
			input.buf.add_copy(filename, ctype::strlen(filename));
		}
		else {
			if (!input.readall(filename, "rb")) {
				error("file '", filename, "' is unloadable!\n");
				return false;
			}
		}
		auto start = std::chrono::system_clock::now();
		if (!client.post(uri, input.buf.get_const(), input.buf.get_size())) {
			error("method 'POST' is faild!\n");
			return false;
		}
		auto end = std::chrono::system_clock::now();
		common_info(client, flags.raw, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}
	else {
		error("method '" ,method ,"' is not allowed.\n");
		return false;
	}
	if (flags.addtional)return true;
	if (is_success(client.statuscode())) {
		return true;
	}
	else {
		return false;
	}
}

bool replace_value(common::String& res,flags_t& flags) {
	if (flags.addtional) {
		bool replaced = false;
		std::string tmpstr = res.get_const();
		for (auto i = 0; i < INPUT_SIZE;i++) {
			char tmp[] = "$input0$";
			tmp[6] = '0' + i;
			while (true) {
				auto pos = tmpstr.find(tmp);
				if (pos == std::string::npos)break;
				tmpstr.replace(pos, 8,flags.input[i].get_const());
				replaced = true;
			}
		}
		while (true) {
			auto pos = tmpstr.find("$count$");
			if (pos == std::string::npos)break;
			tmpstr.replace(pos, 7, std::to_string(flags.count));
			replaced = true;
		}
		if (replaced) {
			res.init();
			res.add_copy(tmpstr.c_str(), tmpstr.length());
		}
	}
	return true;
}

bool read_cmdline(io::Reader& cmdline, common::String& res,flags_t& flags) {
	if (cmdline.ahead("\"") || cmdline.ahead("\'")) {
		auto tmp = cmdline.string(false);
		tmp.remove_top();
		tmp.remove_end();
		res.init();
		for (auto i = 0; i < tmp.get_size();i++) {
			if (tmp[i] == '\\') {
				i++;
			}
			res.add_nz(tmp[i]);
		}
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
	replace_value(res, flags);
	return true;
}

bool read_num(io::Reader& cmdline,flags_t& flags,uint64_t& idx) {
	common::String num;
	if (!read_cmdline(cmdline, num, flags)) {
		print("two argument is required.\n");
		return -1;
	}
	if (!ctype::is_number(num[0])) {
		print("number is required for argument\n");
		return false;
	}
	idx = common::strtoull_ex(num.get_const(), nullptr);
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
		print("'true' or 'false' is required for argument\n");
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

bool host_info(HTTPClient& client,common::String& str) {
	print("wait...\n");
	if (!client.get("http://ip-api.com/json/?fields=status,message,continent,continentCode,country,countryCode,region,regionName,city,district,zip,lat,lon,timezone,offset,currency,isp,org,as,asname,reverse,mobile,proxy,hosting,query")) {
		print("not accessable\n");
		return false;
	}
	if (client.statuscode() != 200) {
		 print(
			"request was sent, but not succeed.\n",
			"status code: " , client.statuscode(),"\n"
		 );
	}
	user_tools::JSON json;
	auto result=json.make_JSON(client.body());
	if (!result) {
		 print(
			 "failed to parse as JSON\n",
			 "raw data:\n"
		 );
		if (client.body()) {
			print(client.body());
		}
		print("\n");
		return false;
	}
	json.get_nodestr(result, str, 0, 2);
	//auto q=json.get_nodes(result, "/query");
	return true;
}



result_t command(HTTPClient& client, io::Reader& cmdline, flags_t& flags, common::String& hostinfo);


int interactive_mode(HTTPClient& client,flags_t& flags) {
	print(
		"interactive mode\n",
		"openssl version:",OpenSSL_version(OPENSSL_VERSION),"\n",
		"type 'help' for more infomation.\n"
	);
	common::String hostinfo;
	while (true) {
		std::cout << ">>";
		std::string input;
		std::getline(std::cin, input);
		io::Reader cmdline(input.size(), input.c_str(), nullptr, io::ignore_space);
		if (command(client, cmdline, flags, hostinfo) == result_t::exit)break;
	}
	return 0;
}

int command_intepreter(const char* filename, HTTPClient& client, flags_t& flags, int safety);

int addtional_command(io::Reader& cmdline,common::String& hostinfo,HTTPClient& client, flags_t& flags,int safety = 0) {
	auto ret = command(client, cmdline, flags, hostinfo);
	if (ret==result_t::notfound) {
		if (cmdline.expect_pf("call", j)) {
			common::String filepath;
			if (!read_cmdline(cmdline, filepath,flags)) {
				print("one argument is required.\n");
				return -1;
			}
			auto res = command_intepreter(filepath.get_const(), client, flags, safety + 1);
			if (res < 0)return res;
			ret = result_t::succeed_f;
		}
		else if (cmdline.expect_pf("system", j)) {
			common::String str;
			if (!read_cmdline(cmdline,str,flags)) {
				print("one argument is required.\n");
				return -1;
			}
			if (ctype::is_unnamed(str.get_const()))return -1;
			if(::system(str.get_const())<0)return -1;
			ret = result_t::succeed_f;
		}
		else if (cmdline.expect_pf("count", j)) {
			if (cmdline.expect_pf("decl",j)) {
				if (flags.count != 0) {
					flags.count--;
				}
			}
			else if (cmdline.expect_pf("reset", j)) {
				flags.count = 0;
			}
			else {
				if (flags.count != ~0) {
					flags.count++;
				}
			}
			ret = result_t::succeed_f;
		}
		else if (cmdline.expect_pf("input", j)) {
			common::String file;
			uint64_t idx = 0;
			if(!read_num(cmdline, flags,idx))return -1;
			if (idx >= INPUT_SIZE) {
				print("large number\n");
				return -1;
			}
			if (!read_cmdline(cmdline, file, flags)) {
				print("one argument is required.\n");
				return -1;
			}
			if (flags.payload) {
				flags.input[idx] = std::move(file);
			}
			else {
				common::io_base::Input input;
				input.readall(file.get_const());
				if (input.buf.get_size()) {
					flags.input[idx] = std::move(input.buf);
				}
			}
			ret = result_t::succeed_f;
		}
		else if (cmdline.expect_pf("set_count", j)) {
			bool inputf = false;
			if (cmdline.expect_pf("input", j)) {
				inputf = true;
			}
			uint64_t num = 0;
			if(!read_num(cmdline, flags,num))return -1;
			if (inputf) {
				if (num >= INPUT_SIZE) {
					print("large number\n");
					return -1;
				}
				if (!ctype::is_number(flags.input[num][0])) {
					print("input[", num, "] is not number\n");
					return -1;
				}
				auto toset=common::strtoull_ex(flags.input[num].get_const(), 0);
				flags.count = toset;
			}
			else {
				flags.count = num;
			}
			ret = result_t::succeed_f;
		}
		else if (cmdline.expect_pf("top", j)) {
			return 3;
		}
	}
	if (ret == result_t::exit)return 2;
	if (ret == result_t::error)return -1;
	if (ret == result_t::notfound)return 1;
	return 0;
}

void flag_do(bool flag, bool judge, int& suc, io::Reader& cmdline,common::String& hostinfo,HTTPClient& client,flags_t& flags,int safety) {
	if (flag) {
		if (judge) {
			suc = addtional_command(cmdline, hostinfo, client, flags, safety);
		}
	}
	else {
		if (!judge) {
			suc = addtional_command(cmdline, hostinfo, client, flags, safety);
		}
	}
}

int command_intepreter(const char* filename,HTTPClient& client,flags_t& flags,int safety=0) {
	if (safety >= 100) {
		error("subcommand has been stopped to stop overflow.\n");
		return -2;
	}
	flags.addtional = true;
	common::String hostinfo=nullptr;
	io::Reader file(filename,false,nullptr,io::not_ignore);
	if (file.eof()) {
		error("file'", filename, "' unloadable\n");
		return -1;
	}
	if (!flags.locale) {
		setlocale(LC_ALL, ".utf-8");
		flags.locale = true;
	}
	while (!file.eof()) {
		io::ReadStatus rs;
		rs.num = '\n';
		file.readwhile(&rs, ctype::reader::Until);
		file.expect("\n");
		if (!rs.buf.get_size())continue;
		io::Reader cmdline(rs.buf, nullptr, io::ignore_space_and_line);
		if (cmdline.expect("#"))continue;
		auto res = addtional_command(cmdline,hostinfo,client,flags,safety);
		if (res <0)return res;
		if (res == 2)return 0;
		if (res == 3)file.seek(0);
		if (res == 1) {
			if (cmdline.expect_pf("code",j)) {
				bool flag = false;
				auto suc = 0;
				if (!set_trueorfalse(cmdline, flag, true, false)) {
					return -1;
				}
				flag_do(flag, is_success(client.statuscode()), suc, cmdline, hostinfo, client, flags, safety);
				if (suc < 0)return suc;
				if (suc == 2)return 0;
				if (res == 3)file.seek(0);
			}
			else if (cmdline.expect_pf("less", j)) {
				common::String num;
				if (!read_cmdline(cmdline, num,flags)) {
					print("one argument is required.\n");
					return -1;
				}
				auto cmp=common::strtoull_ex(num.get_const(),nullptr);
				if (flags.count < cmp) {
					auto suc = addtional_command(cmdline, hostinfo, client, flags, safety);
					if (suc < 0)return suc;
					if (suc == 2)return 0;
					if (res == 3)file.seek(0);
				}
			}
			else if (cmdline.expect_pf("inbuf",j)) {
				uint64_t idx = 0;
				if (!read_num(cmdline, flags, idx))return -1;
				if (idx >= INPUT_SIZE) {
					print("large number\n");
					return -1;
				}
				bool flag = false;
				auto suc = 0;
				if (!set_trueorfalse(cmdline, flag, true, false))return -1;
				flag_do(flag, flags.input[idx].get_size(), suc, cmdline, hostinfo, client, flags, safety);
				if (suc < 0)return suc;
				if (suc == 2)return 0;
				if (res == 3)file.seek(0);
			}
			else {
				io::ReadStatus rs{ 0 };
				rs.num = ' ';
				cmdline.readwhile(&rs, ctype::reader::Until);
				error("unknown command: ",rs.buf.get_const() , "\n");
				return -1;
			}
		}
	}
	return 0;
}



result_t command(HTTPClient& client, io::Reader& cmdline,flags_t& flags,common::String& hostinfo) {
	result_t result = result_t::succeed_f;
	if (cmdline.expect_pf("exit", j)) {
		return result_t::exit;
	}
	else if (cmdline.expect_pf("help", j)) {
		run_method(client, "help", nullptr, nullptr,flags);
		print(
			"other command\n",
			"exit:finish interactive mode\n",
			"help:show this help\n",
			"clear:clear cmdline buffer\n",
			"sslinfo:show OpenSSL info\n",
			"global [refresh]:get global ip address and info from the Internet\n",
			"raw <true|false>:set flag of visibility of HTTP Headers\n",
			"redirect <true|false>:set flag to redirect request\n",
			"effort <true|false>:set flag of behavior when 'GET' method response has no 'content-length' or 'transfer-encoding'\n",
			"sslmsg <true|false>:set flag of OpenSSL callback\n",
			"payload <true|false>:set flag of data payload\n",
			"all <true|false>:set all flag\n"
		);
	}
	else if (cmdline.expect_pf("sslinfo", j)) {
		print("openssl version:", OpenSSL_version(OPENSSL_VERSION), "\n");
	}
	else if (cmdline.expect_pf("global", j)) {
		if (cmdline.expect_pf("refresh", j)) {
			hostinfo.init();
		}
		if (ctype::is_unnamed(hostinfo.get_const())) {
			if (!host_info(client, hostinfo))return result_t::error;
		}
		print(
			"global host info:\n", hostinfo.get_const(), "\n",
			"(by ip-api.com)\n"
		);
	}
	else if (cmdline.expect_pf("clear", j)) {
		::system("cls");
	}
	else if (cmdline.expect_pf("get", j)) {
		common::String url = nullptr, file = nullptr;
		if (!read_cmdline(cmdline, url,flags)) {
			print("one argument is required.\n");
			return result_t::error;
		}
		if (!cmdline.eof()) {
			read_cmdline(cmdline, file,flags);
		}
		if(!run_method(client, "get", url.get_const(), file.get_const(),flags))return result_t::error;
		result = result_t::succeed_m;
	}
	else if (cmdline.expect_pf("head", j) || cmdline.expect_pf("options", j)) {
		common::String url = nullptr;
		if (!read_cmdline(cmdline, url,flags)) {
			print("one argument is required.\n");
			return result_t::error;
		}
		if(!run_method(client, cmdline.prev(), url.get_const(), nullptr, flags))return result_t::error;
		result = result_t::succeed_m;
	}
	else if (cmdline.expect_pf("put", j) || cmdline.expect_pf("post", j)) {
		common::String url = nullptr, file = nullptr;
		if (!read_cmdline(cmdline, url,flags)) {
			print("two argument is required.\n");
			return result_t::error;
		}
		if (!read_cmdline(cmdline, file,flags)) {
			print("two argument is required.\n");
			return result_t::error;
		}
		if (!run_method(client, cmdline.prev(), url.get_const(), file.get_const(), flags))return result_t::error;
		result = result_t::succeed_m;
	}
	else if (cmdline.expect_pf("raw", j)) {
		if(!set_trueorfalse(cmdline, flags.raw, true, false))return result_t::error;
	}
	else if (cmdline.expect_pf("redirect", j)) {
		if(!set_trueorfalse(cmdline, flags.redirect, true, false))return result_t::error;
	}
	else if (cmdline.expect_pf("effort", j)) {
		if(!set_trueorfalse(cmdline, flags.effort, BodyFlag::effort, BodyFlag::onheader))return result_t::error;
	}
	else if (cmdline.expect_pf("payload", j)) {
		if(!set_trueorfalse(cmdline, flags.payload, true, false))return result_t::error;
	}
	else if (cmdline.expect_pf("sslmsg", j)) {
		bool ssl_f = false;
		if (set_trueorfalse(cmdline, ssl_f, true, false)) {
			set_ssl_cb(client, ssl_f);
		}
		else {
			return result_t::error;
		}
	}
	else if (cmdline.expect_pf("all", j)) {
		bool all = false;
		if (set_trueorfalse(cmdline, all, true, false)) {
			flags.raw = all;
			flags.effort = all ? BodyFlag::effort : BodyFlag::onheader;
			flags.redirect = all;
			flags.payload = all;
			set_ssl_cb(client, all);
		}
		else {
			return result_t::error;
		}
	}
	else if (cmdline.expect_pf("cd", j)) {
		common::String dir;
		if (!read_cmdline(cmdline, dir, flags)) {
			print("one argument is required.\n");
			return result_t::error;
		}
		if (_chdir(dir.get_const())) {
			print(dir.get_const(), ":no such dirctry");
			return result_t::error;
		}
	}
	else {
		if (flags.addtional)return result_t::notfound;
		io::ReadStatus rs{ 0 };
		rs.num = ' ';
		cmdline.readwhile(&rs, ctype::reader::Until);
		auto cmd = rs.buf.get_const();
		if (ctype::is_unnamed(cmd)) {
			print("input command!\n");
		}
		else {
			print("unknown command:", cmd, "\n");
		}
		return result_t::error;
	}
	return result;
}

#include<consoleapi3.h>

void http2_test();

int main(int argc, char** argv){
	//http2_test();
	SetConsoleOutputCP(CP_UTF8);
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
	bool use_for_payload = false;
	BodyFlag effort = BodyFlag::onheader;
	const char* cacert = "cacert.pem",*interpret=nullptr;
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
		else if (streaq(arg[ofs], "-q")) {
			nomsg = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs],"-p")) {
			use_for_payload = true;
			ofs++;
			continue;
		}
		else if (streaq(arg[ofs], "-s")) {
			ofs++;
			auto tmp = arg[ofs];
			if (!tmp) {
				error("syntax of option -s is invalid\n");
				return -1;
			}
			cacert = tmp;
			ofs++;
			continue;
		}
		else if(streaq(arg[ofs], "-m")){
			ofs++;
			auto tmp = arg[ofs];
			if (!tmp) {
				error("syntax of option -m is invalid\n");
				return -1;
			}
			interpret = tmp;
			ofs++;
			continue;
		}
		break;
	}
	if (!arg[ofs]&&!interactive&&!interpret) {
		error("no method is in command line.\n");
		return -1;
	}
	if (streaq(arg[ofs], "get")|| streaq(arg[ofs], "head")) {
		if (!arg[ofs + 1]) {
			error("one argument is required.\n");
			return -1;
		}
	}
	else if (streaq(arg[ofs], "put")||streaq(arg[ofs],"post")) {
		if (!arg[ofs + 2]) {
			error("two argument is required\n");
			return -1;
		}
	}
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data)) {
		error("failed to initialize WinSock.\n");
		return -1;
	}
	HTTPClient client(cacert);
	if (ssl_cb) {
		client.set_infocb(info_cb);
		client.set_errcb(err_cb);
	}
	int res = 0;
	flags_t flags;
	flags.raw = rawdata;
	flags.redirect = redirect;
	flags.effort = effort;
	flags.payload = use_for_payload;
	flags.addtional = false;
	if (arg[ofs]) {
		res=run_method(client,arg[ofs], arg[ofs + 1], arg[ofs + 2], flags)?0:-1;
		if (ctype::streaq(arg[ofs], "help")) {
			print(
				"other command\n"
				"help:show this help\n"
				"command line options\n",
				"-i:use interactive mode\n",
				"-r:see raw (http response) data\n"
				"-a:enable auto redirection\n",
				"-c:enable OpenSSL info callback(for debug)\n",
				"-e:read body even 'content-length','transfer-encoding',or 'content-type' is missing\n",
				"-q:show only error message\n",
				"-p:if use 'post' or 'put' method, second argument will be used as data payload\n",
				"-s <file>:set CAcert file. default:./cacert.pem\n"
				"-m <file>:interpreter mode\n",
				"Usage\n"
				"httpclient <options> <method> <args>\n"
			);
		}
	}
	if (interpret) {
		res = command_intepreter(interpret, client, flags);
	}
	if (interactive) {
		nomsg = false;
		SetConsoleCP(CP_UTF8);
		if (!flags.locale) {
			setlocale(LC_ALL, ".utf8");
			flags.locale = true;
		}
		res=interactive_mode(client,flags);
	}
	WSACleanup();
	OutDebugMemoryInfo(ShowGraph());
	return res;
}

void http2_test() {
	char tests[] = 
	{0x00,0x00,0x01,
	 1,9,
	 0,0,0,1};
	io::Reader testr(sizeof(tests),tests,nullptr,io::not_ignore);
	HTTP2Client testc;
}