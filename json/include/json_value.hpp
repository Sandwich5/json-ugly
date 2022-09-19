#pragma once

#include <string>
#include <map>
#include <list>
#include <variant>
#include <ostream>

#include <memory>

namespace sandwich
{
	
struct json_value;
using json_object = std::map<std::string, json_value>;
using json_array = std::list<json_value>;
	
struct json_value
{
	
	struct json_null
    {
        friend std::ostream& operator<<(std::ostream& out, const json_null& self)
        {
            return out << "null";
        }
    };
	
	std::variant<
		json_object,
		json_array,
		std::string,
		int,
		double,
		bool,
		json_null> m_value;
		
	void set_object(const json_object&);
	void set_array(const json_array&);
	void set_string(const std::string&);
	void set_number(double);
	// void set_integer(int);
	void set_boolean(bool);
	void set_null();
    
    bool is_object() const;
    bool is_array() const;
    bool is_string() const;
    bool is_int() const;
    bool is_double() const;
    bool is_boolean() const;
    bool is_null() const;
    
    std::string to_string(size_t i = 0) const;
};

using json_value_ptr = std::shared_ptr<json_value>;
	
}  // namespace sandwich