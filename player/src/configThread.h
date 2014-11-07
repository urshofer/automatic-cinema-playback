#ifndef _CONF_THREAD
#define _CONF_THREAD

#include "ofMain.h"
#include "ofxNetwork.h"
#include "json/json.h"

class configThread : public ofThread{
    private:
        ofxUDPManager udpConfigsocket;
        string udpMessage;
        string multicastIp;
        int baseport;
        vector<string> ipSplit;
        bool isConfig, error;
    
    public:


		//--------------------------
		configThread(){
		}
		
		void start(string _ip, int _baseport){
            multicastIp    = _ip;
            baseport       = _baseport;
            startThread(false, false);   // blocking, verbose
            isConfig = false;
            error   = false;
        }
    
        bool isConfigured() {
            return isConfig;
        }

        bool isError() {
            return error;
        }
    
        vector<string> getConfiguration() {
            return ipSplit;
        }
    
        void stop(){
            stopThread();
        }

		void threadedFunction(){
            char * mIp = new char[multicastIp.size() + 1];
            copy(multicastIp.begin(), multicastIp.end(), mIp);
            mIp[multicastIp.size()] = '\0';

            
            //create the listener socket and wait for the multicast broadcast on port 20000
            std::cout  << "+ Waiting for Configuration on Port " << (baseport+1) << "...";
            udpConfigsocket.Create();
            udpConfigsocket.BindMcast(mIp, baseport+1);
            udpConfigsocket.SetNonBlocking(false);
            
            
            char multiCastIPRaw[100];
            if (udpConfigsocket.Receive(multiCastIPRaw,100)>1) {
                std::cout  << "ok" << endl;
                udpConfigsocket.Close();
                if (lock()) {
                    ipSplit = ofSplitString(multiCastIPRaw, "|");
                    ofHttpResponse resp = ofLoadURL(ipSplit[0] + "/Texttiming/" + ipSplit[1]);
                    Json::Reader reader;
                    Json::Value root;
                    bool parsingSuccessful = reader.parse(resp.data, root );
                    if ( !parsingSuccessful ) {
                        isConfig = true;
                        error = true;
                    }
                    else {
                        ipSplit.push_back(root["spl"].asString());
                        ipSplit.push_back(root["lps"].asString());
                        isConfig = true;
                        error = false;
                    }
                    unlock();
                }
            }
            else {
                if (lock()) {
                    isConfig = true;
                    error = true;
                    unlock();
                }
            }
            stopThread();
		}


};

#endif