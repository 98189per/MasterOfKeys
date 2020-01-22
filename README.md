# MasterOfKeys

## Preamble
Master of keys is a project within a project within a project. It is firstly a way of graphically interfacing with the Windows terminal, upon which a game has been implemented, which in turn generates its levels by interpreting user-defined psuedocode. To make things even worse for me, I have not used any parsing libraries to accomplish any of this, only the infamous <windows.h> to interface with the terminal. And unlike my previous C++ 'project', I did not just copy my code from an msdn tutorial. Every single line in all 10 source files was meticulously typed and debugged within the timeframe... Every. Single. One.  
   
I first started thinking about any of this in the last decade, when I uncovered an old copy of Deitel & Deitel's *C: How to Program* that had been lost in my attic. I had done some work in C before, but it was mainly syntactical and I had no idea of any of the deeper concepts. As I dove deeper into the book and became fascinated by the potential of low-level programming languages I thought about using this for my computer science culminating. Of course, I had to make things super extra and couldn't settle for just *any* C program. So I combined a bunch of things I had heard about elsewhere, and basically tried to cram as much as I could learn into one program. Which would have been fine had I had many moons left to deliver, but I only had until the week before exams. Nonetheless, I was determined to have something ready within four weeks time. But soon four weeks became three weeks, and three weeks became two weeks. The first week of 2020 came and I still hadn't started... Things were not looking so good for my project. I finally decided to start coding somewhere near the end of the first week, and I got one function done. Or so I thought. When I tried to implement it, nothing worked no matter what I tried. The program kept crashing without throwing any errors, and the compiler gave no hint as to what was going on either. After hours of debugging and commenting out line by line, I finally got to the source: a one-off error where I forgot to include the string termination character. When I finally fixed it I realized that it would be much harder than I thought to work in a low-level environment. Alas, the final deadline has arrived, and while I certainly do not have everything I dreamed of in this program, I've learned quite a lot and I hope that you will too as you try to understand **exactly what the heck have I programmed...**  

## Getting Started
  
##### DISCLAIMER: I AM NOT RESPONSIBLE FOR ANY INFRACTIONS SUSTAINED BY THE USER FOR VIOLATING THE WCDSB COMPUTER AND NETWORK USE EXPECTATIONS AS OUTLINED [HERE](https://resurrection.wcdsb.ca/student-services/guidance/student-handbook/computer-and-network-use/), WHICH CLEARLY STATE - 
> 1. It is **NOT** acceptable for users to:  
> ...
> 7. f) store or run executable files (.exe)
  
That being said, let's get right into it!
  
In order to run Master of Keys you will require -   

*  a windows computer  

I think that's it, but honestly so much of my code is system-dependent so I'll never really know...  
  
Download the contents of the 'bin' folder, 'Resources' folder, and 'Songs' folder. The 'src' folder contains, you guessed it, the source code. 'Resources' and 'Songs' only contain my test cases for now, they're pretty bad so you can update them later if you want. (Why wouldn't you?). Create a directory to store all the folders in, preferebly called 'MasterOfKeys' or something like that. Put all the folders you downloaded into there. Make sure that the 'bin' folder contains the .exe as well as the .dll dependencies (libwinpthread-1.dll). Double-click the .exe to run it, and voila, you will be greeted by the menu screen, or a bunch of garbage if it doesn't work.  
  
Use the arrow keys to navigate up and down the selections, it is kinda slow, so be patient. Use the enter key to select an option. I haven't coded 'configure' yet so pick 'play game' I guess. The song selection screen looks like it has only four options but it you scroll past the bottom you'll see more (right now there's only five, just to demonstrate that feature I put 5+ hours implementing). Selecting a song should start playing the song and bring up the gameplay screen. I haven't gotten around to coding the actual game, so you can just listen to music while the computer plays it for you. :-)  
  
## The grand scheme of things
![Program Flowchart](MasterOfKeys-Flowchart.jpg)  
  
I tried to keep it as simple as possible for the purposes of this flowchart, but don't worry, it's definitely much more complicated in the actual source code.  

The program starts with initialization from the main() function call, which just sets the default values and stuff. From there two main threads are called (it's actually *3* threads, but they behave like 2). The update thread is fairly straightforward, calling the updateDisplay() function, then the updateScreen() function at a constant rate of X times per second. Likewise, the input thread also sequentially calls functions, but it also handles both the input stack and the loading stack. Also, instead of updating a certain number of times per second, it updates with every getch() call for user input. (well technically the loading stack is checked at a fixed rate, but it only ever gets updated with user input).  

The main data types and structs can be found in "typedefs.h", but the only complicated one is Element, which is really just a static linked list. The three main source files are fileprocessing.c, screenhandling.c, and main.c . Of which the last one concerns us most. (it certainly causes me much concern) The first two do exactly what they say they do, and if you want to know exactly how they work then by all means, take a peek inside, but main.c requires some explaining. The 'game' aspect of this project can be augmented by adding functions to the body of main.c . When .txt files from 'Resources' are parsed by fileprocessing.c, the resulting property chains are passed to main.c as strings, from which they are parsed for data and something is done with that data. What you do is entirely up to you, and I strongly encourage trying to implement more properties in main.c .
  
## Adding functions to main.c

In the file you will see a long list of function declarations starting with
```C
    unsigned long hash( CHAR_BYTE * );
```
Add your function to (preferably) the bottom of the list. Then update the following values to include your function as well.
```C
    FunctionPtr functions[] = {...};
    const char* keyValues[] = {...};
    const int noKeys = ... ;
```
Define your function with 
```C
    void functionName(int mod, const char * val, Element * element) {...}
```
You cannot use other parameters, as this is what will be passed to your function by addElement(). The 'val' parameter is the string value of the property corresponding to the name of your function. When you use that property in an element .txt file, the right side of the definition will be sent to this function. The 'mod' parameter is used for consecutive names of properties (eg, color0; color1; color2; etc.), but can be omitted in most cases. The 'element' parameter is a pointer to the element that holds this property. In addition to 'element' and 'val', user-defined functions also have access to global variables such as 'display', which is the CHAR_BYTE array that prints out to the screen with updateScreen. You may want to look more in depth at the main.c file before you actually implement your own functions, but there is a lot that can be done so feel free to do it.  

## Creating element .txt files

.txt files are at the heart of parsing and processing information for Master of Keys. All elements on the screen come from their respective .txt files located either in 'Resources' or 'Songs'. The program always starts by loading 'default.txt', the main title screen. From there, elements can link to other elements with various property functions. All element .txt files have the same structure, a dimension specification of the form `number1_x_number2`. It must be a lowercase 'x' with one space on each side. Following a line break is an n by n block of text representing the screen character by character. Digits 1-8 are used to represent colored rectangles, and '0' is used to format and display text. The color of the rectangles, as well as the format of displayed strings is controlled by the properties in the next segment. After the n by n block is entered, no more, no less, all spaces filled by numbers 0-8, there is a line break followed by a curly bracket '{'. This signals the start of the property list. Each property name is entered within double quotes, then a colon, then the value, then a comma. The format for the value is property specific, and must be enetered accurately because I have to error-checking, so if it isn't the program will just unexpectedly exit... Documentation for all included properties can be found [here](https://docs.google.com/document/d/14qPJVofan0vjLcivWO9WT2R7gS1hnxjaSeriEb0WOv0/edit?usp=sharing).

## A word about .mid files
  
The midi files from which notes for the songs are read can be made by any music program capable of generating them, but stick to one track because multiple tracks can have extra bits in the track header, which crashes the program...  
  
***

If this guide was not enough for you, definitely check out my uh... *very descriptive* inline comments to get a feel for how the code fits together. If this guide was enough then either  

* you don't care  
* you were too lazy to read the whole thing 

It doesn't matter to me, all that matters is to **HAVE FUN!!**  
(which is probably very difficult with this game...)