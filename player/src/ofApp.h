#pragma once

#include "ofMain.h"
#ifdef TARGET_OF_IPHONE
    #include "ofxiOS.h"
    #include "ofxiOSExtras.h"
#endif
#include "configThread.h"
#include "threadedLoader.h"
#include "subtitleThread.h"
#include "threadedSoundplayer.h"
#include "ofxGaplessVideoplayer.h"
#include "syncThread.h"
#include "ofxFontStash.h"
#include "ofxUI.h"
#include "ofxOpenNI.h"


#define MIX_AUDIO  .5f
#define MIX_VIDEO  .25f
#define MIX_AUDIO_MINVOL .1f
#define MIX_AUDIO_MAXVOL 1.0f

#ifdef TARGET_OF_IPHONE
	class ofApp : public ofxiOSApp {
#else
	class ofApp : public ofBaseApp{
#endif
    private:

        ofxUICanvas *gui;
        ofImage *bg;
        string                  guifont;
        
        void drawWaitForConfig(string message);
        void loadXMLConfig();
        void drawSyncFiles(string filename);
        void setupGUI(string guifont, bool drawchannels);
        
	public:
		void setup();
		void update();
		void draw();
        void exit();


		#ifdef TARGET_OF_IPHONE
	    void touchDown(ofTouchEventArgs & touch);
		#else
	    void keyPressed  (int key);
        void windowResized(int w, int h);
        void mouseMoved(int x, int y );
		#endif

        ofxGaplessVideoPlayer   MO;
        threadedSoundplayer     SN;
        threadedLoader          TO;
        syncThread              SC;
        configThread            CN;
        subtitleThread          SU;

        
        ofxFontStash            cinetype;
        ofTrueTypeFont          cinetype_1,cinetype_2;
        ofImage                 syncicon,logoicon;
        
        #ifdef TARGET_OSX
            ofxOpenNI               device;
        #endif
        bool                    deviceready;
        float                   _playground[4] = {-1000.0,1000.0,1000,2000};
        bool                    kinekt = false;
        
        float               sound_mix_audio, sound_mix_video, sound_mix_minvolume, sound_mix_maxvolume;
        
        struct _baseconfig {
            vector<string> channel;
            bool hasAudio;
            bool hasVideo;
            bool hasText;
            bool drawLogo;
            bool fullscreen;
            bool verbose;
            int  baseport;
            string multicastIp;
            vector<string> serverconfig;
        };
    
        _baseconfig config;
    
        void guiEvent(ofxUIEventArgs &e);

        int mouseTimer;
        bool cursorflag;

};
