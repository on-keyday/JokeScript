/*license*/

#include"common_io.h"

using namespace jokescript;

bool common::io::Input::readall(const char* filename) {
	FILE* fp = nullptr;
	fopen_s(&fp,filename, "r");
	if (!fp)return false;
	this->name.add_copy(filename,strlen(filename));
	return readfromfp(fp);
}

bool common::io::Input::readfromfp(FILE* fp) {
	if (!fp)return false;
	while (1) {
		int got = fgetc(fp);
		if (got == EOF) {
			break;
		}
		this->buf.add((char)got);
	}
	return true;
}