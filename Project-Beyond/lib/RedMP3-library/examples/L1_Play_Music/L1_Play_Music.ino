/*
/--------basic operations---------------/
mp3.play();
mp3.pause();
mp3.nextSong();
mp3.previousSong();
mp3.volumeUp();
mp3.volumeDown();
mp3.forward();    //fast forward
mp3.rewind();     //fast rewind
mp3.stopPlay();  
mp3.stopInject(); //when you inject a song, this operation can stop it and come back to the song befor you inject
mp3.singleCycle();//it can be set to cycle play the currently playing song 
mp3.allCycle();   //to cycle play all the songs in the TF card
/--------------------------------/

mp3.playWithIndex(int8_t index);//play the song according to the physical index of song in the TF card

mp3.injectWithIndex(int8_t index);//inject a song according to the physical index of song in the TF card when it is playing song.

mp3.setVolume(int8_t vol);//vol is 0~0x1e, 30 adjustable level

mp3.playWithFileName(int8_t directory, int8_t file);//play a song according to the folder name and prefix of its file name
                                                            //foler name must be 01 02 03...09 10...99
                                                            //prefix of file name must be 001...009 010...099

mp3.playWithVolume(int8_t index, int8_t volume);//play the song according to the physical index of song in the TF card and the volume set

mp3.cyclePlay(int16_t index);//single cycle play a song according to the physical index of song in the TF

mp3.playCombine(int16_t folderAndIndex[], int8_t number);//play combination of the songs with its folder name and physical index
      //folderAndIndex: high 8bit is folder name(01 02 ...09 10 11...99) , low 8bit is index of the song
      //number is how many songs you want to play combination

About SoftwareSerial library:
The library has the following known limitations:
If using multiple software serial ports, only one can receive data at a time.

************************************************** **************************************************/
#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 4 // RX of Serial MP3 module connect to D4 of Arduino
#define MP3_TX 5 // TX to D2, note that D2 can not be used as RX on Mega2560, you should modify this if you do not use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);

int8_t index  = 3; // 1 - 10
int8_t volume = 15; // 0 - 30 

void setup() {
  Serial.begin(9600); // Initialize serial communication
  delay(500); // Requires 500ms to wait for the MP3 module to initialize  
  mp3.playWithVolume(index, volume);
  Serial.println("Playing music...");
  delay(50); // you should wait for >=50ms between two commands
}

void loop() {
  if (volume > 0) {
    volume -= 1;
    mp3.setVolume(volume); // Update the volume in the MP3 player
    Serial.println(volume);
  } else {
    volume = 15;
    mp3.setVolume(volume); // Reset the volume in the MP3 player
    Serial.println(volume);
  }

  delay(500);
}

