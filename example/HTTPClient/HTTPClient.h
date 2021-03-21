#pragma once
#include<WinSock2.h>
#include"common/structs.h"
#include"common/filereader.h"
#include"common/tools.h"
#include<openssl/ssl.h>
#include<openssl/err.h>

namespace network {
	struct Header {
		char* name=nullptr;
		char* value=nullptr;
		~Header() { common::free(name); common::free(value); }
	};

	struct HTTPHeaders {
	private:
		common::String raw_data;
		unsigned short statuscode=0;
		common::EasyVector<Header*> headers;
	public:
		bool add(char* name,char* value);
		bool parse(io::Reader& reader);
		char* host = nullptr;
		bool set_code(unsigned short code) { statuscode = code; return true; }
		bool set_raw(common::String& raw) { raw_data = std::move(raw); return true; }
		unsigned short code() const{ return statuscode; };
		const char* raw() const{ return raw_data.get_const(); }
		~HTTPHeaders(){
			headers.remove_each(common::kill);
		}
		char* idx(const char* s,uint64_t pos=0) {
			auto res=common::search_T(s, this->headers,pos);
			if (res) {
				return res->value;
			}
			return nullptr;
		}

	};

	enum class SameSite {
		unset,
		strict,
		lax,
		none
	};

	struct Cookie {
		char* name=nullptr;
		char* value = nullptr;
		char* domain = nullptr;
		char* path = nullptr;
		bool secure = false;
		bool httponly = false;
		SameSite site = SameSite::unset;
	};

	enum class BodyFlag {
		onheader,
		mustnot,
		effort
	};

	struct HTTPClient {
	private:
		typedef void (*InfoCallback)(const SSL* ssl,int type,int val);
		typedef int (*ErrorCallback)(const char* str,size_t len,void* opt);
		InfoCallback infocb = nullptr;
		ErrorCallback errcb=nullptr;
		static const int invalid = ~0;
		const char* cacert_file="cacert.pem";
		int socket=invalid;
		SSL_CTX* ctx = nullptr;
		SSL* ssl=nullptr;
		bool secure = false;
		bool dont_call_shutdown = false;
		HTTPHeaders* headers=nullptr;
		common::EasyVector<Cookie*> cookies;
		char* _body = nullptr;
		uint64_t body_len=0;
		bool set_cookie();
		void invoke_errcb() { if (errcb)ERR_print_errors_cb(errcb,nullptr); }
		bool set_up_socket(const char* protocol,const char* hostname, unsigned short port);
		bool set_up_secure(const char* hostname);
		bool clean_up_secure();
		bool clean_up_socket();
		bool parse_uri(const char* uri,common::String& protocol,common::String& hostname,uint16_t& port, common::String& path,const char* defaultpath);
		bool make_request(common::String& res,const char* method, const char* path, const char* host,const char* body,size_t bodysize);
		bool send(common::String& str);
		bool recv(common::String& str);
		bool parse_httpresponse(common::String& httpresponce, BodyFlag bodyf);
		bool read_body(io::Reader& reader, BodyFlag bodyf);
		bool read_by_content_type(io::Reader& reader, common::String& read,const char* type);
		bool method_detail(const char* uri,const char* method,BodyFlag bodyf,const char* defaultpath,const char* body,size_t bodysize);
	public:
		HTTPClient() = default;
		HTTPClient(HTTPClient&) = delete;
		HTTPClient(const char* file):cacert_file(file){}
		InfoCallback set_infocb(InfoCallback cb);
		ErrorCallback set_errcb(ErrorCallback cb);
		bool method(const char* method, const char* uri,const char* body=nullptr,size_t bodysize=0,bool redirect = false, unsigned char depth = 255,const char* defaultpath="/",BodyFlag bodyf=BodyFlag::onheader);
		bool get(const char* uri, bool redirect = false,unsigned char depth=255,bool head = false,BodyFlag flag=BodyFlag::onheader);
		bool options(const char* uri);
		bool trace(const char* uri);
		bool post(const char* uri,const char* body, size_t bodysize);
		bool put(const char* uri, const char* body, size_t bodysize);
		const char* body() const{ return _body; }
		uint64_t len() const { return body_len; };
		const char* raw() const { if (!headers)return nullptr; return headers->raw(); }
		const char* header(const char* name,uint64_t pos=0) { if (!headers)return nullptr; return headers->idx(name,pos); };
		unsigned short statuscode()const{ if (!headers)return 0; return headers->code(); }
		~HTTPClient(){
			common::free(_body);
			common::kill(headers);
			if (ssl) {
				SSL_free(ssl);
			}
			if (ctx) {
				SSL_CTX_free(ctx);
			}	
		}
	};
}