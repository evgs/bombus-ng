#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"
#include "basetypes.h"

//////////////////////////////////////////////////////////////////////////


class HtmlView : public Wnd {
public:
    HtmlView();
    HtmlView(HWND parent, const std::string & title);
    virtual ~HtmlView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;


    typedef boost::shared_ptr<HtmlView> ref;


protected:

    virtual HBITMAP getImage(LPCTSTR url, DWORD cookie);
    void setImage(HBITMAP bmp, DWORD cookie);

    virtual void onWmUserUpdate();
    virtual void onHotSpot(LPCSTR url, LPCSTR param){};

    void startHtml();

    void addText(const std::string &text);
    void addText(const char *text);
    void addText(const wchar_t *text);
    void addImg(const wchar_t *src);

    void beginForm(const char *name, const char *action);

    void button(const std::string &label);
    void button(const char *name, const std::string &label);

    void selectList(const char *name, const std::string &label, bool multiple);
    void endSelectList();
    void option(const char *name, const std::string &label, bool checked);


    void checkBox(const char *name, const std::string &label, bool checked);

    void textBox(const char *name, const std::string &label, const std::string &value);
    void passBox(const char *name, const std::string &label, const std::string &value);

    void textML(const char *name, const std::string &label, const std::string &value);
    void textConst(const std::string &label, const std::string &value);
    void url(const std::string &label, const std::string &url);
    
    void endForm();
    void endHtml();

    HWND htmlHWnd;

    std::wstring title;
    WndTitleRef  wt;    

    void init();
    static StringMapRef splitHREFtext(LPCSTR ht);

private:
    int width;    

    static ATOM windowClass;
    static HINSTANCE htmlViewInstance;

    ATOM RegisterWindowClass();
};


