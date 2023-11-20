# IoT_ChatBLE_ESP32s3
This program connects 3 Esp32S3 in BLE, and establishes a chat between them.  The program uses the serial port of each user.  It first asks the user to enter their name for the chat.  The user can then enter messages (choosing the receiver) and receive messages from other connected users.

## Step 1 = Unidirectional chat in ./draft
inspired by the **ArduinoBLE/Examples/Central/LedControl** and **ArduinoBLE/Examples/Peripheral/Led** in which the central modifies the value of the peripheral characteristic when its own reboot button is pressed --> the peripheral Led color changes

We start the project by building an unidirectional chat :
1/ connects both boards in serial
2/ the first board is used as a Central and connects to the peripheral (2nd board)
3/ when the central's reboot button is pressed and released, the characteristic (a string) of the peripheral takes the value of the message : "Hello, this is a message from central"
4/ the peripheral print his new characteristic

## Step 2 = Bidirectional chat thanks to a relay
improvement inspired **by step ./draft**
1/ upload ESP32_A.ino in one board, ESP32_r, in a second board, ESP32_B in a third board  
2/ connects all boards in serial  
3/ the board A starts as a central with the Board R as a Peripheral, the B starts as a Peripheral but doesn't have central yet  
4/ The user can write a message to send to B, by writing the board's R characteristic  
5/ 30s later, the board A switches to the peripheral state, while the board R switch to central with B as a peripheral  
6/ the board R transmits the message to B by writing B's characteristic with R's own characteristic value that was written by A at step 4  
7/ 30s later, the board B switches to the central state with A as a peripheral and R to the peripheral state. B's characteristic is print to display the message sent by A  
7bis/ then B user can write a message to send to B, by writing the board's A characteristic  
8/ 30s later, the board A switches to Central with R as a peripheral and B switch to Peripheral without any central yet. A'scharacteristic is print to display the message sent by B  and A is ready to write and send a new message again
9/ ...repeat...

In short, the state changes and abilities can be summed up in the following tables :

### i/ states ability

| State   |Abilities|
| ------- | ------- |
| CENTRAL | Print eventual message received + write in serial a message to send  |
| PERIPHERAL| Wait for its characteristic to be written + the board's characteristic is written by its central |

### ii/ states switches

|Timeslot Start |state of Board A|State of Board Relay| State of Board B|
| -------| -------        | -------            | -------         |
| 0s     | Central of Relay | Peripheral of A  | Peripheral scanning|
| +30s     | Peripheral scanning | Central of B  | Peripheral of Relay|
| +60s     | Peripheral of B | Peripheral scanning | Central of A|
| +90s     | Central of Relay | Peripheral of A  | Peripheral scanning|
| ...| repeat | repeat | repeat |