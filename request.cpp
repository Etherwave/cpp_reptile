//
// Created by amzing on 2021/5/24.
//

#include "request.h"
//-------------------------------------ICONV_FUNCTION begin---------------------------------------------------
int UTF8ToGBK(char* input, size_t charInPutLen, char* output, size_t charOutPutLen)
{

    int ret =0;
    iconv_t cd;
    cd = iconv_open("GBK","utf-8");
    ret = iconv(cd, &input, &charInPutLen, &output, &charOutPutLen);
    iconv_close(cd);
    return ret;
}

int UTF8ToGBK(const string& input, string& output)
{
    int ret =0;
    size_t charInPutLen = input.length();
    if( charInPutLen == 0)
        return 0;
    char *pSource =(char *)input.c_str();
    size_t charOutPutLen = 2*charInPutLen;
    char *pTemp = new char[charOutPutLen];
    memset(pTemp,0,2*charInPutLen);

    iconv_t cd;
    char *pOut = pTemp ;
    cd = iconv_open("utf-8", "GBK");
    ret = iconv(cd, &pSource, &charInPutLen, &pTemp, &charOutPutLen);
    iconv_close(cd);
    output = pOut;
    delete []pOut;//注意这里，不能使用delete []pTemp, iconv函数会改变指针pTemp的值
    return ret;
}

int GBKToUTF8(char* input, size_t charInPutLen, char* output, size_t charOutPutLen)
{
    int ret = 0;
    iconv_t cd;
    cd = iconv_open("utf-8", "GBK");
    ret = iconv(cd, &input, &charInPutLen, &output, &charOutPutLen);
    iconv_close(cd);
    return ret;
}


int GBKToUTF8(const string& input, string& output)
{
    int ret = 0;
    size_t charInPutLen = input.length();
    if( charInPutLen == 0)
        return 0;

    size_t charOutPutLen = 2*charInPutLen+1;
    char *pTemp = new char[charOutPutLen];
    memset(pTemp,0,charOutPutLen);
    iconv_t cd;
    char *pSource =(char *)input.c_str();
    char *pOut = pTemp;
    cd = iconv_open("utf-8", "GBK");
    ret = iconv(cd, &pSource, &charInPutLen, &pTemp, &charOutPutLen);
    iconv_close(cd);
    output= pOut;
    delete []pOut; //注意这里，不能使用delete []pTemp, iconv函数会改变指针pTemp的值
    return ret;
}

//-------------------------------------ICONV_FUNCTION end---------------------------------------------------

//-------------------------------------URL_HTTP_HOST_PATH begin---------------------------------------------------

URL_HTTP_HOST_PATH::URL_HTTP_HOST_PATH()
{
    ;
}

URL_HTTP_HOST_PATH::URL_HTTP_HOST_PATH(string url)
{
    this->url = url;
    this->analyse();
}

bool URL_HTTP_HOST_PATH::is_https() {
    return this->https_flag;
}

string URL_HTTP_HOST_PATH::get_host()
{
    return this->host;
}

string URL_HTTP_HOST_PATH::get_path()
{
    return this->path;
}

bool URL_HTTP_HOST_PATH::set_url(string url)
{
    this->url = url;
    this->analyse();
    return true;
}

void URL_HTTP_HOST_PATH::analyse()
{
    //用来标记是否是一个正常的url，来判断是否解析出来的内容有意义
    this->is_a_good_url= false;
    //将url分成http_https, host, path三部分
    //在host后边有可能跟port，这里需要注意
    unsigned long http_https_start = this->url.find("http");
    if(http_https_start==this->url.npos || http_https_start!=0)return;

    //http://
    this->https_flag= false;
    unsigned long host_start = 7;

    if(this->url.size()<=4)return;

    //https://
    if(this->url[4]=='s'){
        this->https_flag = true;
        this->port=443;
        host_start = 8;
    }

    if(this->url.size()<=host_start)return;

    //一旦https://后边有内容，那么这个内容可以被看作一个host，满足了url的基本要求
    //可以把is_a_good_url设置为真了，并且可以给path一个默认的/路径
    this->is_a_good_url = true;
    this->path = "/";

    unsigned long host_end = this->url.find("/", host_start);

    //如果找不到host的结束，也就是path的开始位置/，那么就认为没给path，https://后边的全是host
    if(host_end==this->url.npos){
        this->host = this->url.substr(host_start, this->url.size()-host_start);
        //处理host中的port
        int port_start=this->host.find(":");
        if(port_start!=this->host.npos)
        {
            string port_str = this->host.substr(port_start+1, this->host.size()-host_start);
            try {this->port = stoi(port_str);}
            catch (...) {this->port=80;}
            this->host = this->host.substr(0, port_start);
        }
        return;
    }
    //如果找到了host的结束，那么就设置host，再去解析path
    this->host = this->url.substr(host_start, host_end-host_start);
    //处理host中的port
    int port_start=this->host.find(":");
    if(port_start!=this->host.npos)
    {
        string port_str = this->host.substr(port_start+1, this->host.size()-host_start);
        try {this->port = stoi(port_str);}
        catch (...) {this->port=80;}
        this->host = this->host.substr(0, port_start);
    }

    unsigned long path_start = host_end;

    this->path = this->url.substr(path_start, this->url.size()-path_start);
}

unsigned int URL_HTTP_HOST_PATH::get_port() {
    return this->port;
}

ostream &operator << (ostream &out, const URL_HTTP_HOST_PATH &url_http_host_path)
{
    out<<"http: "<<url_http_host_path.https_flag
       <<" host: "<<url_http_host_path.host
       <<" path: "<<url_http_host_path.path
       <<" port: "<<url_http_host_path.port;
    return out;
}

//-------------------------------------URL_HTTP_HOST_PATH end---------------------------------------------------


//-------------------------------------Response begin---------------------------------------------------

Response::Response()
{
    ;
}

Response::Response(string text)
{
    this->raw_text = text;
    this->analyse();
}

void Response::reset_text(string text) {
    this->raw_text = text;
    this->analyse();
}

void Response::analyse() {
    //首先找到header部分
    int header_end = this->raw_text.find("\r\n\r\n");
    if(header_end<0)return;
    //让header最后包含一个\r\n好处理
    header_end+=2;
    string header = this->raw_text.substr(0, header_end);

    //获取第一行，刷新header
    int first_line_end = header.find("\r\n");
    if(first_line_end<0)return;
    string first_line = header.substr(0, first_line_end);
    header = header.substr(first_line_end+2, header.size()-first_line_end);
    int http_version_end = first_line.find(" ");
    if(http_version_end<0)return;
    int state_code_end = first_line.find(" ", http_version_end+1);
    if(state_code_end<0)return;
    //获取http_version
    this->HTTP_Version = first_line.substr(0, http_version_end);
    //获取state_code
    string state_code_str = first_line.substr(http_version_end, state_code_end-http_version_end);
    try {this->state_code = stoi(state_code_str);}
    catch (...) {this->state_code=-1;}
    //获取Reason-Phrase
    this->Reason_Phrase = first_line.substr(state_code_end+1, first_line.size()-state_code_end-1);

    //解析header
//    cout<<header<<endl;
    while(header.size())
    {
        //找到第一行根新header
        int line_end = header.find("\r\n");
        if(line_end<0)return;
        string line = header.substr(0, line_end);
        header = header.substr(line_end+2, header.size()-line_end-1);

        int key_end = line.find(":");
        if(key_end<0)return;
        string key = line.substr(0, key_end);
        string value = line.substr(key_end+1, line.size()-key_end-1);
        this->headers[key] = value;
    }
    //获取text
    this->text = this->raw_text.substr(header_end+2, this->raw_text.size()-header_end);
}

//-------------------------------------Response end---------------------------------------------------


//-------------------------------------Request begin---------------------------------------------------
Request::Request()
{
    ;
}

Response Request::get(string url, string headers)
{
    Response response;
    int client_socket;
    struct sockaddr_in server_addr;
    const SSL_METHOD *meth;
    SSL_CTX *ctx;
    SSL *ssl;
    //分析url
    URL_HTTP_HOST_PATH url_http_host_path(url);
//    cout<<url_http_host_path<<endl;
#ifdef _WIN32
    //Winsows下启用socket
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(1,1),&wsadata)==SOCKET_ERROR) {
        printf("WSAStartup() fail\n");
        exit(0);
    }
#endif

    //创建socket
    client_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0) {
        cout << "socket error !";
        return response;
    }
    if(url_http_host_path.is_https())
    {
        //添加SSL的加密/HASH算法
        SSLeay_add_ssl_algorithms();
        //客户端，服务端选择SSLv23_server_method()
        meth = SSLv23_client_method();
        //建立新的SSL上下文
        ctx = SSL_CTX_new(meth);
        if (ctx == nullptr) {
            cout << "SSL_CTX_new error !";
            return response;
        }
    }

    //获取服务器的ip
    struct hostent* host_ip = gethostbyname(url_http_host_path.get_host().c_str());
    //设置socket
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family =AF_INET;
    server_addr.sin_addr = *(struct in_addr *)host_ip->h_addr_list[0];
    server_addr.sin_port = htons(url_http_host_path.get_port());

    //连接服务器
    int flag = connect(client_socket, (struct sockaddr *) &server_addr, sizeof (struct sockaddr_in ));
    if(flag<0){
        cout << "connect error 1";
        return response;
    }

    if(url_http_host_path.is_https())
    {
        //建立SSL
        int ret;
        ssl = SSL_new(ctx);
        if (ssl == nullptr) {
            cout << "SSL NEW error";
            return response;
        }
        //将SSL与TCP SOCKET 连接
        SSL_set_fd(ssl, client_socket);
        // SSL连接
        ret = SSL_connect(ssl);
        if (ret == -1) {
            cout << "SSL ACCEPT error ";
            return response;
        }
    }

    //初始化headers
    if(headers.size()==0)headers = get_default_headers(url);
//    cout<<headers<<endl;

    string data = "";
    string send_package = headers+data;

    //发送请求
    if(url_http_host_path.is_https())
    {
        int flag = SSL_write(ssl, send_package.c_str(), send_package.size());
        if (flag == -1) {
            cout << "SSL write error !";
            return response;
        }
    }
    else
    {
        int flag = send(client_socket, send_package.c_str(), send_package.size(), 0);
        if(flag==-1)
        {
            cout<<"send failed!"<<endl;
            return response;
        }
    }

    //接收响应
    const int buff_size = 1024;
    char buff[buff_size];
    int recv_size = 0;
    //注意这里要使用stringstream而不能用普通的string。
    //这样当访问到图片信息或其他二进制文件时，才不会被文件中出现的\0所终止string的增长。
    stringstream response_stream;

    if(url_http_host_path.is_https())
    {
        while((recv_size = SSL_read(ssl, buff, buff_size))>0)
        {
//            cout<<recv_size<<endl;
            response_stream.write(buff, recv_size);
        }
    }
    else
    {
        while((recv_size = recv(client_socket, buff, buff_size, 0))>0)
        {
//            cout<<recv_size<<endl;
            response_stream.write(buff, recv_size);
        }
    }
    response.reset_text(response_stream.str());

    //关闭连接
    if(url_http_host_path.is_https())
    {
        //关闭SSL套接字
        SSL_shutdown(ssl);
        //释放SSL套接字
        SSL_free(ssl);
        //释放SSL会话环境
        SSL_CTX_free(ctx);
    }

#ifdef _WIN32
    //Winsows下关闭socket
    closesocket(client_socket);
#else
    close(client_socket);
#endif
    return response;
}

Response Request::post(string url, string headers, string data)
{
    Response response;
    int client_socket;
    struct sockaddr_in server_addr;
    const SSL_METHOD *meth;
    SSL_CTX *ctx;
    SSL *ssl;
    //分析url
    URL_HTTP_HOST_PATH url_http_host_path(url);
//    cout<<url_http_host_path<<endl;
#ifdef _WIN32
    //Winsows下启用socket
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(1,1),&wsadata)==SOCKET_ERROR) {
        printf("WSAStartup() fail\n");
        exit(0);
    }
#endif

    //创建socket
    client_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0) {
        cout << "socket error !";
        return response;
    }
    if(url_http_host_path.is_https())
    {
        //添加SSL的加密/HASH算法
        SSLeay_add_ssl_algorithms();
        //客户端，服务端选择SSLv23_server_method()
        meth = SSLv23_client_method();
        //建立新的SSL上下文
        ctx = SSL_CTX_new(meth);
        if (ctx == nullptr) {
            cout << "SSL_CTX_new error !";
            return response;
        }
    }

    //获取服务器的ip
    struct hostent* host_ip = gethostbyname(url_http_host_path.get_host().c_str());
    //设置socket
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family =AF_INET;
    server_addr.sin_addr = *(struct in_addr *)host_ip->h_addr_list[0];
    server_addr.sin_port = htons(url_http_host_path.get_port());

    //连接服务器
    int flag = connect(client_socket, (struct sockaddr *) &server_addr, sizeof (struct sockaddr_in ));
    if(flag<0){
        cout << "connect error 1";
        return response;
    }

    if(url_http_host_path.is_https())
    {
        //建立SSL
        int ret;
        ssl = SSL_new(ctx);
        if (ssl == nullptr) {
            cout << "SSL NEW error";
            return response;
        }
        //将SSL与TCP SOCKET 连接
        SSL_set_fd(ssl, client_socket);
        // SSL连接
        ret = SSL_connect(ssl);
        if (ret == -1) {
            cout << "SSL ACCEPT error ";
            return response;
        }
    }

    //初始化headers
    if(headers.size()==0)headers = get_default_headers(url);
//    cout<<headers<<endl;

    string send_package = headers+data;

    //发送请求
    if(url_http_host_path.is_https())
    {
        int flag = SSL_write(ssl, send_package.c_str(), send_package.size());
        if (flag == -1) {
            cout << "SSL write error !";
            return response;
        }
    }
    else
    {
        int flag = send(client_socket, send_package.c_str(), send_package.size(), 0);
        if(flag==-1)
        {
            cout<<"send failed!"<<endl;
            return response;
        }
    }

    //接收响应
    const int buff_size = 1024;
    char buff[buff_size];
    int recv_size = 0;
    //注意这里要使用stringstream而不能用普通的string。
    //这样当访问到图片信息或其他二进制文件时，才不会被文件中出现的\0所终止string的增长。
    stringstream response_stream;

    if(url_http_host_path.is_https())
    {
        while((recv_size = SSL_read(ssl, buff, buff_size))>0)
        {
//            cout<<recv_size<<endl;
            response_stream.write(buff, recv_size);
        }
    }
    else
    {
        while((recv_size = recv(client_socket, buff, buff_size, 0))>0)
        {
//            cout<<recv_size<<endl;
            response_stream.write(buff, recv_size);
        }
    }
    response.reset_text(response_stream.str());

    //关闭连接
    if(url_http_host_path.is_https())
    {
        //关闭SSL套接字
        SSL_shutdown(ssl);
        //释放SSL套接字
        SSL_free(ssl);
        //释放SSL会话环境
        SSL_CTX_free(ctx);
    }

#ifdef _WIN32
    //Winsows下关闭socket
    closesocket(client_socket);
#else
    close(client_socket);
#endif
    return response;
}

string Request::get_default_headers(string url)
{
    URL_HTTP_HOST_PATH url_http_host_path(url);
    stringstream header_stream;

    header_stream<<"GET "+url_http_host_path.get_path()+" HTTP/1.0\r\n";
    header_stream<<"Host: " + url_http_host_path.get_host()+"\r\n";
    header_stream<<"Accept: */*\r\n";
    header_stream<<"Accept-Language: zh-Hans-CN, zh-Hans; q=0.8, en-US; q=0.5, en; q=0.3\r\n";
    header_stream<<"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0\r\n";
    header_stream<<"Connection: close\r\n";
    header_stream<<"\r\n";
    return header_stream.str();
}

string Request::post_default_headers(string url)
{
    URL_HTTP_HOST_PATH url_http_host_path(url);
    stringstream header_stream;

    header_stream<<"POST "+url_http_host_path.get_path()+" HTTP/1.0\r\n";
    header_stream<<"Host: " + url_http_host_path.get_host()+"\r\n";
    header_stream<<"Accept: */*\r\n";
    header_stream<<"Accept-Language: zh-Hans-CN, zh-Hans; q=0.8, en-US; q=0.5, en; q=0.3\r\n";
    header_stream<<"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0\r\n";
    header_stream<<"Connection: close\r\n";
    header_stream<<"\r\n";
    return header_stream.str();
}
//-------------------------------------Request end---------------------------------------------------