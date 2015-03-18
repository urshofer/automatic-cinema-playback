#ifndef _THREADED_SOUND
#define _THREADED_SOUND
#define MAX_SOUND 5

#include "ofMain.h"

class threadedSoundplayer : public ofThread{

private:

	ofSoundPlayer  		sound[MAX_SOUND];			// 10 simultanoues sound layers are enough
	int					currentSound;	
	int					runningSound;
	bool 				enabled;
    bool                forceStart;
    string              appendend;
    
public:

   //--------------------------
   bool start(bool _enabled){
		enabled = _enabled;
		if (!enabled) {
			return false;
		}
		currentSound = 0;
		runningSound = -1;
        appendend = "";
        forceStart = false;
		for ( unsigned int _index = 0; _index < MAX_SOUND; _index++ )  {
			sound[_index].setVolume(1.0f);
			sound[_index].setMultiPlay(false);
		}
		startThread();
		return true;
   }
   
   // Load: Add to Playlist and play immediately
   
   void loadSound(string movieFile) {
       if (!enabled) {
           return false;
       }
       if (lock()) {
           appendend = movieFile;
           forceStart = true;
           unlock();
       }
   }
   
   // Append: Add to playlist
   
   void appendSound(string movieFile) {
       if (!enabled) {
           return false;
       }
        if (lock()) {
            appendend = movieFile;
            forceStart = false;
            unlock();
        }
        ofLogError("SoundThread") << "appendSound";
   }

   // Trigger: Start Playback
   
   void triggerSound(string movieFile) {
       if (!enabled) {
           return false;
       }
       if (lock()) {
           forceStart = true;
           unlock();
       }
   }

   //--------------------------
   bool stop(){
   	stopThread();
	std::cout  << "TO Stop\n";
   	return true;
   }

   //--------------------------
	void threadedFunction(){
		while (isThreadRunning()) {
            if (lock()) {
                if (appendend != "") {
                    currentSound = currentSound<MAX_SOUND-1?currentSound+1:0;
                    ofLogError("SoundThread") << "Loading into " << currentSound;
                    sound[currentSound].load(appendend, true);
                    appendend = "";
                }
                
                ofSoundUpdate();
                if (forceStart) {
                    if (sound[currentSound].isLoaded()) {
                        sound[currentSound].play();
                    }
                    forceStart = false;
                    ofLogError("SoundThread") << "Triggered";
                }
                unlock();
            }
			ofSleepMillis(25);
		}
	}	

};

#endif
