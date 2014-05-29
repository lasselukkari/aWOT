#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <StreamServerAdapter.h>
#include <EthernetServerAdapter.h>
#include <aJSON.h>
#include <IRremote.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
StreamServerAdapter serialServerAdapter(&Serial);
EthernetServerAdapter ethernetServerAdapter(&server);
HTTPServer httpServer("");
IRsend irsend;

byte curX = 0;
byte curY = 0;

byte writingNormal = 0;
byte writingCaps = 1;
byte writingSpecial = 2;

byte writingMode = writingNormal;

void up(){
  irsend.sendNEC(0x77E15057, 32);
}

void down(){
  irsend.sendNEC(0x77E13057, 32);
}

void left(){
  irsend.sendNEC(0x77E19057, 32);
}

void right(){
  irsend.sendNEC(0x77E16057, 32);
}

void select(){
  irsend.sendNEC(0x77E13A57, 32);
}

void menu(){
  irsend.sendNEC(0x77E1C057, 32);
}

void play(){
  irsend.sendNEC(0x77E1FA57, 32);
}

char charTable [6][6] = {
  {'a','b','c','d','e','f'},
  {'g','h','i','j','k','l'},
  {'m','n','o','p','q','r'},
  {'s','t','u','v','w','x'},
  {'y','z','1','2','3','4'},
  {'5','6','7','8','9','0'}
};

char specialCharTable [6][6] = {
  {'!','@','#','$','%','&' },
  {'~','*','\\','/','?','^'},
  {'_','‘',';',':','|','='},
  {'é','ñ','[',']','{','}'},
  {'ç','ü','.',',','+','-'},
  {'<','>','(',')','\'',34}
};


void setWritingMode(char c){
  byte newMode;

  if (c >= 'a' && c <= 'z'){
    newMode = writingNormal;
  }
  else if (c >= 'A' && c <= 'Z'){
    newMode = writingCaps;
  }
  else if (c >= '0' && c <= '9'){
    newMode = writingNormal;
  }
  else {
    newMode = writingSpecial;
  }
  
  if (newMode != writingMode){
    if(writingMode == writingNormal && newMode == writingCaps){
      play();
    }

    else if (writingMode == writingNormal && newMode == writingSpecial){
      play();
      play();
    }

    else if(writingMode == writingCaps && newMode == writingNormal){
      play();
      play();
      play();
    }

    else if (writingMode == writingCaps && newMode == writingSpecial){
      play();
    }

    else if(writingMode == writingSpecial && newMode == writingNormal){
      play();
      play();
    }

    else if (writingMode == writingSpecial && newMode == writingCaps){
      play();
      play();
      play();
    }
    writingMode = newMode;
  }
}




void writeChar(char c){
  if (c == ' '){
    moveTo(0,5);
    down();
    select();
    up();
  }
  else{
    setWritingMode(c);
    if (writingMode == writingCaps){
      c +=32;
    }
    for (byte x = 0; x < 6; x++){
      for (byte y = 0; y < 6; y++){

        if(charTable[y][x] == c || (writingMode == writingSpecial && specialCharTable[y][x] == c) ){
          moveTo(x,y);
          select();
        }
      }
    }
  }
}

void relocateCursor(){
  
  left();left();left();left();left();left();
  up();up();up();up();up();up();up();
  left();left();left();
  down();
  
  curX = 0;
  curY = 0;
  
  writingMode = writingNormal;
}

void writeString(char * string){
  relocateCursor();
  int stringLenght = strlen( string );
  for (int i = 0; i < stringLenght; i++){
    writeChar(string[i]);
  }
}

void move(int xSteps, int ySteps){

  while (xSteps != 0){
    if(xSteps < 0){
      left();
      xSteps++;
    }
    else{
      right();
      xSteps--;
    }
  }

  while (ySteps != 0){
    if(ySteps < 0){
      up();
      ySteps++;
    }
    else{
      down();
      ySteps--;
    }
  }
}

void moveTo(byte x, byte y){
  move(x - curX, y - curY);
  curX = x;
  curY = y;
}

void indexCmd(HTTPServer &server, HTTPServer::MethodType type, char *, bool) {
  server.httpSuccess();
  P(indexHTML) =
  "<!DOCTYPE html>\n"
  "<html>\n"
  "<head>\n"
  "<meta name=\"viewport\" content=\"width=480px, user-scalable=no\">\n"
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"https://netdna.bootstrapcdn.com/bootstrap/3.0.3/css/bootstrap.min.css\" />\n"
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"https://www.cs.helsinki.fi/u/ljlukkar/appletv/css/custom.css\" />\n"
  "<title>ATV</title>\n"
  "</head>\n"
  "<body>\n"
  "<div class=\"swipe-container container\"></div>\n"
  "<div class=\"buttons-container container\"></div>\n"
  "<script data-main=\"https://www.cs.helsinki.fi/u/ljlukkar/appletv/js/main.js\" \n"
  " src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.1.10/require.min.js\"></script>\n"
  "</body>\n"
  "</html>";
  server.printP(indexHTML);
}

void postMove(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  
  aJsonStream serverStream(&server);
  aJsonObject* moveModel = aJson.parse(&serverStream);

  server.httpSuccess("application/json");
  aJson.print(moveModel, &serverStream);

  int x = aJson.getObjectItem(moveModel, "x")->valueint;
  int y = aJson.getObjectItem(moveModel, "y")->valueint;

  if (x==0 && y == 0){
    select();
  }
  else{
    move(x,y);
  }

  aJson.deleteItem(moveModel);

}

void postText(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  
  aJsonStream serverStream(&server);
  aJsonObject* textModel = aJson.parse(&serverStream);
  
  server.httpSuccess("application/json");
  aJson.print(textModel, &serverStream);
  
  writeString(aJson.getObjectItem(textModel, "text")->valuestring);
  aJson.deleteItem(textModel);
  
}

void postCommand(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  
  aJsonStream serverStream(&server);
  aJsonObject* commandModel = aJson.parse(&serverStream);
  
  server.httpSuccess("application/json");
  aJson.print(commandModel, &serverStream);


  char * commandName = aJson.getObjectItem(commandModel, "name")->valuestring;

  if(strcmp(commandName, "up") == 0){
    up(); 
  }
  else if(strcmp(commandName, "down") == 0){
    down();
  }
  else if(strcmp(commandName, "right") == 0){
    right();
  }
  else if(strcmp(commandName, "left") == 0){
    left();
  }
  else if(strcmp(commandName, "select") == 0){
    select();
  }
  else if(strcmp(commandName, "menu") == 0){
    menu();
  }

  else if(strcmp(commandName, "play") == 0){
    play();
  }

  aJson.deleteItem(commandModel);

}


void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac);
  pinMode(9,OUTPUT); // The library initializes pin 3 as an output
  digitalWrite(9, LOW);// Since our LED is connected to pin 9 on Arduino Mega, we initialize it here
  
  httpServer.setDefaultCommand(&indexCmd);
  httpServer.onPOST("move", &postMove);
  httpServer.onPOST("text", &postText);
  httpServer.onPOST("command", &postCommand);

}

void loop() {
  httpServer.processConnection(&serialServerAdapter);
  httpServer.processConnection(&ethernetServerAdapter);
}




