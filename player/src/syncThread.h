#ifndef _SYNC_THREAD
#define _SYNC_THREAD

#include "ofMain.h"
#include "ofxNetwork.h"
#include "json/json.h"

class syncThread : public ofThread{

	public:

		Json::Reader reader;
		Json::Value root;
		ofxUDPManager udpConnection;	
		threadedLoader          *TO;
		ofxGaplessVideoPlayer	*MO;
		threadedSoundplayer     *SN;
        subtitleThread          *SU;
		vector<string> channel;
		bool hasVideo, hasAudio, hasText;
		
		//--------------------------
		syncThread(){
		}
		
		
		//--------------------------------------------------------------
		string parseJSON(string s) {
			string result, movieFile;

            
			bool parsingSuccessful = reader.parse( s, root );
			if ( !parsingSuccessful ) {
		//		std::cout  << "Failed to parse JSON\n" << reader.getFormatedErrorMessages();
				return result;
			}

//		std::cout  << "Parsed: " << s << "Type:" << root.isArray() << endl;

			// Parsing Array JSON Strings (e.g. triggered movies)

			if (root.isArray() == 1) {
				for ( unsigned int index = 0; index < root.size(); ++index )  {


					// Todo: Multiple Channels available
					// "Video","Bild","Musik","Sprache","Text"

					if (root[index]["f"].isString() && (std::find(channel.begin(), channel.end(), root[index]["c"].asString())!=channel.end())) {
						// File Check

                        
                        
#ifdef TARGET_OF_IPHONE
                        movieFile = ofxiOSGetDocumentsDirectory() + "movies/" + root[index]["f"].asString();
#else
						movieFile =  "movies/" + root[index]["f"].asString();
#endif
                        
                        
						bool available = true;
#ifdef TARGET_OF_IPHONE
						string fileNameInOF = movieFile;
#else
						string fileNameInOF = ofToDataPath(movieFile);
#endif
                        
						if (access(fileNameInOF.c_str(), R_OK) == -1 || TO->downloadFile() == root[index]["f"].asString())
						{
							// Try to download, it might be added to the list later...
                          
							available = false;
                            TO->startDownload();
						}


						if (root[index]["t"].asString()=="text" && available && hasText && (root[index]["m"].asString() == "t" || root[index]["m"].asString() == "p"))  {
                            ofLogVerbose() << "+ NET: loadSubtitles";
                            
							SU->loadSubtitles(fileNameInOF);
						}

						if ((root[index]["t"].asString()=="audio") && hasAudio)  {
							if (available) {
								// Trigger or append
								if (root[index]["m"].asString() == "t") {
									SN->loadSound(movieFile);
									result = "Sound: load/trigger";
								}
								if (root[index]["m"].asString() == "a") {
									SN->appendSound(movieFile);
									result = "Sound: load";
								}
								if (root[index]["m"].asString() == "p") {
									SN->triggerSound(movieFile);
									result = "Sound: trigger";								
								}
							}
							else {
								result = "Sound: missing";
							}
						}

						if (root[index]["t"].asString()=="video" && hasVideo)  {
							// If File exists...
							if (available) {
								// Effect Switchy
								bool _in = root[index]["fxin"].asString()=="FadeIn"?true:false;
								bool _out = root[index]["fxout"].asString()=="FadeOut"?true:false;
                                
//                                cout << "Fx: " << root[index]["fxin"].asString() << "/" << root[index]["fxout"].asString() << "\n";
//                                cout << "Bo: " << _in << "/" << _out << "\n";

                                
								// Trigger or append
								if (root[index]["m"].asString() == "t") {
									MO->loadMovie(movieFile, _in, _out);
									ofLogVerbose() << "+ NET: loadMovie";
								}
								if (root[index]["m"].asString() == "a") {
									MO->appendMovie(movieFile, _in, _out);
									ofLogVerbose() << "+ NET: appendMovie";
								}
								if (root[index]["m"].asString() == "p") {
									MO->triggerMovie(movieFile);
									ofLogVerbose() << "+ NET: triggerMovie";
								}

							}
							// Error Crawl
							else {
								result = "Movie: missing";
                                
								#ifdef TARGET_OF_IPHONE
									MO->loadMovie("static/not-found.m4v", false, false);
								#else
									MO->loadMovie("static/not-found.mov", false, false);
								#endif
							}
			
						}
					}
				}
			}
			return result;
		}

    
        void configNet(string multicastIp, int _port) {
            if (udpConnection.HasSocket()) {
                ofLogError() << "+ NET: closing Port";
                udpConnection.Close();
            }
            
            if (isThreadRunning()) {
                ofLogError() << "+ NET: stopping Thread";
                stopThread();
                waitForThread();
                ofLogError() << "+ NET: stopped Thread";
            }
            char * mIp = new char[multicastIp.size() + 1];
            copy(multicastIp.begin(), multicastIp.end(), mIp);
            mIp[multicastIp.size()] = '\0';

            ofLogError() << "+ NET: Settig up net " << multicastIp << ":" << _port;
            udpConnection.Create();
            udpConnection.BindMcast(mIp, _port);
            udpConnection.SetNonBlocking(false);
            udpConnection.SetReuseAddress(true);
            if (udpConnection.HasSocket()) {
                startThread();   // blocking, verbose
            }
        }
	
		
		void start(string multicastIp, int _port, vector<string> _channel, threadedLoader &_TO, ofxGaplessVideoPlayer &_MO, threadedSoundplayer	&_SN, subtitleThread &_SU, bool _hasVideo, bool _hasAudio, bool _hasText){
            
            char * mIp = new char[multicastIp.size() + 1];
            copy(multicastIp.begin(), multicastIp.end(), mIp);
            mIp[multicastIp.size()] = '\0';
            
			MO = &_MO;
			TO = &_TO;
			SN = &_SN;
            SU = &_SU;
			hasVideo = _hasVideo;
			hasAudio = _hasAudio;
			hasText  = _hasText;
			channel = _channel;
		   	udpConnection.Create();
		   	udpConnection.BindMcast(mIp, _port);
		   	udpConnection.SetNonBlocking(false);
            udpConnection.SetReuseAddress(true);
            if (udpConnection.HasSocket()) {
                startThread();   // blocking, verbose
            }
        }
    
    void updateConfig(vector<string> _channel, bool _hasVideo, bool _hasAudio, bool _hasText) {
		if (lock()) {
			hasVideo = _hasVideo;
			hasAudio = _hasAudio;
			hasText  = _hasText;
			channel = _channel;
			unlock(); 
		}
    }

        void stop(){
            if (udpConnection.HasSocket()) {
                udpConnection.Close();
            }
            stopThread();
        }

		void threadedFunction(){
            ofLogError() << "+ NET: Starting Thread";
			while (isThreadRunning()) {
				char udpMessage[1000];
				udpConnection.Receive(udpMessage,1000);
				string message=udpMessage;
				if(message.length() > 5){
					parseJSON(message);
				}
			}
		}


};

#endif