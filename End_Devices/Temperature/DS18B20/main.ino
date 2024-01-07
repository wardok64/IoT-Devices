/*
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
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTCZero.h>
#include <XBee.h> 

#define ONE_WIRE_BUS 3

End_Device End_Device1, End_Device2;
RTCZero rtc;

XBee xbee = XBee(); 
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40E642D4); 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

volatile bool matched = false; 

int alarmMinutes;

int frec_muestra_min = 30; // Every 30 minutes

void setup() {

  End_Device1.begin();
  xbee.setSerial(Serial1); 
  End_Device1.sleep_xbee();
  rtc.begin();
  rtc.setTime(12, 0, 0);
  rtc.setDate(12, 1, 0);
  rtc.setAlarmTime(12, 0, 10); // first wake up cycle 10 sec.
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.attachInterrupt(alarmMatch);
  rtc.standbyMode();
  sensors.begin();
}

void loop()
{

  if (matched)
  {
    
    byte data1[8], data2[8];
    static boolean bandera_reset_LWT = true;// flag to send once the LWT vBATT alarm 
    static boolean on_line = true;
    static boolean bandera_wakeup_sleep_lowbatt = false;
      
    boolean estadoAnt = false;
    boolean estadoAct = false;

    float batt = End_Device1.readVBatt();
      
    if(!bandera_wakeup_sleep_lowbatt)
    {    
      do
      { 
        End_Device1.wake_up_xbee();
        if(End_Device1.contador() > 10)
        { // ready to send API
          //if (batt >= (4.2)) { // it is correct
          sensors.requestTemperatures();
          float temp = sensors.getTempCByIndex(0);
          delay(100);

          while(temp < 0.0)
          { 
            sensors.requestTemperatures();
            temp = sensors.getTempCByIndex(0);
            delay(100);
          }
                      
          End_Device1.setConversion("M1","B", batt);
          End_Device2.setConversion("M2","T", temp);
      
          for(int i = 0; i < 8; i++)
          { 
            data1[i] = End_Device1.getConversion(i); 
            data2[i] = End_Device2.getConversion(i);
          }
          /*
          // Send the data to Coordinator
          if(on_line)
          {
            //we send once LWT
            byte LWT_[4] = {'D', '1', 'A', '0'};
            ZBTxRequest zbTx0 = ZBTxRequest(addr64, LWT_, sizeof(LWT_)); // send to coordinator
            xbee.send(zbTx0);
            delay(200); 
            //--------------------------------------------------------------------
            on_line = false;
          }*/
          ZBTxRequest zbTx = ZBTxRequest(addr64, data1, sizeof(data1)); // enviamos al coordinador 
          xbee.send(zbTx);
          delay(200);
          ZBTxRequest zbTx2 = ZBTxRequest(addr64, data2, sizeof(data2)); // enviamos al coordinador 
          xbee.send(zbTx2);
          delay(200);

          matched = false;                           
          End_Device1.reset_counter(0);                                     
        }
      } while(matched);
    } 
    /*
    // Flag reset  and set new alarm time to the next cycle
    bandera_reset_LWT = true; 
    bandera_wakeup_sleep_lowbatt = false;
    alarmMinutes = rtc.getMinutes(); // se crea una variable tipo entera, se obtiene los minutos actuales con get Minutes
    alarmMinutes += 5; // se le suman 5, segundo ciclo wake up 5 minutos
    if (alarmMinutes >= 60) 
    { // si sobre pasa los 60 minutos o es igual a 60, 
      alarmMinutes -= 60; // se convierte en 1 minuto, esto es si los minutos obtenidos son 59, y sumamos 1, son 60, entonces, se convierte en 1.
    }
    rtc.setAlarmTime(rtc.getHours(), alarmMinutes, rtc.getSeconds()); // reestablecemos la siguiente alarma, con el nuevo valor en minutos (cada min).                            
    //}*/
    if(batt < (4.2)) 
    { //** Si el voltaje de la bateria ya es bajo       
      if (bandera_reset_LWT)
      {
        //enviamos el mensaje LWT una unica vez-------------------------------------------------------------------
        byte LWT[4] = {'D', '1', 'A', '1'};
        ZBTxRequest zbTx3 = ZBTxRequest(addr64, LWT, sizeof(LWT)); // enviamos al coordinador 
        xbee.send(zbTx3);
        delay(200); 
        //--------------------------------------------------------------------
        bandera_wakeup_sleep_lowbatt = true;
        bandera_reset_LWT = false;
      }
      //**Reasignamos tiempo de alarma de seguridad (mayor tiempo), para el prosimo ciclo.                                     
      alarmMinutes = rtc.getMinutes(); // se crea una variable tipo entera, se obtiene los minutos actuales con get Minutes
      alarmMinutes += 59; // se le suman 59 minutos, ciclo wake up de alarma low power
      
      if (alarmMinutes >= 60) 
      { // si sobre pasa los 60 minutos o es igual a 60, 
        alarmMinutes -= 60; // se convierte en 5 minutos, esto es si los minutos obtenidos son 59, y sumamos 5, son 65, entonces, se convierte en 5.
      }
      rtc.setAlarmTime(rtc.getHours(), alarmMinutes, rtc.getSeconds()); // reestablecemos la siguiente alarma, con el nuevo valor en minutos (cada 5 min).                                                        
    }
    if (batt >= (4.2)) 
    { // si el voltaje de la bateria esta correcto
      bandera_reset_LWT = true; 
      bandera_wakeup_sleep_lowbatt = false;
      alarmMinutes = rtc.getMinutes(); // se crea una variable tipo entera, se obtiene los minutos actuales con get Minutes
      alarmMinutes += frec_muestra_min; // se le suma la variable de tiempo inicializada 
      if (alarmMinutes >= 60) 
      { // si sobre pasa los 60 minutos o es igual a 60, 
        alarmMinutes -= 60; // se convierte en n minutos siguientes, esto es si los minutos obtenidos son 59, y sumamos 1, son 60, entonces, se convierte en 1.
      }
      rtc.setAlarmTime(rtc.getHours(), alarmMinutes, rtc.getSeconds()); // reestablecemos la siguiente alarma, con el nuevo valor en minutos (cada valor de variable).                                   
    }
                                                  
    End_Device1.sleep_xbee(); // poner al Xbee en modo Sleep    
    rtc.standbyMode();    // Sleep until next alarm match // funcion para dormir al XIAO StandbyMODE  
  }       
}

void alarmMatch() 
{ // funcion de interrupcion que se activa cuando se cumple el ciclo de la alarma.
  matched = true; // convierte el valor de la bandera en true.
}
