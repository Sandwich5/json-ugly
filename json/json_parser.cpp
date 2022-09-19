#include "include/json_parser.hpp"

#include <vector>
#include <map>
#include <string>

namespace sandwich
{
	
json_parser::json_parser(std::ifstream& input_file)
	: m_lexer(input_file)
	, m_value(nullptr)
	, m_index(0u)
	, m_lexer_error(false)
	, m_tokens()
{
	// populate
	do {
		m_tokens.push_back(m_lexer.get_token());
		if (m_tokens.back().code() == JSON_TOKEN::ERROR)
		{
			m_lexer_error = true;
			break;
		}
	} while (m_tokens.back().code() != JSON_TOKEN::END);
}

bool json_parser::parse()
{
	m_value = std::make_shared<json_value>();
	if (!m_lexer_error && json(*m_value))
	{
		return true;
	}
	else
	{
		m_value.reset();
		return false;
	}
}

json_value_ptr json_parser::result()
{
	return m_value;
}

bool json_parser::consume(JSON_TOKEN code)
{
    if (m_tokens[m_index].code() == code)
    {
        m_index++;
        return true;
    }
    return false;
}

bool json_parser::json(json_value& result)
{
	if (!element(result))
	{
		// ERROR
		return false;
	}
	if (!consume(JSON_TOKEN::END))
	{
		// ERROR: expected EOF
		return false;
	}
	return true;
}

bool json_parser::value(json_value& result)
{
	size_t crt_pos = m_index;
	if (consume(JSON_TOKEN::STRING))
	{
		result.set_string(m_tokens[crt_pos].value());
		return true;
	}
	if (consume(JSON_TOKEN::NUMBER))
	{
		// fixme: number conversion
        double num = atof(m_tokens[crt_pos].value().c_str());
        result.set_number(num);
		// old:result.set_string(m_tokens[crt_pos].value());
		return true;
	}
	if (consume(JSON_TOKEN::TRUE))
	{
		result.set_boolean(true);
		return true;
	}
	if (consume(JSON_TOKEN::FALSE))
	{
		result.set_boolean(false);
		return true;
	}
	if (consume(JSON_TOKEN::NUL))
	{
		result.set_null();
		return true;
	}
	if (object(result) || array(result))
	{
		return true;
	}
	
	// ERROR: could not find a valid value
	m_index = crt_pos;
	return false;
}

bool json_parser::object(json_value& result)
{
	std::map<std::string, json_value> obj;
	size_t crt_pos = m_index;
	if (!consume(JSON_TOKEN::LEFT_CB))
	{
		// error: cannot find object start
		m_index = crt_pos;
		return false;
	}
	members(obj);
	if (!consume(JSON_TOKEN::RIGHT_CB))
	{
		// ERROR: cannot find closing '}'
		m_index = crt_pos;
		return false;
	}
	
	result.set_object(obj);
	return true;
}

bool json_parser::array(json_value& result)
{
	std::list<json_value> lst;
	size_t crt_pos = m_index;
	if (!consume(JSON_TOKEN::LEFT_SB))
	{
		// error: cannot find array start
		m_index = crt_pos;
		return false;
	}
	elements(lst);
	if (!consume(JSON_TOKEN::RIGHT_SB))
	{
		// ERROR: cannot find closing ']'
		m_index = crt_pos;
		return false;
	}
	
	result.set_array(lst);
	return true;
}

bool json_parser::members(std::map<std::string, json_value>& obj)
{
	size_t crt_pos = m_index;
	json_value val;
	std::string key;
	
	if (!member(key, val))
	{
		// warning: 0 member object
		m_index = crt_pos;
		return false;
	}
	if (consume(JSON_TOKEN::COMMA) && !members(obj))
	{
		// ERROR: ',' without next object member
		m_index = crt_pos;
		return false;
	}
	
	obj.insert({key, val});
	return true;
}

bool json_parser::member(std::string& key, json_value& val)
{
	size_t crt_pos = m_index;
	if (!consume(JSON_TOKEN::STRING))
	{
		// error: no member key
		m_index = crt_pos;
		return false;
	}
	
	key = m_tokens[crt_pos].value();
	
	if (!consume(JSON_TOKEN::COLON))
	{
		// ERROR: no colon
		m_index = crt_pos;
		return false;
	}
	
	if (!element(val))
	{
		// ERROR: no member val
		m_index = crt_pos;
		return false;
	}
	return true;
}

bool json_parser::elements(std::list<json_value>& lst)
{
	size_t crt_pos = m_index;
	json_value val;
	
	if (!element(val))
	{
		// ERROR: no json element
		m_index = crt_pos;
		return false;
	}
	if (consume(JSON_TOKEN::COMMA) && !elements(lst))
	{
		// ERROR: ',' without next element
		m_index = crt_pos;
		return false;
	}
	lst.push_front(val);
	return true;
}

bool json_parser::element(json_value& val)
{
	size_t crt_pos = m_index;
	if (!value(val))
	{
		// ERROR: cannot extract value
		m_index = crt_pos;
		return false;
	}
	return true;
}
	
}  // namespace sandwich