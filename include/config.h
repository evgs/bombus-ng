#pragma once

#include <string>
#include "boostheaders.h"
#include "Serialize.h"

class Config
{
public:
    ~Config(){};

    typedef boost::shared_ptr<Config> ref;

    static Config::ref getInstance();

    bool showOfflines;
    bool showGroups;
    bool sortByStatus;

    bool composing;
    bool delivered;
    bool history;

    bool vibra;
    bool sounds;

    bool raiseSIP;


    void save();
private:
    void serialize(Serialize &s);
    static Config::ref instance;

	Config();
};
