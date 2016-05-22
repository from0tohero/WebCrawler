#include <regex>
#include <string>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>

#include "tinyxml2.h"
#include "webcurl.h"
#include "crawler.h"
#include "url.h"

namespace webcrawler
{
    Crawler::Crawler(int numThreads){
        this->numThreads = numThreads;
    }

    void Crawler::start(std::string& startURL){
        crawl(startURL);
    }

    std::vector<std::string> Crawler::extractLinks(tinyxml2::XMLElement* element,std::vector<std::string>& foundLinks,std::string& baseURL){
        if(std::string(element->Value()) == "a"){
            const char* foundLink = element->Attribute("href");
            if(foundLink != nullptr){
                std::cout << foundLink << std::endl;
                URL url(std::string(foundLink));
                foundLinks.push_back(url.toString());
            }
        }
        for(tinyxml2::XMLElement* e = element->FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
        {
            extractLinks(e,foundLinks);
        }
        return foundLinks;
    }

    void Crawler::crawl(std::string& url){
        std::string pageContent;
        try{
            pageContent = WebCurl::getPage(url);
        }
        catch(std::runtime_error err){
            std::cout << "AN ERROR OCCURED: " << err.what() << std::endl;
            return;//change this is the future
        }
        std::cout << pageContent << std::endl;
        tinyxml2::XMLDocument doc;
        doc.Parse(pageContent.c_str());
        if(doc.ErrorID() != 0){
            std::cout << "BAD XML" << std::endl;
            std::cout << doc.ErrorID() << std::endl;
            return;
        }
        tinyxml2::XMLElement* html = doc.FirstChildElement("html");
        if (html == nullptr) return;// XML_ERROR_PARSING_ELEMENT;
        tinyxml2::XMLElement* body = html->FirstChildElement("body");
        if (body == nullptr) return;// XML_ERROR_PARSING_ELEMENT;
        std::vector<std::string> links;
        extractLinks(body,links,url);
        for(std::string link : links){
            if(foundURLs.find(link) == foundURLs.end()){
                foundURLs.insert(link);
                urlPool.push(link);
            }
        }
        if(!urlPool.empty()){
            std::string nextURL = urlPool.front();
            urlPool.pop();
            crawl(nextURL);
        }

        // const std::string urlRegexStr = "(http|ftp|https)://([\\w_-]+(?:(?:\\.[\\w_-]+)+))"
        //                                 "([\\w.,@?^=%&:/~+#-]*[\\w@?^=%&/~+#-])?";
        // const std::regex urlRegex(urlRegexStr.c_str());
        // std::smatch sm;
        // std::string::const_iterator searchStart( pageContent.cbegin() );
        // while (std::regex_search(searchStart, pageContent.cend() ,sm, urlRegex))
        // {
        //     std::cout << sm[0] << std::endl;
        //     if(foundURLs.find(sm[0]) == foundURLs.end()){
        //         foundURLs.insert(sm[0]);
        //         urlPool.push(sm[0]);
        //     }
        //     searchStart += sm.position() + sm.length();
        // }
        // // std::this_thread::sleep_for(std::chrono::seconds(1));
        // if(!urlPool.empty()){
        //     std::string nextURL = urlPool.front();
        //     urlPool.pop();
        //     crawl(nextURL);
        // }
    }
}