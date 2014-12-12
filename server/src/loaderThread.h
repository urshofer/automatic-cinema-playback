#ifndef _LOADER_THREAD
#define _LOADER_THREAD

#include "ofxNetwork.h"
#include <curl/curl.h>
#include "json/json.h"
#ifndef TARGET_OF_IPHONE
#include <ncurses.h>
#endif
#include <iostream>
#include <fstream>
#include <map>


class loaderThread : public ofThread{

	public:

		struct _clip
		{

			string          id;
			unsigned int 	duration;
			unsigned int 	in;
			unsigned int 	out;			
			double 			duration_asdouble;
			double 			in_asdouble;
			double 			out_asdouble;			
			string 			name;
			string 			type;
			string 			data_checksum;
			bool 			reset;
			bool 			gap_in;
			bool 			gap_out;
		};
		
		

		struct _queue
		{
//			std::string 	name;
			bool 			is_slave;
			std::string		master;
			bool 			reset;
            bool            force_new;
			unsigned int 	last_time;
			string			stack;
			std::vector < _clip > data;
		};
		
        string          channel_time_post;
		typedef std::map<std::string, _queue > 	queue_vector;
		queue_vector 					queue;			// Data Queue
		std::vector < std::string > 	all;
		bool							isReady;
		CURL*							curl;

		string sessionid, apiurl, status;

		//--------------------------
		loaderThread()
		{
		}


		string state()
		{
			ofScopedLock lock(mutex);
			return status;
		}
		
		void logFile(string _string, string localPath) {
//			ofstream SaveFile(localPath.c_str());
//			SaveFile << _string;
//			SaveFile.close();
		}

		static int writer(char *data, size_t size, size_t nmemb, std::string *buffer)  
		{
			int result = 0;  
			if (buffer != NULL)  
			{  
				buffer->append(data, size * nmemb);  
				result = size * nmemb;  
			}  
			return result;  
		}

		string curlConnect(string _url, string _post)
		{
			logFile("curlConnect: " + _url + "\n", "test.log");			
			static string buffer;
			curl = curl_easy_init();
			buffer.clear();
			if(curl) {
				curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());

				/* Now specify the POST data */ 
				if (_post != "") {
			    	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _post.c_str());
				}
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
				curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8" );
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer); 	
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);  
				curl_easy_perform(curl);
				curl_easy_reset(curl);
				curl_easy_cleanup(curl);	
			}
			logFile(buffer + "\n--\n", "test.log");
			return buffer;
		}
		
		char * url_escape(string _url) {
			curl = curl_easy_init();
			char *encodedURL = curl_easy_escape(curl,_url.c_str(), strlen(_url.c_str()));								
			curl_easy_cleanup(curl);							
			return encodedURL;
		}
		
		
		bool isReset(string _channel) {
			ofScopedLock lock(mutex);
		
			map<string,_queue>::iterator qactive = queue.find(_channel);
			if(qactive != queue.end() && qactive->second.data.size()>1)
			{
                if (qactive->second.force_new==true && !qactive->second.is_slave) {
                    qactive->second.force_new = false;
                    return true;
                }
			}
			return false;
		}
    
        void storeChannelTime(string _s) {
            if (lock()) {
                channel_time_post = _s;
                unlock();
            }
        }
		
		bool isSlave(string _channel) {
			ofScopedLock lock(mutex);
//			bool _slave = false;
			map<string,_queue>::const_iterator qactive = queue.find(_channel);
			if(qactive != queue.end())
			{
				return qactive->second.is_slave;
			}	
			else {
				return false;
			}
			
		}		
		
		string getStack(string _channel) {
			ofScopedLock lock(mutex);
			map<string,_queue>::const_iterator qactive = queue.find(_channel);
			if(qactive != queue.end())
			{
				return qactive->second.stack;
			}	
			else {
				return "";
			}
		}			


		string hasMaster(string _channel) {
			ofScopedLock lock(mutex);
			map<string,_queue>::const_iterator qactive = queue.find(_channel);
			if(qactive != queue.end())
			{
				return qactive->second.master;
			}	
			else {
				return "";
			}
		}			
		
		bool waitForReady() {
			bool _ready = false;
			if( lock() ) {_ready =  isReady; unlock();}
			return _ready;
		}

		_clip requestData(string _channel)
		{
			_clip returnData = (_clip) {
				 "",
				 0, 
				 0, 
				 0, 
				 0.0, 
				 0.0, 
				 0.0, 
				 "", 
				 "", 
				 "", 
				 false, 
				 false, 
				 false
			};
			map < string,_queue >::iterator qactive = queue.find(_channel);
			if(qactive != queue.end())
			{
                if (lock()) {
                    if (qactive->second.data.size()>1) {
                        returnData = qactive->second.data.front();
                        qactive->second.data.erase(qactive->second.data.begin());

                        /* Swap Trick: Free Memory after shrinking a vector */
                        std::vector<_clip>(qactive->second.data).swap(qactive->second.data);
                    }
                    unlock();
                }
            }
			return returnData;
		}

    
		std::vector < std::string >& getChannels()
		{
			ofScopedLock lock(mutex);			
			return all;			
		}

		void start(string _sessionid, string _apiurl)
		{
			apiurl 			= _apiurl;
			sessionid 		= _sessionid;
			Json::Reader 	channelJson, masterslaveJson;
			Json::Value 	channelData, masterslaveData;
			isReady 		= false;			

			curlConnect(apiurl + "/Reset/" + sessionid, "");

			/* Load Channels */
			if (channelJson.parse( curlConnect(apiurl + "/Channels/" + sessionid, ""), channelData )) {
				if (channelData.isArray()) {
                    for ( unsigned int index = 0; index < channelData.size(); index++ )  {
						pair <string,vector<Json::Value > > channel;
					
						char *encodedURL = url_escape(channelData[index]["name"].asString());
					
						masterslaveJson.parse( curlConnect(apiurl + "/HasMaster/" + sessionid + "/" + ofToString(encodedURL), ""), masterslaveData );

						_queue _q;
//						_q.name = channelData[index]["name"].asString();
//						_q.gap_in = false;
//						_q.gap_out = false;
						_q.reset = false;
                        _q.force_new = false;
						_q.last_time = 0;
						_q.data.clear();
						_q.is_slave = masterslaveData["slave"].asBool();
						_q.master   = masterslaveData["master"].asString();	
						all.push_back(channelData[index]["name"].asString());
						//queue.push_back(_q);
//						queue.emplace (channelData[index]["name"].asString(),_q);
//                        cout << "Channel " << channelData[index]["name"].asString() << " is " << _q.is_slave << endl;
                        queue[channelData[index]["name"].asString()] = _q;
					}
				}
			}
            
#ifdef TAGET_LINUX
			else {endwin(); std::cout << "Could not connect to API Server and load channels.\n"; exit(1);}
#else
			else {std::cout << "Could not connect to API Server and load channels.\n"; ofExit();}
#endif


//			if( lock() ){
				status =  "loaderThread started";
//				unlock();
//			}

			startThread();   // blocking, verbose
		}

		void stop()
		{
			stopThread();
			waitForThread();			
			std::cout << "LDR Stopped\n";		
		}

		void threadedFunction()
		{
//			CURL *curl;		
//			curl = curl_easy_init();
			Json::Reader resetJson;
			Json::Value resetData;
			Json::Reader nextJson;
			Json::Value nextData;
            channel_time_post = "";
            unsigned long last_check = 0;
			
			while (isThreadRunning()!= 0) {
				string qString = "";

                // Sync State: Sending current time, receiving interaction state (reset)
                if (ofGetElapsedTimeMillis()>last_check+500) {
                    string _c = "";
                    if (lock()) {
                        _c = channel_time_post;
                        unlock();
                    }
                    resetJson.parse( curlConnect(apiurl + "/SyncState/" + sessionid , "data=[{"+_c+"}]"), resetData );
                    last_check = ofGetElapsedTimeMillis();
                    for(queue_vector::iterator qactive = queue.begin(); qactive != queue.end(); ++qactive) {
                        if (resetData[qactive->first]==true && !qactive->second.is_slave) {
                            if( lock() ){
                                //qactive->second.data.clear();
                                if (resetData[qactive->first].asBool()==true) {
                                    qactive->second.reset = true;
                                }
                                unlock();
                            }
                        }
                    }
                }
                
                
                /* Cycle thru all Master Channels
                   Find Element in Queue: qactive */
		
                channel_time_post = "";

				for(queue_vector::iterator qactive = queue.begin(); qactive != queue.end(); ++qactive) {
                    
                    char *encodedURL = url_escape(qactive->first);

					
					/* Check if less than 5 Elements or a Reset happened */
					qString += qactive->first + "(" + ofToString(qactive->second.data.size());
					if(qactive->second.data.size()<5 || qactive->second.reset == true) {
                        
//                        cout << "Loading... " << apiurl  << "/Next/" << sessionid << "/" << ofToString(encodedURL) << " ";
						/* GetNext */
						string getNext = curlConnect(apiurl + "/Next/" + sessionid + "/" + ofToString(encodedURL), "");
						if (nextJson.parse( getNext, nextData )) {

//                            cout << getNext << endl;
							
							/*
							 * Skip if result is false. This means, there is no data for this channel.
							 * Otherwise, the thread can lock up in an endless loop here.
							 */
							
							if (((nextData.isBool() && !nextData.asBool()) || nextData["Error"]["Code"].isInt())) {
								break;
							}
							else {
//                                cout << " done ";
								_clip _nextData;
								_nextData.id				= nextData["element"]["element_id"].asString();
								_nextData.duration			= nextData["element"]["element_duration"].asUInt();
								_nextData.duration_asdouble	= nextData["element"]["element_duration"].asDouble();
								_nextData.in				= nextData["in"].asUInt();
								_nextData.in_asdouble		= nextData["in"].asDouble();
								_nextData.out				= nextData["out"].asUInt();
								_nextData.out_asdouble		= nextData["out"].asDouble();
								_nextData.name				= nextData["element"]["element_name"].asString();
								_nextData.type				= nextData["element"]["element_type"].asString();
								_nextData.data_checksum		= nextData["element"]["element_data_checksum"].asString();
                                _nextData.reset				= nextData["element"]["is_new"].asBool();


                                _nextData.gap_in			= true;
                                _nextData.gap_out			= true;
                             

                                /* Update Gaps */
                                if (qactive->second.data.size()>0) {
                                    unsigned int last_out       = qactive->second.data.back().out;
                                    if (last_out!=_nextData.in) {
                                        _nextData.gap_in = true;
                                        qactive->second.data.back().gap_out	= true;
                                    }
                                    else {
                                        _nextData.gap_in = false;
                                        qactive->second.data.back().gap_out	= false;
                                    }
                                }

                                
								/*
								 * Reset Flag on clip level means: a new timeline has started,
								 * either after user interaction or time limit overflow
								 */

								if( lock() ){
                                    //cout << qactive->first << " Element: " << nextData["element"]["reset"] << endl;
                                    if (_nextData.reset && !qactive->second.is_slave) {
                                        if (nextData["element"]["reset"].asBool()) {
                                            /* We clear the Stack here */
                                            qactive->second.data.clear();
//                                            cout << "- Clear Stack for Master " << qactive->first << endl;
                                        }
                                        /* Clear Slave Stacks after a Master Timeline has expired */
                                        for(queue_vector::iterator slave = queue.begin(); slave != queue.end(); ++slave) {
                                            if (slave->second.is_slave == true && slave->second.master == qactive->first) {
                                                slave->second.data.clear();
//                                                cout << "- Clear Stack for Slave " << slave->first << endl;
                                            }
                                        }
                                    }
                                    qactive->second.data.push_back(_nextData);
                                    if (nextData["element"]["reset"].asBool()) {
                                        qactive->second.force_new = true;
                                    }
                                    qactive->second.reset = false;
									qString += "+";
									qactive->second.stack = "";
									int _c = 1;
									for(std::vector< _clip >::iterator _cactive = qactive->second.data.begin(); _cactive != qactive->second.data.end(); ++_cactive) {
										qactive->second.stack += ofToString(_c==1?"Stack     ":"          ") + ofToString(_c) + " In: " + ofToString(_cactive->in/1000) + "\tOut: " + ofToString(_cactive->out/1000) + "\t\t" + ofToString((_cactive->gap_in==true?"< ":"- ")) +  ofToString((_cactive->gap_out==true?"> ":"- ") + _cactive->name + "\n");									
										_c++;
									}
									unlock();
								}
                                else {
//                                    cout << "whoooooops" << endl;
                                }
//                                cout << " now " << endl;
                                
							}

						}
						/* Emergency */
						//else {
						//	break;
						//}
					}
					qString += ") /";							
				}
                
				/* Sleep for a half second before checking again, if there are 5 elements per channel */
				ofSleepMillis(250);
				/* Update State Message */
				status =  "Queue: " + qString + "                                 ";
				isReady = true;
			}
//			stop();
		}
};

#endif