/*
 Se utilizo SCT-019-000

  GND ---- R2 ---- R1 --- Vbatt
             |
             |
            Vbatt_adc = pin A9 XIAO

  R1 =  330 kOhm
  R2 =  330 kOhm
  ADC resolution is 3.3 volts / 4096 (12bits) units or, 0.0008056 volts (0.80 mV) per unit

  se alimenta con cuatro pilas doble A que dan 5.18 como Vout, por eso cambio el valor de las resistencias del divisor.

  Vbatt_adc = Vbatt * R2/(R1 + R2) * 0.00080
  Vbatt = Vbatt_adc * (R1 + R2)/R2 * 0.00080
  Vbatt = Vbatt_adc * (330 + 330)/330 * 0.00080
  Vbatt = Vbatt_adc * 2 * 0.00080
  Vbatt = Vbatt_adc * 0.0016 // en teoria
*/

#include "End_Device.h"
#include "semonlib.h"
#include <XBee.h> // incluimos la libreria de Xbee para comunicar mediante modo API

#define ADC_INPUT1 0 // current sensor pin 0
#define ADC_INPUT2 3 // current sensor pin 1 se daño pin 1
#define ADC_INPUT3 4 // current sensor pin 2 se daño pin 2

EnergyMonitor emon1; //, emon2, emon3;           // Create three instances

End_Device End_Device1, Corriente1;//, Corriente2, Corriente3; // para mandar 3 mensajes

XBee xbee = XBee(); // creamos un objeto xbee, e inicializamos la libreria

XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40E642D4);  // definimos la direccion a la
//cual nuestro Xbee conectado va a enviar los datos. lo almacenamos en addr64

void setup() {    
  //ICAL 127.65 para corriente 150A Rms con resistencia burden de 47ohms sct-019-000     
  //ICAL 169.29 (en practica -> 130.29) para corriente 200A Rms con resistencia burden de  35.8 (33ohms + 1.5 ohms + 1.5 ohms ) ohms SCT-019-000    
  emon1.current(ADC_INPUT1, 130.29);      // Current: input pin, calibration (SCT-013-000 - 100A-50mA) ICAL=60.6 con resistencia burden de 33ohms naranja-naranja-negro.
  //emon2.current(ADC_INPUT2, 127.65);      // Current: input pin, calibration (100A-50mA).
  //emon3.current(ADC_INPUT3, 127.65);      // Current: input pin, calibration (100A-50mA).

  End_Device1.begin();
  xbee.setSerial(Serial1);
  End_Device1.wake_up_xbee();// o mandar control sleep de xbee a tierra para siempre estar despierto
}

void loop() {
  byte  data1[8];//,data2[8], data3[8];
  float curr1;//, curr2, curr3;

  curr1 = emon1.calcIrms(1480);
  //curr2 = emon2.calcIrms(1480);
  //curr3 = emon3.calcIrms(1480);

  Corriente1.setConversion("M1", "U", curr1);
  for (int i = 0; i < 8; i++) 
  {
    data1[i] = Corriente1.getConversion(i);
  }
  /* 
  Corriente2.setConversion("M1", "V", curr2);
  for (int i = 0; i < 8; i++) 
  {
    data2[i] = Corriente2.getConversion(i);
  }
  
  Corriente3.setConversion("M1", "W", curr3);
  for (int i = 0; i < 8; i++) 
  {
    data3[i] = Corriente3.getConversion(i);
  }
  */
  ZBTxRequest zbTx1 = ZBTxRequest(addr64, data1, sizeof(data1)); // enviamos al coordinador
  xbee.send(zbTx1);
  delay(200);
  /*
  ZBTxRequest zbTx2 = ZBTxRequest(addr64, data2, sizeof(data2)); // enviamos al coordinador
  xbee.send(zbTx2);
  delay(200);
  ZBTxRequest zbTx3 = ZBTxRequest(addr64, data3, sizeof(data3)); // enviamos al coordinador
  xbee.send(zbTx3);
  delay(200);
  */ 
  delay(2000);
}
