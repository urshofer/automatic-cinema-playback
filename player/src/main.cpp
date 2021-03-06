#include "ofMain.h"
#include "ofApp.h"
#ifdef TARGET_LINUX
	#include "ofAppGlutWindow.h"
#endif

//========================================================================
int main( ){
    #ifdef TARGET_LINUX
		ofAppGlutWindow window;
		ofSetupOpenGL(&window, 640, 360, OF_WINDOW);
    #elif TARGET_OF_IPHONE
        ofSetupOpenGL(1024,768,OF_FULLSCREEN);
    #else
 //       ofSetupOpenGL(1280, 960, OF_GAME_MODE);
           ofSetupOpenGL(LED_WIDTH, LED_HEIGHT, OF_WINDOW);
    
    #endif
	ofRunApp( new ofApp());
}
