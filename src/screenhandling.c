#include "screenhandling.h"

char* colorGen(int, int, int, int, BOOL);
int linearGradient(int, int, int);
int rationalGradient(int, int, int, int);

char colorList[CHAR_BYTE_LIMIT][20];
char strcolorList[CHAR_BYTE_LIMIT][20];

//sets the screen to raw mode and enables ANSI escape sequencing
void initScr( void ) {
    system("cls");
    RESIZE( display->width, display->height );

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

//formats and prints the display array, then moves cursor to start of screen again
void updateScreen( void ) {
    int txtCounter = 0, txtColor;
    int sheetSize, size;
    memcpy( &sheetSize, display->sheet, INT_SIZE );

    for( int i = INT_SIZE; i < sheetSize + INT_SIZE; i++) {
        if( display->sheet[i] > 0 ) {
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
            printf( "%s%c%s", strcolorList[txtColor], display->textVals[txtCounter], CLEAR);
            ++txtCounter;
        }
    }
    UP(display->height);
    LEFT(display->width);
	RESIZE( display->width, display->height );
}

//generate palette (i have not implemented this much, so feel free to write property functions utilizing this!!!)
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

//i spent 2 hours trying to figure out rational interpolation ;(
// smh im kinda rusty in math ngl
int rationalGradient(int grad, int val, int alpha, int intensity) {
    float difference = val - alpha;
    float magicNo = ( 1 / ( float ) intensity ) * ( GRAD_SIZE );
    float factor = difference / magicNo;
    float numerator = difference + factor;
    float denominator = ( 1 / ( float ) intensity ) * grad + 1;
    return ( int ) ( numerator / denominator - factor + alpha );
}