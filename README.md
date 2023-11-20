# **IoT project : Building a BLE chat with ESP32s3 boards**

## *Introduction*

I carried out this ~15h projects as part of the course [NET4104: Wireless internet concepts, technologies and architectures](https://enseignements.telecom-sudparis.eu/fiche.php?c=NET4104) (Télécom SudParis master 1 Level)

## *Multi-hop BLE chat*

This program connects 3 Esp32S3 in BLE, and establishes a chat between them.

The program uses the serial port of each user. 
It first asks the user to enter their name for the chat. 
The user can then enter messages (choosing the receiver) and receive messages from other connected users.

The special feature of the **use case allowing the user to choose the receiver** has made it possible to implement the "multi-hop" functionality within this network consisting of 3 esp32s3.

## Composition tu repo Github

* Draft folder : contains the various stages and more/less functional tests as part of the project
* Report_NET4104_chatBLE.pdf : project report with explanations of its design and code
* main_ESP32s3_A.ino : code to *upload* on a first ESP32s3 (called ESP32s3_A)
* main_ESP32s3_R.ino : code to *upload* on a second ESP32s3 (called ESP32s3_B)
* main_ESP32s3_B.ino : code to *upload* on a third ESP32s3 (called ESP32s3_R)


# Arborescence du dossier

├─ [Draft](Draft)
├─ [main_ESP32s3_A.ino](main_ESP32s3_A.ino)
├─ [main_ESP32s3_B.ino](main_ESP32s3_B.ino)
├─ [main_ESP32s3_R.ino](main_ESP32s3_R.ino)
├─ [Report.pdf](Report.pdf)
└─ [README.md](README.md)
 │    ├─ [Unidirectional_Chat](Draft/Unidirectional_Chat)
 │   │   ├─ [Central_UnidirectionalChat.ino](Draft/Unidirectional_Chat/Central_UnidirectionalChat.ino)
  │  │   ├─ [Peripheral_UnidirectionalChat.ino](Draft/Unidirectional_Chat/Peripheral_UnidirectionalChat.ino)
  │  │   └─ [README_UnidirectionalChat.md](Draft/Unidirectional_Chat/README_UnidirectionalChat.md)
  │  ├─ [Bidirectional_Chat](Draft/Bidirectional_Chat)
  │  │   ├─ [ESP32_A.ino](Draft/Bidirectional_Chat/ESP32_A.ino)
  │  │   ├─ [ESP32_B.ino](Draft/Bidirectional_Chat/ESP32_B.ino)
  │  │   ├─ [ESP32_Relay.ino](Draft/Bidirectional_Chat/ESP32_Relay.ino)
 │   │   └─ [README_BidirectionalChat.md](Draft/Bidirectional_Chat/README_BidirectionalChat.md)
  │  └─ [ChatBLE_DestinationChoice](Draft/ChatBLE_DestinationChoice)
│	│        ├─ [ESP32s3_A.ino](Draft/ChatBLE_DestinationChoice/ESP32s3_A.ino)
│	│        ├─ [ESP32s3_B.ino](Draft/ChatBLE_DestinationChoice/ESP32s3_B.ino)
│	 │       ├─ [ESP32s3_Relay.ino](Draft/ChatBLE_DestinationChoice/ESP32s3_Relay.ino)
│	  │      └─ [README_DestinationChoice.md](Draft/ChatBLE_DestinationChoice/README_DestinationChoice.md)







## *System design*
*For more details on the design and implementation of the project, please refer to the document Rapport_NET4104_chatBLE.pdf*.

IDE chosen: Arduino IDE ([local](https://docs.arduino.cc/software/ide-v2)) or Arduino cloud ([online](https://cloud.arduino.cc/))

Using the [ArduinoBLE] library (https://www.arduino.cc/reference/en/libraries/arduinoble/) makes programming Bluetooth with Arduino very easy.

> To build this network, you need : 
* 3 ESP32s3s
* 3 USB ports for serial connection to the microcontrollers
* use one or other of the IDEs mentioned above to implement the code corresponding to each ESP32s3.

## *Implementing the system*
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




