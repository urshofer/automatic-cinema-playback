#ifdef TAGET_LINUX
#include "testApp.h"
/*

	AMCIN SERVER 2 - The Automatic Cinema Trigger Server

*/
//========================================================================
int main( ){
    cout << "gere" << endl;
    testApp amcin;
    amcin.start();
}
#else
#include "ofMain.h"
#include "guiApp.h"
int main( ){
    cout << "Starting GUI" << endl;

#ifdef TARGET_OF_IPHONE
    
    
    ofAppiOSWindow * iOSWindow = new ofAppiOSWindow();
    iOSWindow->enableAntiAliasing(4);
    iOSWindow->enableRetina();
    iOSWindow->enableRendererES2();
    ofSetupOpenGL(iOSWindow, 1024,768,OF_FULLSCREEN);
#else
    ofSetupOpenGL(640, 360, OF_WINDOW);
#endif
    
    ofRunApp( new ofApp());
}
#endif