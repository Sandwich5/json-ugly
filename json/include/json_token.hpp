#pragma once
#include <string>

namespace sandwich {

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
	json_token(
		JSON_TOKEN,
		size_t,
		size_t,
		const std::string& val = std::string());
	void print() const;
	
	JSON_TOKEN code() const;
	const std::string& value() const;
	
private:
	JSON_TOKEN m_code;
	size_t m_ln;
	size_t m_col;
	std::string m_val;
};

}  // namespace sandwich