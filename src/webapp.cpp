#include "webapp.h" 
#include <ESP8266WebServer.h>
#include "loop.h"
#include "webapphtml.h"

static ESP8266WebServer * server;

typedef struct WebAppState {
    ESP8266WebServer * server;
} WebAppState;

static void serveApp(){
    // NOTE - webAppHTML comes from webapphtml.h
    server->send(200, "text/html", webAppHTML);
}

static void handleColor(){
    if(server->args() < 1){
        server->send(400, "text/plain", "please provide rgb color");
        return;
    }

    byte color[3];
    bool found = false;
    char message[100];

	for(int i = 0; i < server->args(); i++){
        if(server->argName(i) == "rgb"){
            const char * colorStr = server->arg(i).c_str();
            sprintf(message, "set color %s", colorStr);
            found = true;
            break; 
        }
	}

    if(!found){
        server->send(400, "text/plain", "please provide rgb color");
        return;
    }

    // TODO - set color on ledstrip

    server->send(200, "text/plain", message);
}

static void handleAnimation(){
    server->send(200, "text/plain", "animation");
}

static void handleReady(){
    // TODO - set esp to custom preset
    server->send(200, "text/plain", "ready");
}

static void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server->uri();
	message += "\nMethod: ";
	message += ( server->method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server->args();
	message += "\n";

	for ( uint8_t i = 0; i < server->args(); i++ ) {
		message += " " + server->argName ( i ) + ": " + server->arg ( i ) + "\n";
	}

	server->send(404, "text/plain", message);
}

static void serverTick(void * s){
    WebAppState * state = (WebAppState*)s;
    state->server->handleClient();
}

int webAppBegin(){
    server = new ESP8266WebServer(80);
    WebAppState * state = (WebAppState*)malloc(sizeof(WebAppState));
    state->server = server;

    // routes
    // html
    server->on("/", serveApp);
    server->onNotFound(handleNotFound);

    // api
    server->on("/api/v1/color", handleColor);
    server->on("/api/v1/animation", handleAnimation);
    server->on("/api/v1/ready", handleReady);

    server->begin();
    loopAttach(serverTick, 0, state);

    return 1;
}
