#ifndef _SYNC_THREAD
#define _SYNC_THREAD

#include "ofMain.h"
#include "ofxNetwork.h"
#include <net/if.h>

class syncThread : public ofThread{

	public:

		ofxUDPManager udpConnection;		


		//--------------------------
		syncThread(){
		}
		
		void start(int _port, string _multicastip){

			
			udpConnection.Create();
			udpConnection.ConnectMcast((char*)_multicastip.c_str(), _port+2);
			udpConnection.SetNonBlocking(true);
			udpConnection.SetEnableBroadcast(true);
            udpConnection.SetReuseAddress(true);            
			
			
			
            startThread();   // blocking, verbose
        }

		void stop(){
			stopThread();
			waitForThread();
			udpConnection.Close();
			std::cout << "SNC Stopped\n";		
		}

		void threadedFunction(){

			while (isThreadRunning()!= 0) {
				string send = ofToString(ofGetElapsedTimeMillis());
				udpConnection.Send(send.c_str(),send.size());
			    ofSleepMillis(5000);
			}

		}


};

#endif













































