#include <ArduinoBLE.h>

/*
  Code to upload in the ESP32s3 that will be used as a Peripheral

*/

BLEService stringService("3dbc8248-5710-450a-aef6-3ab15742c75e"); // UUID generated on www.uuidgenerator.net

// Bluetooth® Low Energy String Characteristic - custom 128-bit UUID, read and writable by central
BLEStringCharacteristic stringCharacteristic("35929a58-c1af-4ce8-a100-a3cfc97a1705", BLERead | BLEWrite,1024); // messsage length of max 1024 bytes 

// String to stock the message(s) received from the central 
String messageFromCentral = "";

void setup() {
  //set up the serial connexion PC <--> board
  Serial.begin(115200);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }
  
    // set advertised local name and service UUID: (adapted from ArduinoBLE/Examples/Peripheral/Led)
  BLE.setLocalName("ESP32s3");
  BLE.setAdvertisedService(stringService);

  // add the characteristic to the service (adapted from ArduinoBLE/Examples/Peripheral/Led)
  stringService.addCharacteristic(stringCharacteristic);

  // add service (adapted from ArduinoBLE/Examples/Central/Led)
  BLE.addService(stringService);

  // set the initial value for the characeristic: (adapted from ArduinoBLE/Examples/Peripheral/Led)
  stringCharacteristic.writeValue("initial value");

  // start advertising (adapted from ArduinoBLE/Examples/Peripheral/Led)
  BLE.advertise();

  Serial.println("BLE Chat Peripheral ready to receive");
}

void loop() {
  // listen for Bluetooth Low Energy peripherals to connect: (pasted from ArduinoBLE/Examples/Peripheral/Led)
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral: (pasted from ArduinoBLE/Examples/Peripheral/Led)
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    //(adapted from ArduinoBLE/Examples/Peripheral/Led)
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to set the variable:
      if (stringCharacteristic.written()) {
        messageFromCentral = stringCharacteristic.value();
        Serial.print("Received message from central: ");
        Serial.println(messageFromCentral);
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

