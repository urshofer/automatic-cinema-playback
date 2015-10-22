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
    ofxAudioUnit            compressor;
	ofxAudioUnitOutput      output;
    
    /* Settings */
    struct _settings {
        float BandpassBandwidthMin;
        float BandpassBandwidthMax;
        float BandpassCenterFrequencyMin;
        float BandpassCenterFrequencyMax;
        float VolumeFactor;
        float MinVolume;
    };
    _settings               settings;
    
    #else
	ofSoundPlayer           sound[MAX_SOUND];			// 10 simultanoues sound layers are enough
    #endif

    
    
    
	int					currentSound;	
	int					runningSound;
    int                 preroll;
	bool 				enabled;
    bool                forceStart;
    string              appendend;
    
public:
    
    
    void setMatrixEffect(float _centerized) {
        std::cout << "[Matrix Effect] Set to " << _centerized << endl;
        #ifdef TARGET_OSX

        fx.setParameter(kBandpassParam_Bandwidth, kAudioUnitScope_Global,       ofMap(_centerized, 0, 1, settings.BandpassBandwidthMin, settings.BandpassBandwidthMax, true));
        fx.setParameter(kBandpassParam_CenterFrequency, kAudioUnitScope_Global, ofMap(_centerized, 0, 1, settings.BandpassCenterFrequencyMin, settings.BandpassCenterFrequencyMax, true));
        float _f = ofMap(settings.VolumeFactor, 0, 1, 0, 1, true);
        mixer.setOutputVolume(_centerized*_f + (settings.MinVolume * (1 - _centerized)));
        #else
        for ( unsigned int _index = 0; _index < MAX_SOUND; _index++ )  {
            sound[_index].setVolume(_centerized);
        }
        #endif
    }

   //--------------------------
   bool start(bool _enabled, float _volume_factor, float _min_volume, int _preroll){

#ifdef TARGET_OSX
       settings.BandpassBandwidthMax = 12000;
       settings.BandpassBandwidthMin = 836;
       settings.BandpassCenterFrequencyMax = 638;
       settings.BandpassCenterFrequencyMin = 300;
       settings.VolumeFactor = _volume_factor;
       settings.MinVolume    = _min_volume;
#endif
       
        enabled = _enabled;
		if (!enabled) {
			return false;
		}
		currentSound = 0;
		runningSound = -1;
        appendend = "";
        forceStart = false;
        preroll = _preroll;
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
        compressor.setup('aufx', 'dcmp', 'appl');
       
       /*
        # # # 7 Global Scope Parameters:
        Parameter ID:0
        Name: compression threshold
        Parameter Type: Decibels
        Values: Minimum = -100.000000, Default = -15.000000, Maximum = 20.000000
        Flags: Readable, Writable
        -parameter PASS
        
        Parameter ID:1
        Name: headroom
        Parameter Type: Decibels
        Values: Minimum = 0.100000, Default = 5.600000, Maximum = 40.000000
        Flags: Readable, Writable
        -parameter PASS
        
        Parameter ID:2
        Name: expansion ratio
        Parameter Type: Rate
        Values: Minimum = 1.000000, Default = 1.000000, Maximum = 50.000000
        Flags: Display Logarithmic, Readable, Writable
        -parameter PASS
        
        Parameter ID:3
        Name: expansion threshold
        Parameter Type: Decibels
        Values: Minimum = -120.000000, Default = -100.000000, Maximum = 0.000000
        Flags: Readable, Writable
        -parameter PASS
        
        Parameter ID:4
        Name: attack time
        Parameter Type: Seconds
        Values: Minimum = 0.001000, Default = 0.050000, Maximum = 0.300000
        Flags: Display Logarithmic, Readable, Writable
        -parameter PASS
        
        Parameter ID:5
        Name: release time
        Parameter Type: Seconds
        Values: Minimum = 0.010000, Default = 0.030000, Maximum = 3.000000
        Flags: Display Logarithmic, Readable, Writable
        -parameter PASS
        
        Parameter ID:6
        Name: master gain
        Parameter Type: Decibels
        Values: Minimum = -40.000000, Default = 0.000000, Maximum = 40.000000
        Flags: Readable, Writable
        -parameter PASS
        
*/

       compressor.setParameter(kDynamicsProcessorParam_Threshold,           kAudioUnitScope_Global,       -55.0, true);
       compressor.setParameter(kDynamicsProcessorParam_HeadRoom,            kAudioUnitScope_Global,       40.0, true);
       compressor.setParameter(kDynamicsProcessorParam_ExpansionRatio,      kAudioUnitScope_Global,       1.0, true);
       compressor.setParameter(kDynamicsProcessorParam_ExpansionThreshold,  kAudioUnitScope_Global,       -100.0, true);
       compressor.setParameter(kDynamicsProcessorParam_AttackTime,          kAudioUnitScope_Global,       0.05, true);
       compressor.setParameter(kDynamicsProcessorParam_ReleaseTime,         kAudioUnitScope_Global,       0.03, true);
       compressor.setParameter(kDynamicsProcessorParam_MasterGain,          kAudioUnitScope_Global,       10.0, true);
       // compressor.showUI();
       
       

        mixer.setOutputVolume(1.0f);
        //fx.showUI();
        mixer.connectTo(fx);
        fx.connectTo(compressor);
        compressor.connectTo(output);
        setMatrixEffect(0);
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
           if (movieFile != appendend) {
               appendend = movieFile;
           }
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
        
        static bool ramp = false;
        
		while (isThreadRunning()) {
            if (lock()) {
                if (appendend != "") {
                    currentSound = currentSound<MAX_SOUND-1?currentSound+1:0;
                    ofLogError("SoundThread") << "Loading into " << currentSound;
                    
                    #ifdef TARGET_OSX
                    sound[currentSound].setFile(ofToDataPath(appendend));
                    sound[currentSound].prime();
                    mixer.setInputVolume(0.0f, currentSound);
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
                        ramp = true;
                    #else
                        if (sound[currentSound].isLoaded()) {
                            sound[currentSound].play();
                        }
                    #endif
                    forceStart = false;
                    ofLogError("SoundThread") << "Triggered";
                }

                #ifdef TARGET_OSX
                if (ramp) {
                    if ((int)sound[currentSound].getCurrentTimestamp().mSampleTime < preroll) {
                        mixer.setInputVolume((float)(1 / preroll * (int)sound[currentSound].getCurrentTimestamp().mSampleTime), currentSound);
                    }
                    else {
                        ramp = false;
                        mixer.setInputVolume(1.0f, currentSound);
                    }
                }
                #endif
                
                unlock();
            }
			ofSleepMillis(25);
		}
	}	

};

#endif
