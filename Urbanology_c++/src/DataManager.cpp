//
//  DataManager.cpp
//  Urbanology
//
//  Manages all data like questions, tokens and the mapping relationship.
//  Loads all data from server/database, update the score of each token when user answers questions.

#include "DataManager.hpp"
#include "cinder/Utilities.h"
#include "jsoncpp/json.h"
#include "cinder/Rand.h"
#include <boost/tokenizer.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

void DataManager::setup()
{
    loadQuestions();
    loadTokenStats();
    loadQuestionTokenMapping();
    
    mTokenTexture = gl::Texture::create( loadImage(loadAsset( "token.png" ) ) );
}

void DataManager::loadQuestions()
{
    //load questions from DB
    try{
        string request = mServerURL + "/questions";
        std::string result = loadString(loadUrl(request, UrlOptions(true, 2.5)));
        
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(result, root) && !root.empty()) {
            
            for (Json::Value item : root["rows"]) {
                int id = item["id"].asInt();
                string question = item["question"].asString();
                //app::console() << id << " , " << question << endl;
                mQuestionList.push_back(question);
                
            }
        }
    }catch(std::exception &e){
        app::console()<< e.what() << endl;
        
        //if could not connect to server
        mQuestionList = {
            "Question 1: Should cities enforce a minimum height for new residential buildings?",
            "Question 2: Absurdly long question to see if the text wraps properly...",
            "Question 3: Will you allow twice as many vendors to be licensed in public parks?",
            "Question 4: Will you authorize a tax on book sales in order to expend library services?",
            "Question 5: Will you authorize the conversation of a downtown affordable-hosing block into a tourist hotel?"
        };
    }
}

void DataManager::loadTokenStats()
{
    
    //load token statistics from DB
    try{
        string request = mServerURL + "/currentStat";
        std::string result = loadString(loadUrl(request, UrlOptions(true, 2.5)));
        
        Json::Value root;
        Json::Reader reader;
        
        Token token;
        if (reader.parse(result, root) && !root.empty()) {
            
            for (Json::Value item : root["rows"]) {
                token.token_id= item["token_id"].asInt();
                token.token_text = item["token"].asString();
                
                // server would return scores for each tokens
                // (from a table that records previous answers)
                // I just set them as 0 for convenience so all tokens stay in the center when the app starts
                
                token.currentScore = 0;
                token.targetScore = 0;
                
                tokens.push_back(token);
           
            }
        }
    }catch(std::exception &e){
        app::console()<< e.what() << endl;
    }
}

void DataManager::loadQuestionTokenMapping()
{
    try{
        string request = mServerURL + "/questionTokenMapping";
        std::string result = loadString(loadUrl(request, UrlOptions(true, 2.5)));
        
        Json::Value root;
        Json::Reader reader;
        
         //app::console()<< result <<endl;
        
        if (reader.parse(result, root) && !root.empty()) {
                        
            for (Json::Value item : root["rows"]) {
                TokenMapping token;
                
                token.question_id = item["question_id"].asInt();
                token.token_id = item["token_id"].asInt();
                token.yes_affect = item["yes_affect"].asInt();
                token.no_affect = item["no_affect"].asInt();

                mQuestionTokenMapping.push_back(token);
                
            }
        }
    }catch(std::exception &e){
        app::console()<< e.what() << endl;
    }
}


void DataManager::answerCurrentQuestion(bool answer)
{
    //calculate the token scores
    int tokenIndex = 0;
    for (int i=0 ;i< mQuestionTokenMapping.size(); i++){
        
        TokenMapping token = mQuestionTokenMapping[i];
        
        if (token.question_id == (mCurrentQuestionId+1)){
            
            if (token.token_id == (tokenIndex+1)){

                float score = answer? token.yes_affect :token.no_affect;

                tokens[tokenIndex].targetScore = tokens[tokenIndex].currentScore + score/25.0;
                tokenIndex ++;
            }
        }
    }
    
    if( hasNextQuestion()){
        mCurrentQuestionId += 1;
    }
    
    // TO DO:update token scores to database?
}

void DataManager::tokenAnimUpdate(Area area)
{
     // Tokens lerp from current score to target score
    
    for( int i = 0; i < tokens.size(); ++i ){
        tokens[i].currentScore += (tokens[i].targetScore -tokens[i].currentScore) * 0.03;
        tokens[i].position.x  = tokens[i].currentScore * ( area.getWidth() - sTokenWidth );
        tokens[i].position.y = float( i ) / float( tokens.size() ) * area.getHeight();

    }
}

void DataManager::drawTokens(Area area){
    gl::ScopedColor bgColor( ColorAf( 0, 0, 0, 0.1 ) );
    gl::drawSolidRoundedRect( area, 30 );
    
    gl::ScopedModelMatrix modelMtx;
    
    gl::translate(area.getCenter().x, area.getUL().y);
    
    for( int i = 0; i < tokens.size(); ++i ) {
        
        gl::ScopedColor color( Color::white() );
        gl::drawString(tokens[i].token_text, vec2((-1)*area.getCenter().x + 50 ,tokens[i].position.y +20), ColorA::black(), Font("Arial", 16));
        gl::draw( mTokenTexture, Rectf( 0, 0, sTokenWidth,sTokenWidth). getOffset(tokens[i].position));
    }
}

bool DataManager::hasNextQuestion(){
    return mCurrentQuestionId < mQuestionList.size();
}

string DataManager::getCurrentQuestion(){
    return mQuestionList[mCurrentQuestionId];
}
