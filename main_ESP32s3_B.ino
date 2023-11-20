// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
ESP32s3_B : launch ~ 30s after ESP32s3_R & at the same time than ESP32s3_A
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
String username; // to store the username chosen by the human user
String messageTo; // store the message that the user want to send
String messageFromA ="(b)"; // store the message received from another device
String serialRecv = ""; // store the received Serial message (from the human user)
bool serialMessageReceived; // Flag to indicate if a complete serial message has been received

//variables to launch and manage the time to switch mode (or not)
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;

// The BLE service for the chat
BLEService bService("2f29538b-8fc4-4bab-ae17-f41a5bfb8c3e");
// The BLE characteristic for the chat
BLEStringCharacteristic bCharacteristic("8b7e081e-13e0-4b72-81f8-f6695b0cd749", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP; // Start as a peripheral


//==========================
// functions
//==========================


void controlMessage(BLEDevice peripheral) {

 // Connection to the peripheral :  try and check 
  if (peripheral.connect()) {

  } else {
    Serial.println("Failed to connect !");
    return;
  }
 
//  Check that the peripheral got the right attribute
  if (peripheral.discoverAttributes()) {

  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
   // retrieve the characteristic of the peripheral (ESP32s3_A)
  BLECharacteristic aCharacteristic = peripheral.characteristic("121cc516-a802-4b39-b809-f68645efe490");
 
 // Check that the peripheral got the right characteristic
  if (!aCharacteristic) {
    Serial.println("Peripheral does not have relay message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!aCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay message characteristic!");
    peripheral.disconnect();
    return;
  }
  
     // if the destination of the incoming message is not the current board (ESP32s3_B) --> act as a relay
  if (messageFromA.charAt(1) == 'a' || messageFromA.charAt(1) == 'r') {
      messageTo = bCharacteristic.value();
      char messageToRelay[512];
      sprintf(messageToRelay, "%s", messageTo.c_str());

// transmit the message to peripheral 
      aCharacteristic.writeValue(messageToRelay);
      Serial.println("transmitted");
      Serial.println("you can't send a message as you serve as a relay for another communication");
   } else  {
  
    // notify the human user that he can choose destination and write a message
  Serial.println("Select Destination's device by its local name:");
  Serial.println("- enter a if you want to communicate with ESP32s3_A");
  Serial.println("- enter r if you wat to communicate with ESP32s3_R");
  Serial.println("- enter b if you wat to communicate with ESP32s3_B");
  Serial.setTimeout(10000);
  destination = Serial.readStringUntil('\n');
  
   // drop the message if the destination is unknown (neither "a", "b", "r")
  if (destination != "a" && destination != "b" && destination != "r") {
    Serial.println("unknown receiver --> message will be dropped");
  }
 
  // notify the human user that he can type a message because the destination is known (either "a", "b", "r")
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
  
  messageTo = Serial.readStringUntil('\n');
  Serial.print(username);
  Serial.print(" : ");
  Serial.println(messageTo);
   
  char messageToRelay[512];
  sprintf(messageToRelay, "(%s) %s: %s", destination.c_str(), username.c_str(), messageTo.c_str());

// transmit the message by writing the Peripheral's characteristic
  aCharacteristic.writeValue(messageToRelay);
   }

}


void centralSetup() {
 
   // begin initialization as a central
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
   
  Serial.print("user : ");
  Serial.print(username);
  Serial.print(" on ESP32s3_B ready to send . . .");
  BLE.scanForName("ESP32s3_A");
  
  BLEDevice peripheral = BLE.available();
 
}


void centralLoop() {
  BLEDevice peripheral = BLE.available();
 
  if (peripheral) {

   
    if (peripheral.localName() != "ESP32s3_A") { 
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
  BLE.setLocalName("ESP32s3_B");
  BLE.setAdvertisedService(bService);
 
  //add the characteristic to the service
  bService.addCharacteristic(bCharacteristic);
 
  //add service
  BLE.addService(bService);
 
  //set charachteristic's initial value
  bCharacteristic.writeValue("");
 
  //start advertising
  BLE.advertise();
//  Serial.println("Setup as a peripheral successful");
}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {
    // if the received message's destination is the current board ('b'), then the message is printed

      if(bCharacteristic.written()) {
        messageFromA = bCharacteristic.value();
        if(messageFromA.charAt(1) == 'b') {
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

    BLE.scanForName("ESP32s3_A");

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





