#include <loop.h>
#include <animator.h>
#include <blendingmode.h>
#include <alltransforms.h>
#include "strip.h"

struct State {
    int numPx;
    Animator animator;
    LightStrip strip;
    Bitmap * bmp;
    byte * buffer;
};
// i dont even care, just global it
struct State stripState;

static byte RED[] = {255,0,0,255};
static byte GREEN[] = {0,255,0,255};
static byte BLUE[] = {0,0,255,255};

static void stripTick(void * s){
    State * state = (State*)s;
    animatorTick(state->animator);
    lightStripTick(state->strip);
}

int stripStartDefault(){
    Bitmap * bmp = stripState.bmp;
    Bitmap_fill(bmp, BLUE);

    // NOTE - if a layer already exists at the specified index
    // it will be freed before the new one is added
    AnimatorLayer l = animatorLayerCreate(stripState.animator, stripState.numPx, 1, 0);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k1, createTransformRGB(RED, GREEN, REPLACE));
    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k2, createTransformRGB(GREEN, BLUE, REPLACE));
    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k3, createTransformRGB(BLUE, RED, REPLACE));

    return 1;
}

int setupLight(int pin, int numPx){
    stripState.numPx = numPx;
    stripState.animator = animatorCreate(numPx, 1);
    if(stripState.animator == NULL){
        Serial.println("couldnt create animator");
        return NULL;
    }
    stripState.buffer = animatorGetBuffer(stripState.animator)->data;
    stripState.strip = lightStripCreate(pin, numPx, 1.0, stripState.buffer);
    if(stripState.strip == NULL){
        Serial.println("couldnt create strip");
        return NULL;
    }

    stripState.bmp = Bitmap_create(numPx, 1);

    // if no layer is loaded, animator tick asplode
    stripStartDefault();

    loopAttach(stripTick, 30, &stripState);

    return 1;
}

int stripLoadEditable(){
    Bitmap * bmp = stripState.bmp;
    Bitmap_fill(bmp, GREEN);

    // NOTE - if a layer already exists at the specified index
    // it will be freed before the new one is added
    AnimatorLayer l = animatorLayerCreate(stripState.animator, stripState.numPx, 1, 0);
    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 30, bmp);
    return 1;
}

int stripSetColor(byte rgb[3]){
    Bitmap * bmp = stripState.bmp;
    byte rgba[] = {rgb[0], rgb[1], rgb[2], 255};
    Bitmap_fill(bmp, rgba);
    return 1;
}
