#pragma once

#include <string>
#include <vector>
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

struct MucBookmarkItem {
    std::string name;
    std::string jid;
    std::string nick;
    std::string password;
    bool autoJoin;
    std::string url; //not used, for compatibility
};

class MucBookmarks : public JabberDataBlockListener {
public:
    MucBookmarks(){};
    ~MucBookmarks(){};

    virtual const char * getType() const{ return NULL; /* result | error */}
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    void doQueryBookmarks(ResourceContextRef rc);
    void doStoreBookmarks(ResourceContextRef rc);

    int getBookmarkCount() const;
    MucBookmarkItem * addNewBookmark();
    MucBookmarkItem * get(int i);

private:
    std::vector<MucBookmarkItem> bookmarks;
    std::string id;
};
