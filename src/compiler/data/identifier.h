/*licnese*/
#pragma once
#include"../../common/structs.h"
#include"../../common/filereader.h"
#include"../../common/ctype.h"

namespace PROJECT_NAME {
	namespace identifier {
		struct Type;
		struct Macro;
		struct Variable;
		struct Template;
		struct TypeOption;
		struct SyntaxTree;

		enum class TypeKind {
			void_t,
			bit_t,
			pointer_t,
			function_t,
			operator_t,
			has_size_t,
			const_t,
			struct_t,
			instance_struct_t,
			instance_functon_t,
			named_t,
			alias_t,
			unknown_t
		};
		
		

		struct Members {
			common::EasyVectorP<Type*> types;
			common::EasyVectorP<Macro*> macros;
			common::EasyVectorP<Variable*> vars;
			common::EasyVectorP<Template*> templates;
		};
		/*
		template<class T, class Char>
		bool check_name_conflict(const Char* name, common::EasyVectorP<T*>& vec) {
			for (auto it : vec) {
				if (ctype::streaq(it->name, name)) {
					return false;
				}
			}
			return true;
		}

		template<class T, class Char>
		T* search_T(const Char* name, common::EasyVectorP<T*>& vec) {
			for (auto it : vec) {
				if (ctype::streaq(name, it->name)) {
					return it;
				}
			}
			return nullptr;
		}*/

		struct MembersP {
		private:
			Members* p;
		public:
			MembersP() { p = nullptr; }
			MembersP(std::nullptr_t) { p = nullptr; }
			operator bool() { return p != nullptr; }
			bool unuse() { delete p; p = nullptr; return true; }
			bool init() { if (p)unuse(); p = common::create<Members>(); return p != nullptr; }

			template<class T>
			bool name_conflict(const char* name);

			template<class T>
			bool add(T* t);

			
			Members* get_base() { return p; };
		};

		struct Type {
			Type(){}
			char* name = nullptr;
			union {
				Type* type = nullptr;
				Template* templates;
			}base;
			TypeKind kind = TypeKind::void_t;
			uint64_t size = 0;
			union {
				MembersP members=nullptr;
				common::EasyVectorP<Type*> params;
			};
			common::EasyVectorP<TypeOption*> options;
			common::EasyVectorP<Type*> derived;
			common::EasyVectorP<Type*> function;
			common::EasyVectorP<Type*> operators;
			~Type() {
				common::free(name);
				if (kind == TypeKind::function_t && params.is_enable()) {
					params.unuse();
				}
				else if (members) {
					members.unuse();
				}
			}
		};

		struct Template {
			char* name = nullptr;
			common::EasyVectorP<SyntaxTree*> syntaxs;
			common::EasyVectorP<Type*> params;
			common::EasyVectorP<Type*> instances;
			~Template() { common::free(name); }
		};


		struct Macro {
			char* name = nullptr;
		};

		struct Variable {
			char* name = nullptr;
			Type* type = nullptr;
			SyntaxTree* init = nullptr;
		};

		enum class OptionKind {
			str,
			flag,
			vars
		};

		struct TypeOption {
			char* name = nullptr;
			OptionKind kind = OptionKind::flag;
			union {
				char* str = nullptr;
				bool flag;
				common::EasyVectorP<Variable*> vars;
			};

			~TypeOption() {
				if (kind == OptionKind::str) {
					common::free(str);
				}
				else if (kind == OptionKind::vars) {
					vars.unuse();
				}
			}
		};

		enum class TreeKind {
			block
		};

		struct SyntaxTree {
			char* symbol = nullptr;
			TreeKind kind = TreeKind::block;
			Type* type = nullptr;
			Variable* var = nullptr;
			MembersP members;
			SyntaxTree* left = nullptr;
			SyntaxTree* right = nullptr;
		};

		struct ScopeStack {
			ScopeStack(MembersP& memb) :memb(memb){}
			ScopeStack(ScopeStack&) = delete;
			ScopeStack(ScopeStack&&) = delete;
			ScopeStack* prev=nullptr;
			MembersP& memb;
			common::EasyVectorP<SyntaxTree*>* tree=nullptr;
		};

		struct Maker {
		private:
			Type* void_t = nullptr;
			Type* bit_t_signed = nullptr;
			Type* bit_t_unsigned = nullptr;
			Type* bit_t_float = nullptr;
			io::ReadStatus status;
			common::EasyVectorP<Type*> types;
			common::EasyVectorP<Macro*> macros;
			common::EasyVectorP<Variable*> vars;
			common::EasyVectorP<Template*> templates;
		public:
			Type* (*type_decider)(Type*,Type*,Maker*)=nullptr;
			ScopeStack* scope=nullptr;
			log::Log* logger=nullptr;
			template<class T>
			bool add_member(T* member) {
				if (!scope)return false;
				if (scope->memb.name_conflict<T>(member->name)) {
					logger->semerr_val("name '*' is already used on this context:name conflicted.",member->name);
					return false;
				}
				auto res=scope->memb.add(member);
				if (!res) {
					logger->syserr("memory is full.");
				}
				return res;
			}

			io::ReadStatus* get_read_status();

			Type* make_type(char* name, TypeKind kind, bool name_must = true, bool hold = true);
			Type* get_derived(Type* base, uint64_t size, TypeKind kind);
			Type* get_function(Type* rettype, common::EasyVectorP<Type*>& params, common::EasyVectorP<TypeOption*>& opts);
			Type* get_void();
			Type* get_bit_t(bool is_unsigned=false,bool is_float=false);

			Variable* make_variable(char* name);

			~Maker() {
				types.remove_each(common::kill);
				macros.remove_each(common::kill);
				vars.remove_each(common::kill);
				templates.remove_each(common::kill);
				common::kill(void_t);
				common::kill(bit_t_float);
				common::kill(bit_t_signed);
				common::kill(bit_t_unsigned);
			}
		};
		bool is_derived(identifier::TypeKind kind);
		bool search_name(const char* name,Maker* maker,Type** type=nullptr,Variable** var=nullptr,Template** temp=nullptr,Macro** macro=nullptr);
		
	}
}
