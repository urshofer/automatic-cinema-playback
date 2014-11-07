#pragma once

#include "ofMain.h"
#include "framerThread.h"
#include "broadcastThread.h"
#ifndef TARGET_OF_IPHONE
    #include <ncurses.h>
#endif
#include <curl/curl.h>
#include "json/json.h"
#include "ofxXmlSettings.h"
#include <vector>
#include "sys/times.h"
#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

using Poco::DigestEngine;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::StreamCopier;


#if TARGET_OS_MAC
// OS code
#else
#include "sys/vtimes.h"
#endif

#include <net/if.h>
//#include "syncThread.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define __IP__ "226.1.1.2"
#define __PORT__ 20000
#define __USER__ ""
#define __PASS__ ""

class testApp {
	public:
		int start();
        void exit();

    int handler(int sig);
    void init();
    double getCpuValue();
    int getMemValue();
    int parseLine(char* line);

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
    void cprint(WINDOW *local_win, string _out);
    void cprintXY(WINDOW *local_win, string _out, int x, int y, int color);
    void cprintXY(WINDOW *local_win, string _out, int x, int y);
    void clineXY(WINDOW *local_win, int sX, int sY, int w);
    WINDOW *init_curses();
    void draw_boxes(WINDOW *local_win);
    void end_curses(WINDOW *local_win);
    char* GetInterfaceAddr(const char* interface);

    
    
    /* Threads */
    framerThread	FRM;
    broadcastThread	BRD;
    //syncThread		SNC;
    
    /* XML Settings */
    ofxXmlSettings XML;
    
    /* Settings */
    string sessionid, device, multicastip, user, pass, url, serverip, broadcastip, apiurl;
    int port, framerate, preroll;
    
    /* Network Stuff */
    ofxUDPManager udpConnection;
    
    /* Json Stuff */
    Json::Reader getdata;
    Json::Value returnval;
    
    
    /* Log Stuff */
    vector<string> logLines;
    
    /* Curses */
	WINDOW *my_win, *list_win, *thread_win, *cpu_win;
    
};
