// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
  Code to upload in the ESP32s3 boards that will be used as A (that can both send and receive)
*/


//==========================
// Variables
//==========================


// List of the different states that the board can be in
enum SetupState {
  CENTRAL_SETUP,
  WAIT,
  PERIPHERAL_SETUP
};


String username;
String messageToB;
String messageFromB;
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService aService("99f0559c-6828-4a2d-ba56-5d109fba2522");
BLEStringCharacteristic aCharacteristic("121cc516-a802-4b39-b809-f68645efe490", BLERead | BLEWrite, 512 );

SetupState currentState = CENTRAL_SETUP; // launched as a central


//==========================
// functions
//==========================

//Connecting to the peripheral(ESP32s3-r) as a central

void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }

// Check if the peripheral (ESP32s3-r) presents the right attribute
 
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
 // Check if the peripheral (ESP32s3-r) presents the right characteristic
  BLECharacteristic relayCharacteristic = peripheral.characteristic("62c7d399-e3b8-4580-876e-5a75db102cf5");
 
  if (!relayCharacteristic) {
    Serial.println("Peripheral does not have relay message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!relayCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay message characteristic!");
    peripheral.disconnect();
    return;
  }
 
 // Typing the message to send in 20s
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
 
// Printing the message to send on the terminal
  messageToB = Serial.readStringUntil('\n');
  Serial.print(username);
  Serial.print(" : ");
  Serial.println(messageToB);
   
  char messageToRelay[512];
  sprintf(messageToRelay, "%s: %s", username.c_str(), messageToB.c_str());

// Sending the message by writing peripheral's characteristic (then the peripheral will send to B)
  relayCharacteristic.writeValue(messageToRelay);
   
 
 
}


void centralSetup() {
 
   // begin initialization as a Central
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
   
   // Notify the user that this board is ready to send
  Serial.print("user : ");
  Serial.print(username);
  Serial.print(" on ESP32s3_A ready to send . . .");
  BLE.scanForName("ESP32s3-Relay");
  
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
  
  // Looking for the peripheral named ESP32s3-relay
    if (peripheral.localName() != "ESP32s3-Relay") { //as defined in /ESP32_Relay
      return;
    }
   
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
  BLE.setLocalName("ESP32s3-A");
  BLE.setAdvertisedService(aService);
 
  //add the characteristic to the service
  aService.addCharacteristic(aCharacteristic);
 
  //add service
  BLE.addService(aService);
 
  //set characteristic's initial value
  aCharacteristic.writeValue("");
 
  //start advertising
  BLE.advertise();
  Serial.println("Setup as a peripheral successful");


}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {

// Print the message when it is received = print the characteristic when it is written
 
      if(aCharacteristic.written()) {
        messageFromB = aCharacteristic.value();
        Serial.println(messageFromB);

    }
  }
}


//============================
// main setup() & loop()
//============================

void setup() {
  // set up the serial connection from the terminal to the board
  Serial.begin(115200);
 
  while (!Serial);
  Serial.println("Enter your name (10s):");
  Serial.setTimeout(10000);
  username = Serial.readStringUntil('\n');
  Serial.print("Hi ");
  Serial.println(username);  
 
}


void loop() {

  // Alternate between central and peripheral states
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - intervalStartTime;

  if (currentState == CENTRAL_SETUP) {
    Serial.println("mode Central");
    // begin initialization
    if (!BLE.begin()) {
      Serial.println("Starting BLE module failed!");
      exit(1);
    }

    Serial.print("user : ");
    Serial.print(username);
    Serial.print(" on ESP32s3_A ready to send . . .");

    BLE.scanForName("ESP32s3-Relay");

    BLEDevice peripheral = BLE.available();
    while (elapsedTime <= intervalDuration) {
      centralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }

    intervalStartTime = currentTime;
    currentState = PERIPHERAL_SETUP; // switch state
  }

  if (currentState == PERIPHERAL_SETUP) {
    Serial.println("mode Peripheral");
    peripheralSetup();
    currentTime = millis();
    elapsedTime = currentTime - intervalStartTime;
    while (elapsedTime <= 60000/*60s intervalDuration*/) {
      peripheralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    intervalStartTime = currentTime;
    currentState = CENTRAL_SETUP; // switch state
  }
}





