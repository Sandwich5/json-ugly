#include "include/json_value.hpp"

#include <sstream>
#include <iostream>  // debug

namespace sandwich
{
	
void json_value::set_object(const json_object& object)
{
	this->m_value = object;
}

void json_value::set_array(const json_array& array)
{
	this->m_value = array;
}

void json_value::set_string(const std::string& string)
{
	this->m_value = string;
}
void json_value::set_number(double number)
{
	this->m_value = number;
}

void json_value::set_boolean(bool boolean)
{
	this->m_value = boolean;
}

void json_value::set_null()
{
	this->m_value = json_value::json_null{};
}

bool json_value::is_object() const
{
    return std::holds_alternative<json_object>(m_value);
}

bool json_value::is_array() const
{
    return std::holds_alternative<json_array>(m_value);
}

bool json_value::is_string() const
{
    return std::holds_alternative<std::string>(m_value);
}

bool json_value::is_int() const
{
    return std::holds_alternative<int>(m_value);
}

bool json_value::is_double() const
{
    return std::holds_alternative<double>(m_value);
}

bool json_value::is_boolean() const
{
    return std::holds_alternative<bool>(m_value);
}

bool json_value::is_null() const
{
    return std::holds_alternative<json_value::json_null>(m_value);
}



std::string json_value::to_string(size_t i) const
{
    static size_t safe_counter = 11;
    safe_counter--;
    if (safe_counter == 0) return "";
    
    std::stringstream ss;
    ss << std::string(4*i, ' ');
    
    if (this->is_string())
    {
        ss << '"' 
            << std::get<std::string>(this->m_value)\
            << '"';
    }
    if (this->is_boolean())
    {
        ss << std::boolalpha
            << std::get<bool>(this->m_value);
    }
    if (this->is_null())
    {
        ss << "null";
    }
    if (this->is_double())
    {
        ss << std::get<double>(this->m_value);
    }
    if (this->is_array())
    {
        ss << "[\n";
        const auto& lst = std::get<json_array>(this->m_value);
        auto prev_last = std::prev(lst.end());
        auto iter = lst.begin();
        for (; iter != prev_last; ++iter)
        {
            ss << iter->to_string(i+1) << ",\n";
        }
        ss << iter->to_string(i+1) << "\n";
        ss << std::string(i*4, ' ') << "]";
    }
    if (this->is_object())
    {
        ss << "{\n";
        const auto& obj = std::get<json_object>(this->m_value);
        auto prev_last = std::prev(obj.end());
        auto iter = obj.begin();
        for (; iter != prev_last; ++iter)
        {
            ss << std::string(4*i+4, ' ')
                << '"'
                << iter->first
                << "\":";
            if (iter->second.is_object() or
                iter->second.is_array())
            {
                ss << '\n'
                    << iter->second.to_string(i+1) << ",\n";
            }
            else
            {
                ss << ' '
                    << iter->second.to_string(0) << ",\n";
            }
        }
        ss << std::string(4*i+4, ' ')
            << '"'
            << iter->first
            << "\":";
        if (iter->second.is_object() or
            iter->second.is_array())
        {
            ss << '\n'
                << iter->second.to_string(i+1) << "\n";
        }
        else
        {
            ss << ' '
                << iter->second.to_string(0) << "\n";
        }
        ss << std::string(i*4, ' ') << '}';
    }
   
    return ss.str();
}
	
}  // namespace sandwich