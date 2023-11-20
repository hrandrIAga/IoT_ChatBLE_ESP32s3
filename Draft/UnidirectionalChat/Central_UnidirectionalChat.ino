#include <Arduino.h>
/*
  Code to upload in the ESP32s3 that will be used as a Central
*/

// variables for button 
const int buttonPin = 0; 
int oldButtonState = LOW;


void setup() {
  //set up the serial connexion PC <--> board
  Serial.begin(115200);
  while (!Serial);

  // configure the button pin as input (from ArduinoBLE/Examples/Central/LedControl)
  pinMode(buttonPin, INPUT_PULLUP);

  // initialize the Bluetooth Low Energy hardware 
  BLE.begin();

  Serial.println("BLE - chat central ready to send");

  // start scanning for peripherals (UUID generated on www.uuidgenerator.net and defined in /chatBLE_peripherique)
  BLE.scanForUuid("3dbc8248-5710-450a-aef6-3ab15742c75e");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "ESP32s3") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    controlMessage(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid("3dbc8248-5710-450a-aef6-3ab15742c75e");
  }
}

void controlMessage(BLEDevice peripheral) {
  // connect to the peripheral (conservé de youtube et ArduinoBLE/Examples/Central/LedControl)
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes (conservé de youtube et ArduinoBLE/Examples/Central/LedControl)
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the string message characteristic defini dans /chatBLE_peripherique
  BLECharacteristic stringCharacteristic = peripheral.characteristic("35929a58-c1af-4ce8-a100-a3cfc97a1705");

  if (!stringCharacteristic) {
    Serial.println("Peripheral does not have string message characteristic!");
    peripheral.disconnect();
    return;
  } else if (!stringCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable string message characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    // read the button pin
    int buttonState = !digitalRead(buttonPin);

    if (oldButtonState != buttonState) {
      // button changed
      oldButtonState = buttonState;

      if (buttonState) {
        Serial.println("button pressed = modifying peripheral's characteristic");

        // button boot is pressed, write the value of String messageFromCentral (defined in chatBLE_peripherique)
        stringCharacteristic.writeValue("Hello, this is a message from central");
      } else {
        Serial.println("button released = message sent");

        // button boot is released
        stringCharacteristic.writeValue("end of message");
      }
    }
  }

  Serial.println("Peripheral disconnected");
}

