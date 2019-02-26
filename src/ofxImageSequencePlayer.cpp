//
//  ofxImageSequencePlayer.cpp
//  Created by Lukasz Karluk on 21/06/13.
//  http://julapy.com/blog
//
//  edited by Brian Eschrich 11/12/14
//

#include "ofxImageSequencePlayer.h"

const std::vector<std::string> ofxImageSequencePlayer::allowedExt = {"jpg", "jpeg", "png"};

bool ofxImageSequencePlayer::isAllowedExt(std::string ext) {
    return std::find(std::begin(allowedExt),  std::end(allowedExt), ext) != std::end(allowedExt);
}

ofxImageSequencePlayer::ofxImageSequencePlayer() {
    bLoaded = false;
    bPlaying = false;
    bPaused = false;
    bNewFrame = false;
    
    frameIndex = 0;
    frameLastIndex = 0;
    framesTotal = 0;
    position = 0;
    duration = 0;
    time = 0;
    
    fps = 30;
    speed = 1;
    loopType = OF_LOOP_NONE;
    isPlayingBackwards = false;
    
    playerTex = nullptr;
    tex.resize(1);
}

ofxImageSequencePlayer::~ofxImageSequencePlayer() {
    close();
}

void ofxImageSequencePlayer::setFrameRate(float value) {
    fps = value;
    duration = framesTotal / fps;
}

bool ofxImageSequencePlayer::load(std::string name) {
    close();
    
    ofFile file(name);
    std::string dirPath = file.getEnclosingDirectory();
    std::string ext = file.getExtension();
    if (file.isDirectory()) {
        dirPath = name;
        ofDirectory dir(dirPath);
        int numOfFiles = dir.listDir();
        for(int i=0; i<numOfFiles; i++) {
            ext = dir.getFile(i).getExtension();
            if (isAllowedExt(ext)) {
                std::pair<std::string, std::string> props = getPrefixAndNumber(name);
                imageSequencePaths = getSequence(dirPath, props.first, props.second, ext);
                break;
            }
        }
    }
    else {
        std::pair<std::string, std::string> props = getPrefixAndNumber(name);
        imageSequencePaths = getSequence(dirPath, props.first, props.second, ext);
    }
    
    bLoaded = imageSequencePaths.size() > 0;
    if(bLoaded == false) {
        return false;
    }
    
    bLoaded = load(imageSequencePaths);
    
    return bLoaded;
}

std::pair<std::string, std::string> ofxImageSequencePlayer::getPrefixAndNumber(std::string name) {
    ofFile file(name);
    std::string number = "";
    std::string prefix = file.getBaseName();
    while (prefix.length() > 0 && isdigit(prefix[prefix.length() - 1])) {
        number = prefix[prefix.length()-1] + number;
        prefix = prefix.substr(0, prefix.length() - 1);
    }
    return std::pair<std::string, std::string>(prefix, number);
}

std::vector<std::string> ofxImageSequencePlayer::getSequence(std::string dirPath, std::string prefix, std::string number, std::string ext) {
    std::vector<std::string> images;
    ofDirectory dir(dirPath);
    dir.allowExt(ext);
    int numOfFiles = dir.listDir();
    dir.sort();
    for(int i=0; i<numOfFiles; i++) {
        std::string baseName = dir.getFile(i).getBaseName();
        if (baseName.substr(0, prefix.length()) == prefix && baseName.substr(prefix.length(), number.length()) == number) {
            images.push_back(dir.getPath(i));
            number = ofToString(ofToInt(number) + 1, number.length(), '0');
        }
    }
    return images;
}

bool ofxImageSequencePlayer::load(const std::vector<std::string> & imagePaths) {

    for(int i=0; i<imagePaths.size(); i++) {
        string imagePath = imagePaths[i];
        ofTexture * imageTexture = new ofTexture();
        bool bLoaded = ofLoadImage(*imageTexture, imagePath);
        if(bLoaded == false) {
            ofLog(OF_LOG_ERROR, "ofxImageSequencePlayer::load, could not load : " + imagePath);
            delete imageTexture;
            imageTexture = NULL;
            continue;
        }
        
        imageSequenceTextures.push_back(imageTexture);
    }
    
    bLoaded = imageSequenceTextures.size() > 0;
    if(bLoaded == false) {
        return false;
    }
    
    frameIndex = 0;
    framesTotal = imageSequenceTextures.size();
    frameLastIndex = framesTotal - 1;
    
    duration = framesTotal / fps;
    
    bNewFrame = true;
    bUpdatePixels = true;
    
    playerTex = imageSequenceTextures[frameIndex];
    
    return bLoaded;
}

void ofxImageSequencePlayer::close() {
    bLoaded = false;
    bPlaying = false;
    bPaused = false;
    bNewFrame = false;
    bUpdatePixels = false;
    
    frameIndex = 0;
    frameLastIndex = 0;
    framesTotal = 0;
    position = 0;
    duration = 0;
    time = 0;
    
    imageSequencePaths.clear();
    
    for(int i=0; i<imageSequenceTextures.size(); i++) {
        delete imageSequenceTextures[i];
        imageSequenceTextures[i] = NULL;
    }
    imageSequenceTextures.clear();
    
    playerTex = nullptr;
    pixels.clear();
}

void ofxImageSequencePlayer::update() {
    bNewFrame = false;
    
    bool bUpdate = true;
    bUpdate = bUpdate && (isLoaded() == true);
    bUpdate = bUpdate && (isPlaying() == true);
    bUpdate = bUpdate && (isPaused() == false);
    
    if(bUpdate == false) {
        return;
    }
    
    // not sure adding the last frame duration is the most accurate approach here.
    // might need to rethink this.
    
    time += ofGetLastFrameTime();
    
    float newPos;
    newPos = ofMap(time, 0.0, duration, 0.0, 1.0);
    
    switch (loopType) {
        case OF_LOOP_NONE:
            if (newPos > 1) {
                newPos = 1;
            }
            break;
        case OF_LOOP_NORMAL:
            if (newPos > 1) {
                time -= duration;
                newPos -= 1;
            }
            break;
        case OF_LOOP_PALINDROME:
            if (!isPlayingBackwards && newPos >1) {
                newPos -= 1;
                time -= duration;
                newPos = 1 - newPos;
                isPlayingBackwards = true;
            }else if (isPlayingBackwards){
                newPos = 1.0 - newPos;
                if (newPos < 0) {
                    time -= duration;
                    newPos *= -1;
                    isPlayingBackwards = false;
                }
            }
            break;
    }
    
    setPosition(newPos);
}

bool ofxImageSequencePlayer::setPixelFormat(ofPixelFormat pixelFormat) {
    if(isLoaded() == false) {
        return false;
    }
    
    // TODO.
    return false;
}

ofPixelFormat ofxImageSequencePlayer::getPixelFormat() const {
    if(isLoaded() == false) {
        return OF_PIXELS_RGBA;
    }
    
    // TODO.
    return OF_PIXELS_RGBA;
}

void ofxImageSequencePlayer::play() {
    if(isLoaded() == false) {
        return;
    }

    if(getIsMovieDone()) {
        setFrame(0);
    }
    
    bPlaying = true;
    bPaused = false;
}

void ofxImageSequencePlayer::stop() {
    bPlaying = false;
    bPaused = false;

    time = 0;
    setPosition(0);
}

bool ofxImageSequencePlayer::isFrameNew() const {
    return bNewFrame;
}

ofPixels & ofxImageSequencePlayer::getPixels() {
    if (!bUpdatePixels) {
        return pixels;
    }
    
    playerTex->readToPixels(pixels);
    
    bUpdatePixels = false;
    return pixels;
}

ofTexture & ofxImageSequencePlayer::getTexture() {
    if(playerTex == nullptr){
        return tex[0];
    }else{
        return *playerTex;
    }
}

vector<ofTexture> & ofxImageSequencePlayer::getTexturePlanes(){
    if(playerTex != nullptr){
        tex.clear();
        tex.push_back(*playerTex);
    }
    return tex;
}

//---------------------------------------------------------------------------
const vector<ofTexture> & ofxImageSequencePlayer::getTexturePlanes() const{
    if(playerTex != nullptr){
        ofxImageSequencePlayer * mutThis = const_cast<ofxImageSequencePlayer*>(this);
        mutThis->tex.clear();
        mutThis->tex.push_back(*playerTex);
    }
    return tex;
}

void ofxImageSequencePlayer::setUseTexture(bool bUseTex) {
    
}

float ofxImageSequencePlayer::getWidth() const {
    if(isLoaded() == false) {
        return 0;
    }
    
    int w = getTexture().getWidth();
    return w;
}

float ofxImageSequencePlayer::getHeight() const {
    if(isLoaded() == false) {
        return 0;
    }
    
    int h = getTexture().getHeight();
    return h;
}

bool ofxImageSequencePlayer::isPaused() const {
    return bPaused;
}

bool ofxImageSequencePlayer::isLoaded() const {
    return bLoaded;
}

bool ofxImageSequencePlayer::isPlaying() const {
    return bPlaying;
}

float ofxImageSequencePlayer::getPosition() {
    return position;
}

float ofxImageSequencePlayer::getSpeed() {
    return speed;
}

float ofxImageSequencePlayer::getDuration() const {
    return duration;
}

bool ofxImageSequencePlayer::getIsMovieDone() {
    bool bFinished = (bPlaying == false) && (getCurrentFrame() == frameLastIndex);
    return bFinished;
}

float ofxImageSequencePlayer::getFrameRate() {
    return fps;
}

void ofxImageSequencePlayer::setPaused(bool bPause) {
    bPaused = bPause;
}

void ofxImageSequencePlayer::setPosition(float value) {
    int index = value * frameLastIndex;
    setFrame(index);
}

void ofxImageSequencePlayer::setVolume(float volume) {
    // not supported.
}

void ofxImageSequencePlayer::setLoopState(ofLoopType value) {
    loopType = value;
}

void ofxImageSequencePlayer::setSpeed(float value) {
    speed = value;
}

void ofxImageSequencePlayer::setFrame(int value) {
    if(isLoaded() == false) {
        return;
    }
    
    int index = ofClamp(value, 0, frameLastIndex);
    if(frameIndex == index) {
        return;
    }
    frameIndex = index;
    playerTex = imageSequenceTextures[frameIndex];
    bNewFrame = true;
    bUpdatePixels = true;
    
    position = frameIndex / (float)frameLastIndex;
}

int	ofxImageSequencePlayer::getCurrentFrame() const {
    return frameIndex;
}

int	ofxImageSequencePlayer::getTotalNumFrames() const {
    return framesTotal;
}

ofLoopType ofxImageSequencePlayer::getLoopState() {
    return loopType;
}

void ofxImageSequencePlayer::firstFrame() {
    setFrame(0);
}

void ofxImageSequencePlayer::nextFrame() {
    if(isLoaded() == false) {
        return;
    }
    int index = getCurrentFrame() + 1;
    if(index > frameLastIndex) {
        if(loopType == OF_LOOP_NONE) {
            index = frameLastIndex;
            if(isPlaying()) {
                stop();
            }
        } else if(loopType == OF_LOOP_NORMAL) {
            index = 0;
        }
    }
    if (loopType == OF_LOOP_PALINDROME) {
        if (isPlayingBackwards) {
            index = getCurrentFrame()-1;
            if (index<0) {
                index = 0;
                isPlayingBackwards = false;
            }
        }else if (index>frameLastIndex){
            index = frameLastIndex;
            isPlayingBackwards = true;
        }
    }
    
    setFrame(index);
}

void ofxImageSequencePlayer::previousFrame() {
    if(isLoaded() == false) {
        return;
    }
    
    int index = getCurrentFrame() - 1;
    if(index < 0) {
        if(loopType == OF_LOOP_NONE) {
            index = 0;
        } else if(loopType == OF_LOOP_NORMAL) {
            index = frameLastIndex;
        }
    }
    if (loopType == OF_LOOP_PALINDROME) {
        if (isPlayingBackwards) {
            index = getCurrentFrame()+1;
            if (index>frameLastIndex) {
                index = frameLastIndex;
                isPlayingBackwards = false;
            }
        }else if (index<0){
            index = 0;
            isPlayingBackwards = true;
        }
    }
    
    setFrame(index);
}

void ofxImageSequencePlayer::draw(float _x, float _y, float _w, float _h) const {
    ofGetCurrentRenderer()->draw(*this,_x,_y,_w,_h);
}
