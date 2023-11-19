// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
ESP32s3_R
*/


//==========================
// Variables
//==========================




enum SetupState {
  CENTRAL_SETUP,
  WAIT,
  PERIPHERAL_SETUP
};

bool box = true;
String destination;
String username;
String messageToB;
String messageFromA = "(r)";
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService relayService("8e412615-8d6a-4982-8c12-2a5d8f5eff29AC");
BLEStringCharacteristic relayCharacteristic("62c7d399-e3b8-4580-876e-5a75db102cf5", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP;


//==========================
// functions
//==========================


void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }
 
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
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
 
      if (messageFromA.charAt(1) == 'b') {
      messageToB = relayCharacteristic.value();
      char messageToRelay[512];
      sprintf(messageToRelay, "%s", messageToB.c_str());

      bCharacteristic.writeValue(messageToRelay);
      Serial.println("transmitted");
      Serial.println("you can't send a message as you serve as a relay for another communication");
   } else if (messageFromA.charAt(1) == 'r') {

  
  Serial.println("Select Destination's device by its local name:");
  Serial.println("- enter b if you want to communicate with ESP32s3_B");
  Serial.println("- enter a if you wat to communicate with ESP32s3_A");
  Serial.setTimeout(10000);
  destination = Serial.readStringUntil('\n');
 
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
  
  messageToB = Serial.readStringUntil('\n');
  Serial.print(username);
  Serial.print(" : ");
  Serial.println(messageToB);
   
  char messageToRelay[512];
  sprintf(messageToRelay, "(%s) %s: %s", destination.c_str(), username.c_str(), messageToB.c_str());


  bCharacteristic.writeValue(messageToRelay);
   }
 
 }
 
 



void centralSetup() {
 
   // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
  
  Serial.println("on ESP32s3_relay ready to send . . .");
   
  BLE.scanForName("ESP32s3_B");
  
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
   
    if (peripheral.localName() != "ESP32s3_B") { //as defined in ESP32s3_B
      return;
    }
   
    // stop scanning
    BLE.stopScan();


    controlMessage(peripheral);
   
   peripheral.disconnect();
   // BLE.scanForName("ESP32s3_Relay");
  }
}


void peripheralSetup() {
   // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
  //set advertised local name and service UUID
  BLE.setLocalName("ESP32s3_R");
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
        messageFromA = relayCharacteristic.value();
        if (messageFromA.charAt(1) == 'r') {
          Serial.println(messageFromA);
          box = true;
        } else {
          box = false;
      }
      }

  }
}


//============================
// main setup() & loop()
//============================

void setup() {
  Serial.begin(115200);
 
  while (!Serial);
  
  Serial.println("Enter your name (10s):");
  Serial.setTimeout(10000);
  username = Serial.readStringUntil('\n');
  
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

    BLE.scanForName("ESP32s3_B");

    BLEDevice peripheral = BLE.available();
    while (elapsedTime <= intervalDuration) {
      centralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    //peripheral.disconnect();
    intervalStartTime = currentTime;
    currentState = PERIPHERAL_SETUP;
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
    currentState = CENTRAL_SETUP;
  }
}






