#include "config.h"

#define CONFIG_FILE L"config\\preferences.bin"

Config::ref Config::instance=Config::ref();

Config::ref Config::getInstance() {
	if (!instance) {
        instance=Config::ref(new Config());
	}
	return instance;
}

Config::Config(void){
    Serialize s(CONFIG_FILE, Serialize::READ);
    serialize(s);
}

void Config::save() {
    Serialize s(CONFIG_FILE, Serialize::WRITE);
    serialize(s);
}


void Config::serialize( Serialize &s ) {

    //////////////////////////////////////////////////////////////////////////
    // Serialization
    int version=1;
    s.streamInt(version, 1);
    
    s.streamBool(showOfflines, true);

    //message events
    s.streamBool(composing, true);
    s.streamBool(delivered, true);

    //local history
    s.streamBool(history, true);

    //signalling
    s.streamBool(vibra, true);
}

