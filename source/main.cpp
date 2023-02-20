/* Sockets Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"
#include "rtos.h"
#include "stdio.h"
#include "Stepper.h"
#include <cstddef>
#include <type_traits>


#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET



class SocketDemo {
    // Classe qui s'occupe de toute la partie communication
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 256;
    const char hostname[64] = "192.168.166.59"; // mettre ici l'adresse IP du serveur.
    #if MBED_CONF_APP_USE_TLS_SOCKET
        static constexpr size_t REMOTE_PORT = 443; // tls port
    #else
        static constexpr size_t REMOTE_PORT = 8000; // standard HTTP port, mettre ici me port du serveur
    #endif // MBED_CONF_APP_USE_TLS_SOCKET
    
    public:
        SocketDemo() : _net(NetworkInterface::get_default_instance())
        {
        }

        ~SocketDemo()
        {
            if (_net) {
                _net->disconnect();
            }
        }

        void run()
        //Methode run qui se connecte au réseau wifi et envoie une fois une message HTTP au serveur et attend la réponse
        {   
            nsapi_size_or_error_t result;
            printf("\r\nwifi connected : %d\r\n",wifi_connected);
            if (wifi_connected==0){
                
                if (!_net) {
                    printf("Error! No network interface found.\r\n");
                    return;
                }

                /* if we're using a wifi interface run a quick scan */
                if (_net->wifiInterface()) {
                    /* the scan is not required to connect and only serves to show visible access points */
                    wifi_scan();

                    /* in this example we use credentials configured at compile time which are used by
                    * NetworkInterface::connect() but it's possible to do this at runtime by using the
                    * WiFiInterface::connect() which takes these parameters as arguments */
                }

                /* connect will perform the action appropriate to the interface type to connect to the network */

                printf("Connecting to the network...\r\n");

                result = _net->connect();
                if (result != 0) {
                    // printf("Error! _net->connect() returned: %d\r\n", result);
                    return;
                }

                // print_network_info();
                wifi_connected = 1;
            }
            /* opening the socket only allocates resources */
            result = _socket.open(_net);
            if (result != 0) {
                printf("Error! _socket.open() returned: %d\r\n", result);
                _net->disconnect();
                _socket.close();
                wifi_connected = 0;
                return;
            }

    #if MBED_CONF_APP_USE_TLS_SOCKET
            result = _socket.set_root_ca_cert(root_ca_cert);
            if (result != NSAPI_ERROR_OK) {
                printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
                return;
            }
            _socket.set_hostqcname(MBED_CONF_APP_HOSTNAME);
    #endif // MBED_CONF_APP_USE_TLS_SOCKET

            /* now we have to find where to connect */

            SocketAddress address;

            if (!resolve_hostname(address)) {
                return;
            }

            address.set_port(REMOTE_PORT);

            /* we are connected to the network but since we're using a connection oriented
            * protocol we still need to open a connection on the socket */
            /*printf("Opening connection to remote port %d\r\n", REMOTE_PORT);*/

        
            result = _socket.connect(address);
            if (result != 0) {
                printf("Error! _socket.connect() returned: %d\r\n", result);

                return;
            }

            /* exchange an HTTP request and response */
            
            if (!send_http_request()) {
                return;
            }

            if (!receive_http_response()) {
                return;
            }
            _socket.close();
        }
    public:
        int newmode=0;
        int changed_mode=0;

    private:
        bool resolve_hostname(SocketAddress &address)
        {

            /* get the host address */
            // printf("\nResolve hostname %s\r\n", hostname);
            nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
            if (result != 0) {
                printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
                return false;
            }

            // printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

            return true;
        }

        bool send_http_request()
        {   
            /* loop until whole request sent */
            char sbuffer[256];
            char message[64];
            sprintf(message, "{mode?}");
            printf("sending http request, Content Length = %d\r\n", (int)strlen(message));
            // Send a simple http request
            sprintf(sbuffer, "GET / HTTP/1.1\r\nHost: 192.168.150.59\r\nContentType: application/json\r\nContent-Length: %d\r\n\r\n%s",(int)strlen(message),message);

            /* printf("Sending HTTP request ...\n");// */    

            nsapi_size_t bytes_to_send = strlen(sbuffer);
            nsapi_size_or_error_t bytes_sent = 0;

            // printf("Sending message: \r\n%s\r\n", sbuffer);

            while (bytes_to_send) {
                bytes_sent = _socket.send(sbuffer + bytes_sent, bytes_to_send);
                if (bytes_sent < 0) {
                    printf("Error! _socket.send() returned: %d\r\n", bytes_sent);
                    return false;
                } else {
                    // printf("sent %d bytes\r\n", bytes_sent);
                }

                bytes_to_send -= bytes_sent;
            }

            printf("Complete message sent\r\n");

            return true;
        }

        bool receive_http_response()
        {
            char rbuffer[MAX_MESSAGE_RECEIVED_LENGTH];
            int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
            int received_bytes = 0;

            /* loop until there is nothing received or we've ran out of buffer space */
            nsapi_size_or_error_t result = remaining_bytes;
            while (result > 0 && remaining_bytes > 0) {
                result = _socket.recv(rbuffer + received_bytes, remaining_bytes);
                if (result < 0) {
                    printf("Error! _socket.recv() returned: %d\r\n", result);
                    return false;
                }

                received_bytes += result;
                remaining_bytes -= result;
            }


            /*printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(rbuffer, "\n") - rbuffer, rbuffer); */
            // printf("response :\r\n%s\r\n", rbuffer);
            int rbuffer_len = (int)strlen(rbuffer);
            // printf("len : %d\r\n", rbuffer_len);
            int requested_mode = rbuffer[rbuffer_len-1]-48;         
            printf("\r\nnew mode : %d\r\n", requested_mode);
            if (newmode != requested_mode){ //stock le nouveau mode en variable emporaire et ajoute une variable indiquant que le code est different
                newmode = requested_mode;
                changed_mode = 1;
                // printf("changed_mode: %d\r\n",changed_mode);
            }

            return true;
        }

        void wifi_scan()
        {
            WiFiInterface *wifi = _net->wifiInterface();

            WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

            /* scan call returns number of access points found */
            int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

            if (result <= 0) {
                // printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
                return;
            }

            // printf("%d networks available:\r\n", result);

            for (int i = 0; i < result; i++) {
                // printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
                //     ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                //     ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                //     ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                //     ap[i].get_rssi(), ap[i].get_channel());
            }
            // printf("\r\n");
        }

        void print_network_info()
        {
            /* print the network info */
            SocketAddress a;
            _net->get_ip_address(&a);
            printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
            _net->get_netmask(&a);
            printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
            _net->get_gateway(&a);
            printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        }

    private:
        NetworkInterface *_net;

    #if MBED_CONF_APP_USE_TLS_SOCKET
        TLSSocket _socket;
    #else
        TCPSocket _socket;
    #endif // MBED_CONF_APP_USE_TLS_SOCKET
        int wifi_connected=0;
};


Thread thread;
Semaphore sem(1);
int program_mode = 1;
SocketDemo *example = new SocketDemo();

void server_connect(){
    while(1){
        example->run();
        if (example->changed_mode == 1){
            sem.acquire(); // demande une permission avant d'assigner program_mode
            program_mode = example->newmode ;
            sem.release(); // libère la variable program_mode pour qu'elle puisse être utilisée par la boucle main
            example->changed_mode=0;
        }
        ThisThread::sleep_for(5s);
    }
}



#define wait_TIME 0.03
#define wait_TIME_1 0.03
// stepper(PinName _en, PinName ms1, PinName ms2, PinName ms3, PinName _stepPin, PinName dir);
stepper moteur_1(D15, D4, D5, D6, D7, D8);
stepper moteur_2(D15, D4, D5, D6, D9, D8);
stepper moteur_3(D15, D4, D5, D6, D10, D11);
stepper moteur_4(D15, D4, D5, D6, D12, D11);

// ms1, ms2, ms3 sont trois variables qui controle le ration du step à effectuer.
// _en : enabling signal
// _stepPin : la rotation du moteur en guidant ses _pas.
// dir : sens de rotation

int dir_1 = 1; //0: Sens trigo | 1: Sens horaire
int dir_2 = 0;
int microstep = 0; //Pas de déplacement
int step_count = 0; //Nombre de pas effectués
int number_of_steps = 50*2^microstep; //Nombre de pas à effectuer
int step_to_initial_position = 35; //Angle à se déplacer une fois détecté intialement
int permanent = false; //Régime permanent actif
int stop = false; //Arrét du fonctionnement


AnalogIn capteur(A1);
DigitalOut alim_capteur(D0);
DigitalOut led(LED1);
DigitalOut lamp_1(D1);
DigitalOut lamp_2(D2);
DigitalOut lamp_3(D3);
DigitalOut lamp_4(D15);




int main() {

    printf("program mode : %d \r\n",program_mode);
    #ifdef MBED_CONF_MBED_TRACE_ENABLE
        mbed_trace_init();
    #endif

    MBED_ASSERT(example);
    thread.start(server_connect);
    led = 1;

    while(1){
        
        if (program_mode==2)
        {
            lamp_1 = 0;
            lamp_2 = 0;
            lamp_3 = 0;
            lamp_4 = 0;
            
            //------------Déplacement initial des moteurs dans l'ordre 1 - 2 - 3 - 4------------
            step_count = 0;
            alim_capteur = 1;
            //------------------MOTEUR 1------------------
            while (abs(capteur) > 0.5f) {
            //Envoi des signaux
                moteur_1.step(0, 0, 1/wait_TIME_1);
                wait_us(100000);
            }
            // printf("moteur 1 capt\r\n");
            //Seuil détecté
            while (step_count < step_to_initial_position) //Déplacement é la position initiale
            {
                step_count++;
                //Envoi des signaux
                moteur_1.step(0, 1, 1/wait_TIME_1);
                wait_us(100000);
            }
            // printf("moteur 1 sync\r\n");
            step_count=0;
            // if (sync_count==0){
            //     if (capteur_det==0){
            //         //Envoi des signaux pour remettre le moteur en position initial
            //         moteur_1.step(0, 0, 1/wait_TIME);// step(microstep, direction, speed)
            //         wait_us(200000);
            //         if (abs(capteur) > 0.5f){
            //             capteur_det=1;
            //         }
            //     }
            //     //Seuil détecté
            //     if (capteur_det==1) //Déplacement à la position initiale
            //     {
            //         step_count++;
            //         //Envoi des signaux
            //         moteur_1.step(0, 1, 1/wait_TIME);
            //         wait_us(200000);
            //         if (step_count >= step_to_initial_position){
            //             sync_count++;
            //             step_count=0;
            //             capteur_det=0;
            //         }
            //     }
            // }
            //------------------MOTEUR 2------------------
            // if (sync_count==1){
            //     if (capteur_det==0){
            //         //Envoi des signaux pour remettre le moteur en position initial
            //         moteur_1.step(0, 0, 1/wait_TIME);// step(microstep, direction, speed)
            //         wait_us(200000);
            //         if (abs(capteur) > 0.5f){
            //             capteur_det=1;
            //         }
            //     }
            //     //Seuil détecté
            //     if (capteur_det==1) //Déplacement à la position initiale
            //     {
            //         step_count++;
            //         //Envoi des signaux
            //         moteur_1.step(0, 1, 1/wait_TIME);
            //         wait_us(200000);
            //         if (step_count >= step_to_initial_position){
            //             sync_count++;
            //             step_count=0;
            //             capteur_det=0;
            //         }
            //     }
            // }
            while (abs(capteur) > 0.5f) {
            //Envoi des signaux
                moteur_2.step(0, 0, 1/wait_TIME_1);
                wait_us(100000);
            }
            // printf("moteur 2 capt\r\n");
            //Seuil détecté
            while (step_count < step_to_initial_position) //Déplacement é la position initiale
            {
                step_count++;
            //Envoi des signaux
                moteur_2.step(0, 1, 1/wait_TIME_1);
                wait_us(100000);
            }
            // printf("moteur 2 sync\r\n");
            step_count = 0;
            //------------------MOTEUR 3------------------
            // if (sync_count==2){
            //     if (capteur_det==0){
            //         //Envoi des signaux pour remettre le moteur en position initial
            //         moteur_1.step(0, 0, 1/wait_TIME);// step(microstep, direction, speed)
            //         wait_us(200000);
            //         if (abs(capteur) > 0.5f){
            //             capteur_det=1;
            //         }
            //     }
            //     //Seuil détecté
            //     if (capteur_det==1) //Déplacement à la position initiale
            //     {
            //         step_count++;
            //         //Envoi des signaux
            //         moteur_1.step(0, 1, 1/wait_TIME);
            //         wait_us(200000);
            //         if (step_count >= step_to_initial_position){
            //             sync_count++;
            //             step_count=0;
            //             capteur_det=0;
            //         }
            //     }
            // }
            // while (abs(capteur) > 0.5f) {
            // //Envoi des signaux
            //     moteur_3.step(0, 1, 1/wait_TIME);
            //     wait_us(200000);
            // }
            // //Seuil détecté
            // while (step_count < step_to_initial_position) //Déplacement é la position initiale
            // {
            //     step_count++;
            //     //Envoi des signaux
            //     moteur_3.step(0, 0, 1/wait_TIME);
            //     wait_us(200000);
            // }
            // step_count = 0;
            //------------------MOTEUR 4------------------
            // if (sync_count==3){
            //     if (capteur_det==0){
            //         //Envoi des signaux pour remettre le moteur en position initial
            //         moteur_1.step(0, 0, 1/wait_TIME);// step(microstep, direction, speed)
            //         wait_us(200000);
            //         if (abs(capteur) > 0.5f){
            //             capteur_det=1;
            //         }
            //     }
            //     //Seuil détecté
            //     if (capteur_det==1) //Déplacement à la position initiale
            //     {
            //         step_count++;
            //         //Envoi des signaux
            //         moteur_1.step(0, 1, 1/wait_TIME);
            //         wait_us(200000);
            //         if (step_count >= step_to_initial_position){
            //             sync_count++;
            //             step_count=0;
            //             capteur_det=0;
            //         }
            //     }
            // }
            // if (sync_count==4) {
            //     program_mode=3;
            //     sync_count = 0;
            // }
            // while (abs(capteur) > 0.5f) {
            // //Envoi des signaux
            //     moteur_4.step(0, 1, 1/wait_TIME);
            //     wait_us(150000);
            // }
            // // printf("moteur 4 capt\r\n");
            // //Seuil détecté
            // while (step_count < step_to_initial_position) //Déplacement é la position initiale
            // {
            //     step_count++;
            //     //Envoi des signaux
            //     moteur_4.step(0, 0, 1/wait_TIME);
            //     wait_us(150000);
            // }
            // printf("moteur 4 sync\r\n");
            step_count = 0;
            // printf("syncro fini");
            if (program_mode==2){
                program_mode = 3;
            }
            
            permanent = true;
            led = 0;
            dir_1 = 1;
            dir_2 = 0;

        }

        else if (program_mode == 3){
            if (permanent){
                //------------------Régime permanent---------------
                //Rotation de 90é dans les 2 sens
                if (step_count < number_of_steps){
                    step_count++;
                }
                else{
                step_count = 0;
                if (dir_1 == 0){dir_1 = 1;}
                else {dir_1 = 0;}
                if (dir_2 == 0){dir_2 = 1;}
                else {dir_2 = 0;}
                }
                //Active les lampes
                lamp_1 = !dir_1;
                lamp_2 = !dir_1;
                lamp_4 = !dir_2;
                lamp_3 = !dir_2;
                //Arrét d'urgence

                //Envoi des signaux
                moteur_1.step(0, dir_1, 1/wait_TIME);
                moteur_2.step(0, dir_1, 1/wait_TIME);
                // moteur_3.step(0, dir_2, 1/wait_TIME);
                // moteur_4.step(0, dir_2, 1/wait_TIME);

                if (not(abs(capteur) > 0.5f)) {
                    led = 1;
                    stop = true;
                    lamp_1 = 0;
                    lamp_2 = 0;
                    lamp_3 = 0;
                    lamp_4 = 0;
                }
                if (stop){
                    program_mode=1;
                    stop=false;
                    permanent = false;
                    step_count = 0;
                    dir_1 = 1;
                    dir_2 = 0;
                }
            }
        }
        else if (program_mode==4){
            lamp_1 = !dir_1;
            lamp_2 = !dir_1;
            lamp_4 = !dir_2;
            lamp_3 = !dir_2;
        }
        else {
            lamp_1 = 0;
            lamp_2 = 0;
            lamp_4 = 0;
            lamp_3 = 0;    
        }
    }   
}
