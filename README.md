Phox Device
------------
Phox Device is the boilerplate for a Phox Device... that is, an esp8266 based piece of hardware that leverages the PhoxCore platform.

Development
------------
There are a few tools required to develop this stuff:
* \*nix box with make, gcc, the usual stuff
* [makeEspArduino makefile](https://github.com/plerup/makeEspArduino)
* [ArduinoEsp](https://github.com/esp8266/Arduino)

First, clone this repo. Be sure to include `--recursive` so that it will pull down submodules:

    git clone --recursive https://github.com/Phoxlights/PhoxDevice.git

Then update the makefile to point to your local copy of the makeEspArduino and the ArduinoEsp repo:

    ESP_MAKE=$(HOME)/src/makeEspArduino
    ESP_LIBS=$(HOME)/src/ArduinoEsp/libraries

Finally, hook up your esp8266 and try `make upload`. Will it work? Probably!

Oh, if you want to send messages to your PhoxDevice, there are some make targets to do that, but you will need nodejs because I am a terrible person and wrote the message sending stuff in JavaScript. If you have nodejs setup, navigate to the PhoxTools submodule and install deps:

    cd libs/PhoxTools/messenger && npm install

Then you can send messages to your PhoxDevice, assuming your dev box is on the same network:

    > make ping
    message sent, awaiting response
    message get <Buffer 02 00 00 00 00 00 01 00 e9 03 00 00>
    PONG!
    message header:
        versionMajor: 2,
        versionMinor: 0,
        responseId: 0,
        requestId: 1,
        opCode: 1001,
        length: 0,

Releasing
-----------
Use the [git flow](http://nvie.com/posts/a-successful-git-branching-model/) release process. 

Release checklist:
* Bump to BIN_VERSION in whatever sketch (TODO - automatically pull this from VERSION)
* Build a new bin and put it in `build`. This can be done with `make build`.
* Update `CHANGELOG`
* Make sure submodules are pinned to a release tag
* *After* release, bump the version in `VERSION` and push to develop

TODO
------------
* Identity object for `who` command is all jacked :/
* Test controller mode
