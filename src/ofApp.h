#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAudioUnit.h"
#include "ofGraphics.h"
#include "ofxSyphon.h"


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
//    ofPoint rec1pos,rec2pos,rec3pos;
//    ofPoint rec1c,rec2c,rec3c;
//    ofPoint circle1,circle2,circle3;
//    float rectlong;
//    float r1,r2,r3;

///////////visual works here/////////////////
    float theta;
    float recta1;
    float rectb1;
    float rectc1;
    
    void drawFboTest();
    void drawFboTest2();
    void drawFboTest3();
    
    ofFbo rgbaFboFloat;
    ofFbo rgbaFboFloat2;
    ofFbo rgbaFboFloat3;
    
    int fadeAmnt,fadeAmnt2,fadeAmnt3;
    vector<float> scale,scale2,scale3;
    vector<float> change,change2,change3;
    ofxIntSlider presure1,presure2,presure3;
    
    vector<float> times;
    vector<float>posx;
    vector<float>posy;
    vector<float>posz;
    float w,h,d;
    float distance,a;
    float speed;
    int NUM;
    float vol1,vol2,vol3;
    ofSpherePrimitive sphere;
    ofSpherePrimitive bigsphere1;
    ofVec3f bigspos;
    float smoothVolume;
    float supersmoothVolume;
    
    ofMesh meshOriginal;
    ofMesh meshWarped;
    ofxSyphonServer mainOutputSyphonServer;
    ofxSyphonServer individualTextureSyphonServer;
    
    ofxSyphonClient mClient;
    ofImage backgrad;

    
///////////sound effect here/////////////////
    ofxAudioUnit compressor;
    ofxAudioUnit distortion;
    ofxAudioUnit delay;
    ofxAudioUnit filter;
    ofxAudioUnitFilePlayer source1, source2, source3;
    ofxAudioUnitMixer mixer;
    ofxAudioUnitOutput output;
    ofxAudioUnitTap tap1, tap2, tap3;
    float dismix,disdelay;
    float delaymix;
    float filter1,filter2,fliterball;
    
///////////fft effect here/////////////////
    ofxAudioUnitFftNode fft1, fft2, fft3;

    vector<float> fft1Spectrum;
    vector<float> fft2Spectrum;
    vector<float> fft3Spectrum;
    
    int fft1pos;
    int fft2pos;
    int fft3pos;
    float 	* fftSmoothed1;
    float 	* fftSmoothed2;
    float 	* fftSmoothed3;
    ofxPanel gui;
    
    ofEasyCam cam,cam2;
    ofxIntSlider camx,camy,camz,fov,high;
///////////serial effect here/////////////////
    
    ofSerial port;
    string serialMsg;
};
