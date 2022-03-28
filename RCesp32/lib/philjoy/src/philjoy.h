#include "Arduino.h"

                                                                          // Strapping Pins
                                                                          //     GPIO 0
                                                                          //     GPIO 2
                                        //affect booting                  //     GPIO 4
                                                                          //     GPIO 5 (must be HIGH during boot)
                                                                          //     GPIO 12 (must be LOW during boot)
                                                                          //     GPIO 15 (must be HIGH during boot)

                                      //ESP read pins
                                      //order:
                                      // elv,ail,rud,thrt
                                      //low pullup, high left, high left,high is high


static int P[4] = {33,25,27,26}; //                    
int dcal = 32;//calibrate pin
word joyval[4];
byte joybyte[4];
//GPIO34-39 can only be set as input mode and do not have software pullup or pulldown 
int swpins = 3;
//int sw[3] = {36,34,35};  //modesetting pin
int sw[3] = {14,36,34};  //modesetting pin   //                whats wrong with above????????????
bool swreadingmode = false;//set whilst reading

//cal[ch][0]   stick min
//cal[ch][1]   stick NEUTRAL set on startup :> need to start with sticks neutral 
//cal[ch][2]   stick max
//cal[ch][3]   stick TRIM = neutral 0 at start up, subsequent centerings  DELTA from neutral  
                    
static int cal[4][4]={
                      1600,2000,2370,0,
                      1470,1950,2410,0,
                      1590,2150,2530,0,
                      1605,2000,2420,0};

bool centering = 1;
int anRead( int pin){
    int adc; 
     adc = analogRead(pin); 
  return adc; 
}
int anReadavg( int pin){
        int sum = 0;
        for(int i =0; i<10;i++){
          sum += anRead( pin);
        }
        return (int) sum /10; 
}

//#########
byte JEncodebyte(int j); // 1000-2000 >> 0 -200 unsigned
word  JDecodeint(byte j);
int  JreadaxisAV(int pin);
void Jcenter(int j, int axis);
void Jtrim(int j ,int axis);
int  JRead(int axis);//normal joystic axis read with conversions ready to send

void Jcal();
void Jcenters();
bool JReads();//normal joystic axis read with conversions ready to send
bool J2bytes();

//JOY FUNCTIONS
//-------------helper functions
int JreadaxisAV(int pin){
  return anReadavg( pin);
}
int Jrangeit(int j ,int axis){
     j= map(j, cal[axis][0], cal[axis][2], 1000, 2000); //remember arrays start at 0 so [1] is middle
    return j;  
}
int JRead(int axis){//normal joystic axis read with conversions ready to send
  int j;
  j= JreadaxisAV(P[axis]);
  j= Jrangeit(j, axis);
  return j;  
}
void Jcenter( int j,int axis){
    cal[axis][1]= j; //setup CEN
    cal[axis][3]= 0; //setup TRIM
}
byte JEncodebyte(word j){ // 1000-2000 >> 0 -200 unsigned
  byte jb;
  jb =   (byte) (j - 1000) / 10;
  return  jb;
}
word JDecodeint(byte j){
  int ji;
  ji =   (int) j*  10 + 1000;
  return  ji;
}
//-------------call functions
void Jcal(){
    int j;
    for(int axis = 0; axis < 4;axis ++){
      j = JRead(axis);
      if(j < cal[axis][0]) cal[axis][0] = j; //min  //remember arrays start at 0 so [1] is middle
      if(j > cal[axis][2]) cal[axis][2] = j; //min  //remember arrays start at 0 so [1] is middle
    }
}
void Jcenters(){
    int j;
    for(int axis = 0; axis <4;axis ++){
      j = JRead(axis);
      Jcenter( j, axis) ;     
    }  
}
void Jtrims(){
    int j;
    for(int axis = 0; axis <4;axis ++){
      j = JRead(axis);
      cal[axis][3]= j - cal[axis][1]; //setup TRIM  AS DELTA +ve or -ve
    }
}

bool JReads(){//normal joystic axis read with conversions ready to send
    int j;
 //void readjoys(int *P,word *joyval){
    for(int axis = 0; axis <4;axis ++){
      j = JRead(axis);
      joyval[axis] = j + cal[axis][3];
      if(joyval[axis]< 1000) joyval[axis]=1000; //sanity check
      if(joyval[axis] >2000 ) joyval[axis]=2000; 
    }
    return true;  
}

bool J2bytes(){//normal joystic axis read with conversions ready to send
 //void readjoys(int *P,word *joyval){
    for(int axis = 0; axis <4;axis ++){
      joybyte[axis] =  joyval[axis];
    }
    return true;  
}

//-----------------------------------

//#########

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//READ MODE SW's
//READ MODE SW's
byte  readmodesws(int * sw ,int swpins){ //operates on sw1 sw2 sw3 switch
    bool z[3];
    byte swb = 0;
    if( !swreadingmode ){
        swreadingmode = true;
        for(int i=0;i< swpins;i++){
          z[i] = digitalRead(sw[i]);
        }
        delay(10);  //inefficient put in interrupt later or rtos    
    }
    if( swreadingmode ){
        swreadingmode = false;
        for(int i=0;i< swpins;i++){
          if( digitalRead(sw[i])== z[i]) z[i] = sw[i];  //phil you dill took 3 hours to spot this bug swapped  z for sw
        }
        //now pack into a byte/int
        swb = z[0]  |(z[1] << 1) |  (z[2] << 2) ;
    } 
for(int i=0;i< swpins;i++) {Serial.print("swAFTER   ");Serial.println(z[i]);}
    return swb;
}
byte  read3modesws(){ //operates on sw1 sw2 sw3 switch
    bool z[3];
    byte swb = 0;
        for(int i=0;i< swpins;i++){
          z[i] = digitalRead(sw[i]);
        }
        //now pack into a byte/int
        swb = z[0]  |(z[1] << 1) |  (z[2] << 2) ;
    
//for(int i=0;i< swpins;i++) {Serial.print("swAFTER   ");Serial.println(z[i]);}
    return swb;
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                                                                                                                          // #include "Arduino.h"


                                                                                                                          //                                     // Strapping Pins
                                                                                                                          //                                     //     GPIO 0
                                                                                                                          //                                     //     GPIO 2
                                                                                                                          //   //affect booting                  //     GPIO 4
                                                                                                                          //                                     //     GPIO 5 (must be HIGH during boot)
                                                                                                                          //                                     //     GPIO 12 (must be LOW during boot)
                                                                                                                          //                                     //     GPIO 15 (must be HIGH during boot)

                                                                                                                          // //ESP read pins

                                                                                                                          // //order:
                                                                                                                          // // elv,ail,rud,thrt
                                                                                                                          // //low pullup, high left, high left,high is high
                                                                                                                          // static int P[4] = {33,25,27,26}; //                     FOR ESP32 NOTE:::respectively BUT DOIT no pin 24!!!!!!!!!
                                                                                                                          // int dcal = 32;//calibrate pin

                                                                                                                          // word joyval[4];

                                                                                                                          // //GPIO34-39 can only be set as input mode and do not have software pullup or pulldown 
                                                                                                                          // int swpins = 3;
                                                                                                                          // int sw[3] = {34,35,39};  //modesetting pin
                                                                                                                          // bool swreadingmode = false;//set whilst reading


                                                                                                                          // //ARDU read pins
                                                                                                                          // // static int P[4] = {A0,A1,A2,A3}; //                     FOR ESP32 NOTE:::respectively BUT DOIT no pin 24!!!!!!!!!
                                                                                                                          // // int dcal = 7; // HIGH is OFF low is action
                                                                                                                          // // int sw1 = 35;  //not used yet



                                                                                                                          // //averaged analog read
                                                                                                                          // // read and accumulate the ADC value 10 times

                                                                                                                          // //cal   calibrations
                                                                                                                          // //run calibrate when digital pin D? is high 
                                                                                                                          // //populate with max and mins
                                                                                                                          // //when button D? released set joyvals and mid neutral pointspoint

                                                                                                                          // //static int cal[4][3]={499,500,501,498,500,502,500,500,500,500,500,500,};
                                                                                                                          // //static int cal[4][3]={3000,1234,0,3000,1234,0,3000,1234,0,3000,0,0,};


                                                                                                                          // //cal[ch][0]   stick min
                                                                                                                          // //cal[ch][1]   stick NEUTRAL set on startup :> need to start with sticks neutral 
                                                                                                                          // //cal[ch][2]   stick max
                                                                                                                          // //cal[ch][3]   stick = neutral at start up, subsequent centerings opposite direction form set stick from neutral  
                                                                                                                                              
                                                                                                                          // static int cal[4][4]={
                                                                                                                          //                       1600,2000,2370,0,
                                                                                                                          //                       1470,1950,2410,0,
                                                                                                                          //                       1590,2150,2530,0,
                                                                                                                          //                       1605,2000,2420,0};


                                                                                                                          // bool centering = 1;

                                                                                                                          // long mapphil(long x, long in_min, long in_max, long out_min, long out_max) {
                                                                                                                          //     long dividend = out_max - out_min;
                                                                                                                          //     long divisor = in_max - in_min;
                                                                                                                          //     long delta = x - in_min;
                                                                                                                          //     long al; 
                                                                                                                          //     if(divisor == 0){
                                                                                                                          //         log_e("Invalid map input range, min == max");
                                                                                                                          //         return -1; //AVR returns -1, SAM returns 0
                                                                                                                          //     }
                                                                                                                          //     //return (delta * dividend + (divisor / 2)) / divisor + out_min;
                                                                                                                          //     al = (delta * dividend /divisor )  + out_min;
                                                                                                                          //     if (al < out_min) al = out_min;
                                                                                                                          //     if (al > out_max) al = out_max;
                                                                                                                          //     return al;
                                                                                                                          // }



                                                                                                                          // int anRead( int pin){
                                                                                                                          //     int adc; 
                                                                                                                          //     adc = analogRead(pin); 
                                                                                                                          //   return adc; 
                                                                                                                          // }
                                                                                                                          // int anReadavg( int pin){
                                                                                                                          //         int sum = 0;
                                                                                                                          //         for(int i =0; i<10;i++){
                                                                                                                          //           sum += anRead( pin);
                                                                                                                          //         }
                                                                                                                          //         return (int) sum /10; 
                                                                                                                          // }


                                                                                                                          //                                                           //function joycalibrate in main because of embedded print statements >>>> rationalise later and move back to here
                                                                                                                          //                                                           //note above center funtion with no prints can stay here
                                                                                                                          //                                                           int to1500(int j ,int ch ){
                                                                                                                          //                                                               //map(joyvalue, frpresses() sets this value  see end below_min) * (out_max - out_min) / (in_max - in_min) + out_min;
                                                                                                                          //                                                               if(j < cal[ch][1])     map(j, cal[ch][0], cal[ch][1], 1000, 1500); //remember arrays start at 0 so [1] is middle
                                                                                                                          //                                                                 else                 map(j, cal[ch][1], cal[ch][2], 1500, 2000); //remember arrays start at 0 so [1] is middle
                                                                                                                          //                                                               return j;  
                                                                                                                          //                                                           }



                                                                                                                          // //////////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                                                                                                                          // //TRIMMING STUFF
                                                                                                                          // long maptrim(){//(long x, long in_min, long in_max, long out_min, long out_max) {
                                                                                                                          // //after centering adjust joyval from the new center val
                                                                                                                          // //a positive trim results in a "neutral stick osition" sending a higher value : we need to store the neutral positions so
                                                                                                                          // //need extra cal array cal[ch][3]
                                                                                                                          //     long al; 
                                                                                                                          //     return al;

                                                                                                                          // }

                                                                                                                          // int totrim(int j ,int ch ){
                                                                                                                          //     maptrim(j, cal[ch][0], cal[ch][2], 1000, 2000); //remember arrays start at 0 so [1] is middle
                                                                                                                          //     return j;  
                                                                                                                          // }
                                                                                                                          // void readtrims(int *P,word *joyval){
                                                                                                                          //       for(int i =0; i<4;i++){
                                                                                                                          //         //joyval[i] = to1500(anReadavg(P[i]), i); //now in range 1000-2000
                                                                                                                          //         joyval[i] = totrim(anReadavg(P[i]), i); //now in range 1000-2000
                                                                                                                          //       }
                                                                                                                          // }

                                                                                                                          // //////////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                                                                                                                          // //READ JOYS
                                                                                                                          // int to2000(int j ,int ch ){
                                                                                                                          //     mapphil(j, cal[ch][0], cal[ch][2], 1000, 2000); //remember arrays start at 0 so [1] is middle
                                                                                                                          //     return j;  
                                                                                                                          // }
                                                                                                                          // void readjoys(int *P,word *joyval){
                                                                                                                          //       for(int i =0; i<4;i++){
                                                                                                                          //         //joyval[i] = to1500(anReadavg(P[i]), i); //now in range 1000-2000
                                                                                                                          //         joyval[i] = to2000(anReadavg(P[i]), i); //now in range 1000-2000
                                                                                                                          //       }
                                                                                                                          // }
                                                                                                                          // //////////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                                                                                                                          // //PROCESS  JOYS
                                                                                                                          // void joycenter(int tt){//tt is either 1 for center calibrating, 4 for trimming
                                                                                                                          //         for(int i =0; i<4;i++){
                                                                                                                          //             cal[i][tt] = anReadavg(P[i]);//mid  //remember arrays start at 0 so [1] is middle
                                                                                                                          //         }
                                                                                                                          // }
                                                                                                                          // //PKT  JOYS
                                                                                                                          // void joybytes(word *joyval){
                                                                                                                          //       for(int i =0; i<4;i++){
                                                                                                                          //         joyval[i] =  (joyval[i] -1000) /4; //now in range 0-250 as an int but can typecast easily to byte in returned vals
                                                                                                                          //       }
                                                                                                                          // }

                                                                                                                          // //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                                                                                                                          // //READ MODE SW's
                                                                                                                          // byte  readmodesws(int * sw ,int swpins){ //operates on sw1 sw2 sw3 switch
                                                                                                                          //     bool z[3];
                                                                                                                          //     byte swb = 0;
                                                                                                                          //     if( !swreadingmode ){
                                                                                                                          //         swreadingmode = true;
                                                                                                                          //         for(int i=0;i< swpins;i++){
                                                                                                                          //           z[i] = digitalRead(sw[i]);
                                                                                                                          //         }
                                                                                                                          //         delay(10);  //inefficient put in interrupt later or rtos    
                                                                                                                          //     }
                                                                                                                          //     if( swreadingmode ){
                                                                                                                          //         swreadingmode = false;
                                                                                                                          //         for(int i=0;i< swpins;i++){
                                                                                                                          //           if( digitalRead(sw[i])== z[i]) sw[i] = z[i];
                                                                                                                          //         }
                                                                                                                          //         //now pack into a byte/int
                                                                                                                          //         swb = z[0]  |(z[1] << 1) |  (z[2] << 2) ;
                                                                                                                          //     } 
                                                                                                                          //     return swb;

                                                                                                                          // }
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


//for copter channel 5 is used for modes
//     // @Description: Flight mode for switch position 1 
// 1 (910 to 1230      1100
// 2 (1231 to 1360)    1300
// 3 (1361 to 1490)    1410
// 4 (1491 to 1620)    1550
// 5 (1621 to 1749)    1700
// 6                   1900


// // Auto Pilot modes PLANE
//     // ----------------
//     enum Number : uint8_t {
//         MANUAL        = 0,
//         CIRCLE        = 1,
//         STABILIZE     = 2,
//         TRAINING      = 3,
//         ACRO          = 4,
//         FLY_BY_WIRE_A = 5,
//         FLY_BY_WIRE_B = 6,
//         CRUISE        = 7,
//         AUTOTUNE      = 8,
//         AUTO          = 10,
//         RTL           = 11,
//         LOITER        = 12,
//         TAKEOFF       = 13,
//         AVOID_ADSB    = 14,
//         GUIDED        = 15,
//         INITIALISING  = 16,
//         QSTABILIZE    = 17,
//         QHOVER        = 18,
//         QLOITER       = 19,
//         QLAND         = 20,
//         QRTL          = 21,
//         QAUTOTUNE     = 22,
//         QACRO         = 23,
//         THERMAL       = 24,
//     };


    // Rudder Arming. Hold the rudder stick fully to the right and the throttle stick fully down for 2 seconds.
    // Arming Switch. An RC channel can be configured as an ARM/DISARM switch by using the RCx_OPTION for that channel set to 41.
    // GCS Arming. Press the arming button on your ground station


// // @Param: FLTMODE_CH
//     // @DisplayName: Flightmode channel
//     // @Description: RC Channel to use for flight mode control
//     // @Range: 1 16
//     // @Increment: 1
//     // @User: Advanced
//     GSCALAR(flight_mode_channel,    "FLTMODE_CH",     FLIGHT_MODE_CHANNEL),

//     // @Param: FLTMODE1
//     // @DisplayName: FlightMode1
//     // @Description: Flight mode for switch position 1 (910 to 1230 and above 2049)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode1,           "FLTMODE1",       FLIGHT_MODE_1),

//     // @Param: FLTMODE2
//     // @DisplayName: FlightMode2
//     // @Description: Flight mode for switch position 2 (1231 to 1360)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode2,           "FLTMODE2",       FLIGHT_MODE_2),

//     // @Param: FLTMODE3
//     // @DisplayName: FlightMode3
//     // @Description: Flight mode for switch position 3 (1361 to 1490)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode3,           "FLTMODE3",       FLIGHT_MODE_3),

//     // @Param: FLTMODE4
//     // @DisplayName: FlightMode4
//     // @Description: Flight mode for switch position 4 (1491 to 1620)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode4,           "FLTMODE4",       FLIGHT_MODE_4),

//     // @Param: FLTMODE5
//     // @DisplayName: FlightMode5
//     // @Description: Flight mode for switch position 5 (1621 to 1749)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode5,           "FLTMODE5",       FLIGHT_MODE_5),

//     // @Param: FLTMODE6
//     // @DisplayName: FlightMode6
//     // @Description: Flight mode for switch position 6 (1750 to 2049)
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Standard
//     GSCALAR(flight_mode6,           "FLTMODE6",       FLIGHT_MODE_6),

//     // @Param: INITIAL_MODE
//     // @DisplayName: Initial flight mode
//     // @Description: This selects the mode to start in on boot. This is useful for when you want to start in AUTO mode on boot without a receiver.
//     // @Values: 0:Manual,1:CIRCLE,2:STABILIZE,3:TRAINING,4:ACRO,5:FBWA,6:FBWB,7:CRUISE,8:AUTOTUNE,10:Auto,11:RTL,12:Loiter,13:TAKEOFF,14:AVOID_ADSB,15:Guided,17:QSTABILIZE,18:QHOVER,19:QLOITER,20:QLAND,21:QRTL,22:QAUTOTUNE,23:QACRO,24:THERMAL
//     // @User: Advanced
//     GSCALAR(initial_mode,        "INITIAL_MODE",     Mode::Number::MANUAL),


////////////////////////////////////////////////////////////////////////////////////////////////
//



//esp_err_t gpio_isr_register(void (*fn)(void *), void *arg, int intr_alloc_flags, gpio_isr_handle_t *handle, )
//You can either read the input register using the gpio_input_get() function (see gpio.h in NONOS SDK) or access the register directly as described here: https://github.com/esp8266










// int     button_switch =                       2; // external interrupt pin
// #define switched                            true // value if the button switch has been pressed
// #define triggered                           true // controls interrupt handler
// #define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
// #define debounce                              10 // time to wait in milli secs

// volatile  bool interrupt_process_status = {
//   !triggered                                     // start with no switch press pending, ie false (!triggered)
// };

// void Onebutton_interrupt_handler(){
//     if (interrupt_process_status == !triggered) {
//       if (digitalRead(button_switch) == HIGH) {
//         interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
//       }
//     }
// } // end of button_interrupt_handler

// bool read_button(int button_reading) {
//     static bool     switching_pending = false;
//   static long int elapse_timer;
//   if (interrupt_process_status == triggered) {
//     button_reading = digitalRead(button_switch);
//     if (button_reading == HIGH) {
//       switching_pending = true;
//       elapse_timer = millis(); // start elapse timing for debounce checking
//     }
//     if (switching_pending && button_reading == LOW) {
//       if (millis() - elapse_timer >= debounce) {
//         switching_pending = false;            
//         interrupt_process_status = !triggered; 
  








// int     button_switch =                       2; // external interrupt pin
// #define switched                            true // value if the button switch has been pressed
// #define triggered                           true // controls interrupt handler
// #define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
// #define debounce                              10 // time to wait in milli secs

// volatile  bool interrupt_process_status = {
//   !triggered                                     // start with no switch press pending, ie false (!triggered)
// };

// void Onebutton_interrupt_handler(){
//     if (interrupt_process_status == !triggered) {
//       if (digitalRead(button_switch) == HIGH) {
//         interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
//       }
//     }
// } // end of button_interrupt_handler

// bool read_button(int button_reading) {
//     static bool     switching_pending = false;
//   static long int elapse_timer;
//   if (interrupt_process_status == triggered) {
//     button_reading = digitalRead(button_switch);
//     if (button_reading == HIGH) {
//       switching_pending = true;
//       elapse_timer = millis(); // start elapse timing for debounce checking
//     }
//     if (switching_pending && button_reading == LOW) {
//       if (millis() - elapse_timer >= debounce) {
//    id multiPinInterrupt(int* pins, int pinnos) {
//   for(int j =0;j<pinnos;j++){
//       attachInterrupt(digitalPinToInterrupt(pins[j]),Onebutton_interrupt_handler, interrupt_trigger_type);

//   }
// } // end of setup function
