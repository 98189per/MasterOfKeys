#include <conio.h>
#include "fileprocessing.h"
#include "screenhandling.h"
#include "main.h"
//#include "test.h"

//#define DEBUG
#define MAX_FRAMES 10

Element elements[MAX_ELEMENTS];

GameState gamestate;

pthread_t inputCycle, loadCycle;

int screenCycles ( void );
void * loadElements ( void* );
void * inputCycles ( void* );
void pushInputEvent ( long, int );

int main() {
// Testing block
#ifdef DEBUG
NoteCollection * currentSongNotes;
//currentSongNotes = calloc( 50, 1 * sizeof( NoteCollection ) );
if((currentSongNotes=loadSongNotes("../Songs/test-a/The_Seven_Seas_[Tutorial]_rev1.mid"))==NULL){
	ERR("bruh","bruh");
}
for(int i = 0; i < 50; i++){
	printf("%d %ld\t",currentSongNotes[i].note,currentSongNotes[i].time);
}
return 0;
#endif

	//Game initialization 
	int initZero = 0;
	gamestate = INACTIVE;
	memcpy( inputEvents, &initZero, sizeof( int ) );
	memcpy( elementsToLoad, &initZero, sizeof( int ) );
	if( pthread_mutex_init(&elementLoaderLock, NULL) != 0 ) {
		ERR("element mutex","could not be initialized");
	}
	QueryPerformanceFrequency(&Frequency);
	initGamevals();

	//Load default.txt
	strcpy(elements[BACKGROUND].name,"screen");
	elements[BACKGROUND].elementId = BACKGROUND;
	if( loadElement("../Resources/default.txt", &elements[BACKGROUND]) != 0 ) {
		ERR(elements[BACKGROUND].name,"could not be opened");
	}
	addElement(&elements[BACKGROUND]);

	initScr();
	updatePalette(0,LINEAR,0,FG);
	updatePalette(0,LINEAR,0,TXT);
	updateScreen();

	//start game cycles
	gamestate = ACTIVE;
	screenCycles();

	for( int i = 0; i < MAX_ELEMENTS; i++) {
		unloadElement( &elements[i] ); 
	}
	puts("successfully exited program");

	return 0;
}

int screenCycles( void ) {
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	const long sleepTime = 1000000 / MAX_FRAMES;
	static int prev_init = 0;
	
	//Spawn threads
	if( pthread_create( &inputCycle, NULL, &inputCycles, NULL ) != 0 ) {
		ERR("cannot create","thread");
	}
	if( pthread_create( &loadCycle, NULL, &loadElements, NULL ) != 0 ) {
		ERR("cannot create","thread cycles");
	}

	initScrollList(0);

	//constant loop
	while ( gamestate == ACTIVE ) {
		QueryPerformanceCounter(&StartingTime);
		{
			gamestate = updateDisplay( &elements[BACKGROUND] );
			pthread_mutex_lock(&screenLock);
			updateScreen();
			pthread_mutex_unlock(&screenLock);
			if( display->initList != prev_init ) {
				initScrollList(display->initList);
			}
			prev_init = display->initList;
		}
		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
		usleep( sleepTime - ElapsedMicroseconds.QuadPart % sleepTime );
	}

	//Exit prompt
	system("cls");
    puts("\x1b[1mPress any key to exit" CLEAR);
	pthread_join( inputCycle, NULL );
	pthread_join( loadCycle, NULL );
	return 0;
}

void* loadElements( void* args ) {
	const long sleepTime = 1000000 / MAX_FRAMES;
	char fileName[MAX_MOD * MAX_SIZE];
	ElementIds elementId;
	int currentsize;

	//process loading stack
	while ( gamestate == ACTIVE ) {
		memcpy( &currentsize, elementsToLoad, sizeof( int ) );
		while ( currentsize > 0 ) {
			pthread_mutex_lock(&elementLoaderLock);
			{
				if( currentSong.loadFrom[0] == '.' ) {
					free( currentSong.notes );
					if( ( currentSong.notes = loadSongNotes( currentSong.loadFrom ) ) == NULL ) {
						ERR("couldn't load",currentSong.loadFrom);
					}
					strncpy( currentSong.loadFrom, "\0", 2 );
				}
				memcpy( &currentsize, elementsToLoad, sizeof( int ) );
				elementId = elementsToLoad[currentsize].elementId;
				strcpy( fileName, elementsToLoad[currentsize].fileName );
				--currentsize;
				memcpy( elementsToLoad, &currentsize, sizeof( int ) );
				if( elementId > MAX_ELEMENTS ) {
					if( currentSong.skins[elementId - MAX_ELEMENTS - 1] != NULL ) {
						unloadElement( currentSong.skins[elementId - MAX_ELEMENTS - 1] );
					}
					if( ( currentSong.skins[elementId - MAX_ELEMENTS - 1] = malloc( sizeof( Element ) ) ) == NULL ) {
						ERR(fileName,"could not be alloc'd");
					}
					if( loadElement( fileName, currentSong.skins[elementId - MAX_ELEMENTS - 1] ) != 0 ) {
						ERR(fileName,"could not be opened");
					}
					pthread_mutex_unlock(&elementLoaderLock);
					continue;
				}
				if( loadElement( fileName, &elements[elementId] ) != 0 ) {
					ERR(fileName,"could not be opened");
				}
			}
			pthread_mutex_unlock(&elementLoaderLock);
			elements[elementId].elementId = elementId;
			addElement( &elements[elementId] );
		}
		usleep(sleepTime);
	}

	pthread_mutex_destroy(&elementLoaderLock);
}

void* inputCycles( void* args ) {
	int input;
	LARGE_INTEGER InitTime, EventTime;
	QueryPerformanceCounter(&InitTime);
	InitTime.QuadPart *= 1000000;
	InitTime.QuadPart /= Frequency.QuadPart;
	initTime = InitTime.QuadPart;
	if( pthread_mutex_init(&inputEventsLock, NULL) != 0 ) {
		ERR("mutex","could not be initialized");
	}

	//get user input one character at a time
	while ( gamestate == ACTIVE ) {
		input = getch();
		if( input == SPEC_KEY || input == 0 ) {
			input = CHAR_BYTE_LIMIT + getch();
		}
		QueryPerformanceCounter(&EventTime);
		EventTime.QuadPart *= 1000000;
		EventTime.QuadPart /= Frequency.QuadPart;
		EventTime.QuadPart -= initTime;
		pushInputEvent( EventTime.QuadPart, input );
	}

	pthread_mutex_destroy(&inputEventsLock);
	return NULL;
}

void pushInputEvent ( long time, int event ) {
	//push to input stack
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