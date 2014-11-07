#ifndef _BROADCAST_THREAD
#define _BROADCAST_THREAD

#include "ofxNetwork.h"
#include <net/if.h>


class broadcastThread : public ofThread{

	public:

		ofxUDPManager udpConnection;		
		string sessionid, apiurl, status, framerate, preroll, channel_collapsed;

		//--------------------------
		broadcastThread(){
		}
		
		//--------------------------------------------------------------------------------
		char* GetInterfaceBroadcastAddr(const char* interface)
		{
			int fd;
			struct ifreq ifr;
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			ifr.ifr_addr.sa_family = AF_INET;
			strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
			ioctl(fd, SIOCGIFBRDADDR, &ifr);
			close(fd);
			return(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr));
		}
		
		string state() {
/*			string _status;
			if( lock() ){
				_status = status;
				unlock();
			}*/
			return status;
		}		
		
		void start(int _port, string _multicastip, string _sessionid, string _apiurl, int _framerate, int _preroll, string _channel_collapsed){

			/* Create Broadcast Interface */
//			udpConnection.Create();
//			udpConnection.Connect(GetInterfaceBroadcastAddr(_device.c_str()),_port+1);
//			udpConnection.SetNonBlocking(false);
//			udpConnection.SetEnableBroadcast(true);
            
            udpConnection.Create();
			udpConnection.ConnectMcast((char*)_multicastip.c_str(), _port+1);
			udpConnection.SetNonBlocking(false);
			udpConnection.SetEnableBroadcast(true);
            udpConnection.SetReuseAddress(true);

			/* Clone Values */
			framerate = ofToString(_framerate);
			sessionid = _sessionid;
			apiurl = _apiurl;
			preroll = ofToString(_preroll);
			channel_collapsed = _channel_collapsed;
            startThread();   // blocking, verbose
        }

        void stop(){
            stopThread();
			waitForThread();
			udpConnection.Close();
			std::cout << "BRD Stopped\n";		
        }

		void threadedFunction(){
			string send = apiurl+"|"+sessionid+"|"+framerate+"|"+preroll+"|"+channel_collapsed;
			int count = 0;
			while (isThreadRunning()!= 0) {
				if (lock()) {
					status = "Broadcast Started: " + send + "(" + ofToString(count) + ")";		
					unlock();
				}
				udpConnection.Send(send.c_str(),send.size());
			    ofSleepMillis(1000);
				count++;
			}
//			stop();
		}


};

#endif













































