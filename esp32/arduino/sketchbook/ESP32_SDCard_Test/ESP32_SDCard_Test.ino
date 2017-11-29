/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

SPIClass mySPI(VSPI);
#define MAX_BUFSIZE 16384

static int rng_func(uint8_t *dest, unsigned size) {

    uint32_t rand=0;
    unsigned pos=0;
    uint8_t step=0;
    uint8_t rand8=0;

    while (pos<size) {
      if (step>=4) {
        step=0;
      }
      if (step==0) {
        rand=esp_random();
          // ESP_LOGI(TAG, "rand 0x%08X",rand);
      }
      // faster then 8*step ?
        switch(step) {
        case 0:
            rand8=rand&0xFF;
            break;
        case 1:
            rand8=(rand>>8)&0xFF;
            break;
        case 2:
            rand8=(rand>>16)&0xFF;
            break;
        case 3:
            rand8=(rand>>24)&0xFF;
            break;
        }
      // ESP_LOGI(TAG, "%d) rand 8 0x%02X",pos,rand8);
    *dest++=rand8;
      step++;
      pos++;
    }

    return 1; // random data was generated
}

void testFileIO2(const char *filename,uint32_t bufsize) {
    FILE* f;
    static uint8_t buf[MAX_BUFSIZE];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    uint32_t loops;
    struct stat _stat;

    if (bufsize > MAX_BUFSIZE)  {
      bufsize = MAX_BUFSIZE;
    }
    rng_func(buf, bufsize);

    f = fopen(filename, "w");
    if(!f){
      Serial.printf("Failed to open file for writing\n");
        return;
    }

    size_t i;
    loops = 1048576 / bufsize;
    // ESP_LOGI(TAG,"loops %u bufsize (%d)", loops, bufsize);

    start = millis();
    for(i=0; i<loops; i++){
      fwrite(buf, bufsize, 1, f);
    }
    end = millis() - start;
    Serial.printf("%u bytes (%u) written in %u ms\n", loops*bufsize, bufsize, end);
    fclose(f);

    f = fopen(filename, "r");
    if(f){
        stat(filename,&_stat);

        len = _stat.st_size;
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > bufsize){
                toRead = bufsize;
            }
            fread(buf, toRead, 1, f);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read in %u ms\n", flen, end);
        fclose(f);
    } else {
      Serial.printf("Failed to open file for reading\n");
    }


}


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
                listDir(fs, file.name(), levels -1);
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

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
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

void setup(){
    Serial.begin(115200);
    mySPI.begin(4, 16, 17, 2);
    if(!SD.begin(2,mySPI,4000000,"/sd")){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    
    testFileIO2("/sd/TEST.txt",512);
    testFileIO2("/sd/TEST.txt",1024);
    testFileIO2("/sd/TEST.txt",2048);
    testFileIO2("/sd/TEST.txt",8192);
    testFileIO2("/sd/TEST.txt",16384);
}

void loop(){

}
