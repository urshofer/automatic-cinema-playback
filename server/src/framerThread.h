#ifndef _FRAMER_THREAD
#define _FRAMER_THREAD


#include "ofxNetwork.h"
#include "loaderThread.h"
#include "json/json.h"

/*#include <gst/gst.h>
#include <gst/gstclock.h>
#include <gst/net/gstnet.h>
*/

class framerThread : public ofThread{

	public:
		
		/* Channel List */

    
        struct _verbose
        {
            string          channel;
            string 			thisclip;
            string 			nextclip;
            bool 			fadein;
            bool 			fadeout;
            unsigned int    length;
            unsigned int    position;
            unsigned int    nextin;
            unsigned int    waiting;
        };
		typedef std::vector < _verbose >	verbose_info;
        verbose_info        verbose;
    

		enum _waitstate 
		{
			READY, 
			LOADED, 
			WAITING, 
			PIVOT, 
			SWITCH, 
			PLAYING
		};
		
		struct _channels
		{
			std::string name;
			_waitstate state;
			unsigned long time;
			unsigned long last_in;
			unsigned long last_duration;
			loaderThread::_clip data;
			loaderThread::_clip data_next;
			bool is_slave;	
			bool force_reset;
			std::string master;
				
		};
		typedef std::vector < _channels >	channel_state;
		channel_state						chstate;
		

		/* Clock */

//		GstClock *		gstclock;
//		GstNetTimeProvider * netclock;

		
		int framerate, port;
		loaderThread	LDR;
		string sessionid, apiurl, multicastip, stack, state;
		ofxUDPManager udpConnection;		

		//--------------------------
		framerThread(){
		}
    
        verbose_info getInfo() {
//			ofScopedLock lock(mutex);
			return verbose;
        }

		string loaderstate() {
			return (LDR.state());
		}

		string getState() {
			ofScopedLock lock(mutex);
			return state;
		}

		string getStack() {
/*			string _stack;
			if( lock() ){
				_stack = stack;
				unlock();
			}*/
			ofScopedLock lock(mutex);
			return stack;
		}
		
		void setupnet() {
			udpConnection.Create();
			udpConnection.ConnectMcast((char*)multicastip.c_str(), port);
			udpConnection.SetNonBlocking(false);
			udpConnection.SetEnableBroadcast(true);
            udpConnection.SetReuseAddress(true);            
		}

		void start(int _framerate, string _multicastip, int _port, string _sessionid, string _apiurl) {

			/* Create Multi Cast Interface */
			multicastip = _multicastip;
			framerate = _framerate;		
			apiurl = _apiurl;
			sessionid = _sessionid;				
			port = _port;
			setupnet();
//			gst_init (NULL, NULL);
			state = "Initializing...";
//		    gstclock = gst_system_clock_obtain();
//			netclock = gst_net_time_provider_new (gstclock, _multicastip.c_str(), port + 10);

			/* Playlist Loader Thread: Polls every ten Seconds if a new Playlist exists. */
			LDR.start(sessionid, apiurl);
			
			// Wait till first Elements are loaded
			while (!LDR.waitForReady()) {
				state = "Waiting for Data...";
				ofSleepMillis(1000);
			}

			/* Starting Framer */
			startThread();   // blocking, verbose
		}

		void stop(){
			LDR.stop();
			stopThread();
			waitForThread();
			udpConnection.Close();
			std::cout << "FRM Stopped\n";		
		}
		
/*		void setState(string _state) {
			if (lock()) {
				state = _state;
				unlock();
			}
		}
*/
		void threadedFunction(){
//			setState("Framer Running");		
//			if (lock()) {
				state = "Framer running...";
//				unlock();
//			}


			Json::Value senddata;
			loaderThread::_clip _cl = (loaderThread::_clip) {
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
			/* refresh channel list */
			std::vector <std::string> channels = LDR.getChannels();

//			unsigned long nowTime = (unsigned long)((double)gst_clock_get_time(gstclock) / (double)1000000);
   			unsigned long nowTime = ofGetElapsedTimeMillis();
			for(std::vector <std::string>::size_type i = 0; i != channels.size(); i++) {
				_channels _c;
				_c.name  = channels[i];
				_c.state = READY; 
				_c.time  = nowTime;
				_c.last_in = 0;
				_c.is_slave = LDR.isSlave(channels[i]);
				_c.master = LDR.hasMaster(channels[i]);
				_c.last_duration = 0;
				_c.data = 	
				_c.data_next 	= _cl;
				_c.force_reset	= false;
				chstate.push_back(_c);
			}
			
			while (isThreadRunning()!= 0) {
				/* Reset Send Data */
				Json::Value::UInt store_index=0;
				senddata.clear();
				string _stack = "";
                string channel_time_post = "";
                verbose_info __verbose;
				/* Cycle Thru States */
				for(channel_state::iterator sactive = chstate.begin(); sactive != chstate.end(); ++sactive) {
                    
					/* 
	                    Current Time:
	                    - Current Time holds the running time of the current channel
	                 */
				
					unsigned long currentTime = nowTime - (*sactive).time;

                    /* Update channel time */
                    channel_time_post += channel_time_post != "" ? ", " : "";
                    channel_time_post += "\"" + (*sactive).name + "\": " + ofToString(currentTime);
                    
					/* 
                        Action for Reset: 
                        - State Ready, Reset Time 
                        - Flag force_reset to true
                     */
					
					if (LDR.isReset((*sactive).name)==true || (*sactive).force_reset == true) {
						(*sactive).state = READY;
                        (*sactive).time  = nowTime;
						/*(*sactive).force_reset = true;*/
						(*sactive).data         =
						(*sactive).data_next 	= _cl;
                        (*sactive).force_reset = false;
                        cout << "Resetting Slave Channel " << (*sactive).name << endl;

					}
					
					/*
                        Action for State READY:
                        - Load Next Data
                        - Prepare JSON Message
                        - If force_reset, trigger Message, jump to SWITCH
                        - Normally, append message, jump to WAITING
                     */
					
					if ((*sactive).state == READY) {
						(*sactive).data_next = LDR.requestData((*sactive).name);
						if ((*sactive).data_next.id != "") {
                            senddata["s"][store_index]["c"]		= (*sactive).name;
                            senddata["s"][store_index]["m"]		= (*sactive).data_next.in==0?"t":"a";
                            senddata["s"][store_index]["t"]		= (*sactive).data_next.type;
                            senddata["s"][store_index]["f"]		= (*sactive).data_next.data_checksum;
                            senddata["s"][store_index]["fxin"]	= (*sactive).data_next.gap_in?"FadeIn":"-";
                            senddata["s"][store_index]["fxout"]	= (*sactive).data_next.gap_out?"FadeOut":"-";
                            store_index++;
                            if ((*sactive).data_next.in==0) {
                                (*sactive).state = SWITCH;
                            }
                            else {
                                (*sactive).state = WAITING;
                            }
						}
					}
                    
                    /* 
                        State WAITING:
                        - Wait until current time has reached in point of next clip,
						  or the out point of current clip if the next clip starts earlier
                        - If Point is reached, create Pivot Message
                        - Jump to Switch
                     */


					if ((*sactive).state == WAITING) {
						if (
							(currentTime >= (*sactive).data_next.in && (*sactive).data_next.in > (*sactive).data.in) ||
							(currentTime >= (*sactive).data.out && (*sactive).data_next.in <= (*sactive).data.in)
						) {
                            senddata["s"][store_index]["c"]		= (*sactive).name;
                            senddata["s"][store_index]["m"]		= "p";
                            senddata["s"][store_index]["t"]		= (*sactive).data_next.type;
                            senddata["s"][store_index]["f"]		= (*sactive).data_next.data_checksum;
                            store_index++;
                            (*sactive).state = SWITCH;
						}
					}
					
					/* 
                        State Switch: 
                        - Copy next to Current Clip
                        - Empty next clip
                        - Store last in and last duration
                        - Switch to playing mode
                     */
					
					if ((*sactive).state == SWITCH) {
						if (((*sactive).data_next.in <= (*sactive).data.in) && !(*sactive).is_slave) {
							(*sactive).time = nowTime;
						}
                        /* Force Reset of Slaves */
                        if (!(*sactive).is_slave && (*sactive).data_next.reset) {
                            for(channel_state::iterator slave = chstate.begin(); slave != chstate.end(); ++slave) {
                                if (slave->master == (*sactive).name) {
                                    slave->force_reset = true;
                                }
                            }
                        }
                        (*sactive).last_in = (*sactive).data_next.in;
                        (*sactive).last_duration = (*sactive).data_next.duration;
						(*sactive).data = (*sactive).data_next;
						(*sactive).data_next = _cl;
						(*sactive).state = PLAYING;
					}					
					
					/* 
                        State Playing:
                        - For master Channels only, if current clips starts earlier than the last
                          and the last has finished: reset time and reset the slave channels.
                        - For all channels: if the current time has reached the middle of the last clip
                        - Flag will be set to READY if one of the upper conditions meet.
					*/
					
					if ((*sactive).state == PLAYING) {
						if (currentTime >= (*sactive).last_in + (*sactive).last_duration/2) {
							(*sactive).state = READY;
						}
					}
					
		
					//cout << "Channel " << (*sactive).name << " state: " << (*sactive).state;
					//cout << " C: " << currentTime << " Out: " << (*sactive).data.out;
                    //cout << " Next in: " << (*sactive).data_next.in << " In: " << (*sactive).data.in << endl;

					
					/* Print to Playlist */
//					if (currentTime >= (*sactive).data.out) {
//						(*sactive).data = _cl;
//					}
				
                    _verbose verbosenow;
                    verbosenow.channel  = (*sactive).name;
                    verbosenow.thisclip = (*sactive).data.name;
                    verbosenow.nextclip = (*sactive).data_next.name;
                    verbosenow.fadein   = (*sactive).data.gap_in;
                    verbosenow.fadeout  = (*sactive).data.gap_out;
                    verbosenow.length   = (*sactive).data.duration;
                    verbosenow.position = (*sactive).data.in < currentTime?currentTime - (*sactive).data.in:0;
                    verbosenow.waiting  = (*sactive).data_next.in?(*sactive).data_next.in - currentTime:0;
                    verbosenow.nextin   = (*sactive).data_next.in>(*sactive).data.in+((*sactive).data.duration/2)?(*sactive).data_next.in - ((*sactive).data.in+((*sactive).data.duration/2)):0;
                    __verbose.push_back(verbosenow);
                    
					_stack += "# Channel " + (*sactive).name + " (" + ((*sactive).is_slave?"slave":"master") + ")" + "\n";
					_stack += "Playing   " + ofToString(currentTime/1000) +" s\n";
					_stack += "Current   " + ofToString(((*sactive).data.gap_in==true?"< ":"- ")) + (*sactive).data.name + ofToString(((*sactive).data.gap_out==true?" >\n":" -\n"));
					_stack += "Remaining " + ((*sactive).data.out?ofToString(((*sactive).data.out - currentTime)/1000) + " s of " + ofToString(((*sactive).data.duration)/1000) + " s":"") + "\n";
					_stack += "Next      " + (*sactive).data_next.name + "\n";
					_stack += "Waiting   " + ((*sactive).data_next.in?ofToString(((*sactive).data_next.in - currentTime)/1000) + " s  Gap: "+ ofToString(((*sactive).data_next.in - (*sactive).data.out)/1000):"")  +  "\n";
					_stack += LDR.getStack((*sactive).name);
					_stack += "\n";
				}			
				
				/* Send Data if there is some */
				if (store_index>0) {
					Json::FastWriter writer;
					string send = writer.write( senddata["s"] );
					if (send.size()>5) {
						udpConnection.Send(send.c_str(),send.size());	
						if (lock()) {
							state = ofToString(nowTime/1000) + ": " + send;
							unlock();
						}
						send = "";
					}
				}

				/* Frame Advancing and timer Adjustement */
				ofSleepMillis(5);
				//nowTime = (unsigned long)((double)gst_clock_get_time(gstclock) / (double)1000000);
                nowTime = ofGetElapsedTimeMillis();
				if (lock()) {
					stack = _stack;
                    verbose = __verbose;
					unlock();
				}
                LDR.storeChannelTime(channel_time_post);
			}
//			stop();
		}


};

#endif








