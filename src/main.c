#include "main.h"
#include <mmsystem.h>

PropertyUpdate * updates[MAX_ELEMENTS];
BOOL songIsPlaying = FALSE;
void * soundBuffer;

typedef void (*FunctionPtr)(int, const char*, Element*);

//user-defined function declarations
unsigned long hash( CHAR_BYTE * );
void screen( int, const char *, Element *);
void pos( int, const char *, Element *);
void color( int, const char *, Element *);
void str( int, const char *, Element *);
void strcolor( int, const char *, Element *);
void scroll( int, const char *, Element *);
void active( int, const char *, Element *);
void link( int, const char *, Element *);
void name( int, const char *, Element *);
void onClick( int, const char *, Element *);
void atEsc( int, const char *, Element *);
void initList( int, const char *, Element *);
void listLayout( int, const char *, Element *);
void playSong( int, const char *, Element *);
void loadNotes( int, const char *, Element *);
void setSpeed( int, const char *, Element *);
void noteOffset( int, const char *, Element *);
void noteElement( int, const char *, Element *);
void staffSpec( int, const char *, Element *);

unsigned long * keyHashes;
FunctionPtr functions[] = {                                         //add function pointers here
    &screen, &pos, &color, &str, &strcolor, &scroll, &active, 
    &link, &name, &onClick, &atEsc, &initList, &listLayout,
    &playSong, &loadNotes, &setSpeed, &noteOffset, &noteElement,
    &staffSpec
};
const char* keyValues[] = {                                         //add corresponding property names here (order matters!)
    "screen", "1", "pos", "1", "color1", "8", "str", "1", "strcolor1", "8", 
    "scroll", "1", "active", "1", "link", "1", "name", "1", "onclick", "1", "esc", "1", 
    "startlist", "1", "listformat", "1", "song", "1", "notes", "1", "speed", "1", 
    "offset", "1", "note0", "14", "staff", "1"
};
const int noKeys = 46;                                              //DON'T FORGET TO UPDATE THIS VALUE AS WELL


//Define property functions here
void playSong(int mod, const char * val, Element * element) {
    FILE *soundPtr;
    long soundSize;

    if( ( soundPtr = fopen( val, "rb" ) ) == NULL ) {
        ERR(val,"could not be opened");
    }
    fseek( soundPtr, 0L, SEEK_END );
    soundSize = ftell( soundPtr );
    if( ( soundBuffer = malloc( soundSize ) ) == NULL ) {
        ERR("sound file","was probably too big...");
    }
    rewind( soundPtr );
    if( fread( soundBuffer, 1, soundSize, soundPtr ) != soundSize ) {
        ERR("sound file","would not load");
    }
    PlaySound(soundBuffer, GetModuleHandle(NULL), SND_MEMORY | SND_ASYNC);
}

void loadNotes(int mod, const char * val, Element * element) {
    if( val[0] == '.' ) {
        pthread_mutex_lock(&elementLoaderLock);
        {
            strncpy( currentSong.loadFrom, val, MAX_MOD * MAX_SIZE );
        }
        pthread_mutex_unlock(&elementLoaderLock);
        sleep(1);
        LARGE_INTEGER StartTime;
        QueryPerformanceCounter(&StartTime);
		StartTime.QuadPart *= 1000000;
		StartTime.QuadPart /= Frequency.QuadPart;
		StartTime.QuadPart -= initTime;
        currentSong.start = StartTime.QuadPart;
        songIsPlaying = TRUE;
    }
}

void setSpeed(int mod, const char * val, Element * element) {
    currentSong.speed = atoi( val );
}

void noteOffset(int mod, const char * val, Element * element) {
    currentSong.offset = atoi( val );
}

void noteElement(int mod, const char * val, Element * element) {
    int currentsize;
    memcpy( &currentsize, elementsToLoad, sizeof( int ) );
    pthread_mutex_lock(&elementLoaderLock);
    {
        memcpy( &currentsize, elementsToLoad, sizeof( int ) );
        ++currentsize;
        strcpy( elementsToLoad[currentsize].fileName, val );
        elementsToLoad[currentsize].elementId = MAX_ELEMENTS + mod + 1;
        memcpy( elementsToLoad, &currentsize, sizeof( int ) );
    }
    pthread_mutex_unlock(&elementLoaderLock);
}

void staffSpec(int mod, const char * val, Element * element) {
    char copy[MAX_MOD * MAX_SIZE];
    strncpy(copy, val, MAX_MOD * MAX_SIZE);
    char* cVal = strtok(copy, ";");
    currentSong.ytop = atoi( cVal );
    cVal = strtok(NULL, ";");
    currentSong.yint = atoi( cVal );
    cVal = strtok(NULL, ";");
    currentSong.xright = atoi( cVal );
    cVal = strtok(NULL, ";");
    currentSong.xleft = atoi( cVal );
}

void screen(int mod, const char * val, Element * element) {
    if( INT( *val ) ) {
        free( display->textVals );
        free( display->sheet );
        free( display );
        if( ( display = malloc( sizeof( Screen ) ) ) == NULL ) {
            ERR("display","ran out of memory");
        }
        display->width = element->width;
        display->height = element->height;
        int tmp = 0;
        memcpy( &tmp, element->sheet, INT_SIZE );
        int size = ( tmp + 1 ) * sizeof( CHAR_BYTE ) + sizeof( int );
        if( ( display->sheet = calloc( 1, size ) ) == NULL ) {
            ERR("display sheet","ran out of memory");
        }
        memcpy( display->sheet, element->sheet, size );
        if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
            ERR("display text","ran out of memory");
        }
        display->strLength = 0;
    }
}

void pos(int mod, const char * val, Element * element) {
    int x, y, start, end, count = INT_SIZE;
    sscanf(val, "%d;%d", &x, &y);
    start = display->width * y + x + INT_SIZE;
    end = display->width * ( y + element->height - 1 ) + ( x + element->width ) + INT_SIZE;
    for( int i = start; i < end; i++){
        display->sheet[i] = element->sheet[count];
        if( ( count - INT_SIZE ) % element->width == element->width - 1 ) {
            i += display->width - element->width;
        }
        ++count;
    }
}

void color(int mod, const char * val, Element * element) {
    char* copy;
    if( ( copy = malloc( ( strlen( val ) + 1 ) * sizeof( char ) ) ) == NULL ) {
        ERR("display color buffer","ran out of memory");
    }
    strcpy(copy, val);
    char* cVal = strtok(copy, ";");
    for( int i = 0; i < 3; i++) {
        display->colors[mod][i] = ( CHAR_BYTE ) atoi( cVal );
        cVal = strtok(NULL, ";");
    }
    free( copy );
}

void str(int mod, const char * val, Element * element) {
    int strx;
    char *copy, *str, strc[5];
    if( ( copy = malloc( ( strlen( val ) + 1 ) * sizeof( char ) ) ) == NULL ) {
        ERR("display str buffer","ran out of memory");
    }
    strcpy(copy, val);
    char* cVal = strtok(copy, ";");
    if( ( str = calloc( 1, ( strlen( cVal ) + 1 ) * sizeof( char ) ) ) == NULL ) {
        ERR("display str buffer buffer?","ran out of memory");
    }
    sscanf( cVal, " '%[^'] ", str );
    cVal = strtok(NULL, ";");
    strx = atoi( cVal );
    cVal = strtok(NULL, ";");
    while ( isspace( ( CHAR_BYTE ) *cVal ) ) {
        ++cVal;
    }
    if( *cVal == 0 ) {
        ERR("missing strcolor val in",element->name);
    }
    sprintf( strc, "\x81%s", cVal);
    if( display->strLength < display->height * display->width ) {
        char* tmp = realloc( display->textVals, ( display->strLength + ( strlen( val ) * strx ) + 5 ) * sizeof( char ) );
        if( tmp == NULL ) {
            ERR("display str buffer realloc","ran out of memory");
        }
        display->textVals = tmp;
        strncat( display->textVals, strc, 5 );
        for( int i = 0; i < strx; i++) {
            strcat( display->textVals, str );
        }
        display->strLength = strlen( display->textVals );
    }
    free( str );
    free( copy );
}

void strcolor(int mod, const char * val, Element * element) {
    char* copy;
    if( ( copy = malloc( ( strlen( val ) + 1 ) * sizeof( char ) ) ) == NULL ) {
        ERR("display strcolor buffer","ran out of memory");
    }
    strcpy(copy, val);
    char* cVal = strtok(copy, ";");
    for( int i = 0; i < 3; i++) {
        display->strcolors[mod][i] = ( CHAR_BYTE ) atoi( cVal );
        cVal = strtok(NULL, ";");
    }
    free( copy );
}

Element * scrollList[MAX_ELEMENTS][MAX_ELEMENTS + 2];
void scroll(int mod, const char * val, Element * element) {
    int listNo, listPos, listPosCpy, listTop;
    
    listNo = INT( *val );
    memcpy( &listPos, &(scrollList[listNo][MAX_ELEMENTS]), sizeof( int ) );
    memcpy( &listTop, &(scrollList[listNo][MAX_ELEMENTS + 1]), sizeof( int ) );
    int fixed, i = 0;
    if( scrollList[listNo][0] != NULL ) {
        fixed = strcmp( scrollList[listNo][0]->alt, "layout" );
    }
    while ( scrollList[listNo][i] != NULL ) {
        if( scrollList[listNo][i]->elementId == element->elementId ) {
            if( !fixed ) {
                listPos = i;
            }
            else
            {
                return;
            }
        }
        ++i;
    }
    if( element->elementId == MAX_ELEMENTS - 1 ) {
        listPosCpy = listPos;
        listPos = MAX_ELEMENTS - 1;
    }
    scrollList[listNo][listPos] = element;
    if( !fixed ) {
        char copy[MAX_MOD * MAX_SIZE];
        strcpy(copy, scrollList[listNo][0]->next);
        char* cVal = strtok(copy, ";");
        int noElements = atoi( cVal );
        int index = scrollList[listNo][listPos]->elementId - scrollList[listNo][listTop]->elementId;
        if( index < noElements && index > -1 ) {
            cVal = strtok(NULL, ";");
            int interval = atoi( cVal );
            cVal = strtok(NULL, ";");
            int startx = atoi( cVal );
            cVal = strtok(NULL, ";");
            int starty = atoi( cVal );
            starty += index * interval;
            char args[8];
            sprintf( args, "%d;%d", startx, starty );
            pos( 1, args, scrollList[listNo][listPos] );
        }
    }
    if( listPos == MAX_ELEMENTS - 1 ) {
        ++listPosCpy;
        memcpy( &(scrollList[listNo][MAX_ELEMENTS]), &listPosCpy, sizeof( int ) );
    }
    else
    {
        ++listPos;
        memcpy( &(scrollList[listNo][MAX_ELEMENTS]), &listPos, sizeof( int ) );
    }
}

void active(int mod, const char * val, Element * element) {
    strncpy(element->alt, val, MAX_MOD * MAX_SIZE);
}

void link(int mod, const char * val, Element * element) {
    char fileName[MAX_MOD * MAX_SIZE];
	int elementId;
    int currentsize;
    memcpy( &currentsize, elementsToLoad, sizeof( int ) );
    sscanf(val, "%s %d", fileName, &elementId);
    pthread_mutex_lock(&elementLoaderLock);
    {
        memcpy( &currentsize, elementsToLoad, sizeof( int ) );
        ++currentsize;
        strcpy( elementsToLoad[currentsize].fileName, fileName );
        elementsToLoad[currentsize].elementId = elementId;
        memcpy( elementsToLoad, &currentsize, sizeof( int ) );
    }
    pthread_mutex_unlock(&elementLoaderLock);
}

void name(int mod, const char * val, Element * element) {
    strncpy(element->name, val, MAX_SIZE);
}

void onClick(int mod, const char * val, Element * element) {
    strncpy(element->next, val, MAX_MOD * MAX_SIZE);
}

void atEsc(int mod, const char * val, Element * element) {
    strncpy(display->atEsc, val, MAX_MOD * MAX_SIZE);
}

void initList(int mod, const char * val, Element * element) {
    int init = 0;
    display->initList = atoi( val );
    memcpy( &(scrollList[display->initList][MAX_ELEMENTS]), &init, sizeof( int ) );
    ++init;
    memcpy( &(scrollList[display->initList][MAX_ELEMENTS + 1]), &init, sizeof( int ) );
}

void listLayout(int mod, const char * val, Element * element) {
    strcpy(element->alt, "layout");
    strncpy(element->next, val, MAX_MOD * MAX_SIZE);
}

//hashing function for matching keys
unsigned long hash(CHAR_BYTE *str) {
    unsigned long hash = 5381;
    int c;

    while(c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

//initialization of game values
void initGamevals( void ) {
    int initZero = 0, counter = 0;
    int length = sizeof( keyValues ) / sizeof( keyValues[0] );
    memcpy( updates, &initZero, sizeof( int ) );
    if( ( display = malloc( sizeof( Screen ) ) ) == NULL ) {
        ERR("initialization","ran out of memory");
    }
    if( ( display->sheet = calloc( 1, sizeof( CHAR_BYTE ) ) ) == NULL ) {
        ERR("init display","ran out of memory");
    }
    if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
        ERR("init display text","ran out of memory");
    }
    if( ( currentSong.notes = calloc( 1, sizeof( NoteCollection ) ) ) == NULL ) {
        ERR("song notes","ran out of memory");
    }
    for( int i = 0; i < 14; i++) {
        if( ( currentSong.skins[i] = malloc( sizeof( Element ) ) ) == NULL ) {
            ERR("note skins","could not be alloc'd");
        }
    }
    if( ( keyHashes = malloc( noKeys * sizeof( unsigned long ) ) ) == NULL ) {
        ERR("key hashes","ran out of memory");
    }
    if( pthread_mutex_init(&screenLock, NULL) != 0 ) {
        ERR("screen mutex","could not be initialized");
    }
	strncpy( currentSong.loadFrom, "\0", 2 );

    for( int i = 0; i < length; i += 2 ) {
        for( int j = 0; j < atoi( keyValues[i+1] ); j++) {
            keyHashes[counter] = hash( ( CHAR_BYTE * ) keyValues[i]) + ( unsigned long ) j;
            ++counter;
        }
    }
}

//process property chain and add values
void addElement(Element * element) {
    Property* propertyPtr;

    propertyPtr = element->properties;

    pthread_mutex_lock(&screenLock);
    {
        while ( TRUE ) {
            int fNum = 0, mod = 0, counter = 0;
            unsigned long next, current, tmp = hash( ( CHAR_BYTE * ) propertyPtr->key );
            while( ( current = keyHashes[counter] ) != tmp ) {
                next = keyHashes[counter + 1];
                if( ( next - current ) == 1 ) {
                    ++mod;
                }
                else
                {
                    mod = 0;
                    ++fNum;
                }
                ++counter;
            }
            functions[fNum](mod, propertyPtr->value, element);
            if( ( propertyPtr = propertyPtr->nextProperty ) == NULL ) {
                break;
            }
        }
    }
    pthread_mutex_unlock(&screenLock);
}

//push to loading stack
void activate(Element * element) {
    char fileName[MAX_MOD * MAX_SIZE];
	ElementIds elementId = element->elementId;
    int currentsize;
    memcpy( &currentsize, elementsToLoad, sizeof( int ) );

    strncpy(fileName, element->alt, MAX_MOD * MAX_SIZE);
    pthread_mutex_lock(&elementLoaderLock);
    {
        memcpy( &currentsize, elementsToLoad, sizeof( int ) );
        ++currentsize;
        strcpy( elementsToLoad[currentsize].fileName, fileName);
        elementsToLoad[currentsize].elementId = elementId;
        memcpy( elementsToLoad, &currentsize, sizeof( int ) );
    }
    pthread_mutex_unlock(&elementLoaderLock);
}

void initScrollList( int listNo ) {
    sleep(1);
    if( scrollList[listNo][0] != NULL ) {
        if( strcmp(scrollList[listNo][0]->alt, "layout") != 0 ) {
            activate(scrollList[listNo][0]);
        }
    }
}

//300 line function that executes every second... aka i was too lazy to abstract this part
int updateDisplay( Element * screen ) {
    static int currentListNo = 0, currentListPos = 0;
    static int stdX[MAX_ELEMENTS];
    long time;
    int event;

    //pop input stack
    int currentSize;
    memcpy( &currentSize, inputEvents, sizeof( int ) );
    if( currentSize > 0 ) {
        pthread_mutex_lock(&inputEventsLock);
        {
            time = inputEvents[currentSize].time;
            event = inputEvents[currentSize].event;
            --currentSize;
            memcpy( inputEvents, &currentSize, sizeof( int ) );
        }
        pthread_mutex_unlock(&inputEventsLock);
    }

    //process key events (mainly deals with scrolling menus for now)
    currentListNo = display->initList;
    switch( event ) {
        case KEY_UP:
            if( strcmp( scrollList[currentListNo][0]->alt, "layout" ) == 0 ) {
                if( currentListPos == 0 ) {
                    ++currentListPos;
                }
                if( currentListPos > 1 ) {
                    activate(scrollList[currentListNo][currentListPos]);
                    activate(scrollList[currentListNo][currentListPos - 1]);
                    sleep(1);

                    int listTop, insertion = -1, count = 0;
                    memcpy( &listTop, &(scrollList[currentListNo][MAX_ELEMENTS + 1]), sizeof( int ) );
                    char copy[MAX_MOD * MAX_SIZE];
                    strcpy(copy, scrollList[currentListNo][0]->next);
                    char* cVal = strtok(copy, ";");
                    int noElements = atoi( cVal );
                    for( int i = 0; i < 4; i++) {
                        cVal = strtok(NULL, ";");
                    }
                    if( cVal != NULL ) {
                        insertion = atoi( cVal );
                    }
                    free( display->textVals );
                    if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
                        ERR("display text","ran out of memory");
                    }
                    
                    if( currentListPos == listTop ) {
                        --listTop;
                        memcpy( &(scrollList[currentListNo][MAX_ELEMENTS + 1]), &listTop, sizeof( int ) );
                    }
                    display->strLength = 0;
                    for( int i = listTop; i < listTop + noElements; i++) {
                        if( count == insertion ) {
                            Property *propertyPtr = scrollList[currentListNo][MAX_ELEMENTS - 1]->properties;
                            char list[4];
                            unsigned long result = hash("str");
                            sprintf( list, "%d", currentListNo );
                            scroll( 1, list, scrollList[currentListNo][MAX_ELEMENTS - 1] );
                            while ( propertyPtr != NULL ) {
                                if( hash(propertyPtr->key) == result ) {
                                    str( 1, propertyPtr->value, scrollList[currentListNo][MAX_ELEMENTS - 1] );
                                }
                                propertyPtr = propertyPtr->nextProperty;
                            }
                        }
                        Property *propertyPtr = scrollList[currentListNo][i]->properties;
                        char list[4];
                        unsigned long result = hash("str");
                        sprintf( list, "%d", currentListNo );
                        scroll( 1, list, scrollList[currentListNo][i] );
                        while ( propertyPtr != NULL ) {
                            if( hash(propertyPtr->key) == result ) {
                                str( 1, propertyPtr->value, scrollList[currentListNo][i] );
                            }
                            propertyPtr = propertyPtr->nextProperty;
                        }
                        ++count;
                    }
                    --currentListPos;
                }
            }
            else
            {
                if( currentListPos > 0 ) {
                    activate(scrollList[currentListNo][currentListPos]);
                    --currentListPos;
                    activate(scrollList[currentListNo][currentListPos]);
                }
            }
            break;
        case KEY_DOWN:
            if( strcmp( scrollList[currentListNo][0]->alt, "layout" ) == 0 ) {
                if( scrollList[currentListNo][currentListPos + 1] != NULL ) {
                    if( currentListPos < MAX_ELEMENTS && scrollList[currentListNo][currentListPos]->elementId < scrollList[currentListNo][currentListPos + 1]->elementId && scrollList[currentListNo][currentListPos + 1]->elementId != MAX_ELEMENTS - 1 ) {
                        if( currentListPos != 0 ) {
                            activate(scrollList[currentListNo][currentListPos]);
                        }
                        activate(scrollList[currentListNo][currentListPos + 1]);
                        sleep(1);
                        
                        int listTop, insertion = -1, count = 0;
                        memcpy( &listTop, &(scrollList[currentListNo][MAX_ELEMENTS + 1]), sizeof( int ) );
                        char copy[MAX_MOD * MAX_SIZE];
                        strcpy(copy, scrollList[currentListNo][0]->next);
                        char* cVal = strtok(copy, ";");
                        int noElements = atoi( cVal );
                        for( int i = 0; i < 4; i++) {
                            cVal = strtok(NULL, ";");
                        }
                        if( cVal != NULL ) {
                            insertion = atoi( cVal );
                        }
                        free( display->textVals );
                        if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
                            ERR("display text","ran out of memory");
                        }
                        display->strLength = 0;
                        if( currentListPos == listTop + noElements - 1 ) {
                            ++listTop;
                            memcpy( &(scrollList[currentListNo][MAX_ELEMENTS + 1]), &listTop, sizeof( int ) );
                            int i = 1;
                        }
                        for( int i = listTop; i < listTop + noElements; i++) {
                            if( count == insertion ) {
                                Property *propertyPtr = scrollList[currentListNo][MAX_ELEMENTS - 1]->properties;
                                char list[4];
                                unsigned long result = hash("str");
                                sprintf( list, "%d", currentListNo );
                                scroll( 1, list, scrollList[currentListNo][MAX_ELEMENTS - 1] );
                                while ( propertyPtr != NULL ) {
                                    if( hash(propertyPtr->key) == result ) {
                                        str( 1, propertyPtr->value, scrollList[currentListNo][MAX_ELEMENTS - 1] );
                                    }
                                    propertyPtr = propertyPtr->nextProperty;
                                }
                            }
                            Property *propertyPtr = scrollList[currentListNo][i]->properties;
                            char list[4];
                            unsigned long result = hash("str");
                            sprintf( list, "%d", currentListNo );
                            scroll( 1, list, scrollList[currentListNo][i] );
                            while ( propertyPtr != NULL ) {
                                if( hash(propertyPtr->key) == result ) {
                                    str( 1, propertyPtr->value, scrollList[currentListNo][i] );
                                }
                                propertyPtr = propertyPtr->nextProperty;
                            }
                            ++count;
                        }
                        ++currentListPos;
                    }
                }
            }
            else
            {
                if( currentListPos < MAX_ELEMENTS && scrollList[currentListNo][currentListPos + 1] != NULL ) {
                    activate(scrollList[currentListNo][currentListPos]);
                    ++currentListPos;
                    activate(scrollList[currentListNo][currentListPos]);
                }
            }
            break;
        case KEY_CR:
            {
                char fileName[MAX_MOD * MAX_SIZE];
                ElementIds elementId = BACKGROUND;
                int currentsize;
                memcpy( &currentsize, elementsToLoad, sizeof( int ) );

                if( scrollList[currentListNo][currentListPos]->next[0] != 0 && strcmp( scrollList[currentListNo][currentListPos]->alt, "layout" ) != 0 ) {
                    strncpy(fileName, scrollList[currentListNo][currentListPos]->next, MAX_MOD * MAX_SIZE);
                    if( strcmp( fileName, "exit" ) == 0 ) {
                        pthread_mutex_destroy(&screenLock);
                        return INACTIVE;
                    }
                    pthread_mutex_lock(&elementLoaderLock);
                    {
                        ++currentsize;
                        strcpy( elementsToLoad[currentsize].fileName, fileName);
                        elementsToLoad[currentsize].elementId = elementId;
                        memcpy( elementsToLoad, &currentsize, sizeof( int ) );
                    }
                    pthread_mutex_unlock(&elementLoaderLock);
                    currentListPos = 0;
                }
            }
            break;
        case KEY_ESC:
            {
                LEFT(1);
                char fileName[MAX_MOD * MAX_SIZE];
                ElementIds elementId = BACKGROUND;
                int currentsize;
                memcpy( &currentsize, elementsToLoad, sizeof( int ) );

                strncpy(fileName, display->atEsc, MAX_MOD * MAX_SIZE);                
                if( strcmp( fileName, "exit" ) == 0 ) {
                    pthread_mutex_destroy(&screenLock);
                    return INACTIVE;
                }
                pthread_mutex_lock(&elementLoaderLock);
                {
                    ++currentsize;
                    strcpy( elementsToLoad[currentsize].fileName, fileName);
                    elementsToLoad[currentsize].elementId = elementId;
                    memcpy( elementsToLoad, &currentsize, sizeof( int ) );
                }
                pthread_mutex_unlock(&elementLoaderLock);
                if( songIsPlaying ) {
                    PlaySound(NULL, 0, 0);
                    free( soundBuffer );
                    songIsPlaying = FALSE;
                }
                currentListPos = 0;
            }
            break;
        default:
        break;
    }

    //update song notes
    static long lastDelta = 0;
    if( songIsPlaying ) {
        long noteTime, delta = 0;
        char args[8];
        LARGE_INTEGER deltaTime;
        QueryPerformanceCounter(&deltaTime);
		deltaTime.QuadPart *= 1000000;
		deltaTime.QuadPart /= Frequency.QuadPart;
		deltaTime.QuadPart -= initTime + currentSong.offset * 1000;
        delta = deltaTime.QuadPart + currentSong.xleft * 1000;
        int startx, starty, counter = 0;
        long maxDelta = deltaTime.QuadPart + currentSong.speed * currentSong.usPerNote;
        if( lastDelta == 0 ) {
            lastDelta = delta;
        }
        
        pthread_mutex_lock(&screenLock);
        {
            int tmp = 0;
            memcpy( &tmp, screen->sheet, INT_SIZE );
            int size = ( tmp + 1 ) * sizeof( CHAR_BYTE ) + sizeof( int );
            memcpy( display->sheet, screen->sheet, size );
            while ( TRUE ) {
                if( currentSong.notes[counter].note == EOS ) {
                    if( ( currentSong.start + currentSong.notes[counter - 1].time ) < delta ) {
                        pthread_mutex_lock(&inputEventsLock);
                        {
                            int currentSize;
                            memcpy( &currentSize, inputEvents, sizeof( int ) );
                            ++currentSize;
                            inputEvents[currentSize].time = delta;
                            inputEvents[currentSize].event = KEY_ESC;
                            memcpy( inputEvents, &currentSize, sizeof( int ) );
                        }
                        pthread_mutex_unlock(&inputEventsLock);
                    }
                    break;
                }
                noteTime = currentSong.start + currentSong.notes[counter].time;
                starty = currentSong.ytop + currentSong.yint * ( 14 - currentSong.notes[counter].note );
                if( delta < noteTime && noteTime < maxDelta ) {
                    startx = ( noteTime - lastDelta ) * currentSong.xright / ( currentSong.speed * currentSong.usPerNote );
                    sprintf( args, "%d;%d", startx, starty );
                    if( currentSong.notes[counter].note % 2 == 1 && currentSong.notes[counter].note != 7 ) {
                        pos( 1, args, currentSong.skins[1] );
                    }
                    else
                    {
                        pos( 1, args, currentSong.skins[0] );
                    }
                    startx = ( noteTime - delta ) * currentSong.xright / ( currentSong.speed * currentSong.usPerNote );
                    sprintf( args, "%d;%d", startx, starty );
                    pos( 1, args, currentSong.skins[2] );
                }
                if( maxDelta < noteTime ) {
                    break;
                }            
                ++counter;
            }
        }
        pthread_mutex_unlock(&screenLock);
        lastDelta = delta;
    }
    else
    {        
        lastDelta = 0;
    }

    //determines whether current game cycle is over yet
    return ACTIVE;
}