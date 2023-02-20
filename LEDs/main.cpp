#include "mbed.h"
#include "PololuLedStrip.h"
// #include "stdlib.h"

PololuLedStrip ledStrip1(D8); 
PololuLedStrip ledStrip2(D9); 

// Nombre de LEDs (nécessairement un multiple de 8 suppérieur au nombre de LEDs réel)
#define LED_COUNT 304
// Période globale :
#define T_tot  220

// Tic coefficient :
int tic_coef = 4;

#COPIER COLLER ICI LA MATRICE D'ALLUMAGE (Matrice_LEDs) QUE RENVOIE LE CODE PYTHON 

rgb_color colors[LED_COUNT];

Timer timer;

unsigned long int rstate = 0;

unsigned int myrand()
{
    rstate = (214013*rstate + 2531011)  ;
    return(rstate/65536) ;
}

int main()
{
    timer.start();
    
    // Compteur :
    int ind,ind2=0;     // indices de comptage
    uint8_t v1=255 ;
    uint8_t v2=255 ;
    uint8_t v3=255 ;
    uint8_t vled ;   
    uint32_t time,tp ;  // pour la gestion du temps
    int program=3 ;    // no du programme en cours  
    int period=50 ;    // periode de rafraichissement (en ms)
    int i,j,k ;
    const long int A[4]={ 4096 , -10219 , 8263 , -2139 } ;  // tableau des coefficients pour générateur aléatoire
    const long int B[4]={ 204 , -393 ,  207 ,  -18 } ;  // tableau des coefficients pour générateur aléatoire
    long int vx[LED_COUNT][4]={0,0,0,0} ;      // valeur des entrées du filtre
    long int vy[LED_COUNT][4]={0,0,0,0} ;      // valeur des sorties du filtre  
    
    time=timer.read_ms() ;
    while(1)
    {
        
      // Timer            
      while ((tp=timer.read_ms())<(time+period)) ;  // attente de la periode
      time=tp ;
      program=(tp/50000)%4 ;    // changement r�gulier de programme
            
      if (program==0) {
        // Update the colors array.
        ind=(ind+1)%T_tot;               
        for(i = 0; i < LED_COUNT; i++)
        {
            vled=Matrice_LEDs[ind][i]*255 ;
            colors[i] = (rgb_color){vled, vled, vled};      
        }
      }
      else if (program==1) {
       
        ind=(ind+1)%(LED_COUNT-16) ;
        for(i = 0; i < LED_COUNT; i++)
           colors[i] = (rgb_color){0, 0, 0};   
        //for(i = ind; i < ind+16; i++)
           colors[ind] = (rgb_color){255, 255, 255};   
          
      }
      else if (program==3) {
       
        ind=(ind+1)%(LED_COUNT-16) ;
        ind2=(ind2+1)%768 ;
        for(i = 0; i < LED_COUNT; i++)
           colors[i] = (rgb_color){0, 0, 0};   

        if (ind2<256)
        { v1=ind2 ; v2=255-v1 ; v3=0; }
        else if (ind2<512)
        { v1=0 ; v2=ind2-256 ; v3=255-v2; }
        else
        { v2=0 ; v3=ind2-512 ; v1=255-v3 ; }

         colors[ind] = (rgb_color){v1, v2, v3};    
         colors[ind+1] = (rgb_color){v1, v2, v3};   
         colors[ind+2] = (rgb_color){v1, v2, v3};   
         colors[ind+3] = (rgb_color){v1, v2, v3};   
         colors[ind+4] = (rgb_color){v1, v2, v3};    
         colors[ind+5] = (rgb_color){v1, v2, v3};   
         colors[ind+6] = (rgb_color){v1, v2, v3};   
         colors[ind+7] = (rgb_color){v1, v2, v3};   
          
      }
      else if (program==2) {
        // Update the colors array.

        for(i = 0; i < LED_COUNT; i++) {
 
            vx[i][3]=vx[i][2] ; vx[i][2]=vx[i][1] ; vx[i][1]=vx[i][0] ;
            vy[i][3]=vy[i][2] ; vy[i][2]=vy[i][1] ; vy[i][1]=vy[i][0] ;
        
            vx[i][0]=myrand() ;
            vy[i][0]=B[0]*vx[i][0]+B[1]*vx[i][1]+B[2]*vx[i][2]+B[3]*vx[i][3] ;
            vy[i][0]=(vy[i][0]-A[1]*vy[i][1]-A[2]*vy[i][2]-A[3]*vy[i][3])/A[0] ;
 
            j=(int)vy[i][0];
            if (j<-1000)
              vled=255 ;
            else
              vled=0 ;             
              
            colors[i] = (rgb_color){vled, vled, vled};   
        }
      }  
        
        // Send the colors to the LED strip.
        ledStrip1.write(colors, LED_COUNT);
        ledStrip2.write(colors, LED_COUNT);

    }
}