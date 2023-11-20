// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
ESP32s3_R : launch 30s before other devices (ESP32s3_A & ESP32s3_B)
*/


//==========================
// Variables
//==========================



// List of states in which the board can be in
enum SetupState {
  CENTRAL_SETUP,
  PERIPHERAL_SETUP
};

String destination; // to store the initial of the destination board : 'a' for ESP32s3_A ; 'b' for ESP32s3_B, 'r' for ESP32s3_R
String username; // store the username chosen by the human user
String messageToB; // store the message that the user want to send
String messageFromA = "(r)"; // store the message received from another device
String serialRecv = ""; // store the received Serial message (from the human user)
bool serialMessageReceived;  // Flag to indicate if a complete serial message has been received

//variables to launch and manage the time to switch mode (or not)
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;

// The BLE service for the chat
BLEService relayService("8e412615-8d6a-4982-8c12-2a5d8f5eff29AC");
// The BLE characteristic for the chat
BLEStringCharacteristic relayCharacteristic("62c7d399-e3b8-4580-876e-5a75db102cf5", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP; // start as a Peripheral


//==========================
// functions
//==========================


// Connection to the peripheral :  try and check 
void controlMessage(BLEDevice peripheral) {

  if (peripheral.connect()) {

  } else {

    Serial.println("Failed to connect !");
    return;
  }
 
// Check that the peripheral got the right attribute
  if (peripheral.discoverAttributes()) {

  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
  // retrieve the characteristic of the peripheral (ESP32s3_B)
  BLECharacteristic bCharacteristic = peripheral.characteristic("8b7e081e-13e0-4b72-81f8-f6695b0cd749");
 
 // Check that the peripheral got the right characteristic
  if (!bCharacteristic) {
    Serial.println("Peripheral does not have message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!bCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay message characteristic!");
    peripheral.disconnect();
    return;
  }
   // if the destination of the incoming message is not the current board (ESP32s3_R) --> act as a relay
      if (messageFromA.charAt(1) == 'b' || messageFromA.charAt(1) == 'a' ) {
      messageToB = relayCharacteristic.value();
      char messageToRelay[512];
      sprintf(messageToRelay, "%s", messageToB.c_str());

// transmits the message to peripheral 
      bCharacteristic.writeValue(messageToRelay);
      Serial.println("transmitted");
      Serial.println("you can't send a message as you serve as a relay for another communication");
   } else {

  // notify the human user that he can choose destination and write a message
  Serial.println("Select Destination's device by its local name (within 10s):");
  Serial.println("- enter b if you want to communicate with ESP32s3_B");
  Serial.println("- enter a if you want to communicate with ESP32s3_A");
  Serial.println("- enter r if you want to communicate with yourself");
  Serial.setTimeout(10000);
  destination = Serial.readStringUntil('\n');
  
  // drop the message if the destination is unknown (neither "a", "b", "r")
  if (destination != "a" && destination != "b" && destination != "r") {
    Serial.println("unknown receiver --> message will be dropped");
  }

  // notify the human user that he can type a message because the destination is known (either "a", "b", "r")
 
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
  
  messageToB = Serial.readStringUntil('\n');
  Serial.print(username);
  Serial.print(" : ");
  Serial.println(messageToB);
   
  char messageToRelay[512];
  sprintf(messageToRelay, "(%s) %s: %s", destination.c_str(), username.c_str(), messageToB.c_str());

// transmit the message by writing the Peripheral's characteristic
  bCharacteristic.writeValue(messageToRelay);
   }
}



void centralSetup() {
 
   // begin initialization as a Central
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
 
 // if a peripheral is detected and available
  if (peripheral) {
   // if the detected peripheral's name is not ESP32s3_B, everything stops
    if (peripheral.localName() != "ESP32s3_B") { 
      return;
    }
   
    BLE.stopScan();

    controlMessage(peripheral); // everything that was defined in controlMessage() occurs
   
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


}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {

// if the received message's destination is the current board ('r'), then the message is printed
      if(relayCharacteristic.written()) {
        messageFromA = relayCharacteristic.value();
        if (messageFromA.charAt(1) == 'r') {
          Serial.println(messageFromA);
      }
      }

  }
}


//============================
// main setup() & loop()
//============================
void setup() {
  // setting up the serial connection between the PC and the board
  Serial.begin(115200);
 
  while (!Serial);
  
  // the human user type his name
  Serial.println("Enter your name (10s):");
  Serial.setTimeout(10000);
  username = Serial.readStringUntil('\n');
  
  Serial.print("Hi ");
  Serial.println(username);  
 
}


void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - intervalStartTime;

  if (currentState == CENTRAL_SETUP) {

    if (!BLE.begin()) {
      Serial.println("Starting BLE module failed!");
      exit(1);
    }


    BLE.scanForName("ESP32s3_B");

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

    peripheralSetup();
    currentTime = millis();
    elapsedTime = currentTime - intervalStartTime;
    while (elapsedTime <= 60000) { // stay in peripheral mode for 60s
      peripheralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    intervalStartTime = currentTime;
    currentState = CENTRAL_SETUP; // switch state 
  }
}






