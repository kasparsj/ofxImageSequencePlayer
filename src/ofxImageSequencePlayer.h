//
//  ofxImageSequencePlayer.h
//  Created by Lukasz Karluk on 21/06/13.
//  http://julapy.com/blog
//

#pragma once

#include "ofMain.h"

class ofxImageSequencePlayer : public ofBaseVideoPlayer, public ofBaseVideoDraws {

public:
    
    ofxImageSequencePlayer();
    ~ofxImageSequencePlayer();
    
    void setFrameRate(float value);
    
    bool load(std::string name);
    bool load(const std::vector<std::string> & imagePaths);
    void close();
    void update();
	
	bool setPixelFormat(ofPixelFormat pixelFormat);
	ofPixelFormat getPixelFormat() const;
	
    void play();
    void stop();
	
    bool isFrameNew() const;
    ofPixels & getPixels();
    const ofPixels & getPixels() const {
        return const_cast<ofxImageSequencePlayer *>(this)->getPixels();
    }
    ofTexture & getTexture();
    const ofTexture & getTexture() const {
        return const_cast<ofxImageSequencePlayer *>(this)->getTexture();
    }
    vector<ofTexture> & getTexturePlanes();
    const vector<ofTexture> & getTexturePlanes() const;
    void setUseTexture(bool bUseTex);
    bool isUsingTexture() const {
        return true;
    }
	
    float getWidth() const;
    float getHeight() const;
	
    bool isPaused() const;
    bool isLoaded() const;
    bool isPlaying() const;
	
    float getPosition();
    float getSpeed();
    float getDuration() const;
    bool getIsMovieDone();
    float getFrameRate();
	
    void setPaused(bool bPause);
    void setPosition(float pct);
    void setVolume(float volume); // 0..1
    void setLoopState(ofLoopType state);
    void setSpeed(float speed);
    void setFrame(int frame);  // frame 0 = first frame...
	
    int	getCurrentFrame() const;
    int	getTotalNumFrames() const;
    ofLoopType getLoopState();
	
    void firstFrame();
    void nextFrame();
    void previousFrame();
    
    void draw(float x, float y, float w, float h) const;
    
    static std::pair<std::string, std::string> getPrefixAndNumber(std::string name);
    static std::vector<std::string> getSequence(std::string dirPath, std::string prefix, std::string start, std::string ext);
    static const std::vector<std::string> allowedExt;
    static bool isAllowedExt(std::string ext);
    
protected:
    
    std::vector<std::string> imageSequencePaths;
    std::vector<ofTexture *> imageSequenceTextures;
    std::vector<ofTexture> tex;
    ofTexture * playerTex;
    
    ofPixels pixels;
    
    bool bLoaded;
    bool bPlaying;
    bool bPaused;
    bool bNewFrame;
    bool bUpdatePixels;
    
    int frameIndex;
    int frameLastIndex;
    int framesTotal;
    float position;
    float duration;
    float time;
    
    float fps;
    float speed;
    ofLoopType loopType;
    bool isPlayingBackwards; //for OF_LOOP_PALINDROME
};
