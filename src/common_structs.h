/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#pragma once

#define PROJECT_NAME jokescript

#define OutDebugInfo 1
#if OutDebugInfo 
#define OutDebugMemoryInfo(x) x
#else
#define OutDebugMemoryInfo(x)
#endif
#include"stdcpps.h"
namespace PROJECT_NAME {
    namespace common {
        void* calloc(size_t elm,size_t obj);
        void* realloc(void* p,size_t size);
        void free(void* p);
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

            bool construct_copy(EasyVector& from) {
                this->ps = nullptr;
                this->ps = from.ps;
                from.ps = nullptr;
                this->toadd = from.toadd;
                this->len = from.len;
                from.toadd = 0;
                from.len = 0;
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
                construct_copy(from);
            }

            EasyVector(EasyVector&& from) noexcept{
                construct_copy(from);
            }

            bool init() {
                if (ps && len == 10 && toadd == 0)return true;
                if(ps)free(ps);
                ps = (PType*)calloc(10, sizeof(PType));
                if (!ps)return false;
                len = 10;
                toadd = 0;
                return true;
            }

            bool clear() {
                if (!ps)init();
                for (auto i = 0ull; i < toadd;i++) {
                    ps[i] = 0;
                }
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
                    if(this->ps)free(this->ps);
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
                if (toadd == 0) {
                    unuse();
                    return nullptr;
                }
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
                if (!ps)return 0;
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
                if (ps) {
                    free(ps);
                    ps = nullptr;
                    len = 0;
                    toadd = 0;
                }
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
                if(ps)free(ps);
            }
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
            OutDebugMemoryInfo(std::cout << "create:" << ret << ":" << sizeof(T)<<"\n");
            return ret;
        }

        /*
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
        }*/

        template<class T, class ...Args>
        T* create(Args... args) {
            T* ret = nullptr;
            try {
                ret = new T(args...);
            }
            catch (...) {
                return nullptr;
            }
            OutDebugMemoryInfo(std::cout << "create(arg):" << ret << ":" << sizeof(T)<<"\n");
            return ret;
        }

        template<class T>
        void kill(T* obj) {
            OutDebugMemoryInfo(std::cout << "kill:"<<obj << "\n");
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

        template<class PType>
        struct EasyVectorP {
        private:
            EasyVector<PType>* p;
            bool allocate() {
                if (p)return true;
                p = common::create<common::EasyVector<PType>>();
                if (!p)return false;
                return true;
            }
        public:
            EasyVectorP() {
                p = nullptr;
            }

            EasyVectorP(std::nullptr_t) {
                p = nullptr;
            }
            
            EasyVectorP(EasyVectorP& from) = delete;


            EasyVectorP(EasyVectorP&& from) noexcept{
                this->p = from.p;
                from.p = nullptr;
            }

            PType operator[](uint64_t pos) const {
                if (!p)return 0;
                return p->operator[](pos);
            }

            EasyVectorP& operator=(EasyVectorP&& from) noexcept {
                unuse();
                this->p = from.p;
                from.p = nullptr;
                return *this;
            }

            EasyVectorP& operator=(std::nullptr_t) noexcept {
                unuse();
                return *this;
            }

            EasyVectorP& operator+=(EasyVectorP& from) noexcept {
                if (!from.p)return *this;
                if (!allocate())return *this;
                *this->p += *from.p;
                return *this;
            }

            bool operator==(EasyVectorP& cmp) noexcept {
                if (!this->p && !cmp.p)return true;
                if (!this->p || !cmp.p)return false;
                return *this->p == *cmp.p;
            }

            bool operator==(std::nullptr_t) noexcept {
                return !is_enable();
            }

            bool operator!=(EasyVectorP& cmp)noexcept {
                return !(*this == cmp);
            }

            bool operator!=(std::nullptr_t) noexcept {
                return is_enable();
            }

            bool init() {
                if (!p) {
                    return allocate();
                }
                else {
                    return p->init();
                }
            }

            bool crear() {
                if (!p) {
                    return allocate();
                }
                else {
                    p->clear();
                }
            }

            PType* begin() {
                if (!p)return nullptr;
                return p->begin();
            }

            PType* end() {
                if (!p)return nullptr;
                return p->end();
            }

            bool add(PType a) {
                if (!allocate())return false;
                return p->add(a);
            }

            bool add_nz(PType a) {
                if (!allocate())return false;
                return p->add(a);
            }
            
            bool add_copy(const PType* base, uint64_t size) {
                if (!allocate())return false;
                return p->add_copy(base, size);
            }

            PType* get_raw() {
                if (!p)return nullptr;
                return p->get_raw();
            }

            PType* get_raw_z() {
                if (!allocate())return nullptr;
                return p->get_raw_z();
            }

            const PType* get_const() const {
                if (!p)return nullptr;
                return p->get_const();
            }

            const PType get_end() const {
                if (!p)return 0;
                return p->get_end();
            }

            EasyVector<PType>* get_base() const {
                return p;
            }

            uint64_t get_size() const {
                if (!p)return 0;
                return p->get_size();
            }

            uint64_t get_cap() const {
                if (!p)return 0;
                return p->get_cap();
            }

            uint64_t get_index(PType e) const {
                if(!p)return (uint64_t)(~0);
                return p->get_index(e);
            }

            PType get_if(PType e) {
                if (!p)return 0;
                return p->get_if(e);
            }
            
            PType remove(uint64_t pos) {
                if (!p)return 0;
                return p->remove(pos);
            }

            bool remove_each(void (*rem)(PType)) {
                if (!p)return false;
                return p->remove_each(rem);
            }

            PType remove_if(PType elm) {
                if (!p)return 0;
                return p->remove_if(elm);
            }

            PType remove_top() {
                if (!p)return 0;
                return p->remove_top();
            }

            PType remove_end() {
                if (!p)return 0;
                return p->remove_end();
            }

            bool pack() {
                if (!p)return false;
                return p->pack();
            }

            bool pack_f() {
                if (!p)return true;
                if (!p->get_size()) {
                    return this->unuse();
                }
                else {
                    return p->pack();
                }
            }

            bool unuse() {
                if (p) {
                    kill(p);
                    p = nullptr;
                }
                return true;
            }

            bool is_enable() const {
                if (!p)return false;
                return p->is_enable();
            }
           
            uint64_t memused() const {
                if (!p)return sizeof(EasyVectorP);
                return sizeof(EasyVectorP) + p->memused();
            }

            ~EasyVectorP(){
               if(p)kill(p);
            }

        };

        using String = EasyVector<char>;
        using String16 = EasyVector<char16_t>;
        using String32 = EasyVector<char32_t>;

        using StringP = EasyVectorP<char>;
        using String16P = EasyVectorP<char16_t>;
        using String32P = EasyVectorP<char32_t>;

        struct StringFilter {
        private:
            String s;
        public:
            StringFilter& operator=(const char* str);
            operator char* ();
        };
    }   
}