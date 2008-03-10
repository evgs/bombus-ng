#pragma once

#include <string>
#include <vector>
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"
#include "boostheaders.h"

class MucBookmarkItem {
public:
    std::string name;
    std::string jid;
    std::string nick;
    std::string password;
    bool autoJoin;
    std::string url; //not used, for compatibility

    typedef boost::shared_ptr<MucBookmarkItem> ref;

    static bool compare(MucBookmarkItem::ref left, MucBookmarkItem::ref right);
};

class MucBookmarks : public JabberDataBlockListener {
public:
    MucBookmarks(){ bookmarksAvailable=false; };
    ~MucBookmarks(){};

    virtual const char * getType() const{ return NULL; /* result | error */}
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    void doQueryBookmarks(ResourceContextRef rc);
    void doStoreBookmarks(ResourceContextRef rc) const;

    int getBookmarkCount() const;
    MucBookmarkItem::ref addNewBookmark();
    MucBookmarkItem::ref get(int i);

    bool isBookmarksAvailable() const {return bookmarksAvailable;} ;
private:
    bool bookmarksAvailable;
    std::vector<MucBookmarkItem::ref> bookmarks;
    std::string id;
};
