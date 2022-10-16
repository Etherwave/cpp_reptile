#include "request.h"
#include "BeautifulSoup.h"
#include "ThreadPool.h"
#include <fstream>

using namespace std;

string save_folder = "C:/Users/Amazing/Pictures";

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
    URL_HTTP_HOST_PATH url_http_host_path(image_url);

    map<string, string> headers = {
            {"Host", url_http_host_path.get_host()},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0"},
            {"Accept", "*/*"},
            {"Accept-Language", "zh-Hans-CN, zh-Hans; q=0.8, en-US; q=0.5, en; q=0.3"},
            {"Accept-Encoding", "gzip, deflate"},
            {"Connection", "close"},
            {"upgrade-insecure-requests", "1"},
    };
    Response response = request.get(image_url, headers);

    if(response.state_code!=200)return flag;
    fstream outfile;
    outfile.open(save_folder+"/"+image_name, ios::out|ios::binary);
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
    map<string, string> headers ={
            {"Host", "www.umei.cc"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0"},
            {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
            {"Accept-Language", "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2"},
            {"Accept-Encoding", "gzip, deflate"},
            {"Connection", "close"},
            {"Upgrade-Insecure-Requests", "1"},
    };
    Response response = request.get(group_url, headers);
    #ifdef _WIN32
    EncodeChange(response.text, response.text, "UTF-8", "GBK");
    #endif

    BeautifulSoup soup(response.text);
    BeautifulSoup h1 = soup.find("h1");
    BeautifulSoup div_page = soup.find("div", {{"class", "pages"}});
    vector<BeautifulSoup> div_page_a = div_page.find_all("a");
    picGroup.name = h1.text;

    vector<string> pages_url;
    for(int i=0;i!=div_page_a.size();i++)
    {
        if(div_page_a[i].text[0]>='0'&&div_page_a[i].text[0]<='9')
        {
            if(i==0)pages_url.push_back(group_url);
            else pages_url.push_back("https://www.umei.cc"+div_page_a[i].attribute["href"]);
        }
    }

    for(int i=0;i!=pages_url.size();i++)
    {
        response = request.get(pages_url[i], headers=headers);
        #ifdef _WIN32
        EncodeChange(response.text, response.text, "UTF-8", "GBK");
        #endif
        soup = BeautifulSoup(response.text);
        BeautifulSoup div_big_pic = soup.find("div", {{"class", "big-pic"}});
        BeautifulSoup div_big_pic_img = div_big_pic.find("img");
        string image_url = div_big_pic_img.attribute["src"];
        string new_host ="https://kr.zutuanla.com";
        string old_host = "http://kr.shanghai-jiuxin.com";
        image_url.replace(0, old_host.size(), new_host);
        picGroup.urls.push_back(image_url);
    }
    return picGroup;
}

int main(int argc, char **argv ) {
    string group_url = "https://www.umei.cc/meinvtupian/xingganmeinv/261728.htm";
    PicGroup picGroup = get_group_name_urls(group_url);
    cout<<picGroup.name<<endl;

    // create thread pool with 4 worker threads
    int max_thread_number = 12;
    ThreadPool pool(max_thread_number);

    // enqueue and store future
    vector<future<bool>> results;
    for(int i=0;i!=picGroup.urls.size();i++)
    {
        cout<<picGroup.urls[i]<<endl;
        results.push_back(pool.enqueue(&download_pic, picGroup.urls[i], save_folder+"/"+picGroup.name));
    }

    // get result from future
//    for(int i=0;i!=results.size();i++)
//    {
//        cout<<results[i].get()<<endl;
//    }

    return 0;
}

