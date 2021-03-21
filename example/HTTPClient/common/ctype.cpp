/*license*/
#include"structs.h"
#include"ctype.h"
//#include"compiler_id_analyzer.h"

using namespace PROJECT_NAME;
using namespace PROJECT_NAME::io;

bool ctype::is_escapable_control(char c) {
	return c == '\n' || c == '\t' || c == '\v' || c == '\r'|| c == '\b' || c == '\f';
}

bool ctype::is_first_of_identifier(char c) {
	return isalpha((unsigned char)c) || c == '_';
}

bool ctype::is_usable_for_identifier(char c) {
	return isalnum((unsigned char)c) || c == '_';
}

bool ctype::is_number(char c) {
	return isdigit((unsigned char)c);
}

bool ctype::is_hexnumber(char c) {
	return ctype::is_number(c) || ((unsigned char)c >= 'a' && (unsigned char)c <= 'f') || ((unsigned char)c >= 'A' && (unsigned char)c <= 'F');
}

bool ctype::is_octnumber(char c) {
	return ((unsigned char)c >= '0' && (unsigned char)c <= '7');
}

bool ctype::is_binnumber(char c) {
	return c == '0' || c == '1';
}

bool ctype::is_numbersuffix(char c) {
	return c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F';
}

bool ctype::is_first_of_string(char c) {
	return c == '\"' || c == '\'';
}

bool ctype::is_utf8_later(char c) {
	return 0x80 <= (unsigned char)c && (unsigned char)c < 0xC0;
}

bool ctype::is_unnamed(const char* c) {
	if (!c)return true;
	return c[0]=='\0';
}

bool ctype::reader::Number(const char* s, ReadStatus* status) {
	if (!*s) {
		status->logger->synerr("unexpected end of file.");
		status->failed = true;
		return false;
	}
	if (*s == '\'') {
		return true;
	}
	bool& doted = status->flag1;
	unsigned int& base = status->num;
	bool& u_suf = status->flag2;
	bool& l_suf = status->flag3;
	bool& f_suf = status->flag4;
	bool& eorp = status->flag5;
	if (!status->begin) {
		status->begin = s;
		if (*s!='0') {
			if (!is_number(*s)) {
				status->logger->synerr("unknown number like.");
				status->failed = true;
				return false;
			}
			base = 10;
		}
	}
	else if (!base) {
		if (*s == 'x' || *s == 'X') {
			base = 16;
		}
		else if (*s=='b'||*s=='B') {
			base = 2;
		}
		else if (is_octnumber(*s)) {
			base = 8;
		}
		else if (*s=='.') {
			base = 10;
			doted = true;
		}
		else {
			return false;
		}
	}
	else {
		if (base == 10) {
			if (!is_number(*s)) {
				if (*s == '.') {
					if (eorp) {
						status->logger->unexpected_token("0123456789f", *s);
						status->failed = true;
						return false;
					}
					else if (doted) {
						status->logger->unexpected_token("0123456789ef", *s);
						status->failed = true;
						return false;
					}
					doted = true;
				}
				else if (*s=='e') {
					if (eorp) {
						status->logger->unexpected_token("0123456789f", *s);
						status->failed = true;
						return false;
					}
					eorp = true;
				}
				else if (ctype::is_numbersuffix(*s)) {
					base = 100;
				}
				else if (*s == '+' || *s == '-') {
					if (s[-1] != 'e') {
						return false;
					}
				}
				else {
					if (eorp&&(s[-1]=='+'||s[-1]=='-')||s[-1]=='.') {
						status->logger->unexpected_token("0123456789", *s);
						status->failed = true;
					}
					return false;
				}
			}
		}
		else if (base == 16) {
			if (!eorp) {
				if (!is_hexnumber(*s)) {
					if (*s == '.') {
						if (eorp) {
							status->logger->unexpected_token("0123456789abcdefABCDEF", *s);
							status->failed = true;
							return false;
						}
						else if (doted) {
							status->logger->unexpected_token("0123456789abcdefABCDEFp", *s);
							status->failed = true;
							return false;
						}
						doted = true;
					}
					else if (*s == 'p') {
						if (strcmp(status->buf.get_const(), "0x") == 0) {
							status->logger->unexpected_token("0123456789abcdefABCDEF", *s);
							status->failed = true;
							return false;
						}
						eorp = true;
					}
					else if (is_numbersuffix(*s)) {
						base = 100;
					}
					else {
						if (strcmp(status->buf.get_const(),"0x")==0||s[-1]=='.') {
							status->logger->unexpected_token("0123456789abcdefABCDEF", *s);
							status->failed = true;
						}
						return false;
					}
				}
			}
			else {
				if (*s!='+'&&*s!='-') {
					status->logger->unexpected_token("+-", *s);
					status->failed = true;
					return false;
				}
				base = 10;
			}
			
		}
		else if (base==8) {
			if (!is_octnumber(*s)) {
				if (is_numbersuffix(*s)) {
					base = 100;
				}
				else {
					return false;
				}
			}
		}
		else if (base==2) {
			if (!is_octnumber(*s)) {
				if (is_numbersuffix(*s)) {
					base = 100;
				}
				else {
					if (strcmp(status->buf.get_const(), "0b") == 0) {
						status->logger->unexpected_token("01", *s);
						status->failed = true;
					}
					return false;
				}
			}
		}
		
		if (base == 100) {
			if (*s=='u'||*s=='U') {
				if (u_suf) {
					status->logger->unexpected_token("lL", *s);
					status->failed = true;
					return false;
				}
				u_suf = true;
			}
			else if (*s=='l'||*s=='L') {
				if (l_suf) {
					status->logger->unexpected_token("uU", *s);
					status->failed = true;
					return false;
				}
				l_suf = true;
			}
			else if (*s == 'f' || *s == 'F') {
				if (u_suf || l_suf) {
					status->logger->unexpected_token("lLuU", *s);
					status->failed = true;
					return false;
				}
				f_suf = true;
			}
			else {
				return false;
			}
		}
	}


	status->buf.add(*s);
	if (u_suf && l_suf||f_suf) {
		return false;
	}
	return true;
}

bool ctype::reader::Identifier(const char* s, ReadStatus* status) {
	if (!*s) {
		if (status->logger){
			status->logger->synerr("unexpected end of file.");
			status->failed = true;
		}
		return false;
	}
	if (!status->begin) {
		status->begin = s;
		if (!ctype::is_first_of_identifier(*s)) {
			if (ctype::is_number(*s)) {
				status->logger->unexpected_token("alphabet\" or \"_\"", *s);
				status->failed = true;
			}
			return false;
		}
	}
	else{
		if (!ctype::is_usable_for_identifier(*s)) {
			return false;
		}
	}
	status->buf.add(*s);
	return true;
}

bool ctype::reader::IdentifierPDot(const char* s, ReadStatus* status) {
	if (!*s) {
		if (status->logger) {
			status->logger->synerr("unexpected end of file.");
			status->failed = true;
		}
		return false;
	}
	if (!status->begin) {
		status->begin = s;
		if (!ctype::is_first_of_identifier(*s)) {
			if (ctype::is_number(*s)) {
				status->logger->unexpected_token("alphabet\" or \"_\"", *s);
				status->failed = true;
			}
			return false;
		}
	}
	else {
		if (!ctype::is_usable_for_identifier(*s)&&*s!='.') {
			return false;
		}
	}
	status->buf.add(*s);
	return true;
}

bool ctype::reader::DigitNumber(const char* s, ReadStatus* status) {
	if (!*s) {
		return false;
	}
	bool& doted = status->flag1;
	bool& eorp = status->flag2;
	if (!status->begin) {
		status->begin = s;
		if (*s == '0') {
			if (s[1]=='x'||s[1]=='b'||ctype::is_octnumber(s[1])) {
				status->failed = true;
				return false;
			}
		}
	}
	else {
		if (!is_number(*s)) {
			if (*s == '.') {
				if (eorp) {
					if (status->logger)status->logger->unexpected_token("0123456789", *s);
					status->failed = true;
					return false;
				}
				else if (doted) {
					if (status->logger)status->logger->unexpected_token("0123456789", *s);
					status->failed = true;
					return false;
				}
				doted = true;
			}
			else if (*s == 'e') {
				if (eorp) {
					if(status->logger)status->logger->unexpected_token("0123456789", *s);
					status->failed = true;
					return false;
				}
				eorp = true;
			}
			else if (*s == '+' || *s == '-') {
				if (s[-1] != 'e') {
					status->failed = true;
					return false;
				}
			}
			else {
				if (eorp && (s[-1] == '+' || s[-1] == '-') || s[-1] == '.') {
					if (status->logger)status->logger->unexpected_token("0123456789", *s);
					status->failed = true;
				}
				return false;
			}
		}
	}
	status->buf.add(*s);
	return true;
}

bool ctype::reader::Until(const char* s, ReadStatus* status) {
	if (!*s) {
		return false;
	}
	if (*s == status->num)return false;
	status->buf.add(*s);
	return true;
}

bool ctype::reader::End(const char* s, ReadStatus* status) {
	if (!*s)return false;
	status->buf.add(*s);
	return true;
}

bool ctype::is_utf16_surrogate_high(char16_t c) {
	return 0xD800 <= c && c < 0xDC00;
}

bool ctype::is_utf16_surrogate_low(char16_t c) {
	return 0xDC00 <= c && c < 0xE000;
}

bool ctype::get_number_type(const char* num,uint64_t& numres,int& bit_size,bool& is_float,bool& is_unsigned) {
	if (!num)return false;
	auto size = strlen(num);
	if (!size)return false;
	const auto max_v = 0xFF'FF'FF'FF'FF'FF'FF'FF;
	const char* check = "18446744073709551615";
	bool lf = false;
	int base = 10;
	int ofs = 0;
	char flags = 0;

	if (strstr(num, ".") || strstr(num, "p") || strstr(num, "+") || strstr(num, "-")) {
		if (num[size - 1] == 'f' || num[size - 1] == 'F') {
			bit_size = 32;
			is_float = true;
			is_unsigned = false;
			return true;
		}
		else {
			bit_size = 64;
			is_float = true;
			is_unsigned = false;
			return true;
		}
	}
	if (num[0] != '0' && (strstr(num, "f") || strstr(num, "F"))) {
		bit_size = 32;
		is_float = true;
		is_unsigned = false;
		return true;
	}

	is_float = false;
	if (strstr(num, "u") || strstr(num, "U")) {
		is_unsigned = true;
	}
	if (strstr(num, "l") || strstr(num, "L")) {
		lf = true;
	}

	if (num[0] == '0') {
		if (num[1] == 'x') {
			base = 16;
			ofs = 2;
			//judge = ctype::is_hexnumber;
			//criteria = 2;
			//flags = '8';
			check = "FFFFFFFFFFFFFFFF";
		}
		else if (num[1] == 'b') {
			base = 2;
			ofs = 2;
			//judge = ctype::is_binnumber;
			//criteria = 8;
			//flags = '1';
			check = "1111111111111111111111111111111111111111111111111111111111111111";
		}
		else if (ctype::is_octnumber(num[1])) {
			base = 8;
			ofs = 1;
			//judge = ctype::is_octnumber;
			check = "1777777777777777777777";
		}
	}


	
	char* end = nullptr;
	auto res = strtoull(&num[ofs], &end, base);
	if (*end != '\0' && !ctype::is_numbersuffix(*end)) {
		numres = 0;
		return false;
	}
	if (res == max_v) {
		auto len = strlen(check);
		if (strncmp(&num[ofs], check, len) == 0 && num[ofs + len] != '\0' && !ctype::is_numbersuffix(num[ofs + len])) {
			//holder->logger->semerr("too large number not to be compilable.");
			numres = max_v;
			return false;
		}
	}
	else if (res == 0) {
		if (num[ofs] != '0') {
			numres = 0;
			//holder->logger->synerr("unknown number like.");
			return false;
		}
	}
	if (res <= 0xFF) {
		bit_size = 8;
		if (res > 0x7F) {
			is_unsigned = true;
		}
	}
	else if (res <= 0xFFFF) {
		bit_size = 16;
		if (res > 0x7FFF) {
			is_unsigned = true;
		}
	}
	else if (res <= 0xFFFFFFFF) {
		bit_size = 32;
		if (res > 0x7FFFFFFF) {
			is_unsigned = true;
		}
	}
	else {
		bit_size = 64;
		if (res > 0x7FFFFFFFFFFFFFFF) {
			is_unsigned = true;
		}
	}
	if (lf)bit_size = 8;
	numres = res;
	return true;
}

bool ctype::uri::is_scheme_ok(char c) {
	if (isalnum((unsigned char)c)) {
		return true;
	}
	if (c == '+' || c == '-' || c == '.') {
		return true;
	}
	return false;
}
bool ctype::uri::is_authority_ok(char c) {
	if (isalnum((unsigned char)c)) {
		return true;
	}
	if (c == '+' || c == '-' || c == '.' || c == '@' || c == ':') {
		return true;
	}
	return false;
}

bool ctype::uri::is_path_ok(char c) {
	if (isalnum((unsigned char)c)) {
		return true;
	}
	if (c == '-' || c == '_' || c == '~' || c == '.' || c == '/') {
		return true;
	}
	return false;
}

bool ctype::uri::is_query_ok(char c) {
	if (isalnum((unsigned char)c)) {
		return true;
	}
	if (c == '-' || c == '_' || c == '~' || c == '=' || c == '&') {
		return true;
	}
	return false;
}