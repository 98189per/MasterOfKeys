#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>
#include <pthread.h>

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
#define TXT FALSE
#endif
#ifndef MAX_SIZE
#define MAX_SIZE 20 + 1
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
#ifndef SPEC_KEY
#define SPEC_KEY 0xE0
#endif
#ifndef UP
#define UP(n) printf("\x1b[%dA",n)
#define DOWN(n) printf("\x1b[%dB",n)
#define RIGHT(n) printf("\x1b[%dC",n)
#define LEFT(n) printf("\x1b[%dD",n)
#endif
#ifndef RESIZE
#define RESIZE(x,y) printf("\x1b[8;%d;%dt",y,x)
#endif

enum GradStyles {
    LINEAR,
    RATIONAL
};

enum keyVals {
    KEY_BS = 8, KEY_HT = 9, KEY_CR = 13, KEY_ESC = 27,
    KEY_SPC = 32, EXCLAMATION, DOUBLE_QUOTE, NUMBER_SIGN, DOLLAR_SIGN, 
    PERCENT, AMPERSAND, SINGLE_QUOTE, LEFT_PAR, RIGHT_PAR, ASTERISK,
    PLUS, COMMA, MINUS, PERIOD, FWSH, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    COLON, SEMICOLON, LESS_THAN, EQUAL_SIGN, GREATER_THAN, QUESTION_MARK,
    AT_SIGN, UPPER_A, UPPER_B, UPPER_C, UPPER_D, UPPER_E, UPPER_F, UPPER_G, UPPER_H, 
    UPPER_I, UPPER_J, UPPER_K, UPPER_L, UPPER_M, UPPER_N, UPPER_O, UPPER_P, 
    UPPER_Q, UPPER_R, UPPER_S, UPPER_T, UPPER_U, UPPER_V, UPPER_W, UPPER_X, 
    UPPER_Y, UPPER_Z, LEFT_SQR, BKSH, RIGHT_SQR, CARET, UNDERSCORE, GRAVE,
    LOWER_A, LOWER_B, LOWER_C, LOWER_D, LOWER_E, LOWER_F, LOWER_G, 
    LOWER_H, LOWER_I, LOWER_J, LOWER_K, LOWER_L, LOWER_M, LOWER_N, LOWER_O, LOWER_P, 
    LOWER_Q, LOWER_R, LOWER_S, LOWER_T, LOWER_U, LOWER_V, LOWER_W, LOWER_X, LOWER_Y, LOWER_Z, 
    LEFT_CUR, VERITCAL_BAR, RIGHT_CUR, TILDE,
    KEY_F1 = 314, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_HM = 326, KEY_UP, KEY_PGUP, 
    KEY_LEFT = 330, KEY_RIGHT = 332,
    KEY_END = 334, KEY_DOWN, KEY_PGDN,
    KEY_F11 = 388, KEY_F12, KEY_INS = 337, KEY_DEL
};

typedef enum {
    ACTIVE,
    GAMEPLAY,
    PAUSED,
    INACTIVE
} GameState;

typedef enum {
	BACKGROUND = 0,
	/*...*/
	BUTTON1 = 2, BUTTON2, BUTTON3, BUTTON4, BUTTON5,
	NOTE1 = 7, NOTE2, NOTE3, NOTE4, NOTE5,
	EGG
} ElementIds;

typedef enum {
    C3, D3, E3, F3, G3, A3, B3, 
    C4, D4, E4, F4, G4, A4, B4, 
    EOS
} Scale;

typedef struct {
    long time;
    Scale note;
} NoteCollection;

typedef struct {
    long time;
    int event;
} InputEvent;

typedef struct {
    char fileName[MAX_MOD * MAX_SIZE];
    ElementIds elementId;
} ElementLoader;

typedef struct property {
    char key[MAX_SIZE];
    char value[MAX_MOD * MAX_SIZE];
    struct property* nextProperty;
} Property;

typedef struct {
    char name[MAX_SIZE];
    int width, height;
    CHAR_BYTE* sheet;
    char alt[MAX_MOD * MAX_SIZE];
    char next[MAX_MOD * MAX_SIZE];
    ElementIds elementId;
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
    int width, height;
    CHAR_BYTE* sheet;
    char* textVals;
    int strLength, initList;
    char atEsc[MAX_MOD * MAX_SIZE];
    CHAR_BYTE colors[8][3];
    CHAR_BYTE strcolors[8][3];
} Screen;

typedef struct {
    long start;
    int usPerNote, ticksPerNote;
    char loadFrom[MAX_MOD * MAX_SIZE];
    int offset, speed, ytop, yint, xright;
    Element * skins[14];
    NoteCollection * notes;
} Song;

long initTime;

Screen * display;
Song currentSong;

InputEvent inputEvents[MAX_MOD * MAX_ELEMENTS];
ElementLoader elementsToLoad[MAX_ELEMENTS];
pthread_mutex_t inputEventsLock, elementLoaderLock, screenLock;

LARGE_INTEGER Frequency;