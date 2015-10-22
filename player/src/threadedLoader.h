#ifndef _THREADED_LOADER
#define _THREADED_LOADER

#include "ofMain.h"
#include "string.h"


#include "json/json.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

using Poco::DigestEngine;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::StreamCopier;

class threadedLoader : public ofThread{

private:

	string			apiURL, apiKEY, fileMissing;
	ofMutex			mutex;	
    bool            isDownloading, triggerDownload;

public:

    threadedLoader() {
    }

   bool start(string apiurl, string apikey){
   	apiURL = apiurl;
   	apiKEY = apikey;
    isDownloading = false;
    triggerDownload = false;
    fileMissing = "";
    startThread();
    return true;
   }
   
   void restart() {
   	startThread();   	
   }

   bool stop(){
   	stopThread();
//	waitForThread();
//	progress = true;
	std::cout  << "TO Stop\n";
   	return true;
   }

    string downloadFile() {
        ofScopedLock lock(mutex);
        return fileMissing;
    }
    
    bool active() {
        ofScopedLock lock(mutex);
        return isDownloading;
    }
    
    void startDownload() {
        if (lock()) {
            if (!isDownloading) {
                triggerDownload = true;
				ofLogVerbose() << "- File missing. Restart Download thread.\n";

            }
            else {
				ofLogVerbose() << "- Already running.\n";
            }
            unlock();
        }
    }

    void doDownload() {
        // Get List of Files from Server
        static string checksum = "";
        isDownloading = true;
        triggerDownload = false;
		ofLogVerbose() << "Start Download";
		ofDirectory DIR;
		#ifdef TARGET_OF_IPHONE
		        string localDir = ofxiOSGetDocumentsDirectory() + "movies/";
		#else
		        string localDir = ofToDataPath("movies/", true);
		#endif
        char lastChar = *localDir.rbegin();
        if (lastChar != '/') localDir += "/";
        
		ofLogVerbose() << "Data Path: " << localDir << endl;

        if (!DIR.doesDirectoryExist(localDir)) {
            if (DIR.createDirectory(localDir)) {
				ofLogVerbose() << "Creating local dir" << endl;
            }
            else {
				ofLogError() << "Could not create local dir" << endl;
                ofExit();
            }
        }
        int nImages = DIR.listDir(localDir);
		
        Json::Reader 	reader;
        Json::Value 	root;
        ofHttpResponse resp = ofLoadURL(apiURL + "/List/" + apiKEY);
        
        MD5Engine md5;
        md5.update(resp.data);
        string _checksum = DigestEngine::digestToHex(md5.digest());
        if (_checksum == checksum) {
            isDownloading = false;
            std::cout << "File list is the same" << endl;
            return;
        }
        else {
            checksum = _checksum;
        }
        

        bool parsingSuccessful = reader.parse(resp.data, root );
        if ( !parsingSuccessful ) {
                ofLogError() << "Failed to parse JSON\n" << reader.getFormatedErrorMessages() << " URL: " << apiURL << "/List/" << apiKEY;
            stop();
        }
        
        ofLogVerbose() << "Remote: " << root.size() << " Files. Local: " << nImages << " Files...";
        
        if (root.isArray() == 1) {
            /* Loop thru all elements */
            for ( unsigned int index = 0; index < root.size(); ++index )  {
                
                /* Exit Loop if the thread has been stopped... */
                if (!isThreadRunning()) {
                    break;
                }

                string localPath = localDir + root[index]["f"].asString();

                bool _missing = true;
                for(int i = 0; i < nImages; i++){
                    if (DIR.getPath(i)==localPath) {
                        _missing = false;
//                        ofLogVerbose() << "OK " << DIR.getPath(i) << " vs " << localPath << endl;
                    }
                }
                
                if (_missing)
                {
                    if (lock()) {
                        fileMissing = root[index]["f"].asString();
                        unlock();
                    }
                    ofLogVerbose() << "MISSING " << "movies/" << fileMissing << endl;
                    ofSaveURLTo(apiURL + "/Download/" + apiKEY + "/" + root[index]["f"].asString(), localPath);
                    ofSleepMillis(100);
                    std::cout << " Downloading " << ofToString(index) << " of " << ofToString(root.size()) << endl;
//                    std::cout << "Downloading " << apiURL << "/Download/" << apiKEY + "/" << root[index]["f"].asString() << " into: " << localPath;
                    if (lock()) {
                        fileMissing = "";
                        unlock();
                    }
                }
            }
        }
        isDownloading = false;
    }


   //--------------------------
   void threadedFunction(){
       int timeout = ofGetElapsedTimeMillis();
       while (isThreadRunning()) {
           if (!isDownloading && (triggerDownload || ofGetElapsedTimeMillis() - timeout > 30000)) {
               doDownload();
               timeout = ofGetElapsedTimeMillis();
           }
           ofSleepMillis(25);
       }
   }


};

#endif
