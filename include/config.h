#pragma once

#include <string>
#include "Serialize.h"

class Config
{
public:
	Config * getInstance();

    bool showOfflines;

    bool composing;
    bool delivered;
    bool history;


    void serialize(Serialize &s, bool read);

private:
	Config();
	~Config();
};
