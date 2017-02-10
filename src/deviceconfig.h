#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <network.h>
#include <identity.h>

#define DB_VER 3
#define EVENT_VER 2
#define EVENT_PORT 6767
#define BUTTON_PIN 14
#define STATUS_PIN 2
// TODO - get BIN_VERSION from VERSION file
#define BIN_VERSION 1
#define STRIP_PIN 2
#define NUM_PX 4

// 192.168.4.1 17082560
// 192.168.43.20 338405568
#define SERVER_IP_UINT32 17082560

// number of components that can be registered
// for a controller
#define MAX_COMPONENTS 10

#define HOSTNAME "phoxdevice"
#define OTA_HOSTNAME "phoxdeviceota"

#define PUBLIC_SSID "phoxlight"
#define PUBLIC_PASS "phoxlight"

// TODO - obtain these from controller
#define PRIVATE_SSID "phoxlightpriv"
#define PRIVATE_PASS "phoxlightpriv"

typedef struct DeviceConfig {
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
    char hostname[HOSTNAME_MAX];
    NetworkMode networkMode;
    Identity components[MAX_COMPONENTS];
} DeviceConfig;

typedef struct PrivateNetworkCreds {
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
} PrivateNetworkCreds;

DeviceConfig * getConfig();
Identity * getIdentity();
PrivateNetworkCreds getPrivateCreds();

int loadConfig();
int writeConfig(DeviceConfig * c);
void logConfig(DeviceConfig * c);
int writeDefaultConfig();
int generatePrivateNetworkCreds();

int registerComponent(Identity * component);
// TODO - removeComponent

#endif
