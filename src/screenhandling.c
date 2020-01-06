#include "screenhandling.h"

char* colorGen(int, int, int, int, BOOL);
int linearGradient(int, int, int);
int rationalGradient(int, int, int, int);
COORD GetConsoleCursorPosition(HANDLE);

char colorList[CHAR_BYTE_LIMIT][20];
char strcolorList[CHAR_BYTE_LIMIT][20];

void initScr( void ) {
    system("cls");
    printf("\x1b[8;%d;%dt", display->height, display->width);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( hOut == INVALID_HANDLE_VALUE ) {
        ERR("terminal","failed to respond");
    }

    DWORD outMode = 0;
    if ( !GetConsoleMode( hOut, &outMode ) ) {
        ERR("terminal","failed to get mode");
    }

    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if ( !SetConsoleMode( hOut, outMode ) ) {
        ERR("terminal","failed to change mode");
    }

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if ( hIn == INVALID_HANDLE_VALUE ) {
        ERR("terminal input","failed to respond");
    }

    DWORD inMode = 0;
    if ( !SetConsoleMode( hIn, inMode ) ) {
        ERR("terminal input","failed to change mode");
    }
}

void updateScreen( void ) {
    //system("cls");
    //printf("\x1b[8;%d;%dt", display->height, display->width);
    
    static int prev_sheetSize = 0;
    BOOL update = FALSE;
    int txtCounter = 0, txtColor, sheetSize;
    memcpy( &sheetSize, display->sheet, INT_SIZE );
    //char printBuffer[ 26 * sheetSize ];
    //printf("initialized variables\n");

    if( sheetSize != prev_sheetSize ) {
        free( displayCopy );
        int size = ( sheetSize + 1 ) * sizeof( CHAR_BYTE ) + sizeof( int );
        if( ( displayCopy = malloc( size ) ) == NULL ) {
            ERR("display buffer","ran out of memory");
        }
        memset( displayCopy, 129, size );
    }
    //printf("checked size change\n");
    for( int i = INT_SIZE; i < sheetSize + INT_SIZE; i++) {
        //printf("%d %d : ",display->sheet[i],displayCopy[i]);
        if( display->sheet[i] != displayCopy[i] ) {
            update = TRUE;
            if( display->sheet[i] > 0 ) {
                //sprintf(printBuffer, "%s%c%s", colorList[ display->sheet[i] - 1 ], ' ', CLEAR);
                printf( "%s%c%s", colorList[ display->sheet[i] - 1 ], ' ', CLEAR);
            }
            else if ( display->sheet[i] == 0 ) {
                if( display->textVals[txtCounter] == '\x81' ) {
                    char tmpStr[3];
                    ++txtCounter;
                    txtColor = GRAD_SIZE * INT( display->textVals[txtCounter] );
                    ++txtCounter;
                    sprintf( tmpStr, "%c%c", display->textVals[txtCounter+1], display->textVals[txtCounter+2] );
                    txtColor -= atoi( tmpStr );
                    txtCounter += 3;
                }
                //sprintf(printBuffer, "%s%c%s", strcolorList[txtColor], display->textVals[txtCounter], CLEAR);
                printf( "%s%c%s", strcolorList[txtColor], display->textVals[txtCounter], CLEAR);
                ++txtCounter;
            }
        }
        else
        {   
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            COORD CursorPos = GetConsoleCursorPosition( hOut );
            if( CursorPos.X == display->width ) {
                printf("\x1b[1B");
                printf("\x1b[%dD", CursorPos.X);
            }
            else
            {
                printf("%s", "\x1b[1C");
            }
        }
    }
    //printf("updated buffer %d\n", update);
    if( update ) {
        //system("cls");
        //puts(printBuffer);
    }

    displayCopy = display->sheet;
    prev_sheetSize = sheetSize;
}

void updatePalette( int alpha, int gradientStyle, int intensity, BOOL fgb ) {
    for( int i = 1; i <= ( 8 * GRAD_SIZE ); i++) {
        if( fgb ) {
            strcpy( colorList[i], colorGen( alpha, i, gradientStyle, intensity, fgb ) );
        }
        else
        {
            strcpy( strcolorList[i], colorGen( alpha, i, gradientStyle, intensity, fgb ) );
        }
        //printf("%s%c%s",colorList[i],' ',CLEAR);
    }
    //printf("\n");
}

char* colorGen(int alpha, int i, int gradStyle, int intensity, BOOL fgb) {
    static char str[20];
    int n = ( ( i - 1 ) / GRAD_SIZE );
    int grad = ( GRAD_SIZE - ( i - 1 ) % GRAD_SIZE ), r, g, b;
    switch ( gradStyle ) {
        case LINEAR:
            if( fgb ) {
                r = linearGradient( grad, display->colors[n][0], display->colors[alpha][0] );
                g = linearGradient( grad, display->colors[n][1], display->colors[alpha][1] );
                b = linearGradient( grad, display->colors[n][2], display->colors[alpha][2] );
            }
            else
            {
                r = linearGradient( grad, display->strcolors[n][0], display->strcolors[alpha][0] );
                g = linearGradient( grad, display->strcolors[n][1], display->strcolors[alpha][1] );
                b = linearGradient( grad, display->strcolors[n][2], display->strcolors[alpha][2] );
            }
            break;
        case RATIONAL:
            if( fgb ) {
                r = rationalGradient( grad, display->colors[n][0], display->colors[alpha][0], intensity );
                g = rationalGradient( grad, display->colors[n][1], display->colors[alpha][1], intensity );
                b = rationalGradient( grad, display->colors[n][2], display->colors[alpha][2], intensity );
            }
            else
            {
                r = rationalGradient( grad, display->strcolors[n][0], display->strcolors[alpha][0], intensity );
                g = rationalGradient( grad, display->strcolors[n][1], display->strcolors[alpha][1], intensity );
                b = rationalGradient( grad, display->strcolors[n][2], display->strcolors[alpha][2], intensity );
            }
            break;
    }
    
    sprintf(str, "\x1b[%d8;2;%d;%d;%dm", (fgb) ? 4 : 3, r, g, b);
    return str;
}

int linearGradient(int grad, int val, int alpha) {
    float fac = ( abs( val - alpha ) / ( float ) GRAD_SIZE );
    return ( val < alpha ) ? val + ( fac * grad ) : val - ( fac * grad );
}

int rationalGradient(int grad, int val, int alpha, int intensity) {
    float difference = val - alpha;
    float magicNo = ( 1 / ( float ) intensity ) * ( GRAD_SIZE );
    float factor = difference / magicNo;
    float numerator = difference + factor;
    float denominator = ( 1 / ( float ) intensity ) * grad + 1;
    return ( int ) ( numerator / denominator - factor + alpha );
}

COORD GetConsoleCursorPosition(HANDLE hConsoleOutput) {
    CONSOLE_SCREEN_BUFFER_INFO cbsi;

    if (GetConsoleScreenBufferInfo(hConsoleOutput, &cbsi))
    {
        return cbsi.dwCursorPosition;
    }
    else
    {
        COORD invalid = { 0, 0 };
        return invalid;
    }
}