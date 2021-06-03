#include "request.h"
#include "BeautifulSoup.h"
#include <fstream>

using namespace std;

string save_folder = "/home/nlc/image";

class PicGroup
{
public:
    string name;
    vector<string> urls;
};

bool download_pic(string image_url, string save_folder)
{
    //如果文件夹不存在，则创建文件夹
#ifdef _WIN32
    if(access(save_folder.c_str(), 0)==-1)mkdir(save_folder.c_str());
#else
    if(access(save_folder.c_str(), 0)==-1)mkdir(save_folder.c_str(), 00777);
#endif

    bool flag = false;
    int name_start = image_url.rfind("/")+1;
    if(name_start<0)return flag;
    int name_end = image_url.size();
    string image_name = image_url.substr(name_start, name_end-name_start);
    Request request;

    stringstream header_stream;
    URL_HTTP_HOST_PATH url_http_host_path(image_url);
    header_stream<<"GET "+url_http_host_path.get_path()+" HTTP/1.0\r\n";
    header_stream<<"Host: " + url_http_host_path.get_host()+"\r\n";
    header_stream<<"Accept: */*\r\n";
    header_stream<<"Accept-Language: zh-Hans-CN, zh-Hans; q=0.8, en-US; q=0.5, en; q=0.3\r\n";
    header_stream<<"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0\r\n";
    header_stream<<"Referer: http://cj.jj20.com/\r\n";
    header_stream<<"Connection: close\r\n";
    header_stream<<"\r\n";
    Response response = request.get(image_url, header_stream.str());
    if(response.state_code!=200)return flag;
    fstream outfile;
    outfile.open(save_folder+"/"+image_name, ios::out);
    if(outfile.is_open())
    {
        outfile.write(response.text.c_str(), response.text.size());
        flag = true;
    }
    outfile.close();
    return flag;
}

PicGroup get_group_name_urls(string group_url)
{
    PicGroup picGroup;
    Request request;
    Response response = request.get(group_url);
    GBKToUTF8(response.text, response.text);
    BeautifulSoup soup(response.text);
    BeautifulSoup h1 = soup.find("h1");
    BeautifulSoup span = h1.find("span");
    int name_start=0;
    int name_end = span.text.find("(");
    if(name_end<0)return picGroup;
    picGroup.name = span.text.substr(name_start, name_end-name_start);
    int totle_page_start = span.text.find("/");
    if(totle_page_start<0)return picGroup;
    totle_page_start+=1;
    int totle_page_end = span.text.size()-1;
    string totle_page_str = span.text.substr(totle_page_start, totle_page_end-totle_page_start);
    int totle_page = -1;
    try{totle_page = stoi(totle_page_str);}
    catch (...){totle_page=-1;}
    if(totle_page<0)return picGroup;

    BeautifulSoup body = soup.find("body");
    vector<BeautifulSoup> scripts = body.find_all("script");
    BeautifulSoup script = scripts[2];

    int url_start = script.text.find("\'");
    if(url_start<0)return picGroup;
    int url_end = script.text.find("\'", url_start+1);
    if(url_end<0)return picGroup;
    url_start++;
    URL_HTTP_HOST_PATH url_http_host_path(group_url);
    string url = "";
    if(url_http_host_path.is_https())url+="https://";
    else url+="http://";
    url+="pic.jj20.com";
    url+=script.text.substr(url_start, url_end-url_start-5);

    for(int i=1;i<=totle_page;i++)
    {
        string t_utl = url+(char)(i+'0')+".jpg";
        picGroup.urls.push_back(t_utl);
    }

    return picGroup;
}

int main(int argc, char **argv ) {
    string group_url = "http://www.jj20.com/bz/nxxz/shxz/318786.html";
    PicGroup picGroup = get_group_name_urls(group_url);
    cout<<picGroup.name<<endl;

    for(int i=0;i!=picGroup.urls.size();i++)
    {
        cout<<picGroup.urls[i]<<endl;
        download_pic(picGroup.urls[i], save_folder+"/"+picGroup.name);
    }
    return 0;
}
