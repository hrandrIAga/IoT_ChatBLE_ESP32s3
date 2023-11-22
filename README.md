# **IoT project : Building a BLE chat with ESP32s3 boards**

## *Introduction*

I carried out this ~15h projects as part of the course [NET4104: Wireless internet concepts, technologies and architectures](https://enseignements.telecom-sudparis.eu/fiche.php?c=NET4104) (Télécom SudParis master 1 Level)

## *Multi-hop BLE chat*

This program connects 3 Esp32S3 in BLE, and establishes a chat service between them.

The program uses the serial port of each user. 
It first asks the user to enter their userrname for the chat (to make other human user able to know who's using the board). 
The user can then send messages (and choose the receiver) and receive messages from other connected users.

The special feature of the **use case allowing the user to choose the receiver** has made it possible to implement the "multi-hop" functionality within this network consisting of 3 esp32s3.

## Repository content

* Draft folder : contains the various stages of the chat service
* main_ESP32s3_A.ino : code to *upload* on a first ESP32s3 (called ESP32s3_A)
* main_ESP32s3_R.ino : code to *upload* on a second ESP32s3 (called ESP32s3_B)
* main_ESP32s3_B.ino : code to *upload* on a third ESP32s3 (called ESP32s3_R)

## *Environment to implement the system*

IDE chosen: Arduino IDE ([local](https://docs.arduino.cc/software/ide-v2)) or Arduino cloud ([online](https://cloud.arduino.cc/))

Using the [ArduinoBLE] library (https://www.arduino.cc/reference/en/libraries/arduinoble/) makes programming Bluetooth with Arduino very easy.

> To build this network, you need : 
* 3 ESP32s3s
* 3 USB ports for serial connection to the microcontrollers
* use one or other of the IDEs mentioned above to implement the code corresponding to each ESP32s3.

## *Implementing and using the system*
1/ install Arduino IDE or Arduino cloud  
2/ upload \main_ESP32s3_R.ino on one serial-connected board (called R)   
3/ upload \main_ESP32s3_A.ino on a second serial-connected board (called A)  
4/ upload \main_ESP32s3_B.ino on a third serial-connected board (called B)  
5/ launch (or click on reset button) the board R  
6/ wait for 30 seconds  
7/ Launch (or click on reset reset buttons) A and B  
8/ type your username (=pseudoname) in less than 10 seconds  
9/ type the local name ('r','a', or 'b') of the board you want to chat with  
10/ type the message you want to send  
11/ the received messages are displayed on the terminal  
12/ type new messages you want to send when it's your turn  
13/ ...repeat...  

## *Network system conception*

* Each ESP32s3 can act as a Peripheral or a Central in a BLE connection.
* The Central can modify Peripheral's characteristic (a 1024bytes string)
> The system is a succession of 1 to 1 BLE connection  (as represented on the gif below)
> each device is in Central mode for 30s (in which, it can read and display messages), then switch to Peripheral mode for 1 minute (in which it waits messages to be transmitted by other devices)  before switching to Central for 30s...  
> when a device is in central mode, it checks if its characteristic as been written while it was a peripheral : 
* if its characteristic has been written and if the current device is the desired receiver --> characteristic is displayed on the terminal and the human user can type a new message, whose value will be written on the current Peripheral's characteristic to be transmitted
* if its characteristic has been written and if desired receiver is another device, the message (=characteristic) is not displayed on the terminal, but the current central device transmits automatically the value of its characteristic to the peripheral by writing the peripheral's characteristic. In other terms the central device is used as a relay
* if its characteristic has not been written, no messages has been sent to the current device (central), so there's no new message to be displayed on terminal. The human user can type a new message, whose value will be written on the current Peripheral's characteristic to be transmitted

## *1to1 BLE connection succession*
![1to1 BLE connection succession](https://github.com/hrandrIAga/IoT_ChatBLE_ESP32s3/blob/main/Gif_ChatBLE.gif) 