#pragma once

struct NodeExpr;

struct NodeTermIntLit {
	Token int_lit;
};

struct NodeTermIdentifier {
	Token identifier;
};

struct NodeTermParen {
	NodeExpr* expr;
};

struct NodeTerm {
	std::variant<NodeTermIntLit*,NodeTermIdentifier*,NodeTermParen*> var;
};


struct NodeBinAdd {
	NodeExpr* lhs;
	NodeExpr* rhs;
};

struct NodeBinSubtract {
	NodeExpr* lhs;
	NodeExpr* rhs;
};

struct NodeBinMultiply {
	NodeExpr* lhs;
	NodeExpr* rhs;
};

struct NodeBinDivide {
	NodeExpr* lhs;
	NodeExpr* rhs;
};

struct NodeBinExpr {
	std::variant<NodeBinAdd*,NodeBinSubtract*,NodeBinMultiply*,NodeBinDivide*> var;
};


struct NodeExpr {
	std::variant<NodeTerm*,NodeBinExpr*> var;
};


struct NodeStatement;

struct NodeScope {
	std::vector<NodeStatement*> stmnts;
};


struct NodeStatementExit {
	NodeExpr* expr;
};

struct NodeStatementLet {
	Token identifier;
	NodeExpr* expr;
};

struct NodeStatementIf {
	NodeExpr* expr;
	NodeScope* scope;
};

struct NodeStatement {
	std::variant<NodeStatementExit*,NodeStatementLet*,NodeScope*,NodeStatementIf*> statement;
};

struct NodeProgram {
	std::vector<NodeStatement*> statements;
};

class Parser {
public:
	Parser(std::vector<Token> tokens)
	:m_tokens(tokens),
	m_allocator(1024*1024*4) {
	}

	std::optional<NodeTerm*> parse_term() {
		if (check_token(TokenType::int_lit)) {
			auto node_int_lit=m_allocator.alloc<NodeTermIntLit>();
			node_int_lit->int_lit=pop();
			auto node_term=m_allocator.alloc<NodeTerm>();
			node_term->var=node_int_lit;
			return node_term;
		} else if (check_token(TokenType::identifier)) {
			auto node_ident=m_allocator.alloc<NodeTermIdentifier>();
			node_ident->identifier=pop();
			auto node_term=m_allocator.alloc<NodeTerm>();
			node_term->var=node_ident;
			return node_term;
		} else if (check_token(TokenType::open_paren)) {
			pop();

			std::optional<NodeExpr*> expr=parse_expr();
			if (!expr.has_value()) {
				std::cerr<<"Expected expression\n";
				exit(1);
			}
			check_close_paren();

			auto node_paren=m_allocator.alloc<NodeTermParen>();
			node_paren->expr=expr.value();
			auto node_term=m_allocator.alloc<NodeTerm>();
			node_term->var=node_paren;
			return node_term;
		}
		return {};
	}

	std::optional<NodeExpr*> parse_expr(int min_prec=0) {
		std::optional<NodeTerm*> lhs=parse_term();
		if (!lhs.has_value()) {
			return {};
		}
		auto expr_lhs=m_allocator.alloc<NodeExpr>();
		expr_lhs->var=lhs.value();

		while (true) {
			std::optional<Token> current_token=peek();
			if (!current_token.has_value()) {
				break;
			}
			std::optional<int> prec=get_bin_prec(current_token.value().type);
			if (!prec.has_value()||prec.value()<min_prec) {
				break;
			}

			Token op=pop();
			int new_min_prec=prec.value()+1;
			auto expr_rhs=parse_expr(new_min_prec);
			if (!expr_rhs.has_value()) {
				std::cerr<<"Unable to parse expression\n";
				exit(1);
			}

			auto bin_expr=m_allocator.alloc<NodeBinExpr>();
			auto bin_expr_lhs=m_allocator.alloc<NodeExpr>();
			bin_expr_lhs->var=expr_lhs->var;
			if (op.type==TokenType::plus) {
				auto add=m_allocator.alloc<NodeBinAdd>();
				add->lhs=bin_expr_lhs;
				add->rhs=expr_rhs.value();
				bin_expr->var=add;
			} else if (op.type==TokenType::minus) {
				auto sub=m_allocator.alloc<NodeBinSubtract>();
				sub->lhs=bin_expr_lhs;
				sub->rhs=expr_rhs.value();
				bin_expr->var=sub;
			} else if (op.type==TokenType::star) {
				auto mult=m_allocator.alloc<NodeBinMultiply>();
				mult->lhs=bin_expr_lhs;
				mult->rhs=expr_rhs.value();
				bin_expr->var=mult;
			} else if (op.type==TokenType::divide) {
				auto div=m_allocator.alloc<NodeBinDivide>();
				div->lhs=bin_expr_lhs;
				div->rhs=expr_rhs.value();
				bin_expr->var=div;
			}

			expr_lhs->var=bin_expr;
		}
		return expr_lhs;
	}

	std::optional<NodeScope*> parse_scope() {
		if (check_token(TokenType::open_curly)) {
			pop();
			std::vector<NodeStatement*> stmnts;
			while (auto stmnt=parse_statement()) {
				stmnts.push_back(stmnt.value());
			}
			check_syntax(TokenType::close_curly,'}');
			auto scope=m_allocator.alloc<NodeScope>();
			scope->stmnts=stmnts;
			return scope;
		} else {
			return {};
		}
	}

	std::optional<NodeStatement*> parse_statement() {
		if (check_token(TokenType::t_exit)) {
			pop();
			check_open_paren();

			std::optional<NodeExpr*> expr=parse_expr();
			if (!expr.has_value()) {
				std::cerr<<"Expected expression\n";
				exit(1);
			}
			check_close_paren();
			check_semi();

			auto node_exit=m_allocator.alloc<NodeStatementExit>();
			node_exit->expr=expr.value();
			auto stmnt=m_allocator.alloc<NodeStatement>();
			stmnt->statement=node_exit;
			return stmnt;
		} else if (check_token(TokenType::let)) {
			pop();
			Token identifier=check_syntax(TokenType::identifier,"Expected identifier");
			check_syntax(TokenType::sign_eq,"=");

			std::optional<NodeExpr*> expr=parse_expr();
			if (!expr.has_value()) {
				std::cerr<<"Expected expression\n";
				exit(1);
			}
			check_semi();

			auto node_let=m_allocator.alloc<NodeStatementLet>();
			node_let->identifier=identifier;
			node_let->expr=expr.value();
			auto stmnt=m_allocator.alloc<NodeStatement>();
			stmnt->statement=node_let;
			return stmnt;
		} else if (auto scope=parse_scope()) {
			auto stmnt=m_allocator.alloc<NodeStatement>();
			stmnt->statement=scope.value();
			return stmnt;
		} else if (check_token(TokenType::t_if)) {
			pop();
			check_open_paren();

			std::optional<NodeExpr*> expr=parse_expr();
			if (!expr.has_value()) {
				std::cerr<<"Expected expression\n";
				exit(1);
			}
			check_close_paren();

			std::optional<NodeScope*> scope=parse_scope();
			if (!scope.has_value()) {
				std::cerr<<"Expected expression\n";
				exit(1);
			}


			auto node_if=m_allocator.alloc<NodeStatementIf>();
			node_if->expr=expr.value();
			node_if->scope=scope.value();
			auto stmnt=m_allocator.alloc<NodeStatement>();
			stmnt->statement=node_if;
			return stmnt;
		} else {
			return {};
		}
	}

	NodeProgram parse() {
		std::vector<NodeStatement*> stmnts;
		while (peek().has_value()) {
			if (auto stmnt=parse_statement()) {
				stmnts.push_back(stmnt.value());
			} else {
				std::cerr<<"Failed to parse statement\n";
				exit(1);
			}
		}
		m_index=0;
		return NodeProgram{.statements=stmnts};
	}

private:
	std::vector<Token> m_tokens;
	int m_index=0;
	ArenaAllocator m_allocator;

	std::optional<Token> peek(int offset=0) {
		if (m_index+offset>=m_tokens.size()) {
			return {};
		}
		return m_tokens.at(m_index+offset);
	}

	Token pop() {
		return m_tokens.at(m_index++);
	}

	bool check_token(TokenType t_type, int offset=0) {
		return peek(offset).has_value()&&peek(offset).value().type==t_type;
	}

	Token check_syntax(TokenType t_type, char c, int offset=0) {
		if (!check_token(t_type,offset)) {
			std::cerr<<"Expected: '"<<c<<"'\n";
			exit(1);
		}
		return pop();
	}

	Token check_syntax(TokenType t_type, std::string msg, int offset=0) {
		if (!check_token(t_type,offset)) {
			std::cerr<<msg<<"'\n";
			exit(1);
		}
		return pop();
	}

	void check_open_paren(int offset=0) {
		check_syntax(TokenType::open_paren,'(',offset);
	}

	void check_close_paren(int offset=0) {
		check_syntax(TokenType::close_paren,')',offset);
	}

	void check_semi(int offset=0) {
		check_syntax(TokenType::semi,';',offset);
	}
};