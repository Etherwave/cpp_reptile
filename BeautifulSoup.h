//
// Created by amzing on 2021/5/25.
//

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <codecvt>
#include <locale>

#ifndef AMAZING_BEAUTIFULSOUP_H
#define AMAZING_BEAUTIFULSOUP_H

using namespace std;

class BeautifulSoup
{
public:
    string name="";
    map<string, string> attribute;
    string text="";

    BeautifulSoup();
    BeautifulSoup(string text);
    BeautifulSoup find(string name, map<string, string> attribute=map<string, string>());
    vector<BeautifulSoup> find_all(string name, map<string, string> attribute=map<string, string>());
    BeautifulSoup rfind(string name, map<string, string> attribute=map<string, string>());

private:
    string raw_text="";
    void analyse();
    map<string, string> get_tag_attribute(string tag_begin);
};

#endif //AMAZING_BEAUTIFULSOUP_H
