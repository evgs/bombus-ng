#include "XDataForm.h"
#include "utf8.hpp"
//////////////////////////////////////////////////////////////////////////
void XDataForm::onWmUserUpdate(){
    startHtml();

    constructForm();

    endHtml();
}

void XDataForm::onHotSpot(LPCSTR url, LPCSTR param){
    StringMapRef result=splitHREFtext(param);

    JabberDataBlockRef reply=JabberDataBlockRef(new JabberDataBlock(xdata->getTagName().c_str(),NULL));
    reply->setAttribute("xmlns","jabber:x:data");
    reply->setAttribute("type", "submit");
    
    JabberDataBlockRefList *childs=xdata->getChilds();

    JabberDataBlockRefList::const_iterator i;
    for (i=childs->begin(); i!=childs->end(); i++) {
        JabberDataBlockRef field=*i;
        if (field->getTagName()!="field") continue;

        //const std::string &value=field->getChildText("value");
        //const std::string &label=field->getAttribute("label");
        const std::string &var=field->getAttribute("var");
        const std::string &type=field->getAttribute("type");

        //hidden
        if (type=="hidden") {
            reply->addChild(field); //whole copy of "hidden"
            continue;
        }

        //fixed
        if (type=="fixed") continue; //drop all "fixed"

        field=reply->addChild("field", NULL);
        field->setAttribute("var", var.c_str());
        field->setAttribute("type", type);

        const std::string &value=result->operator [](var); 

        //boolean
        if (type=="boolean") {
            bool checked=value.length()>0;
            field->addChild("value", (checked)? "1" : "0" );
            continue;
        }

        if (type=="text-multi" || type=="jid-multi" || type=="list-multi") {
            std::string valbuf;
            for (size_t i=0; i<value.length(); i++) {
                char c=value[i];
                if (c==0x0a) { 
                    if (valbuf.length()) 
                        field->addChild("value", valbuf.c_str()); 
                    valbuf.clear();
                } else valbuf+=c;
            }
            if (valbuf.length()) field->addChild("value", valbuf.c_str());
            continue;
        }

        //text-private, text-single, jid-single, list-single
        field->addChild("value", value.c_str());

    }
    onSubmit(reply);
}

void XDataForm::onSubmit(JabberDataBlockRef replyForm) {
#ifdef DEBUG
    StringRef out=replyForm->toXML();
#endif
}


void XDataForm::constructForm(){
    if (plainText.length()) {
        addText(plainText);
    }

    if (!xdata) {
        return;
    }

    const std::string &title=xdata->getChildText("title");
    if (title.length()) {
        addText("<big><b>"); addText(title); addText("</b></big><br/><br/>");
    }
    const std::string &instr=xdata->getChildText("instructions");
    if (title.length()) {
        addText("<i>"); addText(instr); addText("</i><br/>");
    }

    beginForm("XData-Form","send");

    JabberDataBlockRefList *childs=xdata->getChilds();

    JabberDataBlockRefList::const_iterator i;
    for (i=childs->begin(); i!=childs->end(); i++) {
        JabberDataBlockRef field=*i;
        if (field->getTagName()!="field") continue;

        bool required=field->getChildByName("required");
        const std::string &value=field->getChildText("value");
        const std::string &var=field->getAttribute("var");
        const std::string &type=field->getAttribute("type");
        std::string label=field->getAttribute("label");
        if (required) label+=" *";

        //hidden
        if (type=="hidden") continue;

        //fixed
        if (type=="fixed") {
            if (label.length()) {
                addText(label);
                addText(": ");
            }
            addText(value); addText("<br/>");
            continue;
        }

        //boolean
        if (type=="boolean") {
            checkBox(var.c_str(), label, value=="1");
            continue;
        }

        //text-private
        if (type=="text-private") {
            passBox(var.c_str(), label, value);
            continue;
        }

        //list-single
        if (type=="list-single" || type=="list-multi") {
            selectList(var.c_str(), label, type=="list-multi");
            JabberDataBlockRefList *options=field->getChilds();
            JabberDataBlockRefList::const_iterator l;
            for (l=options->begin(); l!=options->end(); l++) {
                JabberDataBlockRef opt=*l;
                if (opt->getTagName()!="option") continue;
                const std::string &val=opt->getChildText("value");
                option(val.c_str(), opt->getAttribute("label"), (val.length())? field->hasChildByValue(val.c_str()): false);
            }
            endSelectList();
            continue;
        }

        //text-multi & jid-multi
        if (type=="text-multi" || type=="jid-multi") {
            std::string valbuf;
            JabberDataBlockRefList *lines=field->getChilds();
            JabberDataBlockRefList::const_iterator l;
            for (l=lines->begin(); l!=lines->end(); l++) {
                JabberDataBlockRef value=*l;
                if (value->getTagName()!="value") continue;
                if (valbuf.length()) valbuf+="\n";
                valbuf+=value->getText();
            }
            textML(var.c_str(), label, valbuf);

            continue;
        }

        // last: text-single & jid-single
        textBox(var.c_str(), label, value);
    }
    std::string &type=xdata->getAttribute("type");
    if (type=="form") button("Submit");
    //if (type.empty()) button("Submit");

    endForm();
}


//////////////////////////////////////////////////////////////////////////
// THIS IS TEMPORARY DEBUG CODE
//////////////////////////////////////////////////////////////////////////
#ifdef DEBUG

XDataForm::ref XDataForm::createXDataForm( HWND parent, const std::string &title, ResourceContextRef rc) {
    XDataForm *xf=new XDataForm();

    xf->parentHWnd=parent;
    xf->title=utf8::utf8_wchar(title);
    xf->wt=WndTitleRef(new WndTitle(xf, icons::ICON_COMPOSING_INDEX));

    xf->init();

    SetParent(xf->thisHWnd, parent);

    xf->rc=rc;

    XDataForm::ref xdRef=XDataForm::ref(xf);

    //GetVcard *getv=new GetVcard(jid, vfRef);
    //rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getv));
    //getv->doRequest(rc);

    return xdRef;
}

void XDataForm::formTest() {
    xdata=JabberDataBlockRef(new JabberDataBlock("x"));
    xdata->setAttribute("xmlns","jabber:x:data");
    xdata->setAttribute("type","form");

    xdata->addChild("title","Test form");

    xdata->addChild("instructions","add here any instructions about this form");

    JabberDataBlockRef field;
    JabberDataBlockRef opt;

    // ++ boolean
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","boolean");
    field->setAttribute("var","boolbox");
    field->setAttribute("label","Boolean checkbox");
    field->addChild("value","1");

    // ++ fixed
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","fixed");
    field->addChild("value","this is constant text value");

    // ++ hidden
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","hidden");
    field->setAttribute("var","hvr");
    field->addChild("value","this is hidden text");

    // ++ jid-multi
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","jid-multi");
    field->setAttribute("var","jidm");
    field->setAttribute("label","Jabber ID");
    field->addChild("value","user1@jabber.ru");
    field->addChild("value","user2@jabber.ru");
    field->addChild("value","user3@jabber.ru");
    field->addChild("value","user4@jabber.ru");

    // ++ jid-single
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","jid-single");
    field->setAttribute("var","jid");
    field->setAttribute("label","Jabber ID");
    field->addChild("value","user@jabber.ru");

    // + list-multi
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","list-multi");
    field->setAttribute("var","radio");
    field->setAttribute("label","multi-selection");
    field->addChild("value","p2");
    field->addChild("value","p4");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 1"); opt->addChild("value", "p1");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 2"); opt->addChild("value", "p2");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 3"); opt->addChild("value", "p3");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 4"); opt->addChild("value", "p4");

    // ++ list-single
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","list-single");
    field->setAttribute("var","combo");
    field->setAttribute("label","Single-selection");
    field->addChild("value","p2");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 1"); opt->addChild("value", "p1");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 2"); opt->addChild("value", "p2");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 3"); opt->addChild("value", "p3");
    opt=field->addChild("option", NULL); opt->setAttribute("label","option 4"); opt->addChild("value", "p4");


    // + text-multi
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","jid-multi");
    field->setAttribute("var","mlb");
    field->setAttribute("label","Multiline textbox");
    field->addChild("value","line1");
    field->addChild("value","line2");
    field->addChild("value","line3");
    field->addChild("value","line4");

    // ++ text-private
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","text-private");
    field->setAttribute("var","passbox");
    field->setAttribute("label","Password");
    field->addChild("value","iddqd");

    // ++ text-single
    field=xdata->addChild("field", NULL);
    field->setAttribute("type","text-single");
    field->setAttribute("var","textbox");
    field->setAttribute("label","Singleline textbox");
    field->addChild("value","Sample text");

    onWmUserUpdate();
}

#endif

