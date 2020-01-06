#include <unistd.h>
#include <pthread.h>
#include "fileprocessing.h"
#include "screenhandling.h"
#include "main.h"
//#include "test.h"

//#define DEBUG
#define MAX_FRAMES 10

enum elementIds {
	BACKGROUND = 0,
	/*...*/
	BUTTON1 = 2, BUTTON2, BUTTON3, BUTTON4, BUTTON5,
	NOTE1 = 7, NOTE2, NOTE3, NOTE4, NOTE5,
	EGG
};

Element elements[MAX_ELEMENTS];

GameState gamestate;

pthread_t inputCycle;
pthread_mutex_t inputEventsLock;

LARGE_INTEGER Frequency;

int screenCycles ( void );
void * inputCycles ( void* );
void pushInputEvent ( long, int );

int main() {
	
#ifdef DEBUG
test():
return 0;
#endif

	int initZero = 0;
	gamestate = INACTIVE;
	memcpy( inputEvents, &initZero, sizeof( int ) );
	QueryPerformanceFrequency(&Frequency);
	initGamevals();

	strcpy(elements[BACKGROUND].name, "screen");
	if( loadElement("../Resources/default.txt", &elements[BACKGROUND]) != 0 ) {
		ERR("file","could not be opened");
	}
	addElement(&elements[BACKGROUND]);

	initScr();
	//printf("init\n");
	updatePalette(1,RATIONAL,5,FG);
	updatePalette(1,LINEAR,0,BG);
	//printf("palette\n");
	updateScreen();
	//printf("1st update\n");

	gamestate = ACTIVE;
	screenCycles();

	return 0;
}

int screenCycles( void ) {
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	long sleepTime = 1000000 / MAX_FRAMES;
	
	printf("starting thread\n");
	if( pthread_create( &inputCycle, NULL, &inputCycles, NULL ) != 0 ) {
		ERR("cannot create","thread");
	}

	while ( gamestate == ACTIVE ) {
		QueryPerformanceCounter(&StartingTime);
		{
			//updateDisplay();
			//updateScreen();
		}
		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
		usleep( sleepTime - ElapsedMicroseconds.QuadPart % sleepTime );
	}

	pthread_join( inputCycle, NULL );

	{//comments
		/*
		update display
		update screen
		calculate wait time */}
	return 0;
}

void* inputCycles( void* args ) {
	printf("thread started\n");

	int input;
	LARGE_INTEGER InitTime, EventTime, ElapsedMicroseconds;
	QueryPerformanceCounter(&InitTime);
	if( pthread_mutex_init(&inputEventsLock, NULL) != 0 ) {
		ERR("mutex","could not be initialized");
	}

	//strncpy(display->textVals,"thread started",strlen("thread started"));
	while ( gamestate == ACTIVE ) {
		//input = getchar();
		if( ( input = getchar() ) == 0 ) {
			input = getchar();
		}
		//printf("%c\n",input);
		QueryPerformanceCounter(&EventTime);
		ElapsedMicroseconds.QuadPart = EventTime.QuadPart - InitTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
		pushInputEvent( ElapsedMicroseconds.QuadPart, input );
	}

	pthread_mutex_destroy(&inputEventsLock);

	{//comments
		/*
		sleep on input
		once event processed, send to...
		-> update screen input stack
		(where does the mutex go? try to avoid blocking *too many* processes...)*/}
	return NULL;
}

void pushInputEvent ( long time, int event ) {
	if( event == 27 ) {
		gamestate = INACTIVE;
	}
	else
	{
		pthread_mutex_lock(&inputEventsLock);
		{
			int currentSize;
			memcpy( &currentSize, inputEvents, sizeof( int ) );
			++currentSize;
			inputEvents[currentSize].time = time;
			inputEvents[currentSize].event = event;
			memcpy( inputEvents, &currentSize, sizeof( int ) );
		}
		pthread_mutex_unlock(&inputEventsLock);
	}
}