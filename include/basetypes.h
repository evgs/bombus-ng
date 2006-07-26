#pragma once
#include <map>
#include <string>
#include <boost/smart_ptr.hpp>

typedef std::map<std::string, std::string> StringMap;

typedef boost::shared_ptr<std::string> StringRef;
typedef boost::shared_ptr<StringMap> StringMapRef;
