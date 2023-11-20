// ArduinoBLE - Version: Latest 
#include <ArduinoBLE.h>

/*
ESP32s3_B
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

bool box = true;
String destination;
String username;
String messageTo;
String messageFromA ="(b)";
String serialRecv = "";
bool serialMessageReceived;
unsigned long intervalStartTime = 0;
unsigned long intervalDuration = 30000;


BLEService bService("2f29538b-8fc4-4bab-ae17-f41a5bfb8c3e");
BLEStringCharacteristic bCharacteristic("8b7e081e-13e0-4b72-81f8-f6695b0cd749", BLERead | BLEWrite, 512 );

SetupState currentState = PERIPHERAL_SETUP;


//==========================
// functions
//==========================

// Connection try and check to the peripheral
void controlMessage(BLEDevice peripheral) {
  Serial.println("Connecting . . .");
 
  if (peripheral.connect()) {
    Serial.println("connected");
  } else {
    Serial.println("Failed to connect !");
    return;
  }
  // Checking if the peripheral got the right attribute
  Serial.println("Discovering attribute . . .");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
 
 // Checking if the peripheral got the right characteristics
  BLECharacteristic aCharacteristic = peripheral.characteristic("121cc516-a802-4b39-b809-f68645efe490");
 

  if (!aCharacteristic) {
    Serial.println("Peripheral does not have relay message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!aCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay message characteristic!");
    peripheral.disconnect();
    return;
  }
     //if the destination of the received message is the board 'a'
  if (messageFromA.charAt(1) == 'a') {
      messageTo = bCharacteristic.value();
      char messageToRelay[512];
      sprintf(messageToRelay, "%s", messageTo.c_str());
// the received message is transmitted to 'a' by writing its characteristic and this board serve as a relay
      aCharacteristic.writeValue(messageToRelay);
      Serial.println("transmitted");
      Serial.println("you can't send a message as you serve as a relay for another communication");

   //if the destination of the received message is this board 'b'         
   } else if (messageFromA.charAt(1) == 'b') {


  Serial.println("Select Destination's device by its local name:");
  Serial.println("- enter a if you want to communicate with ESP32s3_A");
  Serial.println("- enter r if you wat to communicate with ESP32s3_R");
  Serial.setTimeout(10000);
  destination = Serial.readStringUntil('\n');
 
 // telling the human user that he can Write a new message
  Serial.println("Type your message (in 20s):");
  Serial.setTimeout(20000);
  
  messageTo = Serial.readStringUntil('\n');
  Serial.print(username);
  Serial.print(" : ");
  Serial.println(messageTo);
   
  char messageToRelay[512];
  sprintf(messageToRelay, "(%s) %s: %s", destination.c_str(), username.c_str(), messageTo.c_str());


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

  // looking for the peripheral ESP32s3_A
  BLE.scanForName("ESP32s3_A");
  
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
   
    if (peripheral.localName() != "ESP32s3_A") { 
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
  Serial.println("Setup as a peripheral successful");
}


void peripheralLoop() {
  BLEDevice central = BLE.central();
 
  if (central) {
// if this current board is the destination of a message, it is printed on the terminal
      if(bCharacteristic.written()) {
        messageFromA = bCharacteristic.value();
        if(messageFromA.charAt(1) == 'b') {
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
  // Set up serial connection from PC to this board
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
    Serial.print(" on ESP32s3_B ready to send . . .");

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
    Serial.println("mode Peripheral");
    peripheralSetup();
    currentTime = millis();
    elapsedTime = currentTime - intervalStartTime;
    while (elapsedTime <= 60000/*intervalDuration*/) {
      peripheralLoop();
      currentTime = millis();
      elapsedTime = currentTime - intervalStartTime;
    }
    intervalStartTime = currentTime;
    currentState = CENTRAL_SETUP; // switch state
  }
}





