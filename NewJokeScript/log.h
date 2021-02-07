/*license*/
#pragma once

namespace jokescript {
	namespace log {
		struct Log {
			bool syserr(const char* message);
			bool synerr(const char* message);
			bool symerr_val(const char* message,const char* val);
			bool unexpected_token(const char* expected,char but);
		};
	}
}