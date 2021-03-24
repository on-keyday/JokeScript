#include "HTTPClient.h"
#include <ws2tcpip.h>
#include"common/ctype.h"
using namespace network;



bool HTTPHeaders::add(char* name, char* value) {
	if (!name || !value)return false;
	auto len = ctype::strlen(name);
	for (auto i = 0u; i < len;i++) {
		name[i]=tolower((unsigned char)name[i]);
	}
	auto res=common::create<Header>();
	if (!res)return false;
	res->name = name;
	res->value = value;
	this->headers.add(res);
	return true;
}

bool HTTPHeaders::parse(io::Reader& reader) {
	auto readpos = reader.get_readpos();
	while (!reader.eof()) {
		io::ReadStatus rs = { 0 }, rs2 = { 0 };
		rs.num = ':';
		reader.readwhile(&rs, ctype::reader::Until);
		if (rs.failed)goto Err;
		if (!reader.expect(": "))goto Err;
		rs2.num = '\r';
		reader.readwhile(&rs2, ctype::reader::Until);
		if (!reader.expect("\r\n"))goto Err;
		if (!add(rs.buf.get_raw_z(), rs2.buf.get_raw_z()))goto Err;
		if (reader.ahead("\r\n")) {
			break;
		}
	}
	if (!reader.expect("\r\n"))goto Err;
	return true;
Err:
	reader.seek(readpos);
	return false;
}


void network::default_header_callback(const HTTPClient* self, common::String& buf) {
	buf.add_copy("user-agent: client\r\naccept: */*\r\n", 33);
}

bool network::recv_common(common::String& str, int sock, SSL* ssl, bool secure) {
	while (true) {
		size_t size = 0;
		char tmpbuf[1024] = { 0 };
		if (secure) {
			if (!ssl)return false;
			while (true) {
				bool retry = false;
				if (!SSL_read_ex(ssl, tmpbuf, 1024, &size)) {
					auto reason = SSL_get_error(ssl, 0);
					switch (reason)
					{
					case(SSL_ERROR_WANT_READ):
					case(SSL_ERROR_WANT_WRITE):
						retry = true;
						break;
					default:
						break;
					}
					if (retry)continue;
					return false;
				}
				break;
			}
		}
		else {
			auto res = ::recv(sock, tmpbuf, 1024, 0);
			if (res <= 0) {
				return false;
			}
			size = res;
		}
		str.add_copy(tmpbuf, size);
		if (size == 1024) {
			continue;
		}
		break;
	}
	return true;
}

bool network::send_common(common::String& str, int sock, SSL* ssl, bool secure) {
	if (sock < 0)return false;
	int res = 0;
	if (secure) {
		if (!ssl)return false;
		size_t written = 0;
		if (!SSL_write_ex(ssl, str.get_const(), str.get_size(), &written)) {
			return false;
		}
		return true;
	}
	else {
		res = ::send(sock, str.get_const(), str.get_size(), 0);
		return res >= 0;
	}
	return true;
}

bool HTTPClient::set_cookie() {
	for (auto i = 0u;;i++) {
		auto cookie = headers->idx("set-cookie", i);
		if (!cookie)break;
		io::Reader reader(ctype::strlen(cookie), cookie,nullptr,io::not_ignore);
		io::ReadStatus rs{ 0 };
		rs.num = '=';
		reader.readwhile(&rs, ctype::reader::Until);
		if (rs.failed)continue;
		if(!common::check_name_conflict(rs.buf.get_const(), cookies))continue;
		reader.expect("=");
	}
	return false;
}

bool HTTPClient::set_up_socket(const char* protocol,const char* hostname,unsigned short port) {
	if (!hostname||!protocol)return false;
	if (this->socket>=0) {
		clean_up_socket();
	}
	common::free(this->addr_str);
	this->addr_str = nullptr;
	addrinfo hint = { 0 }, * res = nullptr;
	hint.ai_socktype = SOCK_STREAM;
	auto num = getaddrinfo(hostname,protocol,&hint,&res);
	if (num != 0)return false;
	auto port_net= htons(port);
	for (auto p = res; p; p = p->ai_next) {
		auto tmpaddr4 = (sockaddr_in*)p->ai_addr;
		auto tmpaddr6 = (sockaddr_in6*)p->ai_addr;
		if (port) {
			tmpaddr4->sin_port = port_net;
		}
		auto sock = ::socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if (sock < 0)continue;
		if (::connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
			this->socket = sock;
			char buf[67] = "";
			if (p->ai_family==AF_INET6) {
				inet_ntop(p->ai_family,&tmpaddr6->sin6_addr,buf,sizeof(buf));
			}
			else {
				inet_ntop(p->ai_family, &tmpaddr4->sin_addr, buf, sizeof(buf));
			}
			this->addr_str = common::StringFilter() = buf;
			break;
		}
	}
	freeaddrinfo(res);
	if (this->socket<0)return false;
	return true;
}


bool HTTPClient::set_up_secure(const char* hostname) {
	if (!ctx) {
		ctx = SSL_CTX_new(TLS_method());
		if (!ctx)return false;
		SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
		unsigned char proto[] = "\x8http/1.1";
		SSL_CTX_set_alpn_protos(ctx, proto, 9);
		SSL_CTX_load_verify_locations(ctx, cacert_file, NULL);
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
		SSL_CTX_set_info_callback(ctx,infocb);
	}
	if (ssl) {
		clean_up_secure();
	}
	ssl = SSL_new(ctx);
	if (!ssl)return false;
	SSL_set_fd(ssl, this->socket);
	SSL_set_tlsext_host_name(ssl, hostname);
	auto param = SSL_get0_param(ssl);
	if (!X509_VERIFY_PARAM_add1_host(param, hostname, 0)) {
		invoke_errcb();
		dont_call_shutdown = true;
		return false;
	}
	if (SSL_connect(ssl) != 1) {
		invoke_errcb();
		dont_call_shutdown = true;
		return false;
	}
	this->secure = true;
	return true;
}

bool HTTPClient::clean_up_secure() {
	if (!ssl)return false;
	if (!dont_call_shutdown) {
		while (true) {
			auto res = SSL_shutdown(ssl);
			if (res < 0) {
				bool retry = false;
				auto reason = SSL_get_error(ssl, 0);
				switch (reason)
				{
				case(SSL_ERROR_WANT_READ):
				case(SSL_ERROR_WANT_WRITE):
					retry = true;
					break;
				default:
					break;
				}
				if (retry)continue;
				break;
			}
			else if (!res) {
				continue;
			}
			break;
		}
	}
	SSL_free(ssl);
	ssl = nullptr;
	secure = false;
	dont_call_shutdown = false;
	return true;
}


bool HTTPClient::clean_up_socket() {
	if (this->socket == invalid)return false;
	shutdown(this->socket, SD_SEND);
	shutdown(this->socket, SD_RECEIVE);
#ifndef _WIN32
#define closesocket(x) close(x)
#endif
	closesocket(this->socket);
	this->socket = invalid;
	return true;
}

bool read_hostname(const char* name,io::ReadStatus* s) {
	if (!*name) {
		return false;
	}
	if (!s->begin) {
		s->begin = name;
	}
	if (ctype::is_usable_for_identifier(*name) || *name == '.' || *name == '-') {
	}
	else if (*name=='/'||*name==':') {
		return false;
	}
	else if (*name=='@') {
		if (s->flag1) {
			s->failed = true;
			return false;
		}
		s->begin = name;
		s->flag1 = true;
		return true;
	}
	else {
		s->failed = true;
		return false;
	}
	s->buf.add(*name);
	return true;
}

bool read_portnum(const char* name, io::ReadStatus* s) {
	if (!s->begin) {
		s->begin = name;
		s->num = 0;
	}
	if (!ctype::is_number(*name)) {
		if (ctype::is_unnamed(s->buf.get_const())) {
			s->failed = true;
		}
		else {
			s->num=strtoul(s->buf.get_const(),nullptr,10);
		}
		return false;
	}
	s->buf.add(*name);
	s->num++;
	if (s->num == 5) {
		s->num= strtoul(s->buf.get_const(), nullptr, 10);
		if (s->num > ((unsigned short)~0))s->failed=true;
		return false;
	}
	return true;
}


bool HTTPClient::parse_uri(const char* uri,common::String& protocol, common::String& hostname, uint16_t& port,common::String& path
						  ,const char* defaultpath) {
	if (!uri)return false;
	io::Reader reader(ctype::strlen(uri),uri,nullptr,io::not_ignore);
	bool has_proto = false;
	if (reader.expect("http:")) {
		protocol.add_copy("http", 4);
		has_proto = true;
	}
	else if (reader.expect("https:")) {
		protocol.add_copy("https", 5);
		has_proto = true;
	}
	else {
		protocol.add_copy("http", 4);
	}
	if (!reader.expect("//"))if(has_proto)return false;
	io::ReadStatus s = { 0 };
	reader.readwhile(&s,read_hostname);
	if (s.failed)return false;
	hostname = std::move(s.buf);
	if (reader.expect(":")) {
		io::ReadStatus s2{ 0 };
		reader.readwhile(&s2,read_portnum);
		if (s2.failed)return false;
		port = (uint16_t)s2.num;
	}
	if (reader.ahead("/")) {
		s.buf.clear();
		reader.readwhile(&s, ctype::reader::End);
		path = std::move(s.buf);
	}
	else {
		path.add_copy(defaultpath, ctype::strlen(defaultpath));
	}
	if (!reader.eof())return false;
	return true;
}

bool HTTPClient::make_request(common::String& res,const char* method, const char* path, const char* host,const char* body,size_t bodysize) {
	if (!method || !path || !host)return false;
	res.add_copy(method, ctype::strlen(method));
	res.add(' ');
	res.add_copy(path, ctype::strlen(path));
	res.add(' ');
	res.add_copy("HTTP/1.1\r\nhost: ", 16);
	res.add_copy(host,ctype::strlen(host));
	res.add_copy("\r\n", 2);
	//res.add_copy("user-agent: client\r\naccept: */*\r\n", 33);
	if (body&&bodysize) {
		res.add_copy("content-length: ", 16);
		auto lenstr = std::to_string(bodysize);
		res.add_copy(lenstr.c_str(), lenstr.size());
		res.add_copy("\r\n", 2);
	}
	if (headercb) {
		headercb(this, res);
	}
	res.add_copy("\r\n",2);
	if (body&&bodysize) {
		res.add_copy(body, bodysize);
	}
	return true;
}

bool HTTPClient::send(common::String& str) {
	if (!send_common(str,this->socket,this->ssl,this->secure)) {
		if (this->secure)invoke_errcb();
		return false;
	}
	return true;
}

bool HTTPClient::recv(common::String& str) {
	/*
	while (true) {
		size_t size = 0;
		char tmpbuf[1024] = { 0 };
		if (this->secure) {
			if (!ssl)return false;
			while (true) {
				bool retry = false;
				if (!SSL_read_ex(ssl, tmpbuf, 1024, &size)) {
					auto reason = SSL_get_error(ssl, 0);
					switch (reason)
					{
					case(SSL_ERROR_WANT_READ):
					case(SSL_ERROR_WANT_WRITE):
						retry = true;
						break;
					default:
						break;
					}
					if (retry)continue;
					invoke_errcb();
					return false;
				}
				break;
			}
		}
		else {
			auto res = ::recv(this->socket, tmpbuf, 1024, 0);
			if (res <= 0) {
				return false;
			}
			size = res;
		}
		str.add_copy(tmpbuf, size);
		if (size == 1024) {
			continue;
		}
		break;
	}*/
	if (!recv_common(str, this->socket, this->ssl, this->secure)) {
		if (this->secure)invoke_errcb();
		return false;
	}
	return true;
}

bool HTTPClient::parse_httpresponse(common::String& httpresponse, BodyFlag bodyf) {
	common::kill(this->headers);
	this->headers = common::create<HTTPHeaders>();
	if (!this->headers) {
		return false;
	}
	io::Reader reader(httpresponse,nullptr,io::not_ignore);
	if (!reader.expect("HTTP/1.1 ")&&!reader.expect("HTTP/1.0 ") && !reader.expect("HTTP/0.9 ")) {
		return false;
	}
	char code[] = "000";
	reader.read_byte(code, io::translate_byte_as_is, 3);
#define Num(x) (x-'0')
	this->headers->set_code(Num(code[0])*100+Num(code[1])*10+Num(code[2]));
	io::ReadStatus s{ 0 };
	s.num = '\r';
	reader.readwhile(&s, ctype::reader::Until);
	reader.expect("\r\n");
	while (!this->headers->parse(reader)) {
		if (!this->recv(reader.buf_ref())) {
			return false;
		}
	}
	this->read_body(reader,bodyf);
	this->headers->set_raw(reader.buf_ref());
	return true;
}

bool HTTPClient::read_body(io::Reader& reader, BodyFlag bodyf) {
	common::free(this->_body);
	this->_body = nullptr;
	this->body_len = 0;
	if (bodyf == BodyFlag::mustnot)return false;
	common::String read;
	bool chunked = false,has_len=false;
	const char* type = nullptr;
	uint64_t len = 0;
	auto tmp1=this->headers->idx("transfer-encoding");
	if (tmp1) {
		if (strstr(tmp1, "chunked")) {
			chunked = true;
		}
	}
	if(!chunked) {
		tmp1 = this->headers->idx("content-length");
		if (tmp1) {
			len = strtoull(tmp1, nullptr, 10);
			has_len = true;
		}
	}
	if (!has_len) {
		tmp1 = this->headers->idx("content-type");
		if (tmp1) {
			type = tmp1;
		}
	}
	if (chunked) {
		if (!ctype::is_hexnumber(reader.abyte())) {
			if (!this->recv(reader.buf_ref()))return false;
			reader.release_eof();
		}
		while (1) {
			io::ReadStatus rs{0};
			rs.num = '\r';
			reader.readwhile(&rs,ctype::reader::Until);
			if (rs.failed)return false;
			reader.expect("\r\n");
			len = strtoull(rs.buf.get_const(), nullptr, 16);
			if (len == 0)break;
			while (reader.buf_ref().get_size() - reader.get_readpos() <= len+2) {
				if (!this->recv(reader.buf_ref()))return false;
				reader.release_eof();
			}
			for (auto i = 0ull; i < len;i++) {
				char tmp2 = 0;
				reader.read_byte(&tmp2);
				read.add(tmp2);
			}
			while(!reader.expect("\r\n")){
				if (!this->recv(reader.buf_ref()))return false;
				reader.release_eof();
			}
			if (!ctype::is_hexnumber(reader.abyte())) {
				if (!this->recv(reader.buf_ref()))return false;
				reader.release_eof();
			}
		}
	}
	else if(has_len){
		while (reader.readable_size() < len) {
			if (!this->recv(reader.buf_ref()))return false;
			reader.release_eof();
		}
		for (auto i = 0ull; i < len; i++) {
			char tmp2 = 0;
			reader.read_byte(&tmp2);
			read.add(tmp2);
		}
	}
	else if (type) {
		if (!read_by_content_type(reader, read, type)) return false;
	}
	else if(bodyf==BodyFlag::effort){
		while (!reader.eof()) {
			char tmp2 = 0;
			reader.read_byte(&tmp2);
			read.add(tmp2);
		}
	}
	else {
		return false;
	}
	this->body_len = read.get_size();
	this->_body = read.get_raw_z();
	if (!this->_body)return false;
	return true;
}

bool HTTPClient::read_by_content_type(io::Reader& reader, common::String& read, const char* type) {
	auto base_readpos = reader.get_readpos();
	if (strstr(type, "html")) {
		bool html = false;
		reader.set_ignore(io::ignore_space_and_line);
		while (!reader.eof()) {
			char tmp = 0;
			reader.read_byte(&tmp);
			if (tmp == '<') {
				if (reader.expect("html")||reader.expect("HTML")) {
					html = true;
					break;
				}
			}
		}
		while (html) {
			if (reader.eof()) {
				if (!this->recv(reader.buf_ref()))return false;
				reader.release_eof();
			}
			char tmp = 0;
			reader.read_byte(&tmp);
			if (tmp == '<') {
				if (reader.eof()) {
					if (!this->recv(reader.buf_ref()))return false;
					reader.release_eof();
				}
				if (reader.expect("/")) {
					if (reader.eof()) {
						if (!this->recv(reader.buf_ref()))return false;
						reader.release_eof();
					}
					if (reader.expect("html") || reader.expect("HTML")) {
						break;
					}
				}
			}
		}
	}
	else if (strstr(type,"json")) {
		if (reader.ahead("{")) {
			while (!reader.block("{","}")) {
				if (!this->recv(reader.buf_ref()))return false;
				reader.seek(base_readpos);
			}
		}
		else if (reader.ahead("[")) {
			while (!reader.block("[", "]")) {
				if (!this->recv(reader.buf_ref()))return false;
				reader.seek(base_readpos);
			}
		}
	}
	reader.seek(base_readpos);
	reader.set_ignore(io::not_ignore);
	while (!reader.eof()) {
		char tmp = 0;
		reader.read_byte(&tmp);
		read.add(tmp);
	}
	return true;
}

bool HTTPClient::method_detail(const char* uri,const char* method,BodyFlag bodyf,const char* defaultpath,const char* body,size_t bodysize,bool fullpath) {
	uint16_t port = 0;
	common::String protocol, host, path;
	if (!parse_uri(uri, protocol, host, port, path,defaultpath))return false;
	common::String request,hostwithport,responce;
	const char* passpath = nullptr;
	hostwithport.add_copy(host.get_const(), host.get_size());
	if (port) {
		auto portstr = std::to_string(port);
		hostwithport.add(':');
		hostwithport.add_copy(portstr.c_str(), portstr.size());
	}
	if (fullpath) {
		passpath = uri;
	}
	else {
		passpath = path.get_const();
	}
	if (!make_request(request, method, passpath, hostwithport.get_const(),body,bodysize))return false;
	if (!set_up_socket(protocol.get_const(), host.get_const(), port))return false;
	bool res = false;
	if (ctype::streaq(protocol.get_const(), "https"))
		if (!set_up_secure(host.get_const()))goto End;
	if (!send(request))goto End;
	if (!recv(responce))goto End;
	if (!parse_httpresponse(responce,bodyf))goto End;
	if (!ctype::streaq(host.get_const(),headers->host)) {
		common::free(headers->host);
		headers->host = host.get_raw_z();
	}
	res = true;
End:
	if (secure)clean_up_secure();
	clean_up_socket();
	return res;
}

HTTPClient::InfoCallback HTTPClient::set_infocb(InfoCallback cb) {
	auto ret = this->infocb;
	this->infocb = cb;
	if (ctx) {
		SSL_CTX_set_info_callback(ctx, cb);
	}
	return ret;
}

HTTPClient::ErrorCallback HTTPClient::set_errcb(ErrorCallback cb) {
	auto ret = this->errcb;
	this->errcb = cb;
	return ret;
}

HTTPClient::HeaderCallback HTTPClient::set_headercb(HeaderCallback cb) {
	auto ret = this->headercb;
	if (cb&&ret!=cb) {
		HTTPHeaders testh;
		common::String test;
		cb(this, test);
		test.add_copy("\r\n", 2);
		io::Reader testr(test,nullptr,io::not_ignore);
		if (!testh.parse(testr))return cb;
		if (!testr.eof())return cb;
		if (testh.idx("content-length") || testh.idx("host"))return cb;
	}
	this->headercb = cb;
	return ret;
}

bool HTTPClient::method(const char* method, const char* uri,const char* body,size_t bodysize,bool redirect, unsigned char depth,const char* defaultpath, BodyFlag bodyf, bool fullpath) {
	if (!method)return false;
	if (!method_detail(uri, method, bodyf, defaultpath,body,bodysize,fullpath))return false;
	if (redirect) {
		if (depth == 0)return true;
		auto code = this->headers->code();
		if (code >= 301 && code <= 308) {
			auto res = this->headers->idx("location");
			if (!res)return false;
			if (res[0] == '/') {
				common::String p;
				if (strstr(uri, "https:")) {
					p.add_copy("https:", 6);
				}
				else {
					p.add_copy("http:", 5);
				}
				p.add_copy("//", 2);
				p.add_copy(this->headers->host, ctype::strlen(this->headers->host));
				p.add_copy(res, ctype::strlen(res));
				return this->method(method, p.get_const(),body,bodysize,true,depth-1,defaultpath,bodyf);
			}
			else {
				return this->method(method, res, body,bodysize,true, depth - 1, defaultpath, bodyf);
			}
		}
	}
	return true;
}

bool HTTPClient::get(const char* uri, bool redirect,unsigned char depth,bool head, BodyFlag flag) {
	if (head) {
		return method("HEAD", uri, nullptr,0,redirect, depth,"/",BodyFlag::mustnot);
	}
	else {
		return method("GET", uri, nullptr,0,redirect, depth,"/",flag);
	}
}

bool HTTPClient::options(const char* uri) {
	return method("OPTIONS",uri,nullptr,0,false,255,"*");
}

bool HTTPClient::trace(const char* uri) {
	return method("TRACE",uri);
}

bool HTTPClient::post(const char* uri, const char* body,size_t bodysize) {
	return method("POST", uri, body,bodysize);
}

bool HTTPClient::put(const char* uri, const char* body,size_t bodysize) {
	return method("PUT", uri, body,bodysize);
}

bool HTTPClient::patch(const char* uri, const char* body, size_t bodysize) {
	return method("PATCH", uri, body, bodysize);
}

bool HTTPClient::_delete(const char* uri) {
	return method("DELETE", uri);
}

bool HTTP2Client::recv(common::String& buf) {
	return false;
}

bool HTTP2Client::send(common::String& buf) {
	return false;
}

bool HTTP2Client::send_error(unsigned char errorcode) {
	return false;
}

bool HTTP2Client::read_frameheader(io::Reader& reader,int& len,unsigned char& type,FrameFlag& flag,int& id) {
	auto base = reader.get_readpos();
	char len_p[4] = { 0 };
	unsigned char flag_b = 0;
	if (reader.read_byte(&len_p[1], io::translate_byte_as_is, 3) != 3)goto Err;
	len = io::translate_byte_reverse<int>(len_p);
	if(reader.read_byte(&type)!=1)goto Err;
	if(reader.read_byte(&flag_b)!=1)goto Err;
	if(reader.read_byte(&id,io::translate_byte_reverse)!=4)goto Err;
	flag.byte = flag_b;
	return true;
Err:
	reader.seek(base);
	return false;
}

HeaderFrame* HTTP2Client::read_header(io::Reader& reader, FrameFlag flag, int id, int len) {
	unsigned char padded = 0,weight=0,more=0;
	int depended=-1;
	bool exclusive = false;
	if (flag.bit.f3) {
		more+=1;
		reader.read_byte(&padded);
	}
	if (flag.bit.f5) {
		reader.read_byte(&depended, io::translate_byte_reverse);
		if (depended<0) {
			exclusive = true;
			depended &= 0x7F'FF'FF'FF;
		}
		reader.read_byte(&weight);
		more += 5;
	}
	common::String databuf=nullptr;
	int size = len - padded - more;
	if (size < 0)return nullptr;
	while (reader.readable_size()<size) {
		this->recv(reader.buf_ref());
		reader.release_eof();
	}
	io::ReadStatus rs{ 0 };
	rs.num = size;
	reader.readwhile(&rs,ctype::reader::Count);
	databuf = std::move(rs.buf);
	if (padded) {
		rs.buf.init();
		rs.num = padded;
		reader.readwhile(&rs, ctype::reader::Count);
	}
	auto data = databuf.get_raw();
	return common::create<HeaderFrame>(data,id,flag,size);
}

bool HTTP2Client::read_continuation(io::Reader& reader, common::String& buf, int id) {
	int len=0;
	unsigned char type = 0;
	FrameFlag flag;
	int cmpid = 0;
	if (!read_frameheader(reader, len, type, flag, cmpid)) {
		if (!this->recv(reader.buf_ref()))return false;
	}
	if (cmpid != id||type!=0x9) {
		this->send_error(0x1);
		return false;
	}

}