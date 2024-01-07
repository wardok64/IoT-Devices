#ifndef End_Device_h
#define End_Device_h

#define Pin_control_Sleep_Xbee 10 // establecemos el pin 10 como el pin SleepXbee
#define AssociatePIN 8
#define VBATT_PIN 9
#define ADC_BITS 12


#include <Arduino.h>

class End_Device
{
  public:
  
     End_Device();// contructor de la clase
     
     void begin();

     void wake_up_xbee();

     void sleep_xbee();

     void reset_counter(int val);
     
     float readVBatt();

     int contador();

     void setConversion(String elemento, String indice, float data);
     
     byte getConversion(int indice_get);
     
     private:

     //variables
     float V_Batt,_data;
     boolean estadoAct, estadoAnt;
     int cont;
     int _val, _indice_get;
     String _data_string, _indice, _elemento;
     char _payload_data[8];
     byte _payloadData[8];
};

#endif
