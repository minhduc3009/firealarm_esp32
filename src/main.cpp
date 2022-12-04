
#include <Arduino.h>
#include <Keypad.h>
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // three columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3' , 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {19, 18, 5, 17}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins{19, 18, 5, 17};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup(void)
{

  Serial.begin(115200);                                         //send and receive at 115200 baud
  inputString.reserve(200);
  Serial.print("*********************************** WaveShare ***********************************\n");

  // Create the BLE Device
  BLEDevice::init("FireAlarmSystem");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop(void)
{
  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
    switch (key)
    {
      case '1':
        inputString = "SOS001";
      break;
      case '2':
        inputString = "SOS002";
      break;
      case '3':
        inputString = "SOS003";
      break;
      case '4':
        inputString = "SOS004";
      break;
      default:
        break;
    }

    Serial.println(inputString); 
    pCharacteristic->setValue((char*)inputString.c_str());
    pCharacteristic->notify();
    inputString = "";
    // delay(1000);
  }
  if (stringComplete) {
    Serial.print("Reciver:===>"); Serial.println(inputString); 
    if(inputString=="alarm") {
      inputString = "SOS001";
      Serial.print("Send BLE:===>"); Serial.println(inputString); 
      pCharacteristic->setValue((char*)inputString.c_str());
      pCharacteristic->notify();
      // inputString = "";
      // stringComplete = false;// clear the string:
    }
    if(inputString=="SOS001" || inputString=="SOS002" || inputString=="SOS003" || inputString=="SOS004" ) {
      Serial.print("Send BLE:===>"); Serial.println(inputString); 
      pCharacteristic->setValue((char*)inputString.c_str());
      pCharacteristic->notify();
      // inputString = "";
      // stringComplete = false;// clear the string:
    }
      inputString = "";
      stringComplete = false;// clear the string:
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }

  delay(50);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if (inChar == '\n') {
      stringComplete = true;
    }
    else inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:

  }
}