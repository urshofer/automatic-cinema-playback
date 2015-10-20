#include "testApp.h"




#if TARGET_OS_MAC

void testApp::init(){
}

double testApp::getCpuValue(){
    return 0.0f;
}
int testApp::getMemValue(){
    return 0;
}

#else
static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

void testApp::init(){
    FILE* file;
    struct tms timeSample;
    char line[128];
    
    
    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
    
    
    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}


double testApp::getCpuValue(){
    struct tms timeSample;
    clock_t now;
    double percent;
    
    
    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
        (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
    
    
    return percent;
}

int testApp::parseLine(char* line){
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
}

int testApp::getMemValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];
    
    
    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
#endif





string testApp::curlConnect(string _url, string _post){
	CURL *curl;
	static string buffer;
	buffer.clear();
	curl = curl_easy_init();
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
		curl_easy_cleanup(curl);
	}
	return buffer;
}

void testApp::cprint(WINDOW *local_win, string _out) {
	int y = 0, x = 0;
	
    //	std::vector<std::string> out = ofSplitString(_out, "\n");
	
	// Insert String at the beginning
    //	for(std::vector<std::string>::iterator sactive = out.begin(); sactive != out.end(); ++sactive) {
    //		if (*sactive != "") {
    //			logLines.insert (logLines.begin(),*sactive);
    //		}
    //	}
	
    //	for(std::vector<std::string>::size_type i = 0; i != out.size(); i++) {
    //		logLines.insert (logLines.begin(),out[i]);
    //	}
    
	
	// Delete all Lines longer than COLS-12
    //	if ((int)logLines.size()>LINES-10) {
    //		logLines.erase (logLines.begin()+(LINES-11), logLines.end());
    //	}
	getyx(local_win, y, x);
	wattron(local_win, COLOR_PAIR(1));
	werase(local_win);
    
    //	for (unsigned int l = 0; l < logLines.size(); ++l) {
    //		wmove(local_win, l,COLS-2);
    //		mvwprintw(local_win, l, 0, logLines[l].c_str());
    //	}
    
	wmove(local_win, y,COLS-2);
	mvwprintw(local_win, y, x, _out.c_str());
    
    
    
	wattroff(local_win, COLOR_PAIR(1));
	wrefresh(local_win);
	wmove(local_win, y,x);
}

void testApp::cprintXY(WINDOW *local_win, string _out, int x, int y, int color) {
    //	int y1, x1;
    //	getyx(local_win, y1, x1);
	wattron(local_win, COLOR_PAIR(color));
    //	wclrtoeol(local_win);
	wmove(local_win, y,COLS-2);
	mvwprintw(local_win, y, x, _out.c_str());
	wattroff(local_win, COLOR_PAIR(color));
	wrefresh(local_win);
    //	wmove(local_win, y1,x1);
}
void testApp::cprintXY(WINDOW *local_win, string _out, int x, int y) {
	cprintXY(local_win, _out, x, y, 1);
}

void testApp::clineXY(WINDOW *local_win, int sX, int sY, int w) {
	wattron(local_win, COLOR_PAIR(1));
	mvwaddch(local_win, sY, sX, ACS_LTEE);
	mvwhline(local_win, sY, sX+1, ACS_HLINE, w-2);
	mvwaddch(local_win, sY, w-1, ACS_RTEE);
	wattroff(local_win, COLOR_PAIR(1));
}

WINDOW *testApp::init_curses()
{
	/* Init Curses */
	initscr();			/* Start curses mode 		*/
	start_color();
	noecho();			/* Don't echo() while we do getch */
	timeout(0);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
    
	WINDOW *local_win;
	local_win = newwin(LINES, COLS, 0, 0);
	curs_set(0);
    
	return local_win;
}

void testApp::draw_boxes(WINDOW *local_win) {
	box(local_win, 0 , 0);
	wattron(local_win, COLOR_PAIR(1));
	box(local_win, 0, 0);
	mvwprintw(local_win, 1, 1, "Automatic Cinema Server / of007");
	clineXY(local_win, 0, 2, COLS);
	clineXY(local_win, 0, LINES-8, COLS);
	clineXY(local_win, 0, LINES-6, COLS);
	cprintXY(local_win, "STATE", 1, LINES-6, 1);
	cprintXY(local_win, "CPU", 1, LINES-8, 1);
	wrefresh(local_win);
}

void testApp::end_curses(WINDOW *local_win)
{
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
	curs_set(1);
	timeout(-1);
	endwin();
}

char* testApp::GetInterfaceAddr(const char* interface)
{
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	return(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}



//--------------------------------------------------------------
int testApp::start(){
    // Some Defaults, normally done by Apprunner
    ofSeedRandom();
	ofResetElapsedTimeCounter();
	ofRestoreWorkingDirectoryToDefault();
	
    //signal(SIGSEGV, handler);   // install our handler
	init();
	my_win = init_curses();
	list_win   = subwin(my_win, LINES-11, COLS-2, 3, 1);
	thread_win = subwin(my_win, 4, COLS-2, LINES-5, 1);
	cpu_win = subwin(my_win, 1, COLS-2, LINES-7, 1);
	draw_boxes(my_win);
	
	/* Load Configuration */
	XML.loadFile("serversettings.xml");
	multicastip	= XML.getValue("IP", __IP__);					// Multicast Group IP
	user	= XML.getValue("USER", __USER__);
	pass	= XML.getValue("PASS", __PASS__);
	port	= (int)XML.getValue("PORT", __PORT__);
    
	/* Getting own ip of the selected net device */
	apiurl	= XML.getValue("URL", "http://localhost/Api");
    
	/* Logging in at the API */
    MD5Engine md5;
    md5.update(pass);
	if (getdata.parse( curlConnect(apiurl + "/Login", "username=" + user + "&password=" + DigestEngine::digestToHex(md5.digest()) ), returnval )) {sessionid = returnval.asString(); cprint (list_win, "Logged In: " + ofToString(sessionid));}
	else {endwin(); std::cout << "Could not connect to API Server and Log In\n"; std::exit(1);}
    
	/* Loading Framerate */
	if (getdata.parse( curlConnect(apiurl + "/Rate/" + sessionid, ""), returnval ))  {framerate = returnval.asInt(); cprint(list_win, "Loaded Framerate: " + ofToString(framerate));}
	else {endwin(); std::cout << "Could not Load Framerate\n"; std::exit(1);}
    
	/* Loading Framerate */
	if (getdata.parse( curlConnect(apiurl + "/Preroll/" + sessionid, ""), returnval ))  {preroll = returnval.asInt(); cprint(list_win, "Loaded Preroll: " + ofToString(preroll));}
	else {endwin(); std::cout << "Could not Load Preroll\n"; std::exit(1);}
    
	/* Loading Channels */
	string channel_collapsed = "";
	if (getdata.parse( curlConnect(apiurl + "/Channels/" + sessionid, ""), returnval )) {
		if (returnval.isArray()) {
			bool first = true;
			for ( unsigned int index = 0; index < returnval.size(); index++ )  {
				if (!first) channel_collapsed += "<:>";
				channel_collapsed += returnval[index]["name"].asString();
				first = false;
			}
		}
	}
	
	/* Starting Broadcast Thread: Sending configuration paramters to the clients */
	BRD.start(port, multicastip, sessionid, apiurl, framerate, preroll, channel_collapsed);
    
	/* Framer Thread: Runs as accurately as possible at the given framerate */
	FRM.start(framerate, multicastip, port, sessionid, apiurl);
    
	/* Starting Sync Thread: Sending server time every 5 seconds */
    //	SNC.start(port, multicastip);
    
	string last = "";
	/* Main Loop: Updating the Screen every Second */
	int stroke = getch();
	while (stroke!=27) {
		draw_boxes(my_win);
		werase(thread_win);
		werase(cpu_win);
        
		// Print Trigger Stack
		string list = FRM.getStack();
		if (list != "") cprint(list_win, list);
		// Update Status Every 5 seconds
		char buffer [50];
		sprintf (buffer, "%.2f CPU (perc) %.2f RAM (mb)", getCpuValue(), (double)getMemValue()/1024);
		cprintXY(cpu_win, ofToString(buffer), 0, 0 ,2);
		cprintXY(thread_win, "LDR: " + (FRM.isThreadRunning()?FRM.loaderstate():"not running"), 0,0,3);
		cprintXY(thread_win, "BRD: " + (BRD.isThreadRunning()?BRD.state():"not running"), 0,1,3);
		cprintXY(thread_win, "FRM: " + (FRM.isThreadRunning()?FRM.getState():"not running"), 0,2,3);
		ofSleepMillis(1000);
		stroke = getch();
	}
	end_curses(my_win);
	BRD.stop();
	FRM.stop();
    //	SNC.stop();
	return 0;
}



void testApp::exit()
{
	std::cout  << "Try to exit...\n";
    
    
    end_curses(my_win);
	BRD.stop();
	FRM.stop();
    //	SNC.stop();
    //	return 0;
    
	std::exit(0);

}