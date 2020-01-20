#include "fileprocessing.h"

#define READ_NEXT( ptr ) if( fread( &ptr, sizeof( CHAR_BYTE ), 1, inPtr ) != 1 ) { fclose( inPtr ); return NULL; }

NoteCollection * loadSongNotes ( const char * fileName ) {
    NoteCollection * songNotes;
    FILE *inPtr;
    CHAR_BYTE nextByte;
    CHAR_BYTE tempChunk[8];
    const CHAR_BYTE headerChunk[] = { 77, 84, 104, 100, 0,  0,  0,  6 };
    //  MThd                          4d  54  68   64   00  00  00  06
    const CHAR_BYTE trackChunk[] =  { 77, 84, 114, 107 };
    //  MTrk                          4d  54  72   6b

    int key = 0, currentPos = 0;
    int size = 0, tempNote;
    long deltaTime = 0;
    BOOL minor = FALSE;

    if( ( inPtr = fopen( fileName, "rb" ) ) == NULL ) {
        return NULL;
    }
    else
    {
        if( fread( tempChunk, sizeof( CHAR_BYTE ), 8, inPtr ) != 8 ) {
            fclose( inPtr );
            return NULL;
        }
        for( int i = 0; i < 8; i++) {
            if( tempChunk[i] != headerChunk[i] ) {
                fclose( inPtr );
                return NULL;
            }
        }
        if( fread( tempChunk, sizeof( CHAR_BYTE ), 4, inPtr ) != 4 ) {
            fclose( inPtr );
            return NULL;
        }
        READ_NEXT(nextByte)
        currentSong.ticksPerNote += nextByte * 256;
        READ_NEXT(nextByte)
        currentSong.ticksPerNote += nextByte;
        if( fread( tempChunk, sizeof( CHAR_BYTE ), 4, inPtr ) != 4 ) {
            fclose( inPtr );
            return NULL;
        }
        for( int i = 0; i < 4; i++) {
            if( tempChunk[i] != trackChunk[i] ) {
                fclose( inPtr );
                return NULL;
            }
        }
        if( fread( tempChunk, sizeof( int ), 1, inPtr ) != 1 ) {
            fclose( inPtr );
            return NULL;
        }
        int shift = 1;
        for( int i = 1; i <= sizeof( int ); i++) {
            size += tempChunk[sizeof( int ) - i] * shift;
            shift <<= 8;
        }
        if( ( songNotes = calloc( size / 4, sizeof( NoteCollection ) ) ) == NULL ) {
            fclose( inPtr );
            return NULL;
        }

        while ( fread( &nextByte, sizeof( CHAR_BYTE ), 1, inPtr ) == 1 ) {
            switch ( nextByte ) {
            case 255:
                READ_NEXT(nextByte)
                switch ( nextByte ) {
                case 47:
                    songNotes[currentPos].note = EOS;
                    songNotes[currentPos].time = deltaTime * currentSong.usPerNote / currentSong.ticksPerNote;
                    fclose( inPtr );
                    return songNotes;
                case 81:
                    READ_NEXT(nextByte)
                    READ_NEXT(nextByte)
                    currentSong.usPerNote += nextByte * 65536;
                    READ_NEXT(nextByte)
                    currentSong.usPerNote += nextByte * 256;
                    READ_NEXT(nextByte)
                    currentSong.usPerNote += nextByte;
                    break;
                case 89:
                    READ_NEXT(nextByte)
                    READ_NEXT(nextByte)
                    key = ( nextByte < 128 ) ? nextByte : ( ~nextByte ) + 1;
                    READ_NEXT(nextByte)
                    minor = nextByte;
                    break;
                default:
                    break;
                }
                break;
            case 144 ... 159:
                READ_NEXT(nextByte)
                while ( nextByte < 128 ) {
                    tempNote = nextByte;
                    READ_NEXT(nextByte)
                    if( nextByte > 0 ) {
                        tempNote += (-7) * key;
                        tempNote %= 24;
                        if( tempNote < 5 ) {
                            if( tempNote % 2 == 1 ) {
                                tempNote += ( minor ) ? 1 : -1;
                            }
                            tempNote /= 2;
                        }
                        else if ( tempNote < 12 ) {
                            if( tempNote % 2 == 0 ) {
                                tempNote += ( minor ) ? 1 : -1;
                            }
                            tempNote /= 2;
                            tempNote += 1;
                        }
                        else if ( tempNote < 17 ) {
                            if( tempNote % 2 == 1 ) {
                                tempNote += ( minor ) ? 1 : -1;
                            }
                            tempNote /= 2;
                            tempNote += 1;
                        }
                        else
                        {
                            if( tempNote % 2 == 0 ) {
                                tempNote += ( minor ) ? 1 : -1;
                            }
                            tempNote /= 2;
                            tempNote += 2;
                        }
                        songNotes[currentPos].note = tempNote;
                        songNotes[currentPos].time = deltaTime * currentSong.usPerNote / currentSong.ticksPerNote;
                        ++currentPos;
                    }
                    READ_NEXT(nextByte)
                    memset( tempChunk, 0, 4 * sizeof( CHAR_BYTE ) );
                    int n = -1;
                    for( int i = 0; i < 4; i++) {
                        ++n;
                        tempChunk[i] = nextByte;
                        if( nextByte < 128 ) {
                            break;
                        }
                        READ_NEXT(nextByte)
                    }
                    int factor = 1;
                    for( int i = n; i > -1; i--) {
                        deltaTime += ( tempChunk[i] % 128 ) * factor;
                        factor <<= 7;
                    }
                    READ_NEXT(nextByte)
                }
                break;
            default:
                break;
            }            
        }
    }

    fclose( inPtr );
    return NULL;
}

int loadElement ( const char * fileName, Element * element ) {
    FILE *inPtr;
    Property* propertyPtr;
    char tmpChar;
    void *bufferPtr;
    BOOL skipSpaces = TRUE;
    BOOL castToByte = TRUE;

    if( ( inPtr = fopen( fileName, "r" ) ) == NULL ) {
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

    fclose( inPtr );
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