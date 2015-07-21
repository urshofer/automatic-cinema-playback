#ifndef _THREADED_SOUND
#define _THREADED_SOUND
#define MAX_SOUND 5

#include "ofMain.h"

#ifdef TARGET_OSX
#include "ofxAudioUnit.h"
#endif

class threadedSoundplayer : public ofThread{

private:

    #ifdef TARGET_OSX
    ofxAudioUnitFilePlayer  sound[MAX_SOUND];
    ofxAudioUnitMixer       mixer;
    ofxAudioUnit            fx;
	ofxAudioUnitOutput      output;
    
    /* Settings */
    struct _settings {
        float BandpassBandwidthMin;
        float BandpassBandwidthMax;
        float BandpassCenterFrequencyMin;
        float BandpassCenterFrequencyMax;
        float VolumeFactor;
    };
    _settings               settings;
    
    #else
	ofSoundPlayer           sound[MAX_SOUND];			// 10 simultanoues sound layers are enough
    #endif

    
    
    
	int					currentSound;	
	int					runningSound;
	bool 				enabled;
    bool                forceStart;
    string              appendend;
    
public:
    
    
    void setMatrixEffect(float _x, float _y, float _min_x = -1, float _max_x = 1, float _min_y = -1, float _max_y = 1) {
        #ifdef TARGET_OSX
        _x = ofMap(_x, _min_x, _max_x, -1, 1, true);
        _y = ofMap(_y, _min_y, _max_y, -1, 1, true);
        float _xcenterized = 1 + abs(_x)*-1;
        float _ycenterized = 1 + abs(_y)*-1;
        float _centerized = _xcenterized * _ycenterized;
        fx.setParameter(kBandpassParam_Bandwidth, kAudioUnitScope_Global,       ofMap(_centerized, 0, 1, settings.BandpassBandwidthMin, settings.BandpassBandwidthMax, true));
        fx.setParameter(kBandpassParam_CenterFrequency, kAudioUnitScope_Global, ofMap(_centerized, 0, 1, settings.BandpassCenterFrequencyMin, settings.BandpassCenterFrequencyMax, true));
        float _f = ofMap(settings.VolumeFactor, 0, 1, 0, 1, true);
        mixer.setOutputVolume(_centerized*_f + (1-_f));
        #endif
    }

   //--------------------------
   bool start(bool _enabled){

#ifdef TARGET_OSX
       settings.BandpassBandwidthMax = 12000;
       settings.BandpassBandwidthMin = 836;
       settings.BandpassCenterFrequencyMax = 638;
       settings.BandpassCenterFrequencyMin = 300;
       settings.VolumeFactor = 0.5;
#endif
       
        enabled = _enabled;
		if (!enabled) {
			return false;
		}
		currentSound = 0;
		runningSound = -1;
        appendend = "";
        forceStart = false;
		for ( unsigned int _index = 0; _index < MAX_SOUND; _index++ )  {
            #ifdef TARGET_OSX
                mixer.setInputBusCount(_index);
                sound[_index].connectTo(mixer, _index);
                mixer.setInputVolume(1.0f, _index);
            #else
                sound[_index].setVolume(1.0f);
                sound[_index].setMultiPlay(false);
            #endif
		}

        #ifdef TARGET_OSX
        fx.setup('aufx', 'bpas', 'appl');
        mixer.setOutputVolume(1.0f);
        //fx.showUI();
        mixer.connectTo(fx);
        fx.connectTo(output);
        setMatrixEffect(-2,-2);
        output.start();
        #endif
       
       
		startThread();
		return true;
   }
   
   // Load: Add to Playlist and play immediately
   
   void loadSound(string movieFile) {
       if (!enabled) {
           return;
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
           return;
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
           return;
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
                    
                    #ifdef TARGET_OSX
                    sound[currentSound].setFile(ofToDataPath(appendend));
                    sound[currentSound].prime();
                    #else
                    sound[currentSound].load(appendend, true);
                    #endif
                    
                    
                    appendend = "";
                }
                #ifndef TARGET_OSX
                    ofSoundUpdate();
                #endif

                if (forceStart) {
                    #ifdef TARGET_OSX
                        sound[currentSound].play();
                    #else
                        if (sound[currentSound].isLoaded()) {
                            sound[currentSound].play();
                        }
                    #endif
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
