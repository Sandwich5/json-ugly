#pragma once

#include <fstream>
#include <vector>
#include "json_value.hpp"
#include "json_lexer.hpp"

namespace sandwich
{
	
struct json_parser
{
	json_parser(std::ifstream&);
	
	bool parse();
	json_value_ptr result();
	
private:
	json_lexer m_lexer;
	json_value_ptr m_value;
	size_t m_index;
	bool m_lexer_error;
	std::vector<json_token> m_tokens;
	
	bool consume(JSON_TOKEN);
	
	bool json(json_value&);
	bool value(json_value&);
	
	bool object(json_value&);
	bool members(std::map<std::string, json_value>&);
	bool member(std::string&, json_value&);  // (?) val type maybe ptr
	
	bool array(json_value&);
	bool elements(json_array&);
	bool element(json_value&);
};
	
}  // namespace sandwich