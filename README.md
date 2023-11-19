# IoT_ChatBLE_ESP32s3
This program connects 3 Esp32S3 in BLE, and establishes a chat between them.  The program uses the serial port of each user.  It first asks the user to enter their name for the chat.  The user can then enter messages (choosing the receiver) and receive messages from other connected users.

## Step 1
inspired by the **ArduinoBLE/Examples/Central/LedControl** and **ArduinoBLE/Examples/Peripheral/Led** in which the central modifies the value of the peripheral characteristic when its own reboot button is pressed --> the peripheral Led color changes

We start the project by building an unidirectional chat :
1/ connects both boards in serial
2/ the first board is used as a Central and connects to the peripheral (2nd board)
3/ when the central's reboot button is pressed and released, the characteristic (a string) of the peripheral takes the value of the message : "Hello, this is a message from central"
4/ the peripheral print his new characteristic