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

extern std::wstring appRootPath;
void Config::save() {
    std::wstring configPath=appRootPath+L"config";
    CreateDirectory(configPath.c_str(), NULL);

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
    s.streamBool(sounds, true);

    s.streamBool(showGroups, true);
    s.streamBool(sortByStatus, true);

    //SIP control
    s.streamBool(raiseSIP, false);

    //Automatic connection
    s.streamBool(connectOnStartup, false);
}

