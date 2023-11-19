// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
Code to upload in the ESP32s3 boards that will be used as B (that can both send and receive)
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


String username;
String messageToA;
String messageFromA;
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService bService("2f29538b-8fc4-4bab-ae17-f41a5bfb8c3e");
BLEStringCharacteristic bCharacteristic("8b7e081e-13e0-4b72-81f8-f6695b0cd749", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP; // launched as a peripheral


//==========================
// functions
//==========================

// Connecting to the peripheral (ESP32s3-A) as a central
void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }
 
 // Check if the peripheral presents the right attribute
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
 // Check if the peripheral presents the right characteristic
  BLECharacteristic aCharacteristic = peripheral.characteristic("121cc516-a802-4b39-b809-f68645efe490");
 
  if (!aCharacteristic) {
    Serial.println("Peripheral does not have message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!aCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable message characteristic!");
    peripheral.disconnect();
    return;
  }
 
  // Typing the message to send in 20s
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
 


// Printing the message to send on the terminal
    messageToA = Serial.readStringUntil('\n');
    Serial.print(username);
    Serial.print(" : ");
    Serial.println(messageToA);
   
    char messageToRelay[512];
    sprintf(messageToRelay, "%s: %s", username.c_str(), messageToA.c_str());

// Sending the message by writing peripheral's characteristic (then the peripheral will send to B)
    aCharacteristic.writeValue(messageToRelay);
   
   
 
 
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
  Serial.print(" on ESP32_B ready to send . . .");
   
  BLE.scanForName("ESP32s3-A");
  
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

  // Looking for the peripheral named ESP32s3-A 
    if (peripheral.localName() != "ESP32s3-A") { //as defined in /ESP32_A
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
  BLE.setLocalName("ESP32s3-B");
  BLE.setAdvertisedService(bService);
 
  //add the characteristic to the service
  bService.addCharacteristic(bCharacteristic);
 
  //add service
  BLE.addService(bService);
 
  //set charachteristic's initial value
  bCharacteristic.writeValue("");
 
  //start advertising
  BLE.advertise();
  Serial.println("Setup as a peripheral successful");


}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {

// Print the message when it is received = print the characteristic when it is written
      if(bCharacteristic.written()) {
        messageFromA = bCharacteristic.value();
        Serial.println(messageFromA);
      }

  }
}


//============================
// main setup() & loop()
//============================

void setup() {
  // setup the serial connection from the terminal to the board
  Serial.begin(115200);
 
  while (!Serial);
  Serial.println("Enter your name (10s):");
  Serial.setTimeout(10000);
  username = Serial.readStringUntil('\n');
  Serial.print("Hi ");
  Serial.println(username);  
 
}


void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - intervalStartTime;

// Alternate between central and peripheral states

  if (currentState == CENTRAL_SETUP) {
    // begin initialization
    if (!BLE.begin()) {
      Serial.println("Starting BLE module failed!");
      exit(1);
    }
    Serial.println("mode Central");

    Serial.print("user : ");
    Serial.print(username);
    Serial.print(" on ESP32_B ready to send . . .");

    BLE.scanForName("ESP32s3-A");

    BLEDevice peripheral = BLE.available();
    while (elapsedTime <= intervalDuration) {
      centralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    peripheral.disconnect();
    intervalStartTime = currentTime;
    currentState = PERIPHERAL_SETUP; // switch state
  }

  if (currentState == PERIPHERAL_SETUP) {
    Serial.println("mode Peripheral");
    peripheralSetup();
    while (elapsedTime <= 60000){
      peripheralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
      
    }
    intervalStartTime = currentTime;
    currentState = CENTRAL_SETUP; // switch state
    
  }
}





