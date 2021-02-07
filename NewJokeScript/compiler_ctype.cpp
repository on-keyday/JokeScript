/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"stdcpps.h"
#include"compiler_ctype.h"

using namespace PROJECT_NAME;

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

bool ctype::is_unnamed(const char* c) {
	if (!c)return true;
	return c[0]=='\0';
}

bool ctype::reader::Number(const char* s, compiler::ReadStatus* status) {
	if (!*s) {
		status->logger->synerr("unexpected end of file.");
		status->failed = true;
		return false;
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

bool ctype::reader::Identifier(const char* s, compiler::ReadStatus* status) {
	if (!*s) {
		status->logger->synerr("unexpected end of file.");
		status->failed = true;
		return false;
	}
	if (!status->begin) {
		status->begin = s;
		if (!ctype::is_first_of_identifier(*s)) {
			if (ctype::is_number(*s)) {
				status->logger->unexpected_token("abcdefghijoklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", *s);
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