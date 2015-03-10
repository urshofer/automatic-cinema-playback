#include "guiApp.h"

void ofApp::drawWaitForConfig(string message) {
    static int angle[3] = {0,0,0};
    static float radius = 10;

    ofPushStyle();
    ofSetColor(255,255,255,80);
    ofRect(0, 0, ofGetWidth(), 43);
    ofPopStyle();
    
    cinetype_2.drawString(message, 12, 28);
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofPath path;
    for (int i=0; i<3; i++) {
        path.setCircleResolution(150);
        if (i==0) path.setFillColor( ofColor (255, 0, 0) );
        if (i==1) path.setFillColor( ofColor (0, 255, 0) );
        if (i==2) path.setFillColor( ofColor (0, 0, 255) );
        path.clear();
        path.moveTo(ofGetWidth()-20, 20);
        path.arc( ofGetWidth()-20, 20, radius, radius, -120+angle[i], angle[i]);
        path.lineTo(ofGetWidth()-20, 20);
        path.draw();
        angle[i] = angle[i]<360?angle[i]+(i+5):0;
    }
	ofDisableBlendMode();
    ofPopStyle();
}

string ofApp::curlConnect(string _url, string _post){
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
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	return buffer;
}

void ofApp::setupGUI(string guifont) {
    gui = new ofxUICanvas();        //Creates a canvas at (0,0) using the default width
    gui->setFont(ofToDataPath(guifont));
    gui->setFontSize(OFX_UI_FONT_SMALL, 8);
    gui->setFontSize(OFX_UI_FONT_MEDIUM, 10);
    gui->setFontSize(OFX_UI_FONT_LARGE, 10);
    ofxUIColor cb = ofxUIColor( 0,0,0,0 );
    ofxUIColor co = ofxUIColor( 128 );
    ofxUIColor coh = ofxUIColor( 0 );
    ofxUIColor cf = ofxUIColor( 128 );
    ofxUIColor cfh = ofxUIColor( 0 );
    ofxUIColor cp = ofxUIColor( 128 );
    ofxUIColor cpo = ofxUIColor( 128 );
    gui->setUIColors( cb, co, coh, cf, cfh, cp, cpo );
    gui->setPosition((ofGetWidth()-509)/2,(ofGetHeight()-309)/2);
    gui->setDimensions(509,609);
    gui->setPadding(3);
    gui->setGlobalSpacerHeight(0);
	gui->setWidgetFontSize(OFX_UI_FONT_SMALL);
    gui->addSpacer(0,65);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addTextInput("Server",         apiurl,305,10,150,10,OFX_UI_FONT_SMALL);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addTextInput("User",       ofToString(user),305,10,150,0,OFX_UI_FONT_SMALL);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addTextInput("Password",       ofToString(pass),305,10,150,0,OFX_UI_FONT_SMALL);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addSpacer();
    gui->addTextInput("Multicast",       ofToString(multicastip),305,10,150,0,OFX_UI_FONT_SMALL);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addTextInput("Port",       ofToString(port),250,10,150,0,OFX_UI_FONT_SMALL);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_FREE);
    gui->addSpacer();
    gui->addButton("Save", true, 15,15,155,10);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui->resetPlacer();
    gui->addImage("BG", bg, 503, 303);
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
}

// Listeners
//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
    
    if(e.getName() == "Server")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        apiurl = ti->getTextString();
    }
    if(e.getName() == "Password")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        pass = ti->getTextString();
    }
    if(e.getName() == "User")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        user = ti->getTextString();
    }
    if(e.getName() == "Multicast")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        multicastip = ti->getTextString();
    }
    if(e.getName() == "Port")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        port = ofToInt(ti->getTextString());
    }
    if(e.getName() == "Save")
    {
        /* Store XML File */
        XML.setValue("IP", multicastip);					// Multicast Group IP
        XML.setValue("USER", user);
        XML.setValue("PASS", pass);
        XML.setValue("PORT", port);
        XML.setValue("URL", apiurl);
        XML.saveFile();

        ofxUIButton *ti = (ofxUIButton *) e.widget;
        if (ti->getValue()) {
            MD5Engine md5;
            md5.update(pass);
            ofLogError() << "- Reconnect to " << apiurl;
            if (getdata.parse( curlConnect(apiurl + "/Login", "username=" + user + "&password=" + DigestEngine::digestToHex(md5.digest()) ), returnval )) {
                sessionid = returnval.asString();
                gui->disable();
            }
            else {
                ofLogError() << "- Could not connect to API Server and Log In";
            }
        }
    }
    
}

	
//--------------------------------------------------------------
void ofApp::setup(){
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources/data/");
#endif
    
	ofSetVerticalSync(false);
    ofSetFrameRate(12);
    ofBackground(0);
    
    /* Load Fonts */
    ofXml FXML;
    if (!FXML.load("fonts.xml")) {
        ofLogError() << "- Could not open xml configuration";
        std::exit(0);
    }
    FXML.setTo("/fonts");
    string guifont = FXML.getValue("gui");
    ofLogError() << "- Loading font " << guifont;
    cinetype_1.load(guifont, 10);
    cinetype_2.load(guifont, 16);
    cinetype_2.setLetterSpacing(0.95);
    
    /* Widget Background */
    bg = new ofImage();
    bg->load("gui.png");
    

	/* Load Configuration */
	XML.loadFile("serversettings.xml");
	multicastip	= XML.getValue("IP", __IP__);					// Multicast Group IP
	user	= XML.getValue("USER", __USER__);
	pass	= XML.getValue("PASS", __PASS__);
	port	= (int)XML.getValue("PORT", __PORT__);
    
	/* Getting own ip of the selected net device */
	apiurl	= XML.getValue("URL", "http://localhost:3000");
    
	/* Logging in at the API */
    
    MD5Engine md5;
    md5.update(pass);
	if (getdata.parse( curlConnect(apiurl + "/Login", "username=" + user + "&password=" + DigestEngine::digestToHex(md5.digest()) ), returnval )) {sessionid = returnval.asString();}
	else {
        ofLogError() << "- Could not connect to API Server and Log In";
        setupGUI(guifont);
    }
    configured = false;
}

//--------------------------------------------------------------
void ofApp::exit(){
    BRD.stop();
	FRM.stop();
    std::exit(0);
}

//--------------------------------------------------------------
void ofApp::update(){
    if (sessionid != "") {
        if (!configured) {
            /* Loading Framerate */
            if (getdata.parse( curlConnect(apiurl + "/Rate/" + sessionid, ""), returnval ))  {framerate = returnval.asInt();}
            else {std::cout << "Could not Load Framerate\n"; ofExit();}
            
            /* Loading Framerate */
            if (getdata.parse( curlConnect(apiurl + "/Preroll/" + sessionid, ""), returnval ))  {preroll = returnval.asInt(); }
            else {std::cout << "Could not Load Preroll\n"; ofExit();}
            
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
            
            configured = true;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (!configured) {

        return;
    }
    float w = ofGetWidth();
    float h = ofGetHeight();
    ofColor c[3] = {ofColor(255,45,85,128),ofColor(85,255,45,128),ofColor(45,85,255,128)};
    
	// Print Trigger Stack
    ofSetColor(255,255,255,210);
    
    drawWaitForConfig("Automatic Cinema Server");

    framerThread::verbose_info v = FRM.getInfo();
    
    int offset_y = 65;
    for(std::vector <framerThread::_verbose>::size_type i = 0; i != v.size(); i++) {
        cinetype_1.drawString(v[i].channel, 10, offset_y);
        ofPushStyle();
        ofFill();
        ofSetColor(255,255,255,80);
//        ofRect(10,offset_y+10, (w-20),50);
        
        ofPath p;
        p.setFilled(true);
        p.setFillColor(ofColor(255,255,255,70));
        p.moveTo(0,49);
        p.moveTo(6,40);
        p.lineTo(w-24,40);
        p.lineTo(w-30,49);
        p.lineTo(w-24,58);
        p.lineTo(6,58);
        p.lineTo(0,49);
        p.close();
        p.draw(12,offset_y);
        
        p.clear();
        p.setFillColor(c[(int)i%3]);
        float _fac = w-36;
        p.moveTo(0,13);
        p.lineTo(v[i].fadein?6:0,0);
        p.lineTo(v[i].fadeout?_fac+6:_fac+12,0);
        p.lineTo(_fac+12,13);
        p.lineTo(v[i].fadeout?_fac+6:_fac+12,26);
        p.lineTo(v[i].fadein?6:0,26);
        p.close();
        p.draw(12,offset_y+12);
        
        if (v[i].length>0) {
            
            ofPath p;
            p.setFilled(true);
            p.setFillColor(c[(int)i%3]);
            float _fac = (w-36)/(float)v[i].length*CLAMP((float)v[i].position, 0, (float)v[i].length);

            p.moveTo(0,13);
            p.lineTo(v[i].fadein?6:0,0);
            p.lineTo(v[i].fadeout?_fac+6:_fac+12,0);
            p.lineTo(_fac+12,13);
            p.lineTo(v[i].fadeout?_fac+6:_fac+12,26);
            p.lineTo(v[i].fadein?6:0,26);
            p.close();
            p.draw(12,offset_y+12);
            
            
            
        }
        ofPopStyle();

        cinetype_1.drawString(v[i].thisclip, (w-cinetype_1.stringWidth(v[i].thisclip))/2, offset_y+30);
        if (v[i].waiting>0 && v[i].nextin > 0) {
            ofPushStyle();
            ofSetColor(255,255,255,50);
            
            ofPath p;
            p.setFilled(true);
            p.setFillColor(ofColor(255,255,255,70));
            p.moveTo(0,49);
            p.moveTo(6,40);
            p.lineTo(((w-30)/(float)v[i].nextin*(float)v[i].waiting)+6,40);
            p.lineTo(((w-30)/(float)v[i].nextin*(float)v[i].waiting),49);
            p.lineTo(((w-30)/(float)v[i].nextin*(float)v[i].waiting)+6,58);
            p.lineTo(6,58);
            p.lineTo(0,49);

            p.close();
            p.draw(12,offset_y);
            cinetype_1.drawString(v[i].nextclip, (w-cinetype_1.stringWidth(v[i].nextclip))/2, offset_y+53);
            ofPopStyle();
        }

        offset_y += 80;
    }
    
    



    
}