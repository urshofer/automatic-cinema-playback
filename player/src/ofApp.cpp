#include "ofApp.h"

void ofApp::drawWaitForConfig(string message) {
    static int angle[3] = {0,0,0};
    static float radius = 25;
    cinetype_1.drawString(message, ofGetWidth()/2-cinetype_1.stringWidth(message)/2, ofGetHeight()/2+40);
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofPath path;
    for (int i=0; i<3; i++) {
        path.setCircleResolution(150);
        if (i==0) path.setFillColor( ofColor (255, 0, 0) );
        if (i==1) path.setFillColor( ofColor (0, 255, 0) );
        if (i==2) path.setFillColor( ofColor (0, 0, 255) );
        path.clear();
        path.moveTo(ofGetWidth()/2, ofGetHeight()/2-30);
        path.arc( ofGetWidth()/2, ofGetHeight()/2-30, radius, radius, -120+angle[i], angle[i]);
        path.lineTo(ofGetWidth()/2, ofGetHeight()/2-30);
        path.draw();
        angle[i] = angle[i]<360?angle[i]+(i+5):0;
    }
	ofDisableBlendMode();
    ofPopStyle();
}

void ofApp::drawSyncFiles(string filename) {
    static int angle = 0;
    ofPushMatrix();
    glTranslatef(ofGetWidth()-30,ofGetHeight()-30,0);
	glRotatef(angle,0,0,1);
    syncicon.draw(-10, -10, 20, 20);
    ofPopMatrix();
    angle = angle<360?angle+5:0;
    cinetype_2.drawString(filename, ofGetWidth()-cinetype_2.stringWidth(filename)-50, ofGetHeight()-25);
}

void ofApp::loadXMLConfig() {
    ofXml XML;

#ifdef TARGET_OF_IPHONE
    if( XML.load(ofxiOSGetDocumentsDirectory() + "playersettings.xml") ){
        ofLogVerbose() << "- Loaded Stored Values";
	}else if( XML.load("playersettings.xml") ){
        ofLogVerbose() << "- Loaded Default";
	}else{
        ofLogError() << "- Could not open xml configuration";
        exit();
	}
#else
    if (!XML.load("playersettings.xml")) {
        ofLogError() << "- Could not open xml configuration";
        exit();
    }
#endif
    

    XML.setTo("/settings");
    config.hasAudio = XML.getBoolValue("support_media/audio");
    config.hasVideo = XML.getBoolValue("support_media/video");
    config.hasText  = XML.getBoolValue("support_media/text");
    config.drawLogo = XML.getBoolValue("drawlogo");
    config.baseport = XML.getIntValue("baseport");
    config.multicastIp = XML.getValue("ip");
    config.fullscreen = XML.getBoolValue("fullscreen");
    config.verbose = XML.getBoolValue("verbose");
    ofSetFullscreen(config.fullscreen);
    MO.setPreview(config.verbose);
    XML.setTo("channel");
    for (int i=0; i<XML.getNumChildren("name"); i++) {
        config.channel.push_back(XML.getValue("name["+ ofToString(i) +"]"));
        
    }
    std::cout  << "+ Loaded Configuration File" << endl;
    std::cout  << "  Audio        : " << config.hasAudio << endl;
    std::cout  << "  Video        : " << config.hasVideo << endl;
    std::cout  << "  Text         : " << config.hasText << endl;
    std::cout  << "  Port         : " << config.baseport << endl;
    std::cout  << "  Multicast IP : " << config.multicastIp << endl;
    for (int i=0; i< config.channel.size(); ++i)
        std::cout  << "  Channel : " << config.channel[i] << endl;

}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_ERROR);
	ofEnableAntiAliasing();
    ofEnableAlphaBlending();
	ofSetVerticalSync(true);
	ofSetFrameRate(50);
    cursorflag = true;    
    ofBackground(0, 0, 0);

	#ifdef TARGET_OF_IPHONE
  		ofSetOrientation(OF_ORIENTATION_90_RIGHT);
	#endif
    
    /* Load Font */
    cinetype.loadFont("static/CinetMed.ttf", 120, 1.35, 512);
    cinetype.setKerning(true);
    cinetype_1.loadFont("static/CinetMed.ttf", 20);
    cinetype_2.loadFont("static/CinetMed.ttf", 10);
    cinetype_2.setLetterSpacing(0.95);
    
    /* Load Icon */
    syncicon.loadImage("static/sync.png");
    logoicon.loadImage("static/logo.png");
    
    /* Loading Configuration */
    loadXMLConfig();

    /* Starting UP Configuration Thread */
    CN.start(config.multicastIp, config.baseport);
   
    /* Starting UP Movie Thread */
    MO.start();
    
    /* Gui Stuff */

    gui = new ofxUICanvas("Automatic Cinema Player");        //Creates a canvas at (0,0) using the default width

    gui->setPosition(ofGetWidth()/30,ofGetWidth()/30);
//    gui->setFont("static/CinetGui.ttf");

    ofxUIColor cb = ofxUIColor( 0,0,0,128 );
    ofxUIColor co = ofxUIColor( 255,0,0 );
    ofxUIColor coh = ofxUIColor( 0,255,0,0 );
    ofxUIColor cf = ofxUIColor( 0,255,0 );
    ofxUIColor cfh = ofxUIColor( 255 );
    ofxUIColor cp = ofxUIColor( 0,0,0,0 );
    ofxUIColor cpo = ofxUIColor( 0,0,0,0 );
    gui->setUIColors( cb, co, coh, cf, cfh, cp, cpo );
    gui->setPosition(ofGetWidth()/30,ofGetWidth()/30);
    gui->setDimensions(ofGetWidth()-(ofGetWidth()/15),ofGetHeight()-(ofGetHeight()/15));
    
    gui->setDrawWidgetPadding(false);
    gui->setPadding(3);
    gui->setGlobalSpacerHeight(0);
    
#ifdef TARGET_OF_IPHONE
	gui->addButton("Close", true);
#endif

    gui->addSpacer();
    gui->addSpacer();

    gui->addLabel("Screen Setting");
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Fullscreen",    config.fullscreen);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Verbose",       config.verbose);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Logo",          config.drawLogo);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addSpacer();
    gui->addSpacer();
    
    gui->addLabel("Multicast IP");
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addTextInput("Multicast",         config.multicastIp);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui->addLabel("Port");
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addTextInput("Port",       ofToString(config.baseport));
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addSpacer();
    gui->addSpacer();

    gui->addLabel("Supported Media");
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Video",         config.hasVideo);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Audio",         config.hasAudio);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("Text",          config.hasText);
	gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);

    gui->toggleVisible();
    ofSleepMillis(500);
    
}

// Listeners
//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
#ifdef TARGET_OF_IPHONE
    if(e.getName() == "Close" && e.getKind()==OFX_UI_WIDGET_BUTTON)
    {
        gui->toggleVisible();
    }
#endif

    if(e.getName() == "Video" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
       ofxUIToggle *t = e.getToggle();
       config.hasVideo = t->getValue();
    }
    if(e.getName() == "Audio" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
        ofxUIToggle *t = e.getToggle();
        config.hasAudio = t->getValue();
    }
    if(e.getName() == "Text" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
        ofxUIToggle *t = e.getToggle();
        config.hasText = t->getValue();
    }
    if(e.getName() == "Logo" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
        config.drawLogo = !config.drawLogo;
    }
    if(e.getName() == "Verbose" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
        ofxUIToggle *t = e.getToggle();
        MO.setPreview(t->getValue());
        config.verbose = t->getValue();
        if (config.verbose) {
                ofSetLogLevel(OF_LOG_VERBOSE);
        }
        else {
                ofSetLogLevel(OF_LOG_ERROR);
        }
    }
    if(e.getName() == "Fullscreen" && e.getKind()==OFX_UI_WIDGET_TOGGLE)
    {
        ofxUIToggle *t = e.getToggle();
        ofSetFullscreen(t->getValue());
        config.fullscreen = t->getValue();
    }
    if(e.getName() == "Multicast")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        config.multicastIp = ti->getTextString();
        SC.configNet(config.multicastIp, config.baseport);
    }
    if(e.getName() == "Port")
    {
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        config.baseport = ofToInt(ti->getTextString());
        SC.configNet(config.multicastIp, config.baseport);
    }
    if(e.getName() == "Channel Settings" && e.getKind()==OFX_UI_WIDGET_DROPDOWNLIST)
    {
        config.channel.clear();
        ofxUIDropDownList *d = (ofxUIDropDownList *) e.widget;
        vector<ofxUIWidget *> selected = d->getSelected();
        for(vector<ofxUIWidget *>::iterator it = selected.begin(); it != selected.end(); ++it)
        {
           ofxUILabelToggle *lt = (ofxUILabelToggle *) (*it);
           config.channel.push_back( lt->getName());
        }
    }
    
    // Storing Configuration
    ofxXmlSettings x;
    x.addTag("settings");
    x.pushTag("settings");
    x.addTag("support_media");
    x.pushTag("support_media");
    x.setValue("video",   config.hasVideo );
    x.setValue("audio",   config.hasAudio );
    x.setValue("text",    config.hasText );
    x.popTag();
    x.setValue("fullscreen",            config.fullscreen );
    x.setValue("drawlogo",              config.drawLogo );
    x.setValue("verbose",               config.verbose );
    x.setValue("baseport",              config.baseport);
    x.setValue("ip",                    config.multicastIp);
    x.addTag("channel");
    x.pushTag("channel");
    for (int i=0; i<config.channel.size(); ++i) {
        x.setValue("name",      config.channel[i], i);
        cout << config.channel[i] << endl;
    }
    x.popTag();
    x.popTag();

#ifdef TARGET_OF_IPHONE
    x.saveFile(ofxiOSGetDocumentsDirectory() + "playersettings.xml");
#else
    x.saveFile("playersettings.xml");
#endif

    /* Update Listener Thread */
    if (SC.isThreadRunning()) {
        SC.updateConfig(config.channel, config.hasVideo, config.hasAudio, config.hasText);
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    /* Waiting for Config */
    static bool waitforconfig = true;

    if (CN.isConfigured() && waitforconfig) {
        if (CN.isError()) {
            std::cout  << "- Error Parsing Configuration message from Server." << endl;
            exit();
        }
        else {
            config.serverconfig = CN.getConfiguration();
            std::cout  << "+ Received Server Configuration" << endl;
            std::cout  << "  API          : " << config.serverconfig[0] << endl;
            std::cout  << "  Session      : " << config.serverconfig[1] << endl;
            std::cout  << "  FPS          : " << config.serverconfig[2] << endl;
            std::cout  << "  Preroll      : " << config.serverconfig[3] << endl;
            std::cout  << "  Avail. Chnl  : " << config.serverconfig[4] << endl;
            std::cout  << "  MS per Line  : " << config.serverconfig[5] << endl;
            std::cout  << "  Line p Scrn  : " << config.serverconfig[6] << endl;

            
            /* Updating GUI */
            gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
            gui->addSpacer();
            vector<string> avail_channels = ofSplitString(config.serverconfig[4], "<:>");
            ofxUIDropDownList *ddl;
            ddl = gui->addDropDownList("Channel Settings", avail_channels);
            ddl->setAllowMultiple(true);

            vector<ofxUILabelToggle *> &selected = ddl->getToggles();
            for(int i = 0; i < selected.size(); i++)
            {
                bool toggle;
                if(std::find(config.channel.begin(), config.channel.end(), selected[i]->getName())!=config.channel.end()) toggle = true;
                else toggle = false;
                selected[i]->setValue(toggle);
            }

            // Network Ready. Start to Synchronize Clips with (Web)Server
            TO.start(config.serverconfig[0],config.serverconfig[1]);
            std::cout  << "+ Clip Synchronisation Thread started"  << endl;
            
            // Subtitle Thread Started
            SU.start(config.hasText, config.serverconfig[5], config.serverconfig[6]);
            std::cout  << "+ Subtitle Thread started"  << endl;
            
            // Clip Trigger Thread
            SC.start(config.multicastIp, config.baseport, config.channel, TO, MO, SN, SU, config.hasVideo, config.hasAudio, config.hasText);
            std::cout  << "+ Trigger Listener Thread started" << endl;
            
            waitforconfig = false;
        }
    }
    
    if (ofGetElapsedTimeMillis()-mouseTimer>15000 && cursorflag) {
        ofHideCursor();
        cursorflag = false;
    }
    
    MO.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
     /* Draw "Wait for Config" Screen */
    
    if (CN.isThreadRunning()) {
        drawWaitForConfig("Connecting to Server");
    }

    /* Draw Movie Screen */
    
    else {
        if (config.hasVideo) {
            if (!MO.draw()) {
                drawWaitForConfig("Waiting for Footage");
            }
        }
    }

    /* Draw "Syncing Files" Screen */
    
    if (TO.active() && TO.downloadFile() != "") {
        drawSyncFiles(TO.downloadFile());
    }
    
    /* Draw Logo */
    if (config.drawLogo) {
        logoicon.draw(ofGetWidth()-60, 10, 50, 32);
    }
    
    /* Draw Subtitles */
    if (SU.getSubtitle() != "" && config.hasText) {
        ofPushStyle();
        ofRectangle box = cinetype.getBBox(SU.getSubtitle(), ofGetHeight()/30, 0, 0);
        ofColor(0);
        cinetype.drawMultiLine(SU.getSubtitle(), ofGetHeight()/30, (ofGetWidth()-box.width)/2, ofGetHeight()-(box.height+(ofGetHeight()/20)));
        ofColor(255);
        cinetype.drawMultiLine(SU.getSubtitle(), ofGetHeight()/30, (ofGetWidth()-box.width)/2, ofGetHeight()-(box.height+(ofGetHeight()/20)));

        ofPopStyle();
    }
    
    #ifdef TARGET_OF_IPHONE
    if (config.verbose) {
        ofPushStyle();
        ofSetColor(255, 0, 0);

        ofDrawBitmapString("Directory : " + ofxiOSGetDocumentsDirectory(), 10, 15);
        ofPopStyle();
    }
    #endif

}


#ifdef TARGET_OF_IPHONE
void ofApp::touchDown(ofTouchEventArgs & touch){
    if (!gui->isVisible()) {
        gui->toggleVisible();
    }
}

#else
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
            gui->toggleVisible();
            break;
    }
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    if (ofGetElapsedTimeMillis()>1000) {
        gui->setPosition(w/30,w/30);
        gui->setDimensions(w-(w/15),h-(w/15));
        if (gui->isVisible()) {
            gui->draw();
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    mouseTimer = ofGetElapsedTimeMillis();
    cursorflag = true;
    ofShowCursor();
}
#endif



//--------------------------------------------------------------
void ofApp::exit() {
	std::cout  << "+ Exiting now" << endl;
    delete gui;
    // stop the threads
	if (MO.isThreadRunning()) {
		MO.stopThread();
        MO.waitForThread();
        std::cout  << "- Movie Thread Stopped" << endl;
	}
	if (TO.isThreadRunning()) {
		TO.stopThread();
        TO.waitForThread();
        std::cout  << "- Download Thread Stopped" << endl;
	}
	if (SN.isThreadRunning()) {
		SN.stopThread();
        SN.waitForThread();
        std::cout  << "- Sound Thread Stopped" << endl;
	}
	if (SC.isThreadRunning()) {
		SC.stopThread();
        SC.waitForThread();
        std::cout  << "- Network Sync Thread Stopped" << endl;
        
	}
	if (SU.isThreadRunning()) {
		SU.stopThread();
        SU.waitForThread();
        std::cout  << "- Subtitle Thread Stopped" << endl;
        
	}
	std::cout  << "Bye Bye!" << endl;
	ofExit();
}


