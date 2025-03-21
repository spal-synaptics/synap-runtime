// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once
#include <string>
#include <vector>


namespace synaptics {
namespace synap {
namespace format_parse {


/// Get format type.
/// Format type if present must be at the beginning of the string.
/// @param s: format string
/// @return format type
std::string get_type(const std::string& s);


/// Search for key in string and return the associated value as integer.
/// Example "val=4"
/// @param s: format string
/// @param key: value key
/// @param default_value: value to be returned if key not found
/// @return value if found or default_value
int get_int(const std::string& s, const char* key, int default_value = 0);


/// Search for key in string and return the associated value as float.
/// Example "val=3.5"
/// @param s: format string
/// @param key: value key
/// @param default_value: value to be returned if key not found
/// @return value if found or default_value
float get_float(const std::string& s, const char* key, float default_value = 0);


/// Search for key in string and return the associated value as boolean.
/// Example "val=true"
/// @param s: format string
/// @param key: value key
/// @param default_value: value to be returned if key not found
/// @return value if found or default_value
bool get_bool(const std::string& s, const char* key, bool default_value = false);


/// Search for key in string and return the associated value as string.
/// Example "val=true"
/// @param s: format string
/// @param key: value key
/// @param default_value: value to be returned if key not found
/// @return value if found or default_value
std::string get_string(const std::string& s, const char* key, std::string default_value = "");


/// Search for key in string and return the position (index) of the associated value
/// @return index of beginnig of value, or string::npos if key not found
size_t value_pos(const std::string& s, const char* key);


/// Search for key in string and return the associated float vector.
/// Example "vv=[1, 0.5, 9]"
/// @param s: format string
/// @param key: value key (or null)
/// @return vector with values or empty vector if key not found
std::vector<float> get_floats(const std::string& s, const char* key);


/// Search for key in string and return the associated int vector.
/// Example "vv=[1, 5, 9]"
/// @param s: format string
/// @param key: value key (or null)
/// @return vector with values or empty vector if key not found
std::vector<int> get_ints(const std::string& s, const char* key);


/// Search for key in string and return the associated vector of int vectors.
/// Example "vv=[[1, 5, 9], [], [1, 2]]"
/// @param s: format string
/// @param key: value key (or null)
/// @return vector with values or empty vector if key not found
std::vector<std::vector<int>> get_ints2(const std::string& s, const char* key);


}  // namespace format_parse
}  // namespace synap
}  // namespace synaptics
