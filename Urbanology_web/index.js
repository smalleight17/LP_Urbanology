var express = require('express');
var app = express();
var pg = require('pg');
var bodyParser = require('body-parser');

app.set('port', (process.env.PORT || 5000));
app.use(bodyParser.urlencoded({ extended: true })); 
app.use(express.static(__dirname + '/public'));
app.set('views', __dirname + '/views');
app.set('view engine', 'ejs');

app.listen(app.get('port'), function() {
  console.log('Node app is running on port', app.get('port'));
});

app.use(function (req, res, next) {
  res.setHeader('Access-Control-Allow-Origin', 'http://localhost:5000');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, PATCH, DELETE');
  res.setHeader('Access-Control-Allow-Headers', 'X-Requested-With,content-type');
  res.setHeader('Access-Control-Allow-Credentials', true);
  next();
});

const connectionString = process.env.DATABASE_URL;
const client = new pg.Client(connectionString);
client.connect();
console.log('Connected to postgres! Getting schemas...');

app.get('/', function (request, response) {
  var mappingQuery = 
      'SELECT a.id as question_id, a.question, array_agg(b.token ORDER BY b.id) as token, '+
      'array_agg(c.yes_affect ORDER BY c.token_id) as yes_affect,'+
      'array_agg(c.no_affect ORDER BY c.token_id) as no_affect '+
      'FROM Questions a, Tokens b, TokenMapping c '+
      'WHERE a.id = c.question_id '+
      'AND c.token_id = b.id '+
      'GROUP BY a.id '+
      'ORDER BY a.id;';

  client.query(mappingQuery, function(err, result) {
    if (err) throw err;
    var tokenQuery = 'SELECT * FROM Tokens ORDER BY id;';
    client.query(tokenQuery, function(err, tokens){
      if (err) throw err;
       response.render('index', {results: result.rows, tokens: tokens.rows} ); 
    });
  });
});

//add a new question to DB from web admin
app.post('/create', function (req, res) {
  var question = req.body.question;

  // insert question
  client.query('INSERT INTO Questions(question) values (\''+question+'\') returning *;',
     function(err, result){
      if (err) throw err;

      var questionInsertId = result.rows[0]['id'];
      var insertQuery = "";

      //insert token and points
      for(var i=1;i<6;i++){
        var yes_affect = req.body[i+'_yes'];
        var no_affect = req.body[i+'_no'];
        insertQuery += 'INSERT INTO TokenMapping(question_id, token_id,yes_affect,no_affect)'+
      ' values('+ questionInsertId +','+ i +','+ yes_affect +','+no_affect+');';
      }
     
     console.log(insertQuery);     
      client.query(insertQuery, function(err,result){
        if (err) throw err;
        //refresh the page
        res.redirect(req.get('referer'));
      });
  });
});

//edit
app.post('/edit', function(req,res){
  //console.log("Oh my god I totally get it!!!!!");
  console.log(req.body);
  var param_id = req.body.id;
  var value = req.body.value;
  var param = param_id.split('_');
  
  if (param.length == 2){
    //update question
    var updateQuery = 'UPDATE Questions SET question=\''+value+'\' WHERE id='+param[1]+';';
    client.query(updateQuery, function(err, result){
      if (err) throw err;
      res.send(value);
    });
  }else if(param.length == 3){
    //update token scores
    var question_id = param[0];
    var token_id = param [1];
    var score = (param[2]=='yes')? 'yes_affect' : 'no_affect';
    var updateQuery = 
    'UPDATE TokenMapping SET '+score+'='+value+
    ' WHERE question_id='+ question_id + 
    ' AND token_id='+token_id+';';
    
    client.query(updateQuery, function(err, result){
      if (err) throw err;
      res.send(value);
    });
  }else{
    res.send("Please check your parameters!");
  }
});


// api for cinder front end
//return json of all the questions in DB
app.get('/questions',function(req,res){
  client.query('SELECT * from Questions ORDER BY id;', function(err, result){
    res.send(result);
  });
});

// api for cinder front end
//return the current statistics. Calculate points for all tokens.
app.get('/currentStat',function(req,res){
  var statQuery = 
    'SELECT A.token_id, MIN(B.token) AS token, SUM(A.yes_affect * A.yes_num + '+
    'A.no_affect * A.no_num) FROM '+
    '(SELECT b.token_id, b.question_id, b.yes_affect, a.yes_num, '+
    'b.no_affect, a.no_num '+
    'FROM QuestionStat a, TokenMapping b '+
    'WHERE a.question_id = b.question_id) as A, '+
    'Tokens B '+
    'WHERE B.id = A.token_id '+
    'GROUP BY A.token_id '+
    'ORDER BY A.token_id; ';

  client.query(statQuery,function (err,result){
    if (err) throw err;
    res.send(result);
  });

});

// api for cinder front end
app.get('/questionTokenMapping',function(req,res){
  var mappingQuery = 'SELECT * FROM TokenMapping ORDER BY question_id, token_id';
  // var mappingQuery =
  // 'SELECT question_id, array_agg(token_id||\';\'||yes_affect||\';\'||no_affect) as tokens '+
  // 'FROM TokenMapping '+
  // 'GROUP BY question_id; ';

    client.query(mappingQuery,function (err,result){
    if (err) throw err;
    res.send(result);
  });
});


