//
//  DataManager.hpp
//  Urbanology
//

#include "cinder/Cinder.h"

using namespace ci;
using namespace std;

typedef struct {
    int question_id;
    int token_id;
    int yes_affect;
    int no_affect;
} TokenMapping;

typedef struct{
    int token_id;
    string token_text;
    float currentScore;
    float targetScore;  //for animation. Tokens lerp from current score to target score
    vec2 position;
} Token;


class DataManager {
public:
    void setup();
    bool hasNextQuestion();
    string getCurrentQuestion();
    void requestStats();
    void answerCurrentQuestion(bool answer);
    void tokenAnimUpdate(Area area);
    void drawTokens(Area area);
    
    //string mServerURL = "http://localhost:5000";
    string mServerURL = "https://obscure-ridge-47723.herokuapp.com";
    vector<string> mQuestionList;
   
    vector<Token> tokens;
    vector<TokenMapping> mQuestionTokenMapping;
    int mCurrentQuestionId = 0;
    
    
private:
    void loadQuestions();
    void loadTokenStats();
    void loadQuestionTokenMapping();
    
    const float sTokenWidth = 60.0f;
    gl::TextureRef mTokenTexture;
};
