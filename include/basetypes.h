#pragma once
#include <map>
#include <string>
#include <vector>
#include <boost/smart_ptr.hpp>

typedef std::map<std::string, std::string> StringMap;

typedef std::vector<std::string> StringVector;
typedef boost::shared_ptr<StringVector> StringVectorRef;

typedef boost::shared_ptr<std::string> StringRef;
typedef boost::shared_ptr<StringMap> StringMapRef;
