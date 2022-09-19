
#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <PacketSerial.h>
#include "CRC.h"
#include <iomanip>
#include <cstdint>  
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "wire.h"

// spi SD stuff
#define VSPI FSPI
static const int spiClk = 1000000; // 1 MHz
SPIClass SDSPI(HSPI);
#define SD_miso  37
#define SD_mosi  35
#define SD_sck   36
#define SD_ss    34

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

//analog input
#define a0 14 //battery voltage
#define a1 13//Current sens
//En pins
#define en7 45
#define en6 6
#define en5 15
#define en4 8
#define en3 20
#define en2 10
#define en1 12
//flag pins  
#define flg7 46  
#define flg6 7
#define flg5 16
#define flg4 19
#define flg3 9
#define flg2 11
#define flg1 21
//i2c address for radio comm board
#define CommBoard 8

typedef struct radioBoard{
    double lat;
    double lng;
    float alt;

} radioBoard;
radioBoard radioBoardRx;

float current;
float voltage;

bool chan1_on = true;
bool chan2_on = true;
bool chan3_on = true;
bool chan4_on = true;
bool chan5_on = true;
bool chan6_on = true;
bool chan7_on = true;

bool chan1_fault = false;
bool chan2_fault = false;
bool chan3_fault = false;
bool chan4_fault = false;
bool chan5_fault = false;
bool chan6_fault = false;
bool chan7_fault = false;

String FileName;
int SecCount = 0;
unsigned long previousMillis = 0; 
const long interval = 1000;


/*
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}



void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);
    
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


void writeFile(fs::FS &fs, const char * path,  String message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.println(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

*/

void logdata(fs::FS &fs, String path){
    String data_array = "";
    data_array += String(SecCount); // save seconds since start of program
    data_array += ",";
    data_array += String(radioBoardRx.lat, 14); 
    data_array += ",";
    data_array += String(radioBoardRx.lng, 14); 
    data_array += ",";
    data_array += String(radioBoardRx.alt); 
    data_array += ",";
    data_array += String(voltage);
    data_array += ",";
    data_array += String(current);
    //Serial.println(data_array);
    File file = fs.open(path, FILE_APPEND);
    // if the file is valid, write to it:
    if(file){
        file.println(data_array);
        file.close();
    }

}

void createLog(fs::FS &fs, String path){
    String dataHeader = "Seconds,Lattitude,Longitude,Altitude,Voltage,Current,Pressure,Humidity,GasSens";
    File file = fs.open(path, FILE_WRITE);
    // if the file is valid, write to it:
    if(file){
        file.println(dataHeader);
        file.close();
    }
}

void check_fault(){
    /*
  if(digitalRead(flg7) == 0){
    digitalWrite(en7, LOW);
    chan7_fault = true;
    Serial.println("Channel 7 over current");
  }
  */
  if(digitalRead(flg6) == 0){
    digitalWrite(en6, LOW);
    chan6_fault = true;
    Serial.println("Channel 6 over current");
  }

  if(digitalRead(flg5) == 0){
    digitalWrite(en5, LOW);
    chan5_fault = true;
    Serial.println("Channel 5 over current");
  }

  if(digitalRead(flg4) == 0){
    digitalWrite(en4, LOW);
    chan4_fault = true;
    Serial.println("Channel 4 over current");
  }

  if(digitalRead(flg3) == 0){
    digitalWrite(en3, LOW);
    chan3_fault = true;
    Serial.println("Channel 3 over current");
  }

  if(digitalRead(flg2) == 0){
    digitalWrite(en2, LOW);
    chan2_fault = true;
    Serial.println("Channel 2 over current");
  }

  if(digitalRead(flg1) == 0){
    digitalWrite(en1, LOW);
    chan1_fault = true;
    Serial.println("Channel 1 over current");
  }

}


void setup() {

    Serial.begin(115200);
    SDSPI.begin(SD_sck, SD_miso, SD_mosi, -1);
    pinMode(SD_ss, OUTPUT); //HSPI SS  set slave select pins as output

    if (!SD.begin(SD_ss, SDSPI)) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    FileName = "/" + String(esp_random()) + "log.csv";
    createLog(SD, FileName);
    

    pinMode(a0, INPUT);
    pinMode(a1, INPUT);

    pinMode(en7, OUTPUT);//en7
    pinMode(en6, OUTPUT);//en6
    pinMode(en5, OUTPUT);//en5
    pinMode(en4, OUTPUT);//en4
    pinMode(en3, OUTPUT);//en3
    pinMode(en2, OUTPUT);//en2
    pinMode(en1, OUTPUT);//en1
  
    pinMode(flg7, INPUT_PULLUP);//flg7
    pinMode(flg6, INPUT_PULLUP);//flg6
    pinMode(flg5, INPUT_PULLUP);//flg5
    pinMode(flg4, INPUT_PULLUP);//flg4
    pinMode(flg3, INPUT_PULLUP);//flg3
    pinMode(flg2, INPUT_PULLUP);//flg2
    pinMode(flg1, INPUT_PULLUP);//flg1

    digitalWrite(en7, HIGH);//en7
    delay(100);
    digitalWrite(en6, HIGH);//en6
    delay(100);
    digitalWrite(en5, HIGH);//en5
    delay(100);
    digitalWrite(en4, HIGH);//en4
    delay(100);
    digitalWrite(en3, HIGH);//en3
    delay(100);
    digitalWrite(en2, HIGH);//en2
    delay(100);
    digitalWrite(en1, HIGH);//en1

    analogSetAttenuation(ADC_11db);
}


void slowLoop(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    SecCount++;
    //run every second
    digitalWrite(en7, HIGH);//en7
    digitalWrite(en6, HIGH);//en6
    digitalWrite(en5, HIGH);//en5
    digitalWrite(en4, HIGH);//en4
    digitalWrite(en3, HIGH);//en3
    digitalWrite(en2, HIGH);//en2
    digitalWrite(en1, HIGH);//en1
    chan1_fault = false;
    chan2_fault = false;
    chan3_fault = false;
    chan4_fault = false;
    chan5_fault = false;
    chan6_fault = false;
    chan7_fault = false;

    
    //Serial.println(analogRead(4));
    current = (analogRead(a1) * .750 / 8191);
    //Serial.print("current: ");
    //Serial.println(current);

    voltage = ((analogRead(a0)*.0025)-.789);
    //Serial.print("Batter Voltage: ");
    //Serial.println(batv);
    logdata(SD, FileName);
    }
    
}

void loop() {
    check_fault();
    slowLoop();
}