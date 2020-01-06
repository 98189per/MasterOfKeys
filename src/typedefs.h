#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef CHAR_BYTE
#define CHAR_BYTE unsigned char
#define CHAR_BYTE_LIMIT 255
#define INT(char) ( char - 48 )
#define GRAD_SIZE 31
#endif
#ifndef INT_SIZE
#define INT_SIZE sizeof( CHAR_BYTE ) * sizeof( int )
#endif
#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif
#ifndef BOOL
#define BOOL int
#endif
#ifndef FG
#define FG TRUE
#define BG FALSE
#endif
#ifndef MAX_SIZE
#define MAX_SIZE 12 + 1
#define MAX_MOD 3
#endif
#ifndef MAX_ELEMENTS
#define MAX_ELEMENTS 50
#endif
#ifndef ERR
#define ERR(text, msg) printf( "\tERROR: %s %s", text , msg ); exit(1);
#endif
#ifndef CLEAR
#define CLEAR "\x1b[0m"
#endif

typedef enum {
    ACTIVE,
    PAUSED,
    INACTIVE
} GameState;

enum GradStyles {
    LINEAR,
    RATIONAL
};

typedef struct {
    long time;
    int event;
} InputEvent;

typedef struct property {
    char key[MAX_SIZE];
    char value[MAX_MOD * MAX_SIZE];
    struct property* nextProperty;
} Property;

typedef struct {
    char name[MAX_SIZE];
    int width;
    int height;
    CHAR_BYTE* sheet;
    Property* properties;
} Element;

typedef void (*FunctionPtr)(int, const char*, Element*);

typedef struct {
    int mod;
    char val[MAX_MOD * MAX_SIZE];
    Element* element;
    FunctionPtr function;
} PropertyUpdate;

typedef struct {
    int width;
    int height;
    CHAR_BYTE* sheet;
    char* textVals;
    int strLength;
    CHAR_BYTE colors[8][3];
    CHAR_BYTE strcolors[8][3];
} Screen;

CHAR_BYTE* displayCopy;
Screen * display;

InputEvent inputEvents[MAX_ELEMENTS];