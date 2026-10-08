#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cassert>

#include "tokenizer.hpp"
#include "arena.hpp"
#include "parser.hpp"
#include "assembler.hpp"

int main(int argc, char* argv[]) {
	if (argc<2 || argc>3) {
		printf("Usage: ./sodium <input_file.na> [--32bit]\n");
		return 1;
	}

	bool output_asm=true;

	bool build_32bit=false;
	if (argc==3 && std::string(argv[2])=="--32bit") {
		build_32bit=true;
	}


	std::ifstream input(argv[1]);
	std::stringstream buffer;
	buffer<<input.rdbuf();
	input.close();
	std::string file=buffer.str();

	Tokenizer tokenizer(std::move(file));
	std::vector<Token> tokens=tokenizer.tokenize();

	Parser parser(std::move(tokens));
	NodeProgram tree=parser.parse();

	Assembler assembler(std::move(tree));
	std::string asm_code=assembler.build(build_32bit);

	std::ofstream output("test.asm");
	output<<asm_code;
	output.close();

	system("clang++ -c test.asm -o test.o");
	system("ld test.o -o test");
	std::remove("test.o");
	if (!output_asm) std::remove("test.asm");

	return 0;
}