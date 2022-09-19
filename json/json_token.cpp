#include "include/json_token.hpp"

#include <iostream>

namespace sandwich {
	
json_token::json_token(
	JSON_TOKEN tk,
	size_t line,
	size_t col,
	const std::string& val)
		: m_code(tk)
		, m_ln(line)
		, m_col(col)
		, m_val(val)
{}
	
void json_token::print() const
{
	std::string name;
    std::string symbol;
	switch(this->m_code) {
		case JSON_TOKEN::LEFT_CB:   name = "left_cb "; symbol = "{"; break;
		case JSON_TOKEN::LEFT_SB:   name = "left_sb "; symbol = "["; break;
		case JSON_TOKEN::RIGHT_CB:  name = "right_cb"; symbol = "}"; break;
		case JSON_TOKEN::RIGHT_SB:  name = "right_sb"; symbol = "]"; break;
		case JSON_TOKEN::COLON: 	name = "colon   "; symbol = ":"; break;
		case JSON_TOKEN::COMMA: 	name = "comma   "; symbol = ","; break;
		case JSON_TOKEN::TRUE: 		name = "true    "; symbol = "true"; break;
		case JSON_TOKEN::FALSE: 	name = "false   "; symbol = "false"; break;
		case JSON_TOKEN::NUL: 		name = "null    "; symbol = "null"; break;
		case JSON_TOKEN::STRING: 	name = "string  "; break;
		case JSON_TOKEN::NUMBER: 	name = "number  "; break;
		case JSON_TOKEN::ERROR: 	name = "error   "; symbol = "[ERROR]"; break;
		case JSON_TOKEN::END: 		name = "end     "; break;
	}
	std::cout << "token: " << name 
		<< " (" << m_ln << ":" << m_col << ")";
		
	if (m_val != "")
		std::cout << "\t\"" << m_val << "\"";
    //else
    //    std::cout << "\t" << symbol;
	
	std::cout << '\n';
}

JSON_TOKEN json_token::code() const
{
	return this->m_code;
}

const std::string& json_token::value() const
{
	return this->m_val;
}
	
}  // namespace sandwich