// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
Code to upload in the ESP32s3 boards that will be used as Relay (that can only transmit from A to B)
*/


//==========================
// Variables
//==========================



// List of the different states in which the board can be in
enum SetupState {
  CENTRAL_SETUP,
  WAIT,
  PERIPHERAL_SETUP
};


//String username;
String messageToB;
String MessageFromA;
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService relayService("8e412615-8d6a-4982-8c12-2a5d8f5eff29AC");
BLEStringCharacteristic relayCharacteristic("62c7d399-e3b8-4580-876e-5a75db102cf5", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP; // Launched as a peripheral


//==========================
// functions
//==========================

// Connecting to the peripheral (ESP32s3-B) as a central
void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }

// Check if the peripheral (ESP32s3-B) presents the right attribute 
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

// Check if the peripheral (ESP32s3-B) presents the right characteristic
  BLECharacteristic bCharacteristic = peripheral.characteristic("8b7e081e-13e0-4b72-81f8-f6695b0cd749");
 
  if (!bCharacteristic) {
    Serial.println("Peripheral does not have message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!bCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay message characteristic!");
    peripheral.disconnect();
    return;
  }
 


// Typing the message to send in 20s
    messageToB = relayCharacteristic.value();
    char messageToRelay[512];
    sprintf(messageToRelay, "%s", messageToB.c_str());


    bCharacteristic.writeValue(messageToRelay);
    Serial.println("transmitted");
    
 
}


void centralSetup() {
 
   // begin initialization as a Central
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
  
  Serial.println("on ESP32s3_relay ready to send . . .");
   
  BLE.scanForName("ESP32s3-B");
  
  BLEDevice peripheral = BLE.available();
 
}


void centralLoop() {
  BLEDevice peripheral = BLE.available();
 
  if (peripheral) {
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

 // Looking for the peripheral named ESP32s3-B  
    if (peripheral.localName() != "ESP32s3-B") { //as dans /ESP32_B
      return;
    }
   
    // stop scanning
    BLE.stopScan();


    controlMessage(peripheral);
   
   peripheral.disconnect();

  }
}


void peripheralSetup() {
   // begin initialization as a peripheral
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
  //set advertised local name and service UUID
  BLE.setLocalName("ESP32s3-Relay");
  BLE.setAdvertisedService(relayService);
 
  //add the characteristic to the service
  relayService.addCharacteristic(relayCharacteristic);
 
  //add service
  BLE.addService(relayService);
 
  //set charachteristic's initial value
  relayCharacteristic.writeValue("");
 
  //start advertising
  BLE.advertise();
  Serial.println("Setup as a peripheral successful");


}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {

      if(relayCharacteristic.written()) {
        MessageFromA = relayCharacteristic.value();
        // Serial.println(MessageFromA); as this board is a relay, there is no need to print the message
      }
     
  }
}


//============================
// main setup() & loop()
//============================

void setup() {
  Serial.begin(115200);
 
  while (!Serial);

  Serial.print("Hi ");

 
}


void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - intervalStartTime;

  if (currentState == CENTRAL_SETUP) {
    Serial.println("mode Central");
    // begin initialization
    if (!BLE.begin()) {
      Serial.println("Starting BLE module failed!");
      exit(1);
    }

    //Serial.print("user : ");
    //Serial.print(username);
    Serial.print(" on ESP32s3_Relay ready to relay. . .");

    BLE.scanForName("ESP32s3-B");

    BLEDevice peripheral = BLE.available();
    while (elapsedTime <= intervalDuration) {
      centralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    //peripheral.disconnect();
    intervalStartTime = currentTime;
    currentState = PERIPHERAL_SETUP; // switch state
  }

  if (currentState == PERIPHERAL_SETUP) {
    Serial.println("mode Peripheral");
    peripheralSetup();
    currentTime = millis();
    elapsedTime = currentTime - intervalStartTime;
    while (elapsedTime <= 60000) {
      peripheralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    intervalStartTime = currentTime;
    currentState = CENTRAL_SETUP; // switch state
  }
}






