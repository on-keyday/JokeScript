/*license*/

#include"io.h"

using namespace PROJECT_NAME;

#ifndef _WIN32
#define fopen_s(fp,filename,mode) ((*fp)=fopen(filename,mode))
#endif

bool common::io_base::Input::readall(const char* filename,const char* mode) {
	FILE* fp = nullptr;
	fopen_s(&fp,filename, mode);
	if (!fp)return false;
	this->name.add_copy(filename,strlen(filename));
	this->name.pack_f();
	auto res=readfromfp(fp);
	fclose(fp);
	return res;
}

bool common::io_base::Input::readfromfp(FILE* fp) {
	if (!fp)return false;
	while (1) {
		int got = fgetc(fp);
		if (got == EOF) {
			break;
		}
		this->buf.add((char)got);
	}
	this->buf.pack_f();
	return true;
}

bool common::io_base::OutPut::writeall(const char* filename,bool bin, const char* byte, uint64_t size) {
	FILE* fp = nullptr;
	if (!bin) {
		fopen_s(&fp, filename, "w");
	}
	else {
		fopen_s(&fp, filename, "wb");
	}
	if (!fp)return false;
	auto res=writetofp(fp, byte, size);
	fclose(fp);
	return res;
}

bool common::io_base::OutPut::writetofp(FILE* fp,const char* byte,uint64_t size) {
	if (!fp)return false;
	return (bool)fwrite(byte, size, 1, fp);
}