//
//  PhotoManager.hpp
//  Urbanology
//



#include <stdio.h>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/Context.h"
#include "cinder/Thread.h"
#include "cinder/gl/Fbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PhotoManager {

public:
    ~PhotoManager();
    
    void setup();
    void getNewFrame();
    void draw();
    void takePortrait();
    void savePortrait(gl::ContextRef sharedGlContext);
    void drawProcesedPortrait();
    void savePortraitBackground();

    bool mGetNewFeed = true;

    gl::TextureRef mPortraitTexture;

    
private:
    Area mPortraitArea;
    bool mWebCamAvailable;
    CaptureRef mWebCamCapture;
    gl::GlslProgRef mImageProcessGlsl;
    Surface8uRef mPortraitSurface;
    
    shared_ptr<thread>		mThread;

};