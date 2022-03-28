//#define BASE
//**********************************************************************************************************************************************
#ifdef BASE
// B A S E// B A S E// B A S E// B A S E// B A S E// B A S E// B A S E// B A S E// B A S E  
				#define OLEDd
				#define JOY
				#define BLINK
				#define lorad
				#define PDEBUG
				#define GPSD

				//  #undef BLINK
				//  #undef lorad
				//  #undef PDEBUG
				//  #undef GPSD
				//  #undef sendFC



#include <Arduino.h>
#include <philjoy.h>
#include <oprt.h>


#include <stdlib_noniso.h> //for the  dtostrf function C sucks badly


///CHANGE int to word  NOneed !!!! same 4 bytes each 
//----------------------------------------------------------------------------------------------------------------------------
//global     
static word FC5ints[5]; //holds 4 joyvals      + rssi/mode/alert  = Rsalmd to send to FC....... FC only uses mode info though
static byte fcinputpkt[32]; //need global as incrementally adding joys/ mode //only used in PLANE
int FCbaud = 115200;//baudrate to FC
                      //BLINK STUFF
                      #ifdef BLINK
  int blkpin =2;
  bool blk = 0;
                      #endif 
                      //end BLINK STUFF
//////////////////////////////////////////////////////////////////////////////////////
                      //LORA  STUFF
                      #ifdef lorad
#include "txrx.h" //this in turn initializes LoRa  //        common for both txbase and plane lenght of packets is the decider
void onRecv(int psize); //declareit here move implimentation definition to bottom
                      #endif
                      //endif LORA  STUFF
//////////////////////////////////////////////////////////////////////////////////////
  long unsigned timestd  = 0;  //for elapsed time  
  int stdperiod  = 1000 ;//1hz rate
  long unsigned timetxrssi2  = 0;  //for elapsed time  
  int txrssi2sendperiod  = 2000 ;//10hz rate
  long unsigned timetx2plane  = 0;  //for elapsed time 
  int txplanesendperiod   = 2000; //  1000/11 ;//11hz rate above the 10 sent from recv to FC  
                        #ifdef sendFC
  long unsigned timefc = 0;  //for elapsed time FC send
  int FCsendperiod  = 1000 /10;//10hz rate
                        #endif
                        //end sendFC
  //timing or use rtos LATERFC send
                        #ifdef GPSD
  long unsigned timetxgps  = 0xFFFFF;  //for elapsed time 
  int txgpssendperiod  = 200000 ;//10hz rate
                        #endif
                        //end sendFC
  int buttonup ;
//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
void setup(void){
                      //BLINK STUFF
                      #ifdef BLINK
  pinMode(blkpin, OUTPUT);
  Serial.begin(115200);
                      #endif 
                      //end BLINK STUFF
 
                      //JOY STUFF
                      #ifdef JOY
  for(int i =0; i<4;i++){
    pinMode(P[i], INPUT); 
  }
  pinMode(32, INPUT_PULLUP); //high with sw off
  for(int i =0; i<3;i++){
    pinMode(sw[i], INPUT_PULLUP); 
  }
                      #endif 
                      //end JOY STUFF
                      //OLED STUFF
                      #ifdef OLEDd
    if(!displ.begin( (uint8_t) SSD1306_SWITCHCAPVCC, (uint8_t) SCREEN_ADDRESS)) {  // this should do this:  displ.begin (21, 22);   // sda= GPIO_21 /scl= GPIO_22
        Serial.println(F("SSD1306 allocation failed"));
    }
    displ.setTextColor(WHITE, BLACK);
    displ.setTextSize(1);          // text size
    displ.setTextColor(WHITE);
                      #endif 
                      //end OLED STUFF
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//CALIBRATE STUFF
                    //JOY STUFF
                      #ifdef JOY
                      //OLED STUFF
                      #ifdef OLEDd
  displ.clearDisplay();
  oprtname( sca, 2);
 
                      #endif 
                      //end OLED STUFF
  delay(2000);
  buttonup = digitalRead(dcal);
  //debounce
  delay(50);
  int temp = digitalRead(dcal);
  if (temp ==  buttonup )buttonup = temp;
  //DEBUG
  //buttonup = 0;
  if(!buttonup){//start calibration otherwise just start with pre hardcoded calibation
      while (!buttonup) {
              for(int i =0; i<4;i++){
                Jcal();
                      //OLED STUFF
                      #ifdef OLEDd
                displ.clearDisplay();
                oprt4byteCAL( 0 , 2 ); //row,linenos
                oprt4byteCAL( 2 , 4 ); //row,linenos
                displ.display();
                delay(50);
                      #endif 
                      //end OLED STUFF
                buttonup = digitalRead(dcal);
                //debounce
                delay(50);
                 temp = digitalRead(dcal);
                if (temp ==  buttonup )buttonup = temp;
              }
      }//endwhile
  }//endif calibrate
    
      Jcenters(); //if this doesn't properly center can always do again in loop
    
                      //OLED STUFF
                      #ifdef OLEDd
      displ.clearDisplay();
      // oprtname( sce, 4); 
      // displ.display();
      // delay(1000);
      // displ.clearDisplay();
      // displ.display();
      oprt4byteCAL( 1 , 4 ); //row,linenos
      displ.display();
                      #endif 
                      //end OLED STUFF
                    #endif 
                      //end JOY STUFF

                      //LORA  STUFF
                      #ifdef lorad
  setupLORA();
                      #endif
                      //endif lorad
}//end setup
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
#ifdef BASE
int cnt = 0; 
bool cancenter = false;

//**********************************************************************************************************************************************
//**********************************************************************************************************************************************

void loop(){
  int mt;
//Serial.print("."); 

  byte localbdata[32];

//$$$$$$$
//just poll and process   later use interrupt method
              // try to parse packet
              int pos = 0;
              int packetSize = LoRa.parsePacket();
              if (packetSize) {
                // received a packet

                // read packet   //int LoRa.read()
                while (LoRa.available() && pos < packetSize) {
                  localbdata[pos]= (byte)LoRa.read();
                  pos += 1;
                }
                PollRecv (localbdata, packetSize); 
              } 


//$$$$$$$  

  mt =millis();            
  if (mt > timestd){ //standard 1Hz loop rate for housekeeping
              timestd = mt + stdperiod ; //set timer
  //Serial.println("inBASE  "); 
  //trim  once per second
                        //CALIBRATE STUFF
                        //at anytime
                            buttonup = digitalRead(dcal);  //button pushed is a zero
                            if ( !buttonup && !cancenter  ) { 
                                    cancenter = true;
                            }        
                            if ( buttonup && cancenter ) {  //no debouncing here -- just centers when  released  
                                Jtrims(); //with trim applied
                                              //OLED STUFF
                                              #ifdef OLEDd
                                oprtclearline(4);
                                oprt4byte( 3 , 4 ); 
                                displ.display();
                                              #endif
                                              //endOLED
                              cancenter = false; //reset the can center clock
                            }
              cnt += 1; 
              if (cnt > 4){//four times std rate
                cnt  = 1;
                //replace centering with Rssi
                                                //OLED STUFF
                                                #ifdef OLEDd
                oprtclearline(4);
                //oprtnameval( srs , rssiiIN, 4 ); //row,linenos
                oprtnameval2( srs , Rsalmdout.brssib -160 , rssiiIN, 4 ); //rssiiOUT not set



                displ.display();
                                                #endif
                        // //MODE ALERT STUFF
                            //Note Modesw is generated on tx and sent to plane 
                            //Can also be received from plane  BUT CANT BE DECODED as will conflict with Modesw SO LEAVE AS IS
                                //implies we need two recv decodes placed in #defines
              }
                                              //OLED STUFF
                                              #ifdef OLEDd
                                oprtclearline(2);
                                oprt4JOY( 2 ); 
                                oprtclearline(5);
                                oprtclearline(6);
                                oprtlatlng(  sgllin.lat,sgllin.lng,5);  //'array' of  TWO longs lat and lng passed in   //testing
                                displ.display();
                                              #endif 

  }
                          if (millis() > timetxrssi2){
                                      timetxrssi2 = millis() + txrssi2sendperiod ; //set timer 
                                  
                                  //getmodeSwitch  
                                  ModeswOUT =  read3modesws( ); //operates on sw1 sw2 sw3 switch
                                  
                                  //silly me >> mode is both in fifth pkt AND
                                  //encodeRsalmd( Rsalmd);
                                  encodeRsalmd( &Rsalmdout); //includes adding rssi in the called function
                                  byte * rp;
                                  rp = (byte*) &Rsalmdout;
                                  localbdata[0] = rp[0];
                                  localbdata[1] = rp[1];
                                  //lora sendpkt
                                  txpkt( localbdata , 2); //later do a "sizeof"  //send to plane
//Serial.print("rout1  "); Serial.println(localbdata[1]  ); 
//Serial.print(" sending from BASE");     Serial.println(Rsalmdout.brssib);

                            //other housekeeping
                                              //BLINK STUFF
                                              #ifdef BLINK
                                      blk = !blk;
                                      digitalWrite(blkpin, blk);
                                                #endif 
                                              //end BLINK STUFF
                          }

                        if (millis() > timetx2plane){
                                      timetx2plane = millis() + txplanesendperiod ; //set timer 
                                  //sendpkt
                                  JReads();//normal joystic axis read with conversions ready to send
                                  // not needed  J2bytes(); as  fourb2send takes ints
                                  //makepkt
                                  byte bdata[4];
                                  joy2byte4 ( bdata , joyval);
                                  //lora sendpkt
                                  txpkt( bdata , 4); //later do a "sizeof"
                          }
                        //not used yet
                        //  if (millis() > timetxgps){
                        //               timetx2plane = millis() + txgpssendperiod ; //set timer 
                        //   }
}
#endif
//BASE


//**********************************************************************************************************************************************
//   if (millis() > 5000 && gps.charsProcessed() < 10)
 


                      //GPS  STUFF
                      #ifdef GPSD
#include <TinyGPS++.h>
//static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 38400; //is it 9600?????????
TinyGPSPlus gps;
                      #endif
                      //endGPS stuff
//////////////////////////////////////////////////////////////////////////////////////
                      #ifdef sendFC
  HardwareSerial hs1(1); //connect to FC
                      #endif//end sendFC
//////////////////////////////////////////////////////////////////////////////////////


#endif
//end BASE



/////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//sizeof 

//esp
      //double = 8byte , 
      //int AND word =4byte 
      //byte  ?? 
      //char 8b

//ard 
      //float  32b 
      //long ;  64b
        //unsigned ,
        //signed 
      //word unsigned 16b
      //int 16b -32k +32k
      //char SIGNED 8b
      //byte UNSIGNED 8b 

/////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%





//         for (int i=0;i<2;i++){
//           Serial.println(localbdata [i]); 
//         }

// Rsalmdfield Rtest ={0,0,0};  

//         Serial.print("  wp    "); 

//         word tempi[] ={1,2,3,4};
//         //bytes2toword (localbdata,  tempi ,2);
//         word* wp;  
//         wp = (word*) &localbdata;
//         // for (int i=0;i<2;i++){
//         //   Serial.print(tempi[i]); 
//         //   Serial.print("  "); 
//         // }
//           Serial.print(wp[0]); 
//           Serial.print("  "); 
//               decodeRsalmd( wp,  &Rtest);
//           Serial.print("testmodesw  "); 
//           Serial.println( Rtest.Modesw); 
//           Serial.println(""); 
//           Serial.print("                                  rssii "); 
//           Serial.println(Rtest.brssib); 
//           Serial.print("                                  Modesw"); 
//           Serial.println(Rtest.Modesw); 
//           Serial.print("                                  alert"); 
//           Serial.println(Rtest.alert); 
//           Serial.print("                   rssii "); 
//           Serial.println(rssii); 
//           Serial.print("                   Modesw"); 
//           Serial.println(Modesw); 
//           Serial.print("                   alert"); 
//           Serial.println(alert); 
//           Serial.println(""); 
//           Serial.println(""); 



// Serial.println("");
// Serial.println("");
// Serial.println(joyval[0]);
// Serial.print(cal[0][0]); Serial.print("  ");
// Serial.print(cal[0][1]); Serial.print("  ");
// Serial.print(cal[0][2]); Serial.print("  ");
// Serial.print(cal[0][3]); Serial.println("  ");
//delay(1000);


// Serial.println("");
// Serial.println("");
// Serial.println(joyval[0]);
// Serial.print(bdata[0]); Serial.print("  ");
// Serial.print(bdata[1]); Serial.print("  ");
// Serial.print(bdata[2]); Serial.print("  ");
// Serial.print(bdata[3]); Serial.println("  ");

/*

void loopy(){
  int mt;
  byte localbdata[32];

//$$$$$$$
//just poll and process   later use interrupt method
              // try to parse packet
              int pos = 0;
              int packetSize = LoRa.parsePacket();
              if (packetSize) {
                // received a packet

                // read packet   //int LoRa.read()
                while (LoRa.available() && pos < packetSize) {
                  localbdata[pos]= (byte)LoRa.read();
                  pos += 1;
                }
                PollRecv (localbdata, packetSize); 
              } 
                          if (millis() > timetxrssi2){
                                      timetxrssi2 = millis() + txrssi2sendperiod ; //set timer 
                                  //getmodeSwitch  
                                                  //Modesw =  readmodesws(int * sw ,int swpins){ //operates on sw1 sw2 sw3 switch
                                  //Modesw =  readmodesws( sw , swpins); //operates on sw1 sw2 sw3 switch
                                  ModeswOUT =  read3modesws( ); //operates on sw1 sw2 sw3 switch
                                  //silly me >> mode is both in fifth pkt AND
                                  //encodeRsalmd( Rsalmdout);
                                  encodeRsalmd( &Rsalmdout);
                                  //makepkt
                                  //makeRsalmd(  Rsalmdout , localbdata  )  ;
                                  int * rp;
                                  rp = (int*) &Rsalmdout;
                                  localbdata[0] = rp[0];
                                  localbdata[1] = rp[1];
                                  //lora sendpkt
                                  txpkt( localbdata , 2); //later do a "sizeof"  //send to plane

                                  //PollRecv (localbdata, 2); 
                                  Serial.print(" out2  ");
                                  Serial.print(localbdata[0]);
                                  Serial.print(" out2  ");
                                  Serial.println(localbdata[1]);


// int e = 4611;
// byte * tm;
// //byte tm[2]={0xff,1};
// tm = (byte *) &e;
// txpkt( tm , 2);
//         Serial.println(" inpkt 2  "); 
//         for (int i=0;i<2;i++){
//           Serial.print(tm[i]); 
//           Serial.println("  "); 
//         }
//           Serial.println("  "); 
//           Serial.println("  "); 
                                
                        //   }
                        }
}


*/

