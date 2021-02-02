/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once 
#include<stdlib.h>
namespace jokescript {
    template<class PType>
    struct EasyVector {
    private:
        PType* ps;
        unsigned long long toadd;
        unsigned long long len;
        bool add_detail(PType p) {
            if (!ps)return false;
            if (toadd + 3 >= len) {
                PType* hold = (PType*)realloc(ps, sizeof(PType) * len * 2);
                if (!hold) {
                    return false;
                }
                for (auto i = len; i < len * 2; i++) {
                    hold[i] = 0;
                }
                ps = hold;
                len *= 2;
            }
            ps[toadd] = p;
            toadd++;
            return true;
        }
    public:

        EasyVector() {
            ps = nullptr;
            init();
        }

        EasyVector(nullptr_t) {
            ps = nullptr;
            toadd = 0;
            len = 0;
        }

        EasyVector(EasyVector& from) {
            this->ps = nullptr;
            this->ps = from.ps;
            from.ps = nullptr;
            this->toadd = from.toadd;
            this->len = from.len;
            from.toadd = 0;
            from.len = 0;
        }

        bool init() {
            free(ps);
            ps = (PType*)calloc(10, sizeof(PType));
            if (!ps)return false;
            len = 10;
            toadd = 0;
            return true;
        }

        PType operator[](unsigned long long pos) const {
            if (!ps)return 0;
            if (pos >= toadd)return 0;
            return ps[pos];
        }

        EasyVector& operator=(EasyVector& from) = delete;

        EasyVector& operator=(EasyVector&& from) noexcept {
            if (this != &from) {
                free(this->ps);
                this->ps = from.ps;
                from.ps = nullptr;
                this->toadd = from.toadd;
                this->len = from.len;
                from.toadd = 0;
                from.len = 0;
            }
            return *this;
        }


        bool add_copy(const PType* base,unsigned long long size) {
            if (!base)return false;
            for (auto i = 0ull; i < size; i++) {
                add(base[i]);
            }
            return true;
        }

        bool add(PType p) {
            return add_detail(p);
        }

        bool add_nz(PType p) {
            if (!p)return false;
            return add_detail(p);
        }


        PType remove(unsigned long long pos) {
            if (!ps)return 0;
            if (pos >= toadd)return 0;
            PType ret = ps[pos];
            for (auto i = pos + 1; i < toadd; i++) {
                ps[i - 1] = ps[i];
            }
            toadd--;
            return ret;
        }

        PType* get_raw() {
            if (!ps)return nullptr;
            PType* ret = (PType*)realloc(ps, toadd * sizeof(PType));
            if (!ret) {
                ret = ps;
            }
            ps = nullptr;
            return ret;
        }

        PType* get_raw_z() {
            if (!ps)return nullptr;
            PType* ret = (PType*)realloc(ps, (toadd + 1) * sizeof(PType));
            if (!ret) {
                ret = ps;
            }
            ps = nullptr;
            return ret;
        }

        const PType* get_const() const{
            return ps;
        }

        const PType get_end() const{
            return ps[toadd - 1];
        }

        unsigned long long get_size() const {
            return toadd;
        }

        bool remove_each(void (*rem)(PType)) {
            if (!ps)return false;
            unsigned long long i = 0;
            while (i < toadd) {
                rem(ps[i]);
                ps[i] = 0;
                i++;
            }
            toadd = 0;
            return true;
        }

        PType remove_if(PType elm) {
            PType ret = 0;
            bool flag = false;
            for (auto i = 0; i < toadd; i++) {
                if (!flag && ps[i] == elm) {
                    ret = elm;
                    flag = true;
                }
                else if (flag) {
                    ps[i - 1] = ps[i];
                }
            }
            if (flag) {
                toadd--;
                ps[toadd] = 0;
            }
            return ret;
        }

        bool pack() {
            if (!ps)return false;
            PType* ret = (PType*)realloc(ps, (toadd + 1) * sizeof(PType));
            if (!ret)return false;
            ps = ret;
            len = toadd + 1;
            return true;
        }

        bool pack_f() {
            if (!ps)return true;
            if (toadd) {
                return pack();
            }
            else {
                return unuse();
            }
        }

        bool unuse() {
            free(ps);
            ps = nullptr;
            len = 0;
            toadd = 0;
            return true;
        }

        bool is_enable() const{
            return ps != nullptr;
        }

        ~EasyVector() {
            free(ps);
        }
    };

    struct StringFilter {
    private:
        EasyVector<char> s;
    public:
        StringFilter& operator=(const char* str);
        operator char* ();
    };


   
}