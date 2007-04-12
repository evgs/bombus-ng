#pragma once

#include "Image.h"
#include "boost/"

class BNode {
    BNode * bTrue;
    BNode * bFalse;
    char sIndex;
    char nodeChar;
};

class SmileParser {
public:
    SmileParser();
    ~SmileParser();

    int findsmile();

protected:
    loadSmiles();
    ImgListRef icons;
    BNode* root;
};