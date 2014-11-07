#pragma once

#include "ofMain.h"
#ifdef TARGET_OF_IPHONE
    #include "ofxiOS.h"
    #include "ofxiOSExtras.h"
    #include "ofxRetinaTrueTypeFont.h"
#endif
#include "framerThread.h"
#include "broadcastThread.h"
#ifndef TARGET_OF_IPHONE
#include <ncurses.h>
#endif
#include <curl/curl.h>
#include "json/json.h"
#include "ofxXmlSettings.h"

#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

using Poco::DigestEngine;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::StreamCopier;

#define __IP__ "226.1.1.2"
#define __PORT__ 20000
#define __USER__ ""
#define __PASS__ ""

#ifdef TARGET_OF_IPHONE
class ofApp : public ofxiOSApp {
#else
	class ofApp : public ofBaseApp{
#endif
	
public:
	void setup();
	void update();
	void draw();
	void exit();
     void drawWaitForConfig(string message);
    
    static int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
    {
        int result = 0;
        if (buffer != NULL)
        {
            buffer->append(data, size * nmemb);
            result = size * nmemb;
        }
        return result;
    };
    
    string curlConnect(string _url, string _post);
    
    
    /* Threads */
    framerThread	FRM;
    broadcastThread	BRD;
    //syncThread		SNC;
    
    /* XML Settings */
    ofxXmlSettings XML;
    
    /* Settings */
    string sessionid, device, multicastip, user, pass, url, serverip, broadcastip, apiurl;
    int port, framerate, preroll;
        
    /* Json Stuff */
    Json::Reader getdata;
    Json::Value returnval;

    /* Network Stuff */
    ofxUDPManager udpConnection;

    /* Font */
#ifdef TARGET_OF_IPHONE
        ofxRetinaTrueTypeFont cinetype_1, cinetype_2;
#else
        ofTrueTypeFont cinetype_1, cinetype_2;

#endif
    
};

