//
// Created by amzing on 2021/5/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

#include <iconv.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <direct.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#endif

#ifndef AMAZING_REQUEST_H
#define AMAZING_REQUEST_H

using namespace std;

int UTF8ToGBK(char* input, size_t charInPutLen, char* output, size_t charOutPutLen);
int UTF8ToGBK(const string& input, string& output);
int GBKToUTF8(char* input, size_t charInPutLen, char* output, size_t charOutPutLen);
int GBKToUTF8(const string& input, string& output);

class URL_HTTP_HOST_PATH
{
    friend ostream &operator << (ostream &out, const URL_HTTP_HOST_PATH &url_http_host_path);

public:
    URL_HTTP_HOST_PATH();
    URL_HTTP_HOST_PATH(string url);
    bool set_url(string url);
    bool is_https();
    string get_host();
    string get_path();
    unsigned int get_port();

private:
    string url="";
    bool https_flag=false;
    string host="";
    string path="";
    unsigned int port=80;
    bool is_a_good_url= false;
    void analyse();
};

class Response
{
public:
    string HTTP_Version="";
    int state_code=200;
    string Reason_Phrase="";
    map<string, string> headers;

    string text="";
    char *content = nullptr;

    Response();
    Response(string text);
    void reset_text(string text);

private:
    string raw_text="";
    void analyse();
};

class Request
{
public:
    unsigned int port=80;
    vector<string> headers;

    Request();
    Response get(string url, string headers="");
    Response post(string url, string headers="", string data="");

private:
    string get_default_headers(string url);
    string post_default_headers(string url);
};

#endif //AMAZING_REQUEST_H
