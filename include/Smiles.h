#pragma once

#include "Image.h"

class BNode {
public:
    BNode();
    ~BNode();
    BNode * bTrue;
    BNode * bFalse;
    char sIndex;
    char nodeChar;
};

class SmileParser {
public:
    SmileParser();
    ~SmileParser();

    int findSmile(LPCTSTR *pstr);
    ImgListRef icons;

protected:
    void loadSmiles();
    BNode* root;

    void addSmile(const char *smile, int index);
};