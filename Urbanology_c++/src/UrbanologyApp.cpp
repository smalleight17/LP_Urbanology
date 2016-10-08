#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "DataManager.hpp"
#include "PhotoManager.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

#define DEBUG_TEXT_BOUNDS

class UrbanologyApp : public App {
  public:
	void setup() override;
    void update() override;
    void draw() override;
	void mouseDown( MouseEvent event ) override;
    void mouseUp( MouseEvent e ) override;
    void mouseDrag( MouseEvent e ) override;
    void keyDown(KeyEvent event) override;
    void drawQuestions( Area area );
    
private:
    
    DataManager mDataMngr;
    PhotoManager mPhotoMngr;
    
    gl::TextureFontRef mTexFont;

    Area mTokenArea;
    Area mQuestionArea;
    
    //mouse drag
    ivec2 mMouseDownPos;
    ivec2 mDragOffset;
    bool mShouldTrackDrag;
    Rectf mQuestionTextBounds, mYesTextBounds, mNoTextBounds;
    enum CurrentChoice { NONE, SELECTED_YES, SELECTED_NO } mCurrentChoice;
};

void UrbanologyApp::setup()
{
    mDataMngr.setup();
    mPhotoMngr.setup();
   
    mTexFont = gl::TextureFont::create( Font( Font::getDefault().getName(), 32 ) );
    mTokenArea = Area( 0, 0, 1050, 300 ).getMoveULTo(ivec2(40, 40));
    mQuestionArea = Area( 0, 0, 600, 300 ).getMoveULTo(ivec2(480, 380));
    
    mCurrentChoice = NONE;
    mMouseDownPos = ivec2( 0 );
    mDragOffset = ivec2( 0 );
    mShouldTrackDrag = false;
    

}

void UrbanologyApp::update()
{
    if(mPhotoMngr.mGetNewFeed){
        mPhotoMngr.getNewFrame();
    }
    
    mDataMngr.tokenAnimUpdate(mTokenArea);
}

void UrbanologyApp::draw()
{
    
    gl::setMatricesWindow( getWindowSize() );
    gl::clear( Color::white() );

    if(mPhotoMngr.mGetNewFeed){
        mPhotoMngr.draw();
    }else{
        mPhotoMngr.drawProcesedPortrait();
        mDataMngr.drawTokens (mTokenArea);
        drawQuestions( mQuestionArea);
    }
}

void UrbanologyApp::mouseDown( MouseEvent e )
{
    if( mDataMngr.hasNextQuestion() && mQuestionTextBounds.contains( vec2( e.getPos() ) ) )
        mShouldTrackDrag = true;
    else
        mShouldTrackDrag = false;
    
    mMouseDownPos = e.getPos();
}

void UrbanologyApp::mouseUp( MouseEvent e )
{

    if( mCurrentChoice == SELECTED_YES ){
         mDataMngr.answerCurrentQuestion( true );
    }
    
    if( mCurrentChoice == SELECTED_NO ){
        mDataMngr.answerCurrentQuestion( false );
    }
    
    //reset values
    mMouseDownPos = ivec2( 0 );
    mDragOffset = ivec2( 0 );
    mCurrentChoice = NONE;
}

void UrbanologyApp::mouseDrag( MouseEvent e )
{
    if( mShouldTrackDrag ) {
        mDragOffset = e.getPos() - mMouseDownPos;
        
        if( mQuestionTextBounds.intersects( mYesTextBounds ) )
            mCurrentChoice = SELECTED_YES;
        else if( mQuestionTextBounds.intersects( mNoTextBounds ) )
            mCurrentChoice = SELECTED_NO;
        else
            mCurrentChoice = NONE;
    }
}

void UrbanologyApp::keyDown(KeyEvent event){
    
    if (event.getCode() == KeyEvent::KEY_SPACE){
        mPhotoMngr.takePortrait();
    }
}


void UrbanologyApp::drawQuestions( Area area )
{
    gl::ScopedColor bgColor( ColorAf( 0, 0, 0, 0.1 ) );
    gl::drawSolidRoundedRect( area, 30 );
    
    ivec2 baselineOffset( 0, -24 );
    vec2 size;
    
    // measure text bounds
    ivec2 yesTextPos( 20, area.getHeight() - 50 );
    size = mTexFont->measureString( "YES" );
    mYesTextBounds = Rectf( 0, 0, 1, 1 ).scaled( size ).getOffset( yesTextPos + area.getUL() + baselineOffset );
    
    ivec2 noTextPos( area.getWidth() - 80, area.getHeight() - 50 );
    size = mTexFont->measureString( "NO" );
    mNoTextBounds = Rectf( 0, 0, 1, 1 ).scaled( size ).getOffset( noTextPos + area.getUL() + baselineOffset );
    
    ivec2 questionTextPos( 20, 60 );
    
    string questionText = mDataMngr.hasNextQuestion() ? mDataMngr.getCurrentQuestion():"No more questions" ;
    size = mTexFont->measureStringWrapped( questionText, area );
    mQuestionTextBounds = Rectf( 0, 0, 1, 1 ).scaled( size ).getOffset( questionTextPos + area.getUL() + mDragOffset + baselineOffset );
    
#ifndef DEBUG_TEXT_BOUNDS
    gl::ScopedColor textFieldColor( ColorAf( 0, 0, 0.8, 0.2 ) );
    gl::drawSolidRect( mQuestionTextBounds );
    gl::drawSolidRect( mYesTextBounds );
    gl::drawSolidRect( mNoTextBounds );
#endif
    
    gl::ScopedColor yesColor( mCurrentChoice == SELECTED_YES ? ColorAf( 0, 0.95, 0, 1 ) : ColorAf( 0.1, 0.1, 0.1, 1 ) );
    mTexFont->drawStringWrapped( "YES", area, yesTextPos );
    
    gl::ScopedColor noColor( mCurrentChoice == SELECTED_NO ? ColorAf( 0.95, 0, 0, 1 ) : ColorAf( 0.1, 0.1, 0.1, 1 ) );
    mTexFont->drawStringWrapped( "NO", area, noTextPos );
    
    gl::ScopedColor questionColor( Color::black() );
    mTexFont->drawStringWrapped( questionText, area, questionTextPos + mDragOffset );
}

CINDER_APP(UrbanologyApp, RendererGl( RendererGl::Options().msaa( 0 ) ), [] ( App::Settings *settings ) {
    settings->setWindowSize( 1120, 720 );
    settings->setHighDensityDisplayEnabled();
})