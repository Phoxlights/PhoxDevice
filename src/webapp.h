#ifndef WEBAPP_H
#define WEBAPP_H

typedef int (*statefulCallback)(void * state);

int webAppBegin();
int webAppOnReady(statefulCallback onReady, void * state);
int webAppOnColor(statefulCallback onColor, void * state);

#endif
