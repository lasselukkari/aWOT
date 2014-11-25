#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <ServerInterface.h>
#include <EthernetServerAdapter.h>
#include <StreamServerAdapter.h>
#include <aJSON.h>
#include <aJsonStore.h>
#include <MemoryStore.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 20);
EthernetServer server(80);
EthernetServerAdapter ethernetServer(&server);
StreamServerAdapter streamServer(&Serial);

WebApp httpServer;
Router router("examples"); // router with "examples" prefix
MemoryStore store;

// use dictionary with aJSON to save RAM with key names
char *dictionary[] = { "id", "name", NULL };

//send HTML that loads the external javascript and css files for the UI
void indexCmd(Request &req, Response &res) {
  res.success("text/html; charset=utf-8");
  P(index) =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    " <meta charset=\"utf-8\">\n"
    " <title>Example test</title>\n"
    " <link rel=\"stylesheet\" href=\"http://code.jquery.com/qunit/qunit-1.14.0.css\">\n"
    "</head>\n"
    "<body>\n"
    "<div id=\"qunit\"></div>\n"
    "<div id=\"qunit-fixture\"></div>\n"
    "<script src=\"http://code.jquery.com/jquery-1.11.0.min.js\"></script>\n"
    "<script src=\"http://code.jquery.com/qunit/qunit-1.14.0.js\"></script>\n"
    "<script src=\"http://www.awot.net/qunit/tests.js\"></script>\n"
    "</body>\n"
    "</html>";
  res.printP(index);
}

void upsertModel(Request &req, Response &res) {

  aJsonStream inStream(&req);

  aJsonObject *model = aJson.parse(&inStream);

  if (!model) {
    res.serverError();
  }
  else {

    aJsonStream outStream(&res);

    store.saveToCollection("examples", model);

    if (req.method() == Request::POST) {
      res.created();
    }
    else {
      res.success();
    }

    aJson.print(model, &outStream);
  }
}


void getModel(Request &req, Response &res) {

  int id = atoi(req.route("id"));
  aJsonObject *model = store.findFromCollection("examples", id);

  if (!model) {
    res.notFound();
  }
  else{
    aJsonStream serverStream(&res);
    res.success();
    aJson.print(model, &serverStream);
  }

}

void deleteModel(Request &req, Response &res) {

  int id = atoi(req.route("id"));
  aJsonObject *model = store.findFromCollection("examples", id);

  if(!model){
    res.notFound();
  }
  else{
    store.removeFromCollection("examples", id);
    res.noContent();
  }

}

void getCollection(Request &req, Response &res) {
  res.success();
  store.findCollection("examples", &res);
}

void setup() {

  aJson.useDictionary(dictionary);
  store.createCollection("examples");

  httpServer.get("", &indexCmd);

  router.get("", &getCollection);
  router.post("", &upsertModel);
  router.get(":id", &getModel);
  router.put(":id", &upsertModel);
  router.del(":id", &deleteModel);

  httpServer.use(&router);


  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  }
  else{
    Serial.println("Ethernet failed");
  }

}

void loop() {
  httpServer.process(&ethernetServer);
  httpServer.process(&streamServer);
}
