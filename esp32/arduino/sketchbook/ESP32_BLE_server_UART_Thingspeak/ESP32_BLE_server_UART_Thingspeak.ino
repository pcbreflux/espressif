/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini, with some additional code by pcbreflux

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <nvs_flash.h>

BLECharacteristic *pCharacteristic;
BLEDescriptor *pDescriptor;
BLEServer *pServer;
BLEService *pService;
bool deviceConnected = false;
bool deviceNotifying = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define STORE_SIZE 100
#define NVS_STORE "storage"
#define STORE_MAX_SIZE 200
#define VAL_MAX_SIZE 20
#define GPIO_DEEP_SLEEP_DURATION     1  // sleep 1 seconds and then wake up

static char storebuf[STORE_MAX_SIZE+1];
static char valbuf[VAL_MAX_SIZE+1];
static uint32_t storeCnt; 

const char* ssid     = "ESP32"; // Your SSID (Name of your WiFi)
const char* pass = "example";   // Your Wifi password

#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT 443
//#define WEB_PORT 80
#define WEB_URL "https://api.thingspeak.com/update.json?api_key=KEYEXAMPLE&field1=" // Change KEYEXAMPLE to your API-KEY !
//#define WEB_URL "http://api.thingspeak.com/update.json?api_key=KEYEXAMPLE&field1="

WiFiClientSecure client;
//WiFiClient client;

/* 
 *  write value to NVS
 */
esp_err_t write_nvs(const char *config_parameter,const char *in_value) {
  nvs_handle my_handle;
  esp_err_t err;

  // Open
  ESP_LOGI(TAG, "NVS Opening Non-Volatile Storage ... ");
  err = nvs_open(NVS_STORE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "NVS Error (%d) opening!", err);
  } else {
    ESP_LOGI(TAG, "Done");

    // Write
      ESP_LOGI(TAG, "NVS Updating %s value %s ... ",config_parameter, in_value);
      err = nvs_set_str(my_handle, config_parameter, in_value);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "NVS Failed! %d",err);
    } else {
      ESP_LOGI(TAG, "NVS Done");

      // Commit written value.
      // After setting any values, nvs_commit() must be called to ensure changes are written
      // to flash storage. Implementations may write to storage at other times,
      // but this is not guaranteed.
      ESP_LOGI(TAG, "NVS Committing updates ... ");
      err = nvs_commit(my_handle);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS Failed!");
      } else {
        ESP_LOGI(TAG, "NVS Done");
      }
    }

    // Close
    nvs_close(my_handle);
  }

  return err;
}

/* 
 *  read value from NVS
 */
esp_err_t read_nvs(const char *config_parameter,char *out_value,size_t* length) {
  nvs_handle my_handle;
  esp_err_t err;
  uint8_t cnt=0;

  // Open
  ESP_LOGI(TAG, "NVS Opening Non-Volatile Storage ... ");
  err = nvs_open(NVS_STORE, NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "NVS Error (%d) opening!", err);
  } else {
    ESP_LOGI(TAG, "Done");

    // Read
      ESP_LOGI(TAG, "NVS Reading config ... ");
    NVS_READ:
    err = nvs_get_str(my_handle, config_parameter, out_value, length);
    switch (err) {
      case ESP_OK:
        ESP_LOGI(TAG, "NVS value %s = %.*s",config_parameter, *(length), out_value);
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGI(TAG, "NVS The value %s is not initialized yet!",config_parameter);
        *(out_value)=0;
        *(length)=0;
        break;
      case ESP_ERR_NVS_INVALID_LENGTH:
        cnt++;
        if (cnt < 3) { // ESP32 erata
          ESP_LOGI(TAG, "NVS try again (%d)!",cnt);
          goto NVS_READ;
        }
        ESP_LOGE(TAG, "NVS Error ESP_ERR_NVS_INVALID_LENGTH!");
        break;
      default :
        ESP_LOGE(TAG, "NVS Error (%d) reading!", err);
    }
    // Close
    nvs_close(my_handle);
  }

  return err;
}


/* 
 *  store value to NVS
 */
void storeValue(String rxValue) {
  esp_err_t err;

  Serial.println("********* Store");
  if (storeCnt<STORE_SIZE) {
    sprintf(valbuf,"STOREVAL%04d",storeCnt);
    rxValue.getBytes((unsigned char *)storebuf, STORE_MAX_SIZE);
    err=write_nvs(valbuf,storebuf);
    if (err==ESP_OK) {
      Serial.print(valbuf);
      Serial.print(" write ");
      Serial.println(storebuf);
    }
  
    sprintf(storebuf,"%d",++storeCnt);
    err=write_nvs("STORECOUNT",storebuf);
    Serial.print("write storeCnt ");
    Serial.println(storeCnt);
  } else {
    Serial.println("Store full!");
  }
  Serial.println("********* Store");
  
}


/* 
 *  BLE Server Callbacks
 */
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
        Serial.println("device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
        Serial.println("device disconnected");
    }
};

/* 
 *  BLE Characteristic Callbacks
 */
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      String strValue;

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
          strValue += rxValue[i];
        }
          Serial.print(" ");
          Serial.print(strValue.substring(0,5));

        Serial.println();
        if (strValue.substring(0,5)=="value") {
          storeValue(strValue.substring(5));
        }

        Serial.println("*********");
      }
    }
};


/* 
 *  BLE Descriptor Callbacks
 */
class MyDisCallbacks: public BLEDescriptorCallbacks {
    void onWrite(BLEDescriptor *pDescriptor) {
      uint8_t* rxValue = pDescriptor->getValue();

      if (pDescriptor->getLength() > 0) {
        if (rxValue[0]==1) {
          //deviceNotifying=true;
        } else {
          deviceNotifying=false;
        }
        Serial.println("*********");
        Serial.print("Received Descriptor Value: ");
        for (int i = 0; i < pDescriptor->getLength(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

/* 
 *  WiFi on
 */
void Connect_to_Wifi() {

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Adress: ");
  Serial.println(WiFi.localIP());
}

/* 
 *  WiFi off
 */
void Disconnect_from_Wifi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}


/* 
 *  Send Data to Cloud-Server via WiFi
 */
void Send_Data() {
  esp_err_t err;
  size_t readlen;

  if (storeCnt>0) {
    sprintf(valbuf,"STOREVAL%04d",storeCnt-1);
    err=read_nvs(valbuf,storebuf,&readlen);
    if (err==ESP_OK) {
      Serial.print(valbuf);
      Serial.print(" ");
      Serial.println(storebuf);
    }
    String txValue = String(storebuf);

    Serial.println("\nStarting connection to server..."+String(WEB_SERVER));
    if (client.connect(WEB_SERVER, WEB_PORT)) { //client.connect(server, 443, test_ca_cert, test_client_cert, test_client_key)
      Serial.println("Connected to server "+String(WEB_SERVER)+" !");
      // Make a HTTP request:
      String strURL = "GET "+String(WEB_URL)+String(txValue)+" HTTP/1.1\nHost: "+String(WEB_SERVER)+"\nUser-Agent: esp-idf/1.0 esp32\n\n";
      Serial.println(strURL);
      client.println(strURL);

      Serial.print("Waiting for response "); //WiFiClientSecure uses a non blocking implementation
      while (!client.available()){
        delay(50); //
        Serial.print(".");
      }  
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available()) {
        char c = client.read();
        Serial.write(c);
      }
      sprintf(storebuf,"%d",--storeCnt);
      err=write_nvs("STORECOUNT",storebuf);
      Serial.print("write storeCnt ");
      Serial.println(storeCnt);

    } else {
      Serial.println("Connection failed!");
    }
  
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
  }
}

 /* 
  *  Activate BLE and start advertising
 */
void Prepare_BLE() {
  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pDescriptor = new BLE2902();
  pCharacteristic->addDescriptor(pDescriptor);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pDescriptor->setCallbacks(new MyDisCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  

}

void setup() {
  esp_err_t err;
  size_t readlen;

  Serial.begin(115200);
  nvs_flash_init();

  err=read_nvs("STORECOUNT",storebuf,&readlen);
  if (err==ESP_OK) {
    sscanf(storebuf,"%d",&storeCnt);
    Serial.print("storeCnt ");
    Serial.println(storeCnt);
  }
  
  if (storeCnt>0) {
    Connect_to_Wifi();
  }
  while (storeCnt>0) {
     Send_Data();
  }  
  Disconnect_from_Wifi();

  Prepare_BLE();
  
}

void loop() {
  
  delay(60000); // BLE Use Time
  // nope, do nothing here
  //vTaskDelay(portMAX_DELAY); // wait as much as posible ...
  Serial.println("enter deep sleep");
  Serial.flush();
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  Serial.println("in deep sleep");
  Serial.flush();
}
