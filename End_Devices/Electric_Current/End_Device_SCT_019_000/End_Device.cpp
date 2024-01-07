#include "Arduino.h"
#include "End_Device.h"



End_Device::End_Device(){ }

void End_Device::begin(){
  
  analogReadResolution(ADC_BITS);
  Serial1.begin(9600); 
  pinMode(AssociatePIN, INPUT);//pin associate led join network
  pinMode(Pin_control_Sleep_Xbee, OUTPUT); // establecemos al pin de control de hibernacion en modo salida
  
}

void End_Device::wake_up_xbee(){
    digitalWrite(Pin_control_Sleep_Xbee, 0);              
}

void End_Device::sleep_xbee(){
    digitalWrite(Pin_control_Sleep_Xbee, 1);              
}
int End_Device::contador(){
   estadoAct = digitalRead(AssociatePIN);
   if(estadoAct != estadoAnt){
        cont++;
            }
   estadoAnt = estadoAct;
   return cont;
}
void End_Device::reset_counter(int val){
  _val = val;
  cont = _val;
}

void End_Device::setConversion(String elemento, String indice, float data){
  _elemento = elemento;
  _indice = indice;
  _data = data;
  _data_string = String(_data);
  _data_string = _elemento + _indice + _data_string;
  _data_string.toCharArray(_payload_data, sizeof(_payload_data));
  for(int i=0;i<sizeof(_payload_data);i++){
    _payloadData[i] = _payload_data[i];//almacenamos nuestro arreglo tipi char en nuestro arreglo byte. byte x byte
    }
}
byte End_Device::getConversion(int indice_get){
  _indice_get = indice_get;
  return _payloadData[indice_get];
}

float End_Device::readVBatt(){
  V_Batt = analogRead(VBATT_PIN)*0.001583;
  return V_Batt;
}
