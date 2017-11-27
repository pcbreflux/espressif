/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/task.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "example";

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:

#define USE_SPI_MODE

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define MAX_BUFSIZE 16384
#endif //USE_SPI_MODE

unsigned long IRAM_ATTR millis() {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

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

void testFileIO(const char *filename,uint32_t bufsize) {
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
    	ESP_LOGE(TAG,"Failed to open file for writing");
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
    ESP_LOGI(TAG,"%u bytes (%u) written in %u ms", loops*bufsize, bufsize, end);
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
        ESP_LOGI(TAG,"%u bytes read in %u ms", flen, end);
        fclose(f);
    } else {
    	ESP_LOGE(TAG,"Failed to open file for reading");
    }


}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing SD card");

#ifndef USE_SPI_MODE
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // host.flags = SDMMC_HOST_FLAG_1BIT;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_modesdmmc_card_print_info(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

#else
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
#endif //USE_SPI_MODE

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    // max_files Max number of open files
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%d). "
                "Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat("/sdcard/foo.txt", &st) == 0) {
        // Delete it if it exists
        unlink("/sdcard/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/sdcard/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    struct dirent *pDirent;
    DIR *pDir;
    struct stat _stat;
    char cPath[1024];

    mkdir("/sdcard/mydir",0777);

    pDir = opendir("/sdcard");
    if (pDir == NULL) {
        printf ("Cannot open directory '/sdcard'\n");
        return;
    }

    while ((pDirent = readdir(pDir)) != NULL) {
    	sprintf(cPath,"/sdcard/%s",pDirent->d_name);
    	stat(cPath,&_stat);
    	if(S_ISDIR(_stat.st_mode)) {
    		printf ("[%s] DIR %ld\n", pDirent->d_name,_stat.st_size);
    	} else {
    		printf ("[%s] FILE %ld\n", pDirent->d_name,_stat.st_size);
    	}
    }
    closedir (pDir);
    testFileIO("/sdcard/TEST.txt",512);
    testFileIO("/sdcard/TEST.txt",1024);
    testFileIO("/sdcard/TEST.txt",2048);
    testFileIO("/sdcard/TEST.txt",8192);
    testFileIO("/sdcard/TEST.txt",16384);
    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "Card unmounted");
}
