//
// Created by amzing on 2021/5/25.
//

#include "BeautifulSoup.h"

BeautifulSoup::BeautifulSoup() {

}

BeautifulSoup::BeautifulSoup(string text) {
    this->raw_text = text;
    this->analyse();
}

//传入一个标签的开始标签，提取其中的属性
//如<a href="asda", taget="dssada">hello</a>
//需要传入<a href="asda", target="dssada">
//他会提取a的属性href和target
map<string, string> BeautifulSoup::get_tag_attribute(string tag_begin) {
    map<string, string> ans_attribute;
    //首先大致确认一个这是一个符合要求的格式
    int tag_begin_start = tag_begin.find("<");
    if(tag_begin_start!=0)return ans_attribute;
    int tag_begin_end = tag_begin.find(">");
    if(tag_begin_end!=tag_begin.size()-1)return ans_attribute;

    //处理当前标签的属性
    //<+tag_begin+空格+属性+>
    int attribute_start = tag_begin.find(" ")+1;
    if(attribute_start<=2)return ans_attribute;
    int attribute_end =-1;
    //循环处理多个属性
    while((attribute_end = tag_begin.find(" ", attribute_start))>0)
    {
        string attribute = tag_begin.substr(attribute_start, attribute_end-attribute_start);
        int eq_pos = attribute.find("=");
        if(eq_pos<0)return ans_attribute;
        string key = attribute.substr(0, eq_pos);
        string value = attribute.substr(eq_pos+2, attribute.size()-eq_pos-3);
        ans_attribute[key]=value;
        attribute_start = attribute_end+1;
    }
    //处理最后一个属性
    if(attribute_start<tag_begin.size()-1)
    {
        attribute_end = tag_begin.size()-1;
        if(tag_begin[attribute_end-1]=='/')attribute_end--;
        string attribute = tag_begin.substr(attribute_start, attribute_end-attribute_start);
        int eq_pos = attribute.find("=");
        if(eq_pos<0)return ans_attribute;
        string key = attribute.substr(0, eq_pos);
        string value = attribute.substr(eq_pos+2, attribute.size()-eq_pos-3);
        ans_attribute[key]=value;
        attribute_start = attribute_end+1;
    }
    return ans_attribute;
}

BeautifulSoup BeautifulSoup::find(string name, map<string, string> attribute) {
    //这里先找标签的开头，不找标签的闭标签
    //就是说先找<a>，但不找</a>
    //这样现提取标签的名字，还有附加的属性，对比完名字属性正确后，最后再找闭标签，提取当前标签的文字
    //也许会碰到没有闭标签的标签如<meta><link><img>等。
    //此时就把开始标签当作标签的全部，构造一个BeautifulSoup传入这个开始标签，返回就好
    //比如<img src="asdasd">我们只需要把这个标签创建一个BeautifulSoup即可
    BeautifulSoup soup;
    string tag_begin_start_str = "<"+name;
    string tag_begin_end_str = ">";

//    for(auto it=attribute.begin();it!=attribute.end();it++)
//    {
//        cout<<it->first<< " "<< it->second<<endl;
//    }

    //找开始标签，并分析属性是否正确
    int tag_begin_start = -1;
    int tag_begin_end = -1;
    string tag_begin = "";
    bool flag = true;
    while(flag)
    {
        flag = false;
        tag_begin_start++;
        tag_begin_start = this->raw_text.find(tag_begin_start_str, tag_begin_start);
        if(tag_begin_start<0)return soup;
        tag_begin_end = this->raw_text.find(tag_begin_end_str, tag_begin_start);
        if(tag_begin_end<0)return soup;
        tag_begin = this->raw_text.substr(tag_begin_start, tag_begin_end-tag_begin_start+1);

        map<string, string> t_attribute = this->get_tag_attribute(tag_begin);
        for(map<string, string>::iterator it=attribute.begin();it!=attribute.end();it++)
        {
            if(t_attribute[it->first]!=it->second)
            {
                flag = true;
                break;
            }
        }
    }

    //找到一个符合要求的tag后，去找整个tag的位置
    string tag_end_str = "</"+name+">";
    int tag_end = this->raw_text.find(tag_end_str, tag_begin_end);
    //如何找不到闭标签，说明有可能是在处理<img>这种没有闭标签的标签，那么我们就把这个标签做成一个BeautifulSoup返回即可
    if(tag_end<0)
    {
//        cout<<tag_begin<<endl;
        soup = BeautifulSoup(tag_begin);
    }
    else
    {
        string all_tag = this->raw_text.substr(tag_begin_start, tag_end+tag_end_str.size()-tag_begin_start);
//        cout<<all_tag<<endl;
        soup = BeautifulSoup(all_tag);
    }

    return soup;
}

vector<BeautifulSoup> BeautifulSoup::find_all(string name, map<string, string> attribute) {
    //与find的唯一区别是，find只找到第一个符合要求的tag即可，我们是找所有符合要求的tag
    //嗯，加个while循环即可
    vector<BeautifulSoup> all_soup;

    string tag_begin_start_str = "<"+name;
    string tag_begin_end_str = ">";

    int tag_begin_start = 0;
    int tag_begin_end = -1;

    //找开始标签，并分析属性是否正确
    string tag_begin = "";
    bool flag = false;
    while(tag_begin_start<this->text.size())
    {
        tag_begin_start = this->raw_text.find(tag_begin_start_str, tag_begin_start);
        if(tag_begin_start<0)break;
        tag_begin_end = this->raw_text.find(tag_begin_end_str, tag_begin_start);
        if(tag_begin_end<0)break;
        tag_begin = this->raw_text.substr(tag_begin_start, tag_begin_end-tag_begin_start+1);

        flag = true;
        map<string, string> t_attribute = this->get_tag_attribute(tag_begin);
        for(map<string, string>::iterator it=attribute.begin();it!=attribute.end();it++)
        {
            if(t_attribute[it->first]!=it->second)
            {
                flag = false;
                break;
            }
        }
        if(flag)
        {
            //找到一个符合要求的tag后，去找整个tag的位置
            string tag_end_str = "</"+name+">";
            int tag_end = this->raw_text.find(tag_end_str, tag_begin_end);
            //如何找不到闭标签，说明有可能是在处理<img>这种没有闭标签的标签，那么我们就把这个标签做成一个BeautifulSoup返回即可
            if(tag_end<0)
            {
                all_soup.push_back(BeautifulSoup(tag_begin));
            }
            else
            {
                string all_tag = this->raw_text.substr(tag_begin_start, tag_end+tag_end_str.size()-tag_begin_start);
                all_soup.push_back(BeautifulSoup(all_tag));
            }
        }
        tag_begin_start++;
    }
    return all_soup;
}

BeautifulSoup BeautifulSoup::rfind(string name, map<string, string> attribute) {
    //和find类似，只不过找到后不是直接返回，而是继续找，直到找不到了，返回最后一次找到的结果
    BeautifulSoup soup;
    string tag_begin_start_str = "<"+name;
    string tag_begin_end_str = ">";

    //找开始标签，并分析属性是否正确
    int tag_begin_start = -1;
    int tag_begin_end = -1;
    string tag_begin = "";
    bool flag = false;
    while(1)
    {
        flag = true;
        tag_begin_start++;
        tag_begin_start = this->raw_text.find(tag_begin_start_str, tag_begin_start);
        if(tag_begin_start<0)return soup;
        tag_begin_end = this->raw_text.find(tag_begin_end_str, tag_begin_start);
        if(tag_begin_end<0)return soup;
        tag_begin = this->raw_text.substr(tag_begin_start, tag_begin_end-tag_begin_start+1);

        map<string, string> t_attribute = this->get_tag_attribute(tag_begin);
        for(map<string, string>::iterator it=attribute.begin();it!=attribute.end();it++)
        {
            if(t_attribute[it->first]!=it->second)
            {
                flag = false;
                break;
            }
        }
        if(flag)
        {
            //找到一个符合要求的tag后，去找整个tag的位置
            string tag_end_str = "</"+name+">";
            int tag_end = this->raw_text.find(tag_end_str, tag_begin_end);
            //如何找不到闭标签，说明有可能是在处理<img>这种没有闭标签的标签，那么我们就把这个标签做成一个BeautifulSoup返回即可
            if(tag_end<0)
            {
                soup = BeautifulSoup(tag_begin);
            }
            else
            {
                string all_tag = this->raw_text.substr(tag_begin_start, tag_end+tag_end_str.size()-tag_begin_start);
                soup = BeautifulSoup(all_tag);
            }
        }
    }
    return soup;
}

void BeautifulSoup::analyse() {
    //分析出当前标签的name，当前标签的属性，当前标签的内容

    //这里先找标签的开头，不找标签的闭标签
    //就是说先找<a>，但不找</a>
    //这样现提取标签的名字，还有附加的属性，最后再找闭标签，提取当前标签的文字
    //也许会碰到没有闭标签的标签如<meta><link><img>等。
    //此时我们也可以正常解析到标签的name和attribute，解析内容时会因为找不到闭标签而返回

    //找到当前标签
    int current_tag_begin_start=0;
    int current_tag_begin_end=0;
    string tag="<!DOCTYPE html>";
    while(tag=="<!DOCTYPE html>")
    {
        current_tag_begin_start=this->raw_text.find("<", current_tag_begin_end);
        if(current_tag_begin_start<0)return;
        current_tag_begin_end = this->raw_text.find(">", current_tag_begin_start);
        if(current_tag_begin_end<0)return;
        tag = this->raw_text.substr(current_tag_begin_start, current_tag_begin_end-current_tag_begin_start+1);
    }
//    cout<<tag<<endl;
    //分析出当前标签的name，当前标签的属性
    int name_start = 1;
    int name_end = tag.find(" ", name_start);
    if(name_end<0)name_end=tag.size()-1;
    this->name = tag.substr(name_start, name_end-name_start);
//    cout<<name<<endl;

    //处理当前标签的属性
    this->attribute = this->get_tag_attribute(tag);

    //获取当前标签的内容
    string start_tag = "<"+name;
    string end_tag = "</"+name;
    //当前标签的内容有可能有一个\r\n，但也不一定
    //有就是这样
    //<div>
    //	hello
    //</div>
    //没有就是
    //<a href="asd">hello</a>
    int text_start = current_tag_begin_end+1;
    while(this->raw_text[text_start]=='\r'||this->raw_text[text_start]=='\n')text_start++;
    int text_end = this->raw_text.find(end_tag, text_start);
    if(text_end<0)return;
    int middle_tag_start = this->raw_text.find(start_tag, text_start);
    while(middle_tag_start>0&&middle_tag_start<text_end)
    {
        text_end = this->raw_text.find(end_tag, text_end+1);
        middle_tag_start = this->raw_text.find(start_tag, middle_tag_start+1);
    }
    this->text = this->raw_text.substr(text_start, text_end-text_start);
}

