#ifndef _THREADED_SOUND
#define _THREADED_SOUND

#include "ofMain.h"

class threadedSoundplayer : public ofThread{

private:

	ofSoundPlayer  		sound[10];			// 10 simultanoues sound layers are enough
	int					currentSound;	
	int					runningSound;
	bool 				enabled;

public:

   //--------------------------
   bool start(bool _enabled){
		enabled = _enabled;
		if (!enabled) {
			return false;
		}
		currentSound = 0;
		runningSound = -1;
		for ( unsigned int _index = 0; _index < 10; _index++ )  {
			sound[_index].setVolume(0.3f);		
			sound[_index].setMultiPlay(false);
		}
		startThread();
		return true;
   }
   
   // Load: Add to Playlist and play immediately
   
   void loadSound(string movieFile) {
       ofScopedLock lock(mutex);
	   currentSound = currentSound<10?currentSound+1:0;
	   sound[currentSound].loadSound(movieFile, true);
	   triggerSound(movieFile);
   }
   
   // Append: Add to playlist
   
   void appendSound(string movieFile) {
		ofScopedLock lock(mutex);
		runningSound = currentSound;
		currentSound = currentSound<10?currentSound+1:0;   	
		sound[currentSound].loadSound(movieFile, true);
   }

   // Trigger: Start Playback
   
   void triggerSound(string movieFile) {
		ofScopedLock lock(mutex);
		sound[currentSound].play();	
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
			ofSoundUpdate();
			ofSleepMillis(25);
		}
	}	

};

#endif
