#include <iostream>
#include <list>
#include <fstream>
#include <vector>
#include <map>
#include <variant>

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
private:
	std::ifstream input_file;
	size_t ln, col, cur_ln, cur_col, prev_col;
	bool syncro;
	
	json_token next_token();
	
	char get();
	void unget(char);
	void update_coord();
	std::string consume_string();
	std::string consume_number(char);
};

std::list<json_token> json_lexer::analyse() {
	std::list<json_token> list;
	json_token token = next_token();
	while (true) {
		list.push_back(token);
		if (token.code == JSON_TOKEN::END) break;
		token = next_token();
	}
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

std::string json_lexer::consume_number(char init) {
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
	std::string buffer = "";
	char cur_char = get();
	//std::cout << "parsing character: [ " << cur_char << " ]\n";
	if (cur_char == EOF) {
		update_coord();
		return json_token(JSON_TOKEN::END, ln, col);
	}
	while (cur_char == ' ' || cur_char == '\t' || cur_char == '\n') cur_char = get();
	if (cur_char == '{') {
		update_coord();
		return json_token(JSON_TOKEN::LEFT_CB, ln, col);
	}
	if (cur_char == '}') {
		update_coord();
		return json_token(JSON_TOKEN::RIGHT_CB, ln, col);
	}
	if (cur_char == '[') {
		update_coord();
		return json_token(JSON_TOKEN::LEFT_SB, ln, col);
	}
	if (cur_char == ']') {
		update_coord();
		return json_token(JSON_TOKEN::RIGHT_SB, ln, col);
	}
	if (cur_char == ':') {
		update_coord();
		return json_token(JSON_TOKEN::COLON, ln, col);
	}
	if (cur_char == ',') {
		update_coord();
		return json_token(JSON_TOKEN::COMMA, ln, col);
	}
	if (cur_char == '"') {
		buffer = consume_string();
		update_coord();
		return json_token(JSON_TOKEN::STRING, ln, col, buffer);
	}
	if (cur_char == '-' or (cur_char >= '0' && cur_char <= '9')) {
		buffer = consume_number(cur_char);
		update_coord();
		if (buffer == "error") return json_token(JSON_TOKEN::ERROR, ln, col);
		return json_token(JSON_TOKEN::NUMBER, ln, col, buffer);
	}
	if (cur_char == 'f') {
		if (get() == 'a' and
			get() == 'l' and
			get() == 's' and
			get() == 'e') return json_token(JSON_TOKEN::FALSE, ln, col);
		else return json_token(JSON_TOKEN::ERROR, ln, col);
	}
	if (cur_char == 't') {
		if (get() == 'r' and
			get() == 'u' and
			get() == 'e') return json_token(JSON_TOKEN::TRUE, ln, col);
		else return json_token(JSON_TOKEN::ERROR, ln, col);
	}
	if (cur_char == 'n') {
		if (get() == 'u' and
			get() == 'l' and
			get() == 'l') return json_token(JSON_TOKEN::NUL, ln, col);
		else return json_token(JSON_TOKEN::ERROR, ln, col);
	}
	update_coord();
	return json_token(JSON_TOKEN::STRING, ln, col);
}

struct json_value {
	using json_object = std::map<std::string, json_value>;
	using json_array = std::list<json_value>;
	struct json_null {};
	
	json_value() : ok(false) {}
	void set_object(const json_object& object) { ok = true; value = object; }
	void set_array(const json_array& array) { ok = true; value = array; }
	void set_string(const std::string& string) { ok = true; value = string; }
	void set_number(double number) { ok = true; value = number; }
	void set_boolean(bool boolean) { ok = true; value = boolean; }
	void set_null() { ok = true; value = json_null{}; }
	
	explicit operator bool() const { return ok; }
	
	std::variant<
		json_object,
		json_array,
		std::string,
		int,
		double,
		bool,
		json_null> value;
	bool ok;
};

struct json_parser {
	json_value make_json() {
		json();
		return result;
	}
	
	json_parser(std::list<json_token> lst) {
		crt_pos = 0;
		for (const auto& tk : lst) {
			if (tk.code == JSON_TOKEN::ERROR) {
				std::cout << "\nlexical error:\n\ttoken found:";
				tk.print();
				lex_err = true;
				break;
			}
			tokens.push_back(tk);
		}
	}
	
	void test() {
		if (lex_err) {
			std::cout << "json parsing halted\n";
		} else if (json()) {
			std::cout << "json parsing OK\n";
		} else {
			std::cout << "json parsing failed!\n";
		}
	}
private:
	size_t crt_pos;
	std::vector<json_token> tokens;
	bool lex_err = false;
	json_value result;
	
	int consume(JSON_TOKEN tk) {
		if (tokens[crt_pos].code == JSON_TOKEN::ERROR) {
			return 0;
		}
		if (tokens[crt_pos].code == tk) {
			crt_pos++;
			return 1;
		}
		return 0;
	}
	
	void parse_error(const std::string& err_message) {
		std::cout << "\n\nparsing error at (ln:" << tokens[crt_pos].ln << ",col:" << tokens[crt_pos].col << ")\n\t" << err_message << "\n\ttoken found:";
		tokens[crt_pos].print();
	}

	int json() {
		//std::cout << "parsing json\n";
		if (!element()) {
			parse_error("cannot parse root element of json");
			return 0;
		}
		if (!consume(JSON_TOKEN::END)) {
			parse_error("expected EOF, but found different token");
			return 0;
		}
		return 1;
	}

	json_value value() {
		//std::cout << "parsing value\n";
		size_t pos_start = crt_pos;
		json_value val;
		if (consume(JSON_TOKEN::STRING)) { val.set_string("test"); return val; }
		if (consume(JSON_TOKEN::NUMBER)) { val.set_number("TEST"); return val; }
		if (consume(JSON_TOKEN::TRUE)   ||
			consume(JSON_TOKEN::FALSE)) { val.set_boolean("lmao"); return val; }
		if (consume(JSON_TOKEN::NUL)) { val.set_null(); return val; }
		
		if ((val = object())) return val;
		if ((val = array())) return val;
		///if (object() || array()) { val.set_null(); return val; }
		crt_pos = pos_start;
		parse_error("could not extract a valid json value");
		// fix me val needs to be set appropriately for each kind of value...
		// investigate differences in copy vs. move for object and array...
		return val;
	}
	
	struct member_result {
		
		std::pair<std::string, json_value> member;
		bool result = false;
		void set_member(const std::string& k, const json_value& v) {
			result = true;
			member = {k, v};
		}
		explicit opeator bool() { return result; }
	};
	
	struct members_result {
		std::list<std::pair<std::string, json_value>> members;
		bool result = true;
		void add_member(const member_result& mr) {
			result &= (bool)mr;
			members.push_back(mr.member);
		}
		explicit operator bool() { return result; }
	};

	int object() {
		//std::cout << "parsing object\n";
		size_t pos_start = crt_pos;
		if (!consume(JSON_TOKEN::LEFT_CB)) { ; crt_pos = pos_start; return 0; }
		members();
		if (!consume(JSON_TOKEN::RIGHT_CB)){ parse_error("object definition must be enclosed in curly brackets, missing right curly bracket"); crt_pos = pos_start; return 0; }
		return 1;
	}
	
	int members() {
		//std::cout << "parsing members\n";
		size_t pos_start = crt_pos;
		if (!member()) { crt_pos = pos_start; return 0; }
		if (consume(JSON_TOKEN::COMMA)) {
			if (!members()) { parse_error("expected member definition after comma in object definition"); crt_pos = pos_start; return 0;
			}
		}
		return 1;
	}

	int member() {
		//std::cout << "parsing member\n";
		size_t pos_start = crt_pos;
		if (!consume(JSON_TOKEN::STRING)) { crt_pos = pos_start; return 0; }
		if (!consume(JSON_TOKEN::COLON)) { parse_error("object member definition is missing key-value separator"); crt_pos = pos_start; return 0; }
		if (!element()) { parse_error("object member definition expects an element as value"); crt_pos = pos_start; return 0; }
		return 1;
	}
	
	int array() {
		//std::cout << "parsing array\n";
		size_t pos_start = crt_pos;
		if (!consume(JSON_TOKEN::LEFT_SB)) { crt_pos = pos_start; return 0; }
		elements();
		if (!consume(JSON_TOKEN::RIGHT_SB)){ parse_error("array definition must be enclosed in square brackets, missing right square bracket"); crt_pos = pos_start; return 0; }
		return 1;
	}
	
	json_value elements() {
		//std::cout << "parsing elements\n";
		size_t pos_start = crt_pos;
		json_value val = element();
		if (!val) {
			parse_error("could not extract a valid json element");
			crt_pos = pos_start;
			//return 0; 
			return json_value();
		}
		if (consume(JSON_TOKEN::COMMA)) {
			auto rest = elements();
			if (!rest) {
				parse_error("expected json element after comma");
				crt_pos = pos_start;
				return json_value();
			}
			if (std::holds_alternative<json_value>(rest)) {
				json_value v;
				v.set_array({val, rest});
				return v;
			} else {
				rest
			}
		}
		return val;
	}

	json_value element() {
		//std::cout << "parsing element\n";
		size_t pos_start = crt_pos;
		json_value val = value();
		if (!val) { 
			parse_error("element definition expects a valid value");
			crt_pos = pos_start;
			//return 0;
		}
		return val;
	}
};

int main() {
	std::ifstream file("test.txt");
	
	json_lexer lexer(file);
	auto result = lexer.analyse();
	std::cout << "\n[LEXER]\nparsed " << result.size() << " tokens\n";

	std::cout << "\n[PARSER]\n";
	json_parser parser(result);
	parser.test();
	
	std::cout << '\n';
	
	return 0;
}
