//
//  PhotoManager.cpp
//  Urbanology
//
// Manage webcam, taking picture shader and storing image.


#include "PhotoManager.hpp"


void PhotoManager::setup()
{
    try {
        mWebCamCapture = Capture::create( 800, 600 );
        mWebCamCapture->start();
        mWebCamAvailable = true;
    }
    catch( ci::Exception &e ) {
        mWebCamAvailable = false;
    }
    mImageProcessGlsl = gl::GlslProg::create( loadAsset( "process.vert" ), loadAsset( "process.frag" ) );
    mPortraitArea = Area( 0, 0, 400, 300).getMoveULTo( ivec2( 40, 380 ));
}

void PhotoManager::getNewFrame()
{
    if( mWebCamAvailable ) {
        if( mWebCamCapture && mWebCamCapture->checkNewFrame() ) {
            if(!mPortraitTexture){
                mPortraitSurface = mWebCamCapture->getSurface();
                mPortraitTexture = gl::Texture::create(*mPortraitSurface, gl::Texture::Format().loadTopDown() );
            }else{
                mPortraitTexture->update( *mWebCamCapture->getSurface() );
            }
        }
    }
    else {
        mPortraitSurface = Surface8u::create( loadImage( loadAsset( "test-portrait.png" ) ) );
        mPortraitTexture = gl::Texture::create( *mPortraitSurface );
    }

}

void PhotoManager::draw()
{
    if( mPortraitTexture ) {
        
        gl::ScopedModelMatrix modelScope;
        gl::draw(mPortraitTexture, Rectf(mPortraitArea));
        
        if(mWebCamAvailable){
            gl::drawStringCentered("Tap Space To Take A Photo ", mPortraitArea.getCenter()- vec2(0,200), ColorA::black(), Font("Arial", 16));
        }else{
            gl::drawStringCentered("Webcam Not Working. Tap Space To Continue", mPortraitArea.getCenter()- vec2(0,200), ColorA::black(), Font("Arial", 16));

        }
    }
}

void PhotoManager::takePortrait()
{
    mGetNewFeed = false;
    
    mPortraitSurface = mWebCamCapture->getSurface();
    mPortraitTexture = gl::Texture::create(*mPortraitSurface );
    mWebCamCapture->stop();
    
    savePortraitBackground();
}

// save image
void PhotoManager::savePortrait(gl::ContextRef sharedGlContext)
{
    if( ! mPortraitTexture )
        return;
    
    int width = max( mPortraitTexture->getWidth() / 2, 800 );
    int height = width / mPortraitTexture->getAspectRatio();

    // resize image
    Surface resizedSurface( width, height, false );
    ip::resize( *mPortraitSurface, &resizedSurface );

    // use cinder's builtin jpeg compression by letting it write to a file
    fs::path tempFilePath = getAssetDirectories().at(0) / "user_portrait.jpeg";

    writeImage( writeFile( tempFilePath ), resizedSurface );
    app::console() << "image saved to:" << tempFilePath << endl;
    
    //could use Fbo to save the image processed by shader

}

void PhotoManager::savePortraitBackground()
{
    gl::ContextRef backgroundCtx = gl::Context::create( gl::context() );
    mThread = shared_ptr<thread>( new thread( bind( &PhotoManager::savePortrait, this, backgroundCtx)));
}

// draw image on screen with shader
void PhotoManager::drawProcesedPortrait()
{

    if( ! mPortraitTexture )
        return;
    
    gl::ScopedGlslProg glsl( mImageProcessGlsl );
    gl::ScopedTextureBind tex( mPortraitTexture );
    
    mImageProcessGlsl->uniform( "uTex", 0 );
    mImageProcessGlsl->uniform( "uTime", float( getElapsedSeconds() ) );
    mImageProcessGlsl->uniform( "uSize", vec2( mPortraitTexture->getSize() ) );
    
    gl::drawSolidRect( mPortraitArea );
}

PhotoManager::~PhotoManager(){
    mThread->join();
}