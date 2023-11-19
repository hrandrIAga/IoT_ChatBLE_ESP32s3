// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
ESP32s3_A
*/


//==========================
// Variables
//==========================


enum SetupState { // Different states that the board can be
  CENTRAL_SETUP,
  WAIT,
  PERIPHERAL_SETUP
};

bool box = true;
String destination;
String username;
String messageToB;
String messageFromB = "(a)";
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService aService("99f0559c-6828-4a2d-ba56-5d109fba2522");
BLEStringCharacteristic aCharacteristic("121cc516-a802-4b39-b809-f68645efe490", BLERead | BLEWrite, 512 );

SetupState currentState = CENTRAL_SETUP; // start as a central


//==========================
// functions
//==========================

// Connection to the peripheral

void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }
 
 // Checking if the peripheral presents right attribute
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
 // Checking if the peripheral presents right characteristic

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
  

    if (messageFromB.charAt(1) == 'r') {
      messageToB = aCharacteristic.value();
      char messageToRelay[512];
      sprintf(messageToRelay, "%s", messageToB.c_str());

      relayCharacteristic.writeValue(messageToRelay);
      Serial.println("transmitted");
      Serial.println("you can't send a message as you serve as a relay for another communication");
   } else if (messageFromB.charAt(1) == 'a') {

  
  Serial.println("Select Destination's device by its local name:");
  Serial.println("- enter b if you want to communicate with ESP32s3_B");
  Serial.println("- enter r if you wat to communicate with ESP32s3_R");
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


  relayCharacteristic.writeValue(messageToRelay);
   }

}


void centralSetup() {
 
   // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
   
  Serial.print("user : ");
  Serial.print(username);
  Serial.print(" on ESP32s3_A ready to send . . .");
  BLE.scanForName("ESP32s3_R");
  
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
   
    if (peripheral.localName() != "ESP32s3_R") { 
      return;
    }
   
    // stop scanning
    BLE.stopScan();


    controlMessage(peripheral);
   
   // BLE.scanForName("ESP32s3_Relay");
   
   peripheral.disconnect();
  }
}


void peripheralSetup() {
   // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    exit(1);
  }
  //set advertised local name and service UUID
  BLE.setLocalName("ESP32s3_A");
  BLE.setAdvertisedService(aService);
 
  //add the characteristic to the service
  aService.addCharacteristic(aCharacteristic);
 
  //add service
  BLE.addService(aService);
 
  //set charachteristic's initial value
  aCharacteristic.writeValue("");
 
  //start advertising
  BLE.advertise();
  Serial.println("Setup as a peripheral successful");
}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 

      if(aCharacteristic.written()) {
        messageFromB = aCharacteristic.value();
        if(messageFromB.charAt(1) == 'a') {
          Serial.println(messageFromB);
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
  Serial.println(username);  
 
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

    Serial.print("user : ");
    Serial.print(username);
    Serial.print(" on ESP32s3_A ready to send . . .");

    BLE.scanForName("ESP32s3_R");

    BLEDevice peripheral = BLE.available();
    while (elapsedTime <= intervalDuration) {
      centralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
 
    intervalStartTime = currentTime;
    currentState = PERIPHERAL_SETUP;
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
    currentState = CENTRAL_SETUP;
  }
}





