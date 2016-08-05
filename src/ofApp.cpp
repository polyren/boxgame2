#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
///////////serial here/////////////////
    port.listDevices();
    port.setup(0,9600);

    gui.setup("settings", "settings.xml");
//    gui.add(presure1.setup("presure1",0,0,1024));
//    gui.add(presure2.setup("presure2",0,0,1024));
//    gui.add(high.setup("high",0,0,ofGetHeight()));
    gui.add(camx.setup("camx",0,0,360));
    gui.add(camy.setup("camy",0,0,360));
    gui.add(camz.setup("camz",0,0,360));
//    gui.add(fov.setup("fov",0,0,100));
    ofBackground(0);
    ofDisableSmoothing();
    ofEnableAlphaBlending();
    
///////////sound effect here/////////////////
    
    source1.setFile(ofFilePath::getAbsolutePath("bass.mp3"));
    source2.setFile(ofFilePath::getAbsolutePath("gitar.mp3"));
    source3.setFile(ofFilePath::getAbsolutePath("vocal.mp3"));
    
    distortion.setup(kAudioUnitType_Effect, kAudioUnitSubType_Distortion);
    delay.setup(kAudioUnitType_Effect, kAudioUnitSubType_Delay);
    filter.setup(kAudioUnitType_Effect, kAudioUnitSubType_LowPassFilter);
    
    source1.connectTo(distortion).connectTo(tap1);
    source2.connectTo(delay).connectTo(tap2);
    source3.connectTo(filter).connectTo(tap3);
    

    tap1.connectTo(fft1);
    tap2.connectTo(fft2);
    tap3.connectTo(fft3);

///////////fft here/////////////////
    mixer.setInputBusCount(3);
    fft1.connectTo(mixer, 0);
    fft2.connectTo(mixer, 1);
    fft3.connectTo(mixer, 2);

    fft1pos=100;
    fft2pos=100;
    fft3pos=180;
    
    compressor.setup(kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor);
    
    mixer.connectTo(compressor).connectTo(output);
    
    output.start();
    
    source1.loop();
    source2.loop();
    source3.loop();
    
    fftSmoothed1 = new float[8192];
    fftSmoothed2 = new float[8192];
    fftSmoothed3 = new float[8192];
    for (int i = 0; i < 8192; i++){
        fftSmoothed1[i] = 0;
        fftSmoothed2[i] = 0;
        fftSmoothed3[i] = 0;
    }
    backgrad.load("fillgray.jpg");

///////////fbo here/////////////////
    rgbaFboFloat.allocate(400, 400, GL_RGBA32F_ARB);
    rgbaFboFloat.begin();
    ofClear(255,255,255, 0);
    rgbaFboFloat.end();
    
    rgbaFboFloat2.allocate(400, 400, GL_RGBA32F_ARB);
    rgbaFboFloat2.begin();
    ofClear(255,255,255, 0);
    rgbaFboFloat2.end();
    
    rgbaFboFloat3.allocate(400, 400, GL_RGBA32F_ARB);
    rgbaFboFloat3.begin();
    ofClear(255,255,255, 0);
    rgbaFboFloat3.end();
    ofSetVerticalSync(true);
///////////visual works for gitar here/////////////////

    NUM=25;
    times.resize(NUM);
    posx.resize(NUM);
    posy.resize(NUM);
    posz.resize(NUM);
    for (int i=0; i<times.size(); i++) {
        times[i]=(ofRandom(10));
    }
    
    w=ofGetWidth();
    h=ofGetHeight();
    d=ofGetWidth();
    speed=0;
    smoothVolume=0;
    meshOriginal = meshWarped = ofMesh::sphere(100, 15);
    
    
    
    
}

//--------------------------------------------------------------
void ofApp::update(){

///////////serial here/////////////////
    while (port.available()) {
        int s = port.readByte();
        if((unsigned char) s == '\n' ){
//            cout << presure1 << endl;
//            cout << presure2 << endl;
//            cout << presure3 << endl;
//            cout << serialMsg << endl;
            if (serialMsg.length()>0) {
                if (serialMsg.at(0) == 'A') {
                    vector<string> numbers = ofSplitString(serialMsg, "\t");
                    if (numbers.size() >= 3) {
                    presure1 = ofToFloat(numbers[1]);
                        
                    presure2 = ofToFloat(numbers[2]);
                        
                    presure3 = ofToFloat(numbers[3]);
                        
                    }
                }
                
            }serialMsg = "";
        }
        else{
            serialMsg +=(unsigned char)s;
        }
    }
    
    float pretovol1 = ofMap(presure1, 0, 1024, 0, 1);
    float pretovol2 = ofMap(presure2, 0, 1024, 0, 1);
    float pretovol3 = ofMap(presure3, 0, 1024, 0, 1);
    mixer.setInputVolume(pretovol1, 0);
    mixer.setInputVolume(pretovol2, 1);
    mixer.setInputVolume(pretovol3, 2);
    
///////////fft here/////////////////
    fft1.getAmplitude(fft1Spectrum);
    fft2.getAmplitude(fft2Spectrum);
    fft3.getAmplitude(fft3Spectrum);
    recta1=ofMap(presure1, 0, 1024, 400, 0);
    rectb1=ofMap(presure2, 0, 1024, 400, 0);
    rectc1=ofMap(presure3, 0, 1024, 400, 0);
    theta +=0.05055;

///////////sound effect changes here/////////////////
    
    dismix=ofMap(presure1, 0, 1024, 60, 0);
    disdelay=ofMap(presure1, 0, 640, 200, 0);
    
    delaymix=ofMap(presure2, 0, 1024, 50, 0);
    
    filter1=ofMap(presure3, 0, 1024, 365, 21830);
    filter2=ofMap(presure3, 0, 1024, 34.79, 1.46);
    
    fliterball=filter1+20*cos(theta)*10;
    
    if (cos(theta)>0.999 || cos(theta)<-0.999 ) {
        float temp1 = 1;
        scale.push_back(temp1);
        change.push_back(0);
        scale2.push_back(temp1);
        change2.push_back(0);
        scale3.push_back(temp1);
        change3.push_back(0);
    }
    
    AudioUnitSetParameter(distortion, kDistortionParam_FinalMix, kAudioUnitScope_Global, 0, dismix, 0);
//    AudioUnitSetParameter(distortion, kDistortionParam_Delay, kAudioUnitScope_Global, 0, disdelay, 0);
    //
    AudioUnitSetParameter(delay, kDelayParam_WetDryMix, kAudioUnitScope_Global, 0, delaymix, 0);
    
    AudioUnitSetParameter(filter, kLowPassParam_CutoffFrequency, kAudioUnitScope_Global, 0, fliterball, 0);
    AudioUnitSetParameter(filter, kLowPassParam_Resonance, kAudioUnitScope_Global, 0, filter2, 0);


///////////fft updates here/////////////////
    
    if (fft1Spectrum.size()>0) {
        vol1=fft1Spectrum[fft1pos];
        speed=ofMap(vol1,0,1,0.000001,0.02);
        fftSmoothed1[fft1pos] *= 0.96f;
        if (fftSmoothed1[fft1pos] < vol1) fftSmoothed1[fft1pos] = 1-vol1;
    }
    if (fft2Spectrum.size()>0) {
    }
    if (fft3Spectrum.size()>0) {
        vol3=fft3Spectrum[fft3pos];
        fftSmoothed3[fft3pos] *= 0.96f;
        if (fftSmoothed3[fft3pos] < vol3) fftSmoothed3[fft3pos] = 1-vol3;

        
    }
    
    float smoothing = 0.78;
    
/////////////vol2 box3 updates to fft////////////////
//    smoothVolume = (smoothVolume * smoothing + fftSmoothed2[fft2pos]  * (1 -smoothing)) ;
    
    vector<ofVec3f> & vertsOriginal = meshOriginal.getVertices();
    vector<ofVec3f> & vertsWarped = meshWarped.getVertices();
    int numOfVerts = meshOriginal.getNumVertices();
    
    float * audioData = new float[numOfVerts];
    
    float meshDisplacement = 100;
    
    if(fft2Spectrum.size()>0){
    for(int i=0; i<numOfVerts; i++) {
        if (fft2Spectrum[i]>0) {
        fftSmoothed2[i] *= 0.96f;
        if (fftSmoothed2[i] < fft2Spectrum[i]) fftSmoothed2[i] = 1-fft2Spectrum[i];
        audioData[i]= fftSmoothed2[i];
        float audioValue = audioData[i];
        ofVec3f & vertOriginal = vertsOriginal[i];
        ofVec3f & vertWarped = vertsWarped[i];
        ofVec3f direction = vertOriginal.getNormalized();
        vertWarped = vertOriginal + direction * meshDisplacement * audioValue;
        }
    }
}

    
    ///////////fbo box1 changes here/////////////////
    rgbaFboFloat.begin();
    drawFboTest();
    rgbaFboFloat.end();
    rgbaFboFloat2.begin();
    drawFboTest2();
    rgbaFboFloat2.end();
    rgbaFboFloat3.begin();
    drawFboTest3();
    rgbaFboFloat3.end();
    


}

//--------------------------------------------------------------
void ofApp::draw(){
//    ofEnableAlphaBlending();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cam.setPosition(ofPoint(ofGetWidth()/2-600,-400,-2600));
    cam.lookAt(ofPoint(ofGetWidth()/2,ofGetHeight()/2,0));
    cam.setOrientation(ofPoint(camx,camy,camz));
    cam.setFov(21);
    cam.begin();
//    cam2.begin();
    
//    ofSetColor(255,255,255);
//    ofPushMatrix();
//    ofNoFill();
//    ofSetColor(255, 0, 0);
//    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
//    ofPopMatrix();
///////////fbo1 here/////////////////
    ofPushMatrix();
    ofSetColor(152, 194, 49,250);
//    ofSetColor(255);
    ofRotate(-90);
    ofTranslate(-ofGetHeight(),ofGetWidth()/2-565,0);
    ofRotate(45,1,0,0);
    ofTranslate(400,400);
    rgbaFboFloat.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(152, 194, 49,190);
//    ofSetColor(255);
    ofTranslate(ofGetWidth()/2+565,ofGetHeight()-400);
    ofRotate(45, 0, 1, 0);
    rgbaFboFloat.draw(-800,-400);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(152, 194, 49,230);
//    ofSetColor(255);
    ofTranslate(ofGetWidth()/2+565/2,ofGetHeight()-400,565/2);
    ofRotate(-90, 1, 0, 0);
    ofRotate(135, 0, 0, 1);
    rgbaFboFloat.draw(0,0);
    ofPopMatrix();
    
///////////fbo2 here/////////////////
    
    ofPushMatrix();
    ofSetColor(16,167,222,250);
//    ofSetColor(255);
    ofRotate(-90);
    ofTranslate(-ofGetHeight(),ofGetWidth()/2,0);
    ofRotate(45,1,0,0);
    rgbaFboFloat2.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(16,167,222,190);
//    ofSetColor(255);
    ofTranslate(ofGetWidth()/2+565,ofGetHeight()-400);
    ofRotate(45, 0, 1, 0);
    rgbaFboFloat2.draw(-400,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(16,167,222,220);
//    ofSetColor(255);
    ofTranslate(ofGetWidth()/2+565,ofGetHeight());
    ofRotate(-90, 1, 0, 0);
    ofRotate(135, 0, 0, 1);
    rgbaFboFloat2.draw(0,0);
    ofPopMatrix();

///////////fbo3 here/////////////////
    
    ofPushMatrix();
    ofSetColor(230, 27, 131,250);
    ofRotate(-90);
    ofTranslate(-ofGetHeight(),ofGetWidth()/2-565,0);
    ofRotate(45,1,0,0);
    rgbaFboFloat3.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(230, 27, 131,190);
    ofTranslate(ofGetWidth()/2,ofGetHeight()-400);
    ofRotate(45, 0, 1, 0);
    rgbaFboFloat3.draw(-400,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(230, 27, 131,220);
    ofTranslate(ofGetWidth()/2,ofGetHeight());
    ofRotate(-90, 1, 0, 0);
    ofRotate(135, 0, 0, 1);
    rgbaFboFloat3.draw(0,0);
    ofPopMatrix();
    
    
///////////fft vocal here/////////////////
    ofEnableDepthTest();
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2-282, ofGetHeight()/2+200,-282);
    ofRotate(45, 0, 1, 0);
    ofFill();
    ofSetColor(255,255,255,100);
    for(int i = 0; i <3; i++){
        for(int j = 0; j < 3 ; j++){
            float vocalvol1 = ofMap(fftSmoothed3[fft3pos], 0, 1, 50, 200);
            float h = vocalvol1*(1-((i+j)*0.2));
            ofDrawBox(0+i*40, h/2, 0+j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0+i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0+j*40, 80, h, 80);
        }
    }
    ofNoFill();
    ofSetColor(28,168,220);
    ofSetLineWidth(1);
    for(int i = 0; i <3; i++){
        for(int j = 0; j < 3 ; j++){
            float vocalvol2 = ofMap(fftSmoothed3[fft3pos], 0, 1, 50, 200);
            float h = vocalvol2*(1-((i+j)*0.2));
            ofDrawBox(0+i*40, h/2, 0+j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0+i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0+j*40, 80, h, 80);
        }}
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2-282, ofGetHeight()/2+200,-282);
    ofRotate(180, 1, 0, 0);
    ofRotate(45, 0, 1, 0);
    ofFill();
    ofSetColor(255,255,255,100);
    for(int i = 0; i <3; i++){
        for(int j = 0; j < 3 ; j++){
            float vocalvol1 = ofMap(fftSmoothed3[fft3pos], 0, 1, 50, 200);
            float h = vocalvol1*(1-((i+j)*0.2));
            ofDrawBox(0+i*40, h/2, 0+j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0+i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0+j*40, 80, h, 80);
        }
    }
    ofNoFill();
    ofSetColor(28,168,220);
    ofSetLineWidth(1);
    for(int i = 0; i <3; i++){
        for(int j = 0; j < 3 ; j++){
            float vocalvol2 = ofMap(fftSmoothed3[fft3pos], 0, 1, 50, 200);
            float h = vocalvol2*(1-((i+j)*0.2));
            ofDrawBox(0+i*40, h/2, 0+j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0+i*40, h/2, 0-j*40, 80, h, 80);
            ofDrawBox(0-i*40, h/2, 0+j*40, 80, h, 80);
        }}
    ofPopMatrix();
    ofDisableDepthTest();
///////////fft gitar here/////////////////
    
    vector<ofVec3f> & vertsOriginal = meshOriginal.getVertices();
    vector<ofVec3f> & vertsWarped = meshWarped.getVertices();
    int numOfVerts = meshOriginal.getNumVertices();
    
    
    ofDisableDepthTest();
    ofPushMatrix();
    float meshDisplacement = 100;
    ofSetColor(230, 27, 131);
    ofTranslate(ofGetWidth()/2+282,ofGetHeight()-200);
    ofScale(1.1,1.1,1.1);
    meshWarped.drawVertices();
    ofPopMatrix();
    
    ofPushMatrix();
    ofSetColor(253, 255, 255,100);
    ofTranslate(ofGetWidth()/2+282,ofGetHeight()-200);
    meshWarped.drawFaces();
    ofPopMatrix();
    
    
///////////fft bass here/////////////////
    ofPushMatrix();
    ofTranslate(0, 0);
    for (int i=1; i<times.size(); i++) {
        times[i]+=speed;
        posx[i] =ofNoise(times[i])*565*1.45;
        posy[i] =ofNoise(times[i]+1)*400*1.45;
        posz[i] =ofNoise(times[i]+2)*565;
        for (int j =1;j<NUM;j++) {
            if(j>i){
                for (int k =1;k<NUM;k+=10) {
                    if(k>j){
                        ofSetLineWidth(1);
                        float touming1=ofMap(fftSmoothed1[fft1pos], 0, 1, 0, 225);
                        ofEnableDepthTest();
                        ofSetColor(255, 255,255,touming1);
                        ofFill();
                        ofPushMatrix();
                        ofTranslate(ofGetWidth()/2-282*1.4, ofGetHeight()/2-282*1.5,282);
                        ofDrawTriangle(posx[i], posy[i], posz[i], posx[j], posy[j],posz[j], posx[k], posy[k], posz[k]);
                        ofPopMatrix();
                        ofDisableDepthTest();
                    }
                }
                ofVec3f p1( posx[i], posy[i],posz[i] );
                ofVec3f p2( posx[j], posy[j],posz[j] );
                float d= ofDist(posx[i], posy[i], posx[j], posy[j]);
                float a=300-(d*3);
                ofPushMatrix();
                ofTranslate(ofGetWidth()/2-282*1.4, ofGetHeight()/2-282*1.5,282);
                ofSetColor(152, 194, 49,a);
                ofSetLineWidth(2);
                ofDrawLine(p1, p2);
                ofPopMatrix();
            }
        }
        
    }
    ofPopMatrix();
    cout<<vol1<<endl;
    cam.end();
//    cam2.end();
    
    mainOutputSyphonServer.publishScreen();
//    individualTextureSyphonServer.publishTexture(&fbo.getTexture());
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == '1') distortion.showUI();
    if(key == '2') delay.showUI();
    if(key == '3') filter.showUI();
    //    if(key == 'a') distortion.saveCustomPreset("distortion_setting");
    if(key == 'b') delay.saveCustomPreset("delay_setting");
    //    if(key == 'c') filter.saveCustomPreset("filter_setting");
    if(key == 'd') distortion.loadCustomPreset("distortion_setting");
    if(key == 'e') delay.loadCustomPreset("delay_setting");
    if(key == 'f') filter.loadCustomPreset("filter_setting");
    if(key == 'r') return;
    
}



//--------------------------------------------------------------
void ofApp::drawFboTest(){
    fadeAmnt = 10;
    
    ofFill();
    ofSetColor(255, 255, 255,fadeAmnt);
    ofDrawRectangle(0,0,400,400);
    
    ofNoFill();
    ofSetColor(255, 255, 255);
    ofSetLineWidth(4);
    
    for (int i=1; i<scale.size()+1; i++) {
        
        scale[i-1]=1-(change[i-1]*0.25);
        
        ofPushMatrix();
        ofTranslate(0,0,0);
        if (scale[i-1]>0) {
            ofScale(scale[i-1], scale[i-1]);
            
            ofDrawRectangle(0, 0, 400,400);
            change[i-1]=change[i-1]+0.03;
            
        }
        
        if(scale[i-1]== 0){
            scale.erase(scale.begin()+i);
            i--;
        }
        
        
        ofPopMatrix();
    }
//    ofFill();
//    ofSetColor(255, 255, 255);
//    ofDrawRectangle(0, 0, recta1, recta1);
    backgrad.draw(0, 0, recta1, recta1);
    }

void ofApp::drawFboTest2(){
    fadeAmnt2 = 10;
    
    ofFill();
    ofSetColor(255,255,255,fadeAmnt2);
    ofDrawRectangle(0,0,400,400);
    
    ofNoFill();
    ofSetColor(255,255,255);
    ofSetLineWidth(4);
    
    for (int i=1; i<scale2.size()+1; i++) {
        
        scale2[i-1]=1-(change2[i-1]*0.25);
        
        ofPushMatrix();
        ofTranslate(0,0,0);
        if (scale2[i-1]>0) {
            ofScale(scale2[i-1], scale2[i-1]);
            
            ofDrawRectangle(0, 0, 400,400);
            change2[i-1]=change2[i-1]+0.03;
            
        }
        
        if(scale2[i-1]== 0){
            scale2.erase(scale2.begin()+i);
            i--;
        }
        
        
        ofPopMatrix();
    }
//    ofFill();
//    ofSetColor(255, 255, 255);
//    ofDrawRectangle(0, 0, rectb1, rectb1);
     backgrad.draw(0, 0, rectb1, rectb1);
}

void ofApp::drawFboTest3(){
    fadeAmnt3 = 10;
    
    ofFill();
    ofSetColor(255, 255, 255,fadeAmnt3);
    ofDrawRectangle(0,0,400,400);
    
    ofNoFill();
    ofSetColor(255, 255, 255);
    ofSetLineWidth(4);
    
    for (int i=1; i<scale3.size()+1; i++) {
        
        scale3[i-1]=1-(change3[i-1]*0.25);
        
        ofPushMatrix();
        ofTranslate(0,0,0);
        if (scale3[i-1]>0) {
            ofScale(scale3[i-1], scale3[i-1]);
            
            ofDrawRectangle(0, 0, 400,400);
            change3[i-1]=change3[i-1]+0.03;
            
        }
        
        if(scale3[i-1]== 0){
            scale3.erase(scale3.begin()+i);
            i--;
        }
        
        
        ofPopMatrix();
    }
//    ofFill();
//    ofSetColor(255, 255, 255);
//    ofDrawRectangle(0, 0, rectc1, rectc1);

 backgrad.draw(0, 0, rectc1, rectc1);}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}
