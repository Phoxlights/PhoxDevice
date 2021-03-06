#include <Arduino.h>
#include <Esp.h>
#include <loop.h>
#include <statuslight.h>
#include <network.h>
#include <ota.h>
#include <digitalbutton.h>
#include <eventReceiver.h>
#include <eventSender.h>
#include <eventRegistry.h>
#include <objstore.h>
#include <event.h>

#include "status.h"
#include "deviceconfig.h"

// NOTE - DEV_MODE may enable security holes
// so be sure it is off for production!
#ifndef DEV_MODE
#define DEV_MODE 0
#endif

// a controller usually brings up an AP and
// keeps a list of components which are registered
// with it
#ifndef CONTROLLER
#define CONTROLLER 0
#endif

// a component usually connects to a controller
#ifndef COMPONENT
#define COMPONENT 1
#endif

void asplode(char * err){
    Serial.printf("ERROR: %s\n", err);
    delay(1000);
    ESP.restart();
}

StatusLight status;
DeviceConfig * config = getConfig();
Identity * id = getIdentity();
DigitalButton btn = buttonCreate(BUTTON_PIN, 50);
IPAddress serverIP = IPAddress(SERVER_IP_UINT32);

void otaStarted(){
    Serial.println("ota start");
    statusLightStop(status);
}

void otaProgress(unsigned int progress, unsigned int total){
    Serial.print("ota progress");
}

void otaError(ota_error_t err){
    Serial.println("ota err");
}

void otaEnd(){
    Serial.println("ota end");
}

int setupStartHeap, setupEndHeap, prevHeap;
void logHeapUsage(void * state){
    int currHeap = ESP.getFreeHeap();
    int delta = setupEndHeap - currHeap;
    Serial.printf("currHeap: %i, delta: %i\n", currHeap, delta);
    prevHeap = currHeap;
}

// flashes status light real quick
void flash(){
    byte white[3] = {50,50,50};
    int pattern[] = {50, 100, 0};
    if(!statusLightSetPattern(status, white, pattern)){
        Serial.println("couldnt flash status light");
    }
    delay(50);
    statusLightStop(status);
}

void setNetworkMode(Event * e, Request * r){
    int mode = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting network mode to %i\n", mode); 
    config->networkMode = (NetworkMode)mode;
    writeConfig(config);
    delay(100);
    flash();
}

void restoreDefaultConfig(Event * e, Request * r){
    Serial.println("restoring default device config");
    if(!writeDefaultConfig()){
        Serial.println("could not restore default config");
        return;
    }
    Serial.println("restored default config");
    delay(100);
    flash();
}

void ping(Event * e, Request * r){
    Serial.printf("got ping with requestId %i. I should respond\n", e->header->requestId);
    Serial.printf("responding to %s:%i\n", r->remoteIP.toString().c_str(), r->remotePort);
    if(!eventSendC(r->client, EVENT_VER, PONG, 0, NULL, NULL)){
        Serial.printf("ruhroh");
        return;
    }
    flash();
}

void who(Event * e, Request * r){
    Serial.printf("someone wants to know who i am\n");
    if(!eventSendC(r->client, EVENT_VER, WHO, sizeof(Identity), (void*)&id, NULL)){
        Serial.printf("ruhroh");
        return;
    }
}

#ifdef CONTROLLER
void generateNetworkCreds(Event * e, Request * r){
    if(!generatePrivateNetworkCreds()){
        Serial.println("couldn't generate new network creds");
    }
}
void requestRegisterComponent(Event * e, Request * r){
    Serial.printf("someone wants me to register a thing\n");
    // TODO - i suspect some sort of sanitization
    // and bounds checking should occur here
    Identity * component = (Identity*)e->body;
    if(!registerComponent(component)){
        Serial.printf("failed to register component\n");
        return;
    }

    // persist new config to disk
    if(!writeConfig(config)){
        Serial.printf("failed to write config to disk\n");
    }

    PrivateNetworkCreds creds = getPrivateCreds();
    Serial.printf("ssid: %s, pass: %s\n", creds.ssid, creds.pass);
    if(!eventSendC(r->client, EVENT_VER, REGISTER_CONFIRM, sizeof(PrivateNetworkCreds), (void*)&creds, NULL)){
        Serial.printf("failed to respond to registration request\n");
        return;
    }
    flash();
}
#endif

#ifdef COMPONENT
bool registrationPending = false;
void resetRegistrationRequest(WiFiClient * client){
    registrationPending = false;
    if(client){
        if(!eventUnListenC(client)){
            Serial.println("couldn't unlisten registration response client; ignoring");
        }
    }
}
void sendRegistrationRequest(){
    Serial.println("sending registration request");

    if(registrationPending){
        Serial.println("but a registration is already pending");
        flashFailStatusLight(status);
        return;
    }

    setBusyStatusLight(status);

    registrationPending = true;
    
    // TODO - avoid using arduino api directly like this
    WiFiClient * client = new WiFiClient();

    if(!client->connect(serverIP, EVENT_PORT)){
        Serial.printf("couldnt connect to %s:%i\n", serverIP.toString().c_str(), EVENT_PORT);
        flashFailStatusLight(status);
        resetRegistrationRequest(client);
        return;
    }

    int ok = eventSendC(client, EVENT_VER, REGISTER_COMPONENT,
        sizeof(Identity), (void*)id, 0);

    if(!ok){
        Serial.println("couldnt send registration request");
        flashFailStatusLight(status);
        resetRegistrationRequest(client);
        return;
    }

    if(!eventListenC(client)){
        Serial.println("couldnt start listening for registration response");
        flashFailStatusLight(status);
        resetRegistrationRequest(NULL);
        return;
    }

    // TODO - setup timeout and call resetRegistrationRequest
    // if timeout is exceeded
    Serial.println("sent registration request. now we wait");
}
void receiveRegistrationResponse(Event * e, Request * r){
    Serial.println("got registration request response");
    
    if(!registrationPending){
        Serial.println("but there was no pending registration request; ignoring");
        resetRegistrationRequest(r->client);
        return;
    }

    // TODO - put this in a shared location because
    // it will be used by any phoxdevice
    struct PrivateNetworkCreds {
        char ssid[SSID_MAX];
        char pass[PASS_MAX];
    };

    // TODO - dont blindly write junk off the network?
    struct PrivateNetworkCreds * creds = (PrivateNetworkCreds*)e->body;
    strcpy(config->ssid, creds->ssid);
    strcpy(config->pass, creds->pass);

    if(!writeConfig(config)){
        Serial.println("failed to save newly registered network creds");
        flashFailStatusLight(status);
        return;
    }

    Serial.printf("successfully registered device to ssid: %s\n", config->ssid);
    flashSuccessStatusLight(status);

    resetRegistrationRequest(r->client);
}
#endif /* COMPONENT */

// events to listen for in run mode
int startRunListeners(){
    int ok = eventListen(EVENT_VER, EVENT_PORT);
    if(ok){
        eventRegister(PING, ping);
        eventRegister(WHO, who);

        Serial.printf("Listening for events with EVENT_VER: %i, eventPort: %i\n",
            EVENT_VER, EVENT_PORT);
    }
    return ok;
}

// events to listen for in sync mode
int startSyncListeners(){
    int ok = eventListen(EVENT_VER, EVENT_PORT);
    if(ok){
        eventRegister(SET_DEFAULT_CONFIG, restoreDefaultConfig);
        eventRegister(SET_NETWORK_MODE, setNetworkMode);
#ifdef COMPONENT
        eventRegister(REGISTER_CONFIRM, receiveRegistrationResponse);
#endif
#ifdef CONTROLLER
        eventRegister(REGISTER_COMPONENT, requestRegisterComponent);
        eventRegister(GENERATE_NETWORK_CREDS, generateNetworkCreds);
#endif

        Serial.printf("Listening for events with EVENT_VER: %i, eventPort: %i\n",
            EVENT_VER, EVENT_PORT);
    }
    return ok;
}

void enterSyncMode(){
    Serial.println("entering sync mode");

    setEnterSyncStatusLight(status);
    // keep flashing sync status light
    // till user releases button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(BUTTON_PIN, HIGH);
    while(digitalRead(BUTTON_PIN) == LOW){
       delay(200); 
    }

    setNetworkConnectStatusLight(status);
    // stop network so it can be restarted in
    // connect mode
    if(!networkStop()){
        Serial.println("couldn't stop network");
    }

    Serial.printf("OTA attempting to connect to ssid: %s, pass: %s\n",
        PUBLIC_SSID, PUBLIC_PASS);

    // start network
    if(config->networkMode == CONNECT){
        Serial.printf("OTA attempting to connect to ssid: %s, pass: %s\n",
            PUBLIC_SSID, PUBLIC_PASS);
        if(!networkConnect(PUBLIC_SSID, PUBLIC_PASS)){
            Serial.println("couldnt bring up network");
            flashFailStatusLight(status);
            return;
        }
    } else {
        Serial.printf("OTA attempting to create ssid: %s, pass: %s\n",
            PUBLIC_SSID, PUBLIC_PASS);
        if(!networkCreate(PUBLIC_SSID, PUBLIC_PASS, IPAddress(SERVER_IP_UINT32))){
            Serial.println("couldnt create network");
            flashFailStatusLight(status);
            return;
        }
    }

    networkAdvertise(OTA_HOSTNAME);
    Serial.printf("OTA advertising hostname: %s\n", OTA_HOSTNAME);

    if(!startSyncListeners()){
        Serial.println("couldnt start listening for events");
    }

    // ota
    otaOnStart(&otaStarted);
    otaOnProgress(&otaProgress);
    otaOnError(&otaError);
    otaOnEnd(&otaEnd);
    otaStart();

#ifdef COMPONENT
    buttonOnTap(btn, sendRegistrationRequest);
#endif

    flashSuccessStatusLight(status);
    setIdleStatusLight(status);
}

int shouldEnterSyncMode(){
    int buttonPosition;
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(BUTTON_PIN, HIGH);
    buttonPosition = digitalRead(BUTTON_PIN);

    for(int i = 0; i < 5; i++){
        if(buttonPosition == HIGH){
            return 0;
        }
        delay(200);
    }

    if(buttonPosition == HIGH){
        return 0;
    }

    return 1;
}

void setup(){
    Serial.begin(115200);
    Serial.println("\n");

    status = statusLightCreate(STATUS_PIN, 16);
    setFSWriteStatusLight(status);

    // load config from fs
    loadConfig();
    logConfig(config);

    // HACK - works around issue where this device
    // cannot make tcp connections to an esp which
    // is serving as an AP
    WiFi.persistent(false);

    if(shouldEnterSyncMode()){
        Serial.println("going to sync mode");
        enterSyncMode();
        return;
    }

    setNetworkConnectStatusLight(status);

    // start network
    switch(config->networkMode){
        case CONNECT:
            if(!networkConnect(config->ssid, config->pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config->hostname);
            break;
        case CREATE:
            if(!networkCreate(config->ssid, config->pass, IPAddress(192,168,4,1))){
                Serial.println("couldnt create up network");
            }
            networkAdvertise(config->hostname);
            break;
        case OFF:
            Serial.println("turning network off");
            if(!networkOff()){
                Serial.println("couldnt turn off network");
            }
            break;
        default:
            Serial.println("couldnt load network mode, defaulting to CONNECT");
            if(!networkConnect(config->ssid, config->pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config->hostname);
            break;
    }

    if(!startRunListeners()){
        Serial.println("couldnt start listening for events");
    }

#ifdef DEV_MODE
    Serial.println("DEV_MODE is on");
    if(!startSyncListeners()){
        Serial.println("couldnt start sync mode listeners");
    }
    otaOnStart(&otaStarted);
    otaOnProgress(&otaProgress);
    otaOnError(&otaError);
    otaOnEnd(&otaEnd);
    otaStart();

    buttonOnTap(btn, sendRegistrationRequest);
#endif

    setIdleStatusLight(status);
}

void loop(){
    loopTick();
}
