/*license*/
#include"common_structs.h"
using namespace jokescript;

common::StringFilter& common::StringFilter::operator=(const char* str) {
    if (!str)return *this;
    unsigned long long i = 0;
    while (str[i]) {
        s.add(str[i]);
        i++;
    }
    return *this;
}

common::StringFilter::operator char* () {
    return s.get_raw_z();
}