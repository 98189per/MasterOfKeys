#include "test.h"
/*//#include <ncurses.h>
//#include <curses.h>
#include <windows.h>
//#include <wchar.h>*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*#define FALSE 0
#define TRUE 1
#define RED "\x1b[31m"
#define CLEAR "\x1b[0m"
#define CHAR_BYTE unsigned char*/

//void * moveSquare(void *);
//void* pritn(void *);
//void foobar(int);

/*char* RGBcolor(CHAR_BYTE, CHAR_BYTE, CHAR_BYTE, CHAR_BYTE);
pthread_t id[2];/*
int str, x = 0, y = 0;
int max_x, max_y;
char ch;
//WINDOW *scrn;
pthread_mutex_t lock;*/



pthread_t id[2];
pthread_mutex_t lock;

void foobar(int,int);
void* pritn(void*);
void test(void);

void foobar(int i,int p){
	static int ctr = 0;
	//pthread_mutex_lock(&lock);
	++ctr;
    printf("function %d - %d execute\n",p,i);
	sleep(2);
    printf("function %d - %d execute\n",p,i+ctr);
	//pthread_mutex_unlock(&lock);
}

void* pritn(void* args) {
	static int ptr = 0;
	pthread_mutex_lock(&lock);
	++ptr;
	int p = *((int*)args);
    foobar(p,ptr);
    //sleep(2);
    //foobar(p+2);
	pthread_mutex_unlock(&lock);
    return NULL;
}

void test( void ) {
    Element elements[MAX_ELEMENTS];

    /*
    pthread_mutex_init(&lock,NULL);
    int i = 0;
	int* ip = &i;
    while(i<2){
        pthread_create(&id[i],NULL,&pritn,ip);
		//printf("call %d\n",i);
		sleep(1);
        ++i;
    }
    pthread_join(id[0],NULL);
    pthread_join(id[1],NULL);
    */
}



/*
void foobar(int i){
    printf("function %d execute\n",i);
}

void* pritn(void* args) {
    foobar(*((int*)args));
    sleep(1);
    foobar((*((int*)args))+2);
    return NULL;
}

void test( void ) {
    pthread_mutex_init(&lock,NULL);
    int i = 0;
    while(i<2){
        pthread_create(&id[i],NULL,&pritn,&i);
        ++i;
    }
    pthread_join(id[0],NULL);
    pthread_join(id[1],NULL);
}
*/
/*void test ( void ) {
    char *msg = "hello world (in red)";
    char in[10];

    /*HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        //return GetLastError();
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        //return GetLastError();
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        //return GetLastError();
    }*//*

    system("cls");
    printf("\x1b[8;250;100t");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    printf(RED "%s\n" CLEAR, msg);
    printf("%s this is (149, 195, 255)" CLEAR "\n", RGBcolor(149, 195, 255, 1));
    //wprintf(L"\x1b[31m%s\n", msg);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fdwMode = 0;
    GetConsoleMode(hIn, &fdwMode);
    fdwMode = ENABLE_LINE_INPUT & ENABLE_ECHO_INPUT & ENABLE_PROCESSED_INPUT;
    if(!SetConsoleMode(hIn, fdwMode)){
        printf("failed");
    };

    fgets(in, 10, stdin);
    printf("%s\n", in);

    printf(RED "%s\n" CLEAR, msg);
    printf("%s this is (149, 195, 255)" CLEAR "\n", RGBcolor(149, 195, 255, 1));
    //wprintf(L"\x1b[31m%s\n", msg);
}

char* RGBcolor(CHAR_BYTE r, CHAR_BYTE g, CHAR_BYTE b, CHAR_BYTE fgb){
    char *str;
    str = (char *) malloc(20 * sizeof(char));
    sprintf(str, "\x1b[%d8;2;%d;%d;%dm%c", (fgb) ? 3 : 4, r, g, b, 0);
    return str;
}*/

/*void test( void ) {
	char* msg = "he;;o wor;d";

    if (pthread_mutex_init(&lock, NULL) != 0){
        printf("burh, it failed... smh");
    }

	initscr();
    if(has_colors() == FALSE){
        printw("no colors");
    }
    start_color();
    if(can_change_color() == FALSE){
        printw("smh also no color");
        printw("\x1b[32;41m gg \x1b[0m");
        printf(RED " red \x1b[0m");
    }
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
    curs_set(FALSE);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_RED);
    getmaxyx(stdscr, max_y, max_x);
    printw("enter F1 to laeve");
    scrn = newwin(LINES, COLS, 1, 0);
    //pthread_create(&id, NULL, moveSquare, NULL);
    while(str != KEY_F(1)){
        str = getch();
        pthread_mutex_lock(&lock);
        switch(str){
            case KEY_UP:
                ch = '^';
                y = -1;
                x = 0;
                break;
            case KEY_DOWN:
                ch = 'v';
                y = 1;
                x = 0;
                break;
            case KEY_LEFT:
                ch = '<';
                x = -2;
                y = 0;
                break;
            case KEY_RIGHT:
                ch = '>';
                x = 2;
                y = 0;
                break;
        }
        pthread_mutex_unlock(&lock);
    }
    pthread_mutex_destroy(&lock);
    delwin(scrn);
	endwin();
}*/

/*void * moveSquare(void *vargp){
    int cur_x = 1, cur_y = 1;
    while(str != KEY_F(1)){
        wclear(scrn);
        cur_x += x;
        cur_y += y;
        usleep(20000);
        if(cur_y > 0 && cur_y < max_y && cur_x < max_x && cur_x > 0){
            mvwaddch(scrn, cur_y, cur_x, ch);
        }
        else
        {
            pthread_mutex_lock(&lock);
            if(cur_x >= max_x || cur_x <= 0){
                x *= -1;
                ch = (x > 0) ? '>' : '<';
            }
            if(cur_y >= max_y || cur_y <= 0){
                y *= -1;
                ch = (y > 0) ? 'v' : '^';
            }
            pthread_mutex_unlock(&lock);
        }
        
        //mvprintw(0, 0, "enter F1 to laeve");
        wrefresh(scrn);
    }
    return NULL;
}*/

/*
void printptr(CHAR_BYTE *);

CHAR_BYTE * getNum( void ) {
    CHAR_BYTE *ret;
    ret = (CHAR_BYTE *) malloc(2 * sizeof(CHAR_BYTE));
    //printptr(ret);
    //ret = realloc(ret, 4 * sizeof(CHAR_BYTE));
    //printptr(ret);
    printf("\nenter numbers - \n");
    scanf("%d",ret + 0);
    scanf("%d",ret + 1);
    //scanf("%d",ret + 2);
    printf("%d + %d\n",*(ret + 0),*(ret + 1));//,*(ret + 2));
    return ret;
}

void printptr(CHAR_BYTE * ptr) {
    printf("\n");
    for(int i = 0; i < 32; i++){
        printf("%d:%d ",i,*(ptr + i));
    }
}

void storeSum(Sum *str, int a, int b){
    str->x = a;
    str->y = b;
}

int add(int x, int y){
    return x + y;
}
*/