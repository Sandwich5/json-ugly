#include "include/json_lexer.hpp"

#include <iostream> // debug

namespace sandwich
{
	
json_lexer::json_lexer(std::ifstream& input_file)
	: m_input(std::move(input_file))
	, m_line(1u)
	, m_column(1u)
	, m_crt_line(1u)
	, m_crt_column(0u)
	, m_prev_column(0u)
	, m_update(false)
{}

char json_lexer::get()
{
	char crt_char = m_input.get();
	if (crt_char == '\n')
	{
		m_crt_line++;
		m_prev_column = m_crt_column;
		m_crt_column = 0;
	}
	else 
	{
		m_crt_column++;
	}
	if (m_update)
	{
		m_update = false;
		m_line = m_crt_line;
		m_column = m_crt_column;
	}
	return crt_char;
}

void json_lexer::unget(char chr)
{
	// note: The purpose of this function is to somewhat keep accurate
	//		records of the current position in the file. However
	//		in this current form cannot do this if is called 2 times
	//      consecutively on newline characters (this corner case
	//		shouldn't normally occur in this project... hopefully)
	m_input.unget();
	if (chr == '\n')
	{
		m_crt_line--;
		m_crt_column = m_prev_column;
	}
	else
	{
		m_crt_column--;
	}
}

void json_lexer::update()
{
	m_update = true;
}

std::string json_lexer::consume_string()
{
	std::string buffer;
	char crt_char = get();
	
	while (crt_char != EOF && crt_char != '"')
	{
		buffer.push_back(crt_char);
		if (crt_char == '\\')
		{
			// TODO: allow only acceptable escapes (\a not okay)
			crt_char = get();
			buffer.push_back(crt_char);
		}
		crt_char = get();
	}
	
	return buffer;
}

std::string json_lexer::consume_number()
{
	std::string buffer;
	char crt_char = get();
	
	// optional '-'
	if (crt_char == '-')
	{
		buffer.push_back(crt_char);
		crt_char = get();
	}
	
	// base
	if (crt_char >= '1' && crt_char <= '9')
	{
		do {
			buffer.push_back(crt_char);
			crt_char = get();
		} while (crt_char >= '0' && crt_char <= '9');
	}
	else if (crt_char != '0')
	{
		return "error";
	}
	else
	{
		buffer.push_back(crt_char);
		crt_char = get();
	}
	
	// fraction
	if (crt_char == '.')
	{
		buffer.push_back(crt_char);
		crt_char = get();
		
		if (crt_char < '0' or crt_char > '9')
		{
			return "error";
		}
		do {
			buffer.push_back(crt_char);
			crt_char = get();
		} while (crt_char >= '0' && crt_char <= '9');
	}
	
	// exponent
	if (crt_char == 'e' or crt_char == 'E')
	{
		buffer.push_back(crt_char);
		crt_char = get();
		
		if (crt_char == '-' or crt_char == '+')
		{
			buffer.push_back(crt_char);
			crt_char = get();
		}
		
		if (crt_char < '0' or crt_char > '9')
		{
			return "error";
		}
		do {
			buffer.push_back(crt_char);
			crt_char = get();
		} while (crt_char >= '0' && crt_char <= '9');
	}
	
	unget(crt_char);
	return buffer;
}

bool json_lexer::consume_sequence(const std::string& seq)
{
	char crt_char;
	for (char chr : seq) {
		crt_char = get();
		if (chr != crt_char) {
			unget(crt_char);
			return false;  // failed to consume sequence
		}
	}
	return true;
}

json_token json_lexer::get_token()
{
	std::string buffer = "";
	char crt_char = get();
	
	if (crt_char == EOF) {
		update();
		return json_token(JSON_TOKEN::END, m_line, m_column);
	}
	
	while (crt_char == ' ' || crt_char == '\t' || crt_char == '\n') 
	{
		crt_char = get();
	}
    
    // not sure about those 2 lines
    m_line = m_crt_line;
	m_column = m_crt_column;
	
	if (crt_char == '{') {
		update();
		return json_token(JSON_TOKEN::LEFT_CB, m_line, m_column);
	}
	if (crt_char == '}') {
		update();
		return json_token(JSON_TOKEN::RIGHT_CB, m_line, m_column);
	}
	if (crt_char == '[') {
		update();
		return json_token(JSON_TOKEN::LEFT_SB, m_line, m_column);
	}
	if (crt_char == ']') {
		update();
		return json_token(JSON_TOKEN::RIGHT_SB, m_line, m_column);
	}
	if (crt_char == ':') {
		update();
		return json_token(JSON_TOKEN::COLON, m_line, m_column);
	}
	if (crt_char == ',') {
		update();
		return json_token(JSON_TOKEN::COMMA, m_line, m_column);
	}
	if (crt_char == '"') {
		buffer = consume_string();
		update();
		return json_token(JSON_TOKEN::STRING, m_line, m_column, buffer);
	}
	if (crt_char == '-' or (crt_char >= '0' && crt_char <= '9')) {
		unget(crt_char);
		buffer = consume_number();
		update();
		if (buffer == "error") 
			return json_token(JSON_TOKEN::ERROR, m_line, m_column);
		else
			return json_token(JSON_TOKEN::NUMBER, m_line, m_column, buffer);
	}
	if (crt_char == 't') {
		unget(crt_char);
		if (consume_sequence("true"))
			return json_token(JSON_TOKEN::TRUE, m_line, m_column);
		else
			return json_token(JSON_TOKEN::ERROR, m_line, m_column);
	}
	if (crt_char == 'f') {
		unget(crt_char);
		if (consume_sequence("false"))
			return json_token(JSON_TOKEN::FALSE, m_line, m_column);
		else
			return json_token(JSON_TOKEN::ERROR, m_line, m_column);
	}
	if (crt_char == 'n') {
		unget(crt_char);
		if (consume_sequence("null"))
			return json_token(JSON_TOKEN::NUL, m_line, m_column);
		else
			return json_token(JSON_TOKEN::ERROR, m_line, m_column);
	}
	
	update();  // is this needed? idk
	return json_token(JSON_TOKEN::ERROR, m_line, m_column);
}
	
}  // namespace sandwich