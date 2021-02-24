/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#define PROJECT_NAME jokescript

#include"stdcpps.h"
namespace PROJECT_NAME {
    namespace common {
        template<class PType> 
        struct EasyVector {
        private:
            PType* ps;
            uint64_t toadd;
            uint64_t len;
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

            using type = PType;

            EasyVector() {
                ps = nullptr;
                init();
            }

            EasyVector(std::nullptr_t) {
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
                if (ps && len == 10 && toadd == 0)return true;
                free(ps);
                ps = (PType*)calloc(10, sizeof(PType));
                if (!ps)return false;
                len = 10;
                toadd = 0;
                return true;
            }

            PType operator[](uint64_t pos) const {
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

            EasyVector& operator=(std::nullptr_t) noexcept {
                unuse();
            }


            EasyVector& operator+=(EasyVector& from) noexcept{
                add_copy(from.get_const(),from.get_size());
                return *this;
            }

            bool operator==(EasyVector& cmp) noexcept{
                if (toadd != cmp.toadd)return false;
                for (auto i = 0ull; i < toadd;i++) {
                    if (ps[i] != cmp.ps[i])return false;
                }
                return true;
            }

            bool operator==(std::nullptr_t) noexcept{
                return !is_enable();
            }

            bool operator!=(EasyVector& cmp)noexcept {
                return !(*this == cmp);
            }

            bool operator!=(std::nullptr_t) noexcept {
                return is_enable();
            }

            bool add_copy(const PType* base, uint64_t size) {
                if (!ps||!base)return false;
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


            PType remove(uint64_t pos) {
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
                toadd = 0;
                len = 0;
                return ret;
            }

            PType* get_raw_z() {
                if (!ps)return nullptr;
                PType* ret = (PType*)realloc(ps, (toadd + 1) * sizeof(PType));
                if (!ret) {
                    ret = ps;
                }
                ps = nullptr;
                toadd = 0;
                len = 0;
                return ret;
            }

            const PType* get_const() const {
                return ps;
            }

            const PType get_end() const {
                return ps[toadd - 1];
            }

            uint64_t get_size() const {
                return toadd;
            }

            uint64_t get_cap() const {
                return len;
            }

            uint64_t get_index(PType p) const{
                auto i = 0ull;
                while (i < toadd) {
                    if (ps[i] == p) {
                        return i;
                    }
                    i++;
                }
                return (uint64_t)(~0);
            }

            PType get_if(PType p) {
                auto i = 0ull;
                while (i < toadd) {
                    if (ps[i] == p) {
                        return ps[i];
                    }
                    i++;
                }
                return 0;
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
                if (!ps)return 0;
                if (toadd == 0)return 0;
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

            PType remove_top() {
                if (!ps)return 0;
                if (toadd == 0)return 0;
                PType ret = 0;
                for (auto i = 0; i < toadd;i++) {
                    if (i == 0) {
                        ret = ps[0];
                    }
                    else {
                        ps[i - 1] = ps[i];
                    }
                }
                ps[toadd - 1] = 0;
                toadd--;
                return ret;
            }

            PType remove_end() {
                if (!ps)return 0;
                if (toadd == 0)return 0;
                PType ret = ps[toadd - 1];
                ps[toadd - 1] = 0;
                toadd--;
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

            bool is_enable() const {
                return ps != nullptr;
            }

            PType* begin() {
                return ps;
            }

            PType* end() {
                return &ps[toadd];
            }

            uint64_t memused() const{
                uint64_t ret = sizeof(EasyVector);
                ret += len * sizeof(PType);
                return ret;
            }

            ~EasyVector() {
                free(ps);
            }
        };

        using String = EasyVector<char>;
        using String16 = EasyVector<char16_t>;
        using String32 = EasyVector<char32_t>;

        struct StringFilter {
        private:
            String s;
        public:
            StringFilter& operator=(const char* str);
            operator char* ();
        };

        template<class T>
        T* create() {
            T* ret = nullptr;
            try {
                ret = new T;
            }
            catch (...) {
                return nullptr;
            }
            return ret;
        }

        template<class T,class ...Args>
        T* create(Args&... args) {
            T* ret = nullptr;
            try {
                ret = new T(args...);
            }
            catch (...) {
                return nullptr;
            }
            return ret;
        }

        template<class T, class ...Args>
        T* create(Args... args) {
            T* ret = nullptr;
            try {
                ret = new T(args...);
            }
            catch (...) {
                return nullptr;
            }
            return ret;
        }

        template<class T>
        void kill(T* obj) {
            delete obj;
        }

        template<class T1, class T2>
        struct SimplePair {
            T1 t1;
            T2 t2;

            SimplePair() = delete;

            SimplePair(T1 one, T2 two) {
                t1 = one;
                t2 = two;
            }

            operator T1() {
                return t1;
            }
            operator T2() {
                return t2;
            }
        };
    }
}