#pragma once

#include "json_token.hpp"
#include <fstream>

namespace sandwich
{
	
struct json_lexer
{
	json_lexer(std::ifstream&);
	json_token get_token();
	
private:
	std::ifstream m_input;
	size_t m_line, m_column;
	size_t m_crt_line, m_crt_column;
	size_t m_prev_column;
	bool m_update;
	
	char get();
	void unget(char);
	void update();
	
	std::string consume_string();
	std::string consume_number();
	bool consume_sequence(const std::string&);
};

}  // namespace sandwich