#pragma once

#include "boostheaders.h"
#include "Contact.h"
#include "Message.h"

class History {
public:
    typedef boost::shared_ptr<History> ref;
private:
    History();
    static History::ref instance;
    
    std::wstring historyPath;

    bool writeHistory;

public:
    static History::ref getInstance();

    void appendHistory(Contact::ref c, Message::ref msg);

};