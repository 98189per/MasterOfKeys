#include "fileprocessing.h"

int loadElement (const char * fileName, Element * element) {
    FILE *inPtr;
    Property* propertyPtr;
    char tmpChar;
    void *bufferPtr;
    BOOL skipSpaces = TRUE;
    BOOL castToByte = TRUE;

    if( (inPtr = fopen( fileName, "r" ) ) == NULL) {
        return 1;
    }
    else
    {
        fscanf(inPtr, "%d x %d", &(element->width), &(element->height));
        int tmp = element->width * element-> height;
        if( ( element->sheet = calloc( 1, ( tmp + 1 ) * sizeof( CHAR_BYTE ) + sizeof( int ) ) ) == NULL ) {
            ERR(element->name,"ran out of memory");
        }
        memcpy( element->sheet, &tmp, INT_SIZE );
        bufferPtr = element->sheet + INT_SIZE;
        element->properties = NULL;
        while ( ( tmpChar = fgetc( inPtr ) ) != EOF ) {
            switch ( tmpChar ) {
            case '\n':
            case '\t':
                break;
            case '"':
                skipSpaces = !skipSpaces;
                break;
            case '{':
                castToByte = FALSE;
                if( skipSpaces ) {
                    if( ( propertyPtr = calloc( 1, sizeof( Property ) ) ) == NULL ) {
                        ERR(element->name,"ran out of memory");
                    }
                    element->properties = propertyPtr;
                    bufferPtr = propertyPtr->key;
                    break;
                }
            case ':':
                if( skipSpaces ) {
                    bufferPtr = propertyPtr->value;
                    break;
                }
            case ',':
                if( skipSpaces ) {
                    Property* prevPtr = propertyPtr;
                    if( ( propertyPtr = calloc( 1, sizeof( Property ) ) ) == NULL ) {
                        ERR(propertyPtr->key,"ran out of memory");
                    }
                    prevPtr->nextProperty = propertyPtr;
                    bufferPtr = propertyPtr->key;
                    break;
                }
            case '}':
                if( skipSpaces ) {
                    propertyPtr->nextProperty = NULL;
                    break;
                }
            default:
                if( !skipSpaces || tmpChar != ' ' ) {
                    if( castToByte ) {
                        CHAR_BYTE writeChar;
                        writeChar = GRAD_SIZE * INT(tmpChar);
                        memcpy( bufferPtr, &writeChar, sizeof( CHAR_BYTE ) );
                        bufferPtr = ( CHAR_BYTE * ) bufferPtr + 1;
                    }
                    else
                    {
                        char writeChar;
                        writeChar = tmpChar;
                        memcpy( bufferPtr, &writeChar, sizeof( char ) );
                        bufferPtr = ( char * ) bufferPtr + 1;
                    }
                }
                break;
            }
        }
    }

    return 0;
}

void unloadElement (Element * element) {
    Property* propertyPtr;

    free(element->sheet);
    propertyPtr = element->properties;
    while( propertyPtr != NULL ) {
        free(propertyPtr->key);
        free(propertyPtr->value);
        propertyPtr = propertyPtr->nextProperty;
    }
}