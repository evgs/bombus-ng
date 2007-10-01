#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include "Serialize.h"

class Config
{
public:
    ~Config(){};

    typedef boost::shared_ptr<Config> ref;

    static Config::ref getInstance();

    bool showOfflines;

    bool composing;
    bool delivered;
    bool history;

    bool vibra;


    void save();
private:
    void serialize(Serialize &s);
    static Config::ref instance;

	Config();
};
