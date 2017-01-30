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

static int byteFromString(char * arg, byte * b){
    int val;
    int ok = sscanf(arg, "%d", &val);
    if(!ok){
        Serial.printf("Failed to convert '%s' to int\n", arg);
        return 0;
    }
    *b = (byte)val;
    return 1;
}

static int byteFromArg(char * argName, byte * b){
    if(!server->hasArg(argName)){
        Serial.printf("Missing arg '%s'\n", argName);
        return 0;
    }

    if(server->arg(argName).length() > 3){
        Serial.printf("Value '%s' for arg '%s' is of invalid length\n", server->arg("r").c_str(), argName);
        return 0;
    }

    char str[3];
    strcpy(str, server->arg(argName).c_str());
    if(!byteFromString(str, b)){
        Serial.printf("couldnt convert value '%s' for arg '%s' to byte\n", str, argName);
        return 0;
    }

    return 1;
}

static void handleColor(){
    if(server->args() < 3){
        server->send(400, "text/plain", "please provide r, g, and b values");
        return;
    }

    byte color[3];
    char message[40];

    if(!server->hasArg("r") || !server->hasArg("g") || !server->hasArg("b")){
        server->send(400, "text/plain", "please provide r, g, and b bro");
        return;
    }

    if(!byteFromArg("r", &(color[0]))){
        server->send(400, "text/plain", "failed to parse value for arg 'r'\n");
        return;
    }
    if(!byteFromArg("g", &(color[1]))){
        server->send(400, "text/plain", "failed to parse value for arg 'g'\n");
        return;
    }
    if(!byteFromArg("b", &(color[2]))){
        server->send(400, "text/plain", "failed to parse value for arg 'b'\n");
        return;
    }

    // TODO - set color on ledstrip
    sprintf(message, "set r: %i, g: %i, b: %i", color[0], color[1], color[2]);

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
