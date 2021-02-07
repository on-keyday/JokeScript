/*license*/
#include"log.h"
using namespace jokescript;

bool log::Log::synerr(const char* message) {
	return false;
}

bool log::Log::syserr(const char* message) {
	return false;
}

bool log::Log::symerr_val(const char* message, const char* val) {
	return false;
}

bool log::Log::unexpected_token(const char* expected, char but) {
	"unexpected token. expected 'x' or 'X','b' or 'B',or octnumber";
	return false;
}