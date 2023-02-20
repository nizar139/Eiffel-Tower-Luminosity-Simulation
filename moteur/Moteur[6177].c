#include "mbed.h"
#include "Stepper.h"

#define WAIT_TIME 0.03
// stepper(PinName _en, PinName ms1, PinName ms2, PinName ms3, PinName _stepPin, PinName dir);
stepper moteur_1(D15, D4, D5, D6, D7, D8);
stepper moteur_2(D15, D4, D5, D6, D9, D8);
stepper moteur_3(D15, D4, D5, D6, D10, D11);
stepper moteur_4(D15, D4, D5, D6, D12, D11);

// ms1, ms2, ms3 sont trois variables qui controle le ration du step à effectuer.
// _en : enabling signal
// _stepPin : la rotation du moteur en guidant ses _pas.
// dir : sens de rotation

int dir_1 = 0; //0: Sens trigo | 1: Sens horaire
int dir_2 = 1;
int microstep = 0; //Pas de déplacement
int step_count = 0; //Nombre de pas effectués
int number_of_steps = 50*2^microstep; //Nombre de pas à effectuer
int step_to_initial_position = 25; //Angle à se déplacer une fois détecté intialement
int permanent = false; //Régime permanent actif
int stop = false; //Arrét du fonctionnement

AnalogIn capteur(A1);
DigitalOut alim_capteur(D0);
DigitalOut led(LED1);
DigitalOut lamp_1(D1);
DigitalOut lamp_2(D2);
DigitalOut lamp_3(D3);
DigitalOut lamp_4(D15);

int main(){
	//------------Déplacement initial des moteurs dans l'ordre 1 - 2 - 3 - 4------------
	alim_capteur = 1;
	//------------------MOTEUR 1------------------
	while (abs(capteur) > 0.5f) {
	//Envoi des signaux pour remettre le moteur en position initial
		moteur_1.step(0, 0, 1/WAIT_TIME);// step(microstep, direction, speed)
		wait(0.2);
	}
	//Seuil détecté
	while (step_count < step_to_initial_position) //Déplacement à la position initiale
	{
		step_count++;
		//Envoi des signaux
		moteur_1.step(0, 1, 1/WAIT_TIME);
		wait(0.2);
	}
	step_count = 0;
	//------------------MOTEUR 2------------------
	while (abs(capteur) > 0.5f) {
	//Envoi des signaux
		moteur_2.step(0, 0, 1/WAIT_TIME);
		wait(0.2);
	}
	//Seuil détecté
	while (step_count < step_to_initial_position) //Déplacement é la position initiale
	{
		step_count++;
	//Envoi des signaux
		moteur_2.step(0, 1, 1/WAIT_TIME);
		wait(0.2);
	}
	step_count = 0;
	//------------------MOTEUR 3------------------
	while (abs(capteur) > 0.5f) {
	//Envoi des signaux
		moteur_3.step(0, 1, 1/WAIT_TIME);
		wait(0.2);
	}
	//Seuil détecté
	while (step_count < step_to_initial_position) //Déplacement é la position initiale
	{
		step_count++;
		//Envoi des signaux
		moteur_3.step(0, 0, 1/WAIT_TIME);
		wait(0.2);
	}
	step_count = 0;
	//------------------MOTEUR 4------------------
	while (abs(capteur) > 0.5f) {
	//Envoi des signaux
		moteur_4.step(0, 1, 1/WAIT_TIME);
		wait(0.2);
	}
	//Seuil détecté
	while (step_count < step_to_initial_position) //Déplacement é la position initiale
	{
		step_count++;
		//Envoi des signaux
		moteur_4.step(0, 0, 1/WAIT_TIME);
		wait(0.2);
	}
	//------------------Régime permanent---------------
	step_count = 0;
	dir_1 = 1;
	dir_2 = 0;
	led = 0;
	while (stop == false){
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
	led = dir_2;
	//Active les lampes
	lamp_1 = !dir_1;
	lamp_2 = !dir_1;
	lamp_4 = !dir_2;
	lamp_3 = !dir_2;
	//Arrét d'urgence
	if (permanent and not(abs(capteur) > 0.5f)) {
		led = 1;
		stop = true;
		lamp_1 = 0;
		lamp_2 = 0;
		lamp_3 = 0;
		lamp_4 = 0;
	}
	//Envoi des signaux
	moteur_1.step(0, dir_1, 1/WAIT_TIME);
	moteur_2.step(0, dir_1, 1/WAIT_TIME);
	moteur_3.step(0, dir_2, 1/WAIT_TIME);
	moteur_4.step(0, dir_2, 1/WAIT_TIME);
	permanent = true;
	}
}
