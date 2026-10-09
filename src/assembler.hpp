#pragma once

std::string tab="    ";

class Assembler {
public:
	Assembler(NodeProgram tree)
	:m_tree(tree) {
	}

	void build_term(const NodeTerm* term) {
		struct TermVisitor {
			Assembler& assembler;
			void operator()(const NodeTermIntLit* node_int_lit) const {
				assembler.m_result<<tab<<"mov rax, "<<node_int_lit->int_lit.value.value()<<"\n";
				assembler.push("rax");
				assembler.m_result<<"\n";
			}
			void operator()(const NodeTermIdentifier* node_identifier) const {
				const auto variable=assembler.get_variable(node_identifier->identifier.value.value());
				std::stringstream offset;
				offset<<"qword ptr [rsp+"<<(assembler.m_stack_size-variable.stack_location-1)*8<<"]";
				assembler.push(offset.str());
				assembler.m_result<<"\n";
			}
			void operator()(const NodeTermParen* node_paren) const {
				assembler.build_expr(node_paren->expr);
			}
		};

		TermVisitor visitor{.assembler=*this};
		std::visit(visitor,term->var);
	}

	void build_bin_expr(const NodeBinExpr* bin_expr) {
		struct BinaryExpressionVisitor {
			Assembler& assembler;
			void operator()(const NodeBinAdd* node_bin_add) const {
				assembler.build_expr(node_bin_add->rhs);
				assembler.build_expr(node_bin_add->lhs);
				assembler.pop("rax");
				assembler.pop("rbx");
				assembler.m_result<<tab<<"add rax, rbx\n";
				assembler.push("rax");
				assembler.m_result<<"\n";
			}
			void operator()(const NodeBinSubtract* node_bin_sub) const {
				assembler.build_expr(node_bin_sub->rhs);
				assembler.build_expr(node_bin_sub->lhs);
				assembler.pop("rax");
				assembler.pop("rbx");
				assembler.m_result<<tab<<"sub rax, rbx\n";
				assembler.push("rax");
				assembler.m_result<<"\n";
			}
			void operator()(const NodeBinMultiply* node_bin_mult) const {
				assembler.build_expr(node_bin_mult->rhs);
				assembler.build_expr(node_bin_mult->lhs);
				assembler.pop("rax");
				assembler.pop("rbx");
				assembler.m_result<<tab<<"mul rbx\n";
				assembler.push("rax");
				assembler.m_result<<"\n";
			}
			void operator()(const NodeBinDivide* node_bin_div) const {
				assembler.build_expr(node_bin_div->rhs);
				assembler.build_expr(node_bin_div->lhs);
				assembler.pop("rax");
				assembler.pop("rbx");
				assembler.m_result<<tab<<"div rbx\n";
				assembler.push("rax");
				assembler.m_result<<"\n";
			}
		};

		BinaryExpressionVisitor visitor{.assembler=*this};
		std::visit(visitor,bin_expr->var);
	}

	void build_expr(const NodeExpr* expr) {
		struct ExpressionVisitor {
			Assembler& assembler;
			void operator()(const NodeTerm* node_term) const {
				assembler.build_term(node_term);
			}
			void operator()(const NodeBinExpr* node_bin_expr) const {
				assembler.build_bin_expr(node_bin_expr);
			}
		};

		ExpressionVisitor visitor{.assembler=*this};
		std::visit(visitor,expr->var);
	}

	void build_statement(const NodeStatement* stmnt) {
		struct StatementVisitor {
			Assembler& assembler;
			void operator()(const NodeStatementExit* stmnt_exit) const {
				assembler.build_expr(stmnt_exit->expr);
				assembler.m_result<<tab<<"mov rax, 60\n";
				assembler.pop("rdi");
				assembler.m_result<<tab<<"syscall\n";
			}
			void operator()(const NodeStatementLet* stmnt_let) const {
				if (assembler.has_variable(stmnt_let->identifier.value.value())) {
					std::cerr<<"Variable with name '"<<stmnt_let->identifier.value.value()<<"' already exists!\n";
					exit(1);
				}
				assembler.m_vars.push_back(Variable{.name=stmnt_let->identifier.value.value(),.stack_location=assembler.m_stack_size});
				assembler.build_expr(stmnt_let->expr);
			}
			void operator()(const NodeScope* scope) const {
				assembler.begin_scope();
				for (auto stmnt:scope->stmnts) {
					assembler.build_statement(stmnt);
				}
				assembler.end_scope();
			}
		};

		StatementVisitor visitor{.assembler=*this};
		std::visit(visitor,stmnt->statement);
	}

	std::string build(bool is_32bit) {
		m_result<<".intel_syntax noprefix\n\n";
		m_result<<".global _start\n";
		m_result<<"_start:\n";
		for (int i=0;i<m_tree.statements.size();i++) {
			build_statement(m_tree.statements.at(i));
		}
		m_result<<tab<<"mov rax, 60\n";
		m_result<<tab<<"mov rdi, 0\n";
		m_result<<tab<<"syscall\n";
		return m_result.str();
	}
private:
	struct Variable {
		std::string name;
		size_t stack_location;
	};

	void push(const std::string& reg) {
		m_result<<tab<<"push "<<reg<<"\n";
		m_stack_size++;
	}

	void pop(const std::string& reg) {
		m_result<<tab<<"pop "<<reg<<"\n";
		m_stack_size--;
	}

	void begin_scope() {
		m_scopes.push_back(m_vars.size());
	}

	void end_scope() {
		size_t diff=m_vars.size()-m_scopes.back();
		m_result<<tab<<"add rsp,"<<diff*8<<"\n";
		m_stack_size-=diff;
		for (int i=0;i<diff;i++) {
			m_vars.pop_back();
		}
		m_scopes.pop_back();
	}

	bool has_variable(std::string name) {
		for (Variable var:m_vars) {
			if (var.name==name) {
				return true;
			}
		}
		return false;
	}

	Variable get_variable(std::string name) {
		for (Variable var:m_vars) {
			if (var.name==name) {
				return var;
			}
		}
		std::cerr<<"Unkown variable '"<<name<<"'\n";
		exit(1);
	}

	NodeProgram m_tree;
	std::stringstream m_result;
	size_t m_stack_size=0;
	std::vector<Variable> m_vars {};
	std::vector<size_t> m_scopes {};
};
