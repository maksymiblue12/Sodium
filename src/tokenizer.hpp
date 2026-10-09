#pragma once

enum TokenType {
	t_exit,
	t_if,
	int_lit,
	let,
	identifier,
	sign_eq,
	plus,
	minus,
	divide,
	star,
	open_paren,
	close_paren,
	open_curly,
	close_curly,
	semi
};

std::optional<int> get_bin_prec(TokenType type) {
	switch (type) {
	case (TokenType::plus):
	case (TokenType::minus):
		return 1;
	case (TokenType::star):
	case (TokenType::divide):
		return 2;
	default:
		return {};
	}
}

struct Token {
	TokenType type;
	std::optional<std::string> value;
};

class Tokenizer {
public:
	Tokenizer(std::string file)
	:m_file(file) {
	}

	std::vector<Token> tokenize() {
		std::vector<Token> tokens;
		std::string buffer;
		while (peek().has_value()) {
			if (isalpha(peek().value())) {
				buffer.push_back(pop());
				while (peek().has_value()&&isalnum(peek().value())) {
					buffer.push_back(pop());
				}
				if (buffer=="exit") {
					tokens.push_back(Token{.type=TokenType::t_exit});
					buffer.clear();
				} else if (buffer=="let") {
					tokens.push_back(Token{.type=TokenType::let});
					buffer.clear();
				} else if (buffer=="if") {
					tokens.push_back(Token{.type=TokenType::t_if,.value=buffer});
					buffer.clear();
				} else {
					tokens.push_back(Token{.type=TokenType::identifier,.value=buffer});
					buffer.clear();
				}
				continue;
			} else if (isdigit(peek().value())) {
				buffer.push_back(pop());
				while (peek().has_value()&&isdigit(peek().value())) {
					buffer.push_back(pop());
				}
				tokens.push_back(Token{.type=TokenType::int_lit,.value=buffer});
				buffer.clear();
				continue;
			}

			if (isspace(peek().value())) {
				pop();
				continue;
			}

			TokenType t_type;

			switch (peek().value()) {
			case ('='):
				t_type=TokenType::sign_eq;
				break;
			case ('+'):
				t_type=TokenType::plus;
				break;
			case ('*'):
				t_type=TokenType::star;
				break;
			case ('-'):
				t_type=TokenType::minus;
				break;
			case ('/'):
				t_type=TokenType::divide;
				break;
			case ('('):
				t_type=TokenType::open_paren;
				break;
			case (')'):
				t_type=TokenType::close_paren;
				break;
			case ('{'):
				t_type=TokenType::open_curly;
				break;
			case ('}'):
				t_type=TokenType::close_curly;
				break;
			case (';'):
				t_type=TokenType::semi;
				break;
			default:
				std::cerr<<"Error in Tokenizer!\n";
				exit(1);
			}

			pop();
			tokens.push_back(Token{.type=t_type});
		}
		m_index=0;
		return tokens;
	}
private:
	std::string m_file;
	int m_index=0;

	std::optional<char> peek(int offset=0) {
		if (m_index+offset>=m_file.size()) {
			return {};
		}
		return m_file.at(m_index+offset);
	}

	char pop() {
		return m_file.at(m_index++);
	}
};