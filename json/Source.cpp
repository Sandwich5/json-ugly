#include <fstream>
#include <iostream>
#include <string>
#include <list>

#include "include/json_parser.hpp"

namespace old {
	enum class JSON_TOKEN {
	LEFT_CB		= 0, 
	RIGHT_CB	= 1,
	LEFT_SB		= 2, 
	RIGHT_SB	= 3,
	COLON		= 4,
	COMMA		= 5,
	TRUE		= 6, 
	FALSE		= 7, 
	NUL			= 8,
	STRING		= 9, 
	NUMBER		= 10,
	ERROR		= 11, 
	END			= 12
};

struct json_token {
	json_token(JSON_TOKEN tk, size_t line, size_t col, const std::string& val = "")
		: code(tk)
		, ln(line)
		, col(col)
		, val(val)
	{}
	void print() const {
		std::string name;
		switch(code) {
			case JSON_TOKEN::LEFT_CB:   name = "left_cb "; break;
			case JSON_TOKEN::LEFT_SB:   name = "left_sb "; break;
			case JSON_TOKEN::RIGHT_CB:  name = "right_cb"; break;
			case JSON_TOKEN::RIGHT_SB:  name = "right_sb"; break;
			case JSON_TOKEN::COLON: 	name = "colon   "; break;
			case JSON_TOKEN::COMMA: 	name = "comma   "; break;
			case JSON_TOKEN::TRUE: 		name = "true    "; break;
			case JSON_TOKEN::FALSE: 	name = "false   "; break;
			case JSON_TOKEN::NUL: 		name = "null    "; break;
			case JSON_TOKEN::STRING: 	name = "string  "; break;
			case JSON_TOKEN::NUMBER: 	name = "number  "; break;
			case JSON_TOKEN::ERROR: 	name = "error   "; break;
			case JSON_TOKEN::END: 		name = "end     "; break;
		}
		std::cout << "token: " << name << " (" << ln << ", " << col << ")";
		if (val != "") { std::cout << " -- val=\"" << val << "\""; }
		std::cout << '\n';
	}
	JSON_TOKEN code;
	size_t ln, col;
	std::string val;
};
	struct json_lexer {
	json_lexer(std::ifstream& input_file)
		: input_file(std::move(input_file))
		, ln(1), col(1), cur_ln(1), cur_col(1), prev_col(0), syncro(false)
	{}
	std::list<json_token> analyse();

	std::ifstream input_file;
	size_t ln, col, cur_ln, cur_col, prev_col;
	bool syncro;
	
	json_token next_token();
	
	char get();
	void unget(char);
	void update_coord();
	std::string consume_string();
	std::string consume_number();
};

std::list<json_token> json_lexer::analyse() {
	std::list<json_token> list;
	
	return list;
}

char json_lexer::get() {
	char cur_char = input_file.get();
	if (cur_char == '\n') { cur_ln++; prev_col = cur_col; cur_col = 0; }
	else { cur_col++; }
	if (syncro) { syncro = false; ln = cur_ln; col = cur_col; }
	return cur_char;
}

void json_lexer::unget(char c) {
	input_file.unget();
	if (c == '\n') { cur_ln--; cur_col = prev_col; }
	else cur_col--;
}

void json_lexer::update_coord() {
	syncro = true;
}

std::string json_lexer::consume_string() {
	//std::cout << "handling string : ";
	char cur_char = get();
	bool escape = false;
	std::string buffer = "";
	while (cur_char != EOF && (escape || cur_char != '"')) {
		buffer.push_back(cur_char);
		//std::cout << cur_char;
		escape = false;
		if (cur_char == '\'') escape = true;
		cur_char = get();
	}
	return buffer;
}

std::string json_lexer::consume_number() {
	char init = get();
	char cur_char = get();
	std::string buffer;
	buffer.push_back(init);
	bool no_whole = false;
	
	if (init == '-') { 
		init = cur_char; 
		if (init >= '0' and init <= '9') buffer.push_back(init);
		else return "error";
		cur_char = get(); 
	}
	if (init == '0') {
		if (cur_char == '.' or cur_char == 'e' or cur_char == 'E') {
		    
			no_whole = true; 
			//buffer.push_back(init); 
		}
		else { 
			//buffer.push_back(init);
			unget(cur_char);
			return buffer;
		}
	}
	// whole part
	if (!no_whole)
	while (cur_char >= '0' && cur_char <= '9') {
		buffer.push_back(cur_char);
		cur_char = get();
	}
	
	// fractional part
	if (cur_char == '.') {
		buffer.push_back(cur_char); cur_char = get();
		bool passed = false;
		while (cur_char >= '0' && cur_char <= '9') {
			passed = true;
			buffer.push_back(cur_char);
			cur_char = get();
		}
		if (!passed) return "error_2";
	}
	
	// exponent part
	if (cur_char == 'e' or cur_char == 'E') {
		buffer.push_back('e'); cur_char = get();
		if (cur_char == '+' or cur_char == '-') {
			buffer.push_back(cur_char);
			cur_char = get();
		}
		bool passed = false;
		while (cur_char >= '0' && cur_char <= '9') {
			passed = true;
			buffer.push_back(cur_char);
			cur_char = get();
		}
		if (!passed) return "error_3";
	}
	unget(cur_char);
	return buffer;
}

json_token json_lexer::next_token() {
	return json_token(JSON_TOKEN::STRING, ln, col);
}
};

void parser_test() {
    using namespace sandwich;
	std::ifstream in1("Test_Tokens_1.json");
	json_parser p(in1);
	
	if (p.parse())
    {
        std::cout << "Parsing successful\n";
        /*json_object obj = std::get<json_object>(p.result()->m_value);
        
        auto webApp = std::get<json_object>(obj["web-app"].m_value);
        auto svList = std::get<json_array>(webApp["servlet"].m_value);
        auto sv1 = std::get<json_object>(svList.front().m_value);
        auto sv1_name = std::get<std::string>(sv1["servlet-name"].m_value);
        auto sc1_class = std::get<std::string>(sv1["servlet-class"].m_value);
        
        std::cout << sv1_name  << ' ' << sc1_class << '\n';*/
        
        std::cout << "str: \n" << p.result()->to_string();
    }
}

void lexer_test() {
    using namespace sandwich;
	std::ifstream in1("Test_Tokens_1.json");
	json_lexer lex(in1);
    
    while (true) {
        auto token = lex.get_token();
        token.print();
        if (token.code() == JSON_TOKEN::END) break;
    }
}

int main() {
	parser_test();
	
	return EXIT_SUCCESS;
}