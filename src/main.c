#include "main.h"

PropertyUpdate * updates[MAX_ELEMENTS];

typedef void (*FunctionPtr)(int, const char*, Element*);

unsigned long hash( CHAR_BYTE * );
void screen( int, const char *, Element *);
void pos( int, const char *, Element *);
void color( int, const char *, Element *);
void str( int, const char *, Element *);
void strcolor( int, const char *, Element *);

unsigned long * keyHashes;
FunctionPtr functions[] = { &screen, &pos, &color, &str, &strcolor};
const char* keyValues[] = { "screen", "1", "pos-x", "2", "color1", "8", "str", "1", "strcolor1", "8"};
const int noKeys = 20;

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
        if( ( display->sheet = calloc( 1, ( tmp + 1 ) * sizeof( CHAR_BYTE ) + sizeof( int ) ) ) == NULL ) {
            ERR("display sheet","ran out of memory");
        }
        display->sheet = element->sheet;
        if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
            ERR("display text","ran out of memory");
        }
    }
}

void pos(int mod, const char * val, Element * element) {
    printf("pos%d, val-%s\n", mod, val);
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
    if( strc == NULL ) {
        ERR("there is less than 1 byte of space available..."," RUN!!!");
    }
    while ( isspace( ( CHAR_BYTE ) *cVal ) ) {
        ++cVal;
    }
    if( *cVal == 0 ) {
        ERR("missing strcolor val in",element->name);
    }
    sprintf( strc, "\x81%s", cVal);
    char* tmp = realloc( display->textVals, ( display->strLength + ( strlen( val ) * strx ) + 5 ) * sizeof( char ) );
    if( tmp == NULL ) {
        ERR("display str buffer","ran out of memory");
    }
    display->textVals = tmp;
    strncat( display->textVals, strc, 5 );
    for( int i = 0; i < strx; i++) {
        strcat( display->textVals, str );
    }
    display->strLength = strlen( display->textVals );
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

unsigned long hash(CHAR_BYTE *str) {
    unsigned long hash = 5381;
    int c;

    while(c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void initGamevals( void ) {
    int initZero = 0, counter = 0;
    int length = sizeof( keyValues ) / sizeof( keyValues[0] );
    memcpy( updates, &initZero, sizeof( int ) );
    if( ( displayCopy = calloc( 1, sizeof( CHAR_BYTE ) ) ) == NULL ) {
        ERR("init display buffer","ran out of memory");
    }
    if( ( display = malloc( sizeof( Screen ) ) ) == NULL ) {
        ERR("initialization","ran out of memory");
    }
    if( ( display->sheet = calloc( 1, sizeof( CHAR_BYTE ) ) ) == NULL ) {
        ERR("init display","ran out of memory");
    }
    if( ( display->textVals = calloc( 1, sizeof( char ) ) ) == NULL ) {
        ERR("init display text","ran out of memory");
    }
    if( ( keyHashes = malloc( noKeys * sizeof( unsigned long ) ) ) == NULL ) {
        ERR("key hashes","ran out of memory");
    }

    for( int i = 0; i < length; i += 2 ) {
        for( int j = 0; j < ( int ) INT( *keyValues[i+1] ); j++) {
            keyHashes[counter] = hash( ( CHAR_BYTE * ) keyValues[i]) + ( unsigned long ) j;
            ++counter;
        }
    }
}

void addElement(Element * element) {
    Property* propertyPtr;

    propertyPtr = element->properties;

    while( TRUE ) {
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

void updateDisplay( void ) {
    long time;
    int event;

    int currentSize;
    memcpy( &currentSize, inputEvents, sizeof( int ) );
    if( currentSize > 0 ) {
        inputEvents[currentSize].time = time;
        inputEvents[currentSize].event = event;
        --currentSize;
        memcpy( inputEvents, &currentSize, sizeof( int ) );
        //sprintf( display->textVals, "time-%ld key-%d", time, event );
        //should check for values other than just the screen
    }

    /*
    called once a cycle
    process update stack
    */
}