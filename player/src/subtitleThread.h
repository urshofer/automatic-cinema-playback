#ifndef _SUB_THREAD
#define _SUB_THREAD

#include "ofMain.h"
#include "ofxNetwork.h"
#include "json/json.h"

class subtitleThread : public ofThread{

    private:

    vector<string> subtitles;
	unsigned int    sub_lines, sub_tempo;
	int     sub_start_time;
	bool    hasText;
    string  triggerSubs, text;
    
    
	public:

		
		//--------------------------
		subtitleThread(){
		}
				
		void start(bool _hasText, string spl, string lps){
            sub_lines = ofToInt(lps);
            sub_tempo = ofToInt(spl) * sub_lines;
            hasText         = _hasText;
            triggerSubs     = "";
            startThread();
        }
    
    
    
        //--------------------------------------------------------------
        void loadSubtitles(string absoluteFileName) {
            ofScopedLock lock(mutex);
            if (triggerSubs == "") triggerSubs = absoluteFileName;
            ofLogVerbose() << "+ Trigger Subs";
        }
        
        //--------------------------------------------------------------
        string getSubtitle() {
            ofScopedLock lock(mutex);
            return text;
        }
    
        //--------------------------------------------------------------
        void stop(){
            stopThread();
        }

        //--------------------------------------------------------------
        void threadedFunction() {

            ofLogVerbose()  << "+ Subtitle Thread started\n";
            
            while (isThreadRunning()) {
                if (triggerSubs != "" && hasText) {
                    if (lock()) {
                        ofBuffer b = ofBufferFromFile(triggerSubs, false);
                        subtitles.clear();
                        subtitles = ofSplitString(b.getText(), "\n");
                        sub_start_time = ofGetElapsedTimeMillis();
                        ofLogVerbose() << "+ Text " + ofToString(subtitles.size()) + " Lines / " + ofToString(sub_lines) + " per Screen / " + ofToString(sub_tempo) + " ms\n";
                        ofLogVerbose() << "+ Add Subs";
                        triggerSubs = "";
                        unlock();
                    }
                }
                
                
                /* Subtitle Drawing */
                
                string _text = "";
                if (hasText && subtitles.size()>0) {
                    bool first = true;
                    for (unsigned int i=0; i<(sub_lines>subtitles.size()?subtitles.size():sub_lines); i++)	{
                        if (subtitles[i]!="") {
                            _text.append((first?"":"\n")+subtitles[i]);
                            first = false;
                        }
                    }
                    // Erasing after the subdisplay time the first n lines
                    if (ofGetElapsedTimeMillis()-sub_start_time>sub_tempo) {
                        sub_start_time = ofGetElapsedTimeMillis();
                        if (subtitles.size()>sub_lines) {subtitles.erase (subtitles.begin(),subtitles.begin()+sub_lines);}
                        else {subtitles.clear();}
                    }
                }
                if (lock()) {
                    text = _text;
                    unlock();
                }
                ofSleepMillis(10);
            }
        }
};

#endif