# Eiffel-Tower-Luminosity-Simulation
This project is an S7 project at CentraleSupélec
## Members :
* Nizar El Ghazal
* Abdennacer Badaoui
* Oussama Kharouiche
## Description :
The goal of the project is to design the lighting of an Eiffel Tower in Eiffel Tower in miniaturized Meccano. This includes the lighting that sweeps 360° at the top of the tower, as well as and the flickering light that can be seen in the evening in Paris. And to be able to control it with our smartphones.

## folders and files :
### app (Oussama Kharouiche): 
the application was developed using reactjs. it consists of a single page composed of an input text to enter the IP address (it is the same as the IP adress of the laptop were the srvhttp code is running) of the server and buttons that allows to make http post requests.
* Note : The microcontroller, the srvhttp and the application must be on the same network.

### LEDs (Abdennacer Badaoui):
Contains the code to reproduce the flickering effect of the Eiffel Tower. 

### Moteur (Oussama Kharouiche):
Contains the code to reproduce the top lighting of the Eiffel tower.

### source (Nizar El Ghazal): 
the code to establish the communication between the microcontroller and the http server and to switch the operating modes according to the received messages.

### srvhttp.py (Nizar EL GHAZAL):
An HTTP server has been programmed in python, it is an interface that will receive a message from the application indicating which mode to use (including off) and store this mode in memory. Then, the server responds to the message from the card by transmitting the new mode.
