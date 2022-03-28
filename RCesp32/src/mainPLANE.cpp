#define PLANE

//**********************************************************************************************************************************************
#ifdef PLANE
// P L A N E  // P L A N E  // P L A N E  // P L A N E  // P L A N E  // P L A N E  // P L A N E  
				#define BLINK
				#define lorad
				#define PDEBUG
				#define GPSD
				#define sendFC

				//  #undef BLINK
				//  #undef lorad
				//  #undef PDEBUG
				//  #undef GPSD
				//  #undef sendFC


#include <Arduino.h>
#include <philjoy.h>

//#include "display.h" //
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdlib_noniso.h> //for the  dtostrf function C sucks badly


///CHANGE int to word  NOneed !!!! same 4 bytes each 
//----------------------------------------------------------------------------------------------------------------------------
//global     
static word FC5ints[5]; //holds 4 joyvals      + rssi/mode/alert  = Rsalmd to send to FC....... FC only uses mode info though
static byte fcinputpkt[32]; //need global as incrementally adding joys/ mode
int FCbaud = 115200;//baudrate to FC
                      //BLINK STUFF
                      #ifdef BLINK
  int blkpin =2;//LED onboard
  bool blk = 0;//for blinking
                      #endif 
                      //end BLINK STUFF
//////////////////////////////////////////////////////////////////////////////////////
                      //LORA  STUFF
                      #ifdef lorad
#include "txrx.h" //this in turn initializes LoRa  //        common for both txbase and plane lenght of packets is the decider
                      #endif
                      //endif LORA  STUFF
//////////////////////////////////////////////////////////////////////////////////////
                      #ifdef sendFC
static const uint32_t FCBaud = 115200; //is it 9600?????????
//HardwareSerial hsfc(1); //need to init to any pins < No31
HardwareSerial hsfc(1); //need to init to any pins < No31
                      #endif//end sendFC

//////////////////////////////////////////////////////////////////////////////////////

                        #ifdef GPSD
                        //GPS  STUFF
#include <TinyGPS++.h>
static const uint32_t GPSBaud = 38400; //is it 9600?????????
TinyGPSPlus gps;
//HardwareSerial hsgp(2);
//HardwareSerial hsgp(2);
                      //endGPS stuff
                        #endif

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
  long unsigned timestd  = 0;  //for elapsed time  
  int stdperiod  = 1000 ;//1hz rate
  long unsigned timetxrssi2  = 0;  //for elapsed time  
  int txrssi2sendperiod  = 3000 ;//1hz rate
  long unsigned timetx2plane  = 0;  //for elapsed time 
  int txplanesendperiod   = 90; //  1000/11 ;//11hz rate above the 10 sent from recv to FC  
                        //#ifdef sendFC
  long unsigned timefc = 0;  //for elapsed time FC send
  int FCsendperiod  = 1000 /10;//10hz rate
                        //#endif
                        //end sendFC
  //timing or use rtos LATERFC send
                        #ifdef GPSD
  long unsigned timetxgps  = 0;  //for elapsed time 
  int txgpssendperiod  = 1000 ;//1hz rate
                        #endif
                        //end sendFC

//test:>
//$$$$$$$$$$$$$
    char fflbuf[12]={'a','a','a','a','a','a','a','a','a','a','a','a'};
    char * pt6( double  along  ,char * buf, int len = 12,int prec=6){
      //int n;
      //n = sprintf (fflbuf, "%d plusd %d is %d",1,2,3);
      char * cp;
      cp = dtostrf(along, len, prec, fflbuf);
      return cp;
     }  




//@#$@!#%!#$%^!#$^!#$^@!$%^@$%@$%&@$%&@$%&@$%&@$%&@$&@$&@$&@$&@#$&$@#&@$&@$&$&*@$&@$*&@#$@$*@#$^*#%^*#%^*#%^*#%^*#%*#%*#%*#%*#%^*%#
void setup(void){
                        //BLINK STUFF
                      #ifdef BLINK
  pinMode(blkpin, OUTPUT);

  //setup serials
  Serial.begin(115200);
                        #ifdef sendFC
  //hsfc.begin(FCBaud,27,28);
  hsfc.begin(FCBaud,SERIAL_8N1,27,26);
                        #endif

                        #ifdef GPSD
                        //GPS  STUFF
//hsgp.begin(GPSBaud,17,16);
//hsgp.begin(GPSBaud,16,17);
//hsgp.begin(GPSBaud,SERIAL_8N1,16,17);
Serial2.begin(GPSBaud);
                      //endGPS stuff
                        #endif


                //delay (4000);
                digitalWrite(blkpin, 0);
                delay (1000);
                      #endif 
                      //end BLINK STUFF
  setupLORA();



  //onRec(onRecv); //inform txrx what the call back function is, which inturn inrms Lra what the same callback function is ..... double "indirection"

}//end setupxmit
//**********************************************************************************************************************************************

char c;

//**********************************************************************************************************************************************
//need to change loop() b/w base and plane
void loop(){

  int mt ;
  mt =millis();  

  byte localbdata[32];
              int pos = 0;
              int packetSize = LoRa.parsePacket();
              if (packetSize) {
                // read packet   //int LoRa.read()
                while (LoRa.available() && pos < packetSize) {
                  localbdata[pos]= (byte)LoRa.read();
                  pos += 1;
                }
                PollRecv (localbdata, packetSize);
              }
if(Serial2.available()){
                c = Serial2.read();
//                if (c) {
                  gps.encode(c);
                  //Serial.print(c);
                  //delay(10);
//                  }
}





//String gpa ="$GPGGA,115739.00,4158.8441367,N,09147.4416929,W,4,13,0.9,255.747,M,-32.00,M,01,0000*6E\n";
//String gpa ="$GNGGA,004856.80,3815.37680,S,14431.84879,E,2,12,0.67,43.7,M,-3.8,M,,0000*73\n";
// for (int i =0;i< gpa.length();i++){
//       char a;
//       a = gpa.charAt(i);
//       gps.encode(a);
//       Serial.print(a);
//       delay (100);
// }
              if (gps.location.isValid()){ 
                  gotafix = 1;
                  // static struct Gll sgllin, sgllout;
                  sgllout.lat =  gps.location.lat(); //smallest  first [0]th in array ie little endian!?
                  sgllout.lng =  gps.location.lng();
                  //DO SOMETHING ELSE?????
                  //Serial.println(gotafix);
                  //Serial.println(sgllout.lat);

              }else{
                   gotafix = 0;
                   alertOUT = 1;// change this when have alert list
                   sgllout.lat =-1.0;
                   sgllout.lng =-1.0;
              }


// sgllout.lat =-1.0;
// sgllout.lng =-1.0;
            //always make a pakt BUT lat long = 0 if no fix -- dont think will be flying in central atlantic
          //lora sendpkt
//$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   

                                //if (mt > timetxgps && gotafix){
                                if (mt > timestd){
                                            timestd = millis() + stdperiod ; //set timer 



                                }

                                if (mt > timefc){
                                            timefc = mt + FCsendperiod ; //set timer 
                                            makeibuspkt( fcinputpkt, FC5ints , 5);
                                            //send to FC uart Ibus pkt
                                            //ele,ail,rud,MODE
                                            //hsfc.write(fcinputpkt,32);
//hsfc.println ("hellow");                                            
                                }

  if (mt > timetxgps /*&& gotafix*/){
              timetxgps = millis() + txgpssendperiod ; //set timer 
              //make a pakt lat long = 0 if no fix -- dont think will be flying in central atlantic

              // static struct Gll sgllin, sgllout;
              // byte  gbb[16]={1};
              // byte* Gll2bytes(Gll * gs, byte * gbb);
              // void b2Gll(byte * b ,Gll din);
          //Gll2bytes(&sgllout, &gbb[0]);
          Gll2bytes(&sgllout, &gbb[0]);
// Serial.println(sgllout.lat); 
// Serial.println(sgllout.lng); 

// for (int i=0;i<16;i++){
//   Serial.print(gbb[i]); 
//   Serial.print("  "); 
// }
// Serial.println("<<<< gpsout ");  //   Serial.println(rssiiOUT);


//$$$$$$$$$$$$$
//     char fflbuf[12]={'a','a','a','a','a','a','a','a','a','a','a','a'};
//     char * pt6( double  along  ,char * buf, int len = 12,int prec=6){
//       //int n;
//       //n = sprintf (fflbuf, "%d plusd %d is %d",1,2,3);
//       char * cp;
//       cp = dtostrf(along, len, prec, fflbuf);

//       return cp;
//     }  

Serial.print("<<<< gpsout               ");  Serial.println(pt6(sgllout.lat,fflbuf));

// //$$$$$$$$$$$$$




              txpkt(gbb, 16 );
        //other housekeeping
      
  }          
  if (mt > timetxrssi2){
                          if (millis() > timetxrssi2){
                                      timetxrssi2 = millis() + txrssi2sendperiod ; //set timer 
                                  
                                  //getmodeSwitch  
                                  //ModeswOUT =  read3modesws( ); //operates on sw1 sw2 sw3 switch
                                  
                                  //silly me >> mode is both in fifth pkt AND
                                  //encodeRsalmd( Rsalmd);
                                  encodeRsalmd( &Rsalmdout); //includes adding rssi in the called function
                                  byte * rp;
                                  rp = (byte*) &Rsalmdout;
                                  localbdata[0] = rp[0];
                                  localbdata[1] = rp[1];
                                  //lora sendpkt
                                  txpkt( localbdata , 2); //later do a "sizeof"  //send to plane
//Serial.print(" sending from PLANE");     Serial.println(rssiiOUT);

                            //other housekeeping
                                      //         //BLINK STUFF
                                      //         #ifdef BLINK
                                      // blk = !blk;
                                      // digitalWrite(blkpin, blk);
                                      //           #endif 
                                      //         //end BLINK STUFF
                                
                          }
  }                        
}  

// // }//end loop plane

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// void loop(){

//   int mt ;
//   mt =millis();  

//   byte localbdata[32];
// //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   

// //Serial.println("inPLANE  "); 

// //just poll and process   later use interrupt method
//               // try to parse packet
//               int pos = 0;
//               int packetSize = LoRa.parsePacket();
//               if (packetSize) {
//                 // read packet   //int LoRa.read()
//                 while (LoRa.available() && pos < packetSize) {
//                   localbdata[pos]= (byte)LoRa.read();
//                   pos += 1;
//                 }
//                 PollRecv (localbdata, packetSize);
//               }
// //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   
//                 char c;
//                 c = Serial2.read();
//                 if (c) {
//                   (gps.encode(c));
//                   Serial.print(c);
//                   }


//               if (gps.location.isUpdated()){ 
//                   gotafix = 1;
//                   // static struct Gll sgllin, sgllout;
//                   sgllout.lat =  gps.location.lat(); //smallest  first [0]th in array ie little endian!?
//                   sgllout.lng =  gps.location.lng();
//                   //DO SOMETHING ELSE?????
//               }else{
//                    gotafix = 0;
//                    alertOUT = 1;// change this when have alert list
//                    sgllout.lat =-1.0;
//                    sgllout.lng =-1.0;
//               }

// // sgllout.lat =-1.0;
// // sgllout.lng =-1.0;
//             //always make a pakt BUT lat long = 0 if no fix -- dont think will be flying in central atlantic
//           //lora sendpkt
// //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   //$$$$$$$   

//                                 //if (mt > timetxgps && gotafix){
//                                 if (mt > timestd){
//                                             timestd = millis() + stdperiod ; //set timer 



//                                 }

//                                 if (mt > timefc){
//                                             timefc = mt + FCsendperiod ; //set timer 
//                                             makeibuspkt( fcinputpkt, FC5ints , 5);
//                                             //send to FC uart Ibus pkt
//                                             //ele,ail,rud,MODE
//                                             //hsfc.write(fcinputpkt,32);
// //hsfc.println ("hellow");                                            
//                                 }

//   if (mt > timetxgps /*&& gotafix*/){
//               timetxgps = millis() + txgpssendperiod ; //set timer 
//               //make a pakt lat long = 0 if no fix -- dont think will be flying in central atlantic

//               // static struct Gll sgllin, sgllout;
//               // byte  gbb[16]={1};
//               // byte* Gll2bytes(Gll * gs, byte * gbb);
//               // void b2Gll(byte * b ,Gll din);
//           //Gll2bytes(&sgllout, &gbb[0]);
//           Gll2bytes(&sgllout, &gbb[0]);
// // Serial.println(sgllout.lat); 
// // Serial.println(sgllout.lng); 

// // for (int i=0;i<16;i++){
// //   Serial.print(gbb[i]); 
// //   Serial.print("  "); 
// // }
// // Serial.println("<<<< gpsout ");  //   Serial.println(rssiiOUT);

//               txpkt(gbb, 16 );
//         //other housekeeping
      
//   }          
//   if (mt > timetxrssi2){
//                           if (millis() > timetxrssi2){
//                                       timetxrssi2 = millis() + txrssi2sendperiod ; //set timer 
                                  
//                                   //getmodeSwitch  
//                                   //ModeswOUT =  read3modesws( ); //operates on sw1 sw2 sw3 switch
                                  
//                                   //silly me >> mode is both in fifth pkt AND
//                                   //encodeRsalmd( Rsalmd);
//                                   encodeRsalmd( &Rsalmdout); //includes adding rssi in the called function
//                                   byte * rp;
//                                   rp = (byte*) &Rsalmdout;
//                                   localbdata[0] = rp[0];
//                                   localbdata[1] = rp[1];
//                                   //lora sendpkt
//                                   txpkt( localbdata , 2); //later do a "sizeof"  //send to plane
// //Serial.print(" sending from PLANE");     Serial.println(rssiiOUT);

//                             //other housekeeping
//                                       //         //BLINK STUFF
//                                       //         #ifdef BLINK
//                                       // blk = !blk;
//                                       // digitalWrite(blkpin, blk);
//                                       //           #endif 
//                                       //         //end BLINK STUFF
                                
//                           }
//   }  

// }//end loop plane

#endif
//END PLANE
//**********************************************************************************************************************************************


/*

    // Serial.print("FC5   ");
    // for (int ch = 0; ch < 5; ch++) {
    //    //ddg(">   ",testval[ch] );
    //    Serial.print( FC5ints[ch]); Serial.print("  ");
    // }  
    // // Serial.println("  ");
    // // Serial.print(packetSize);
    // // Serial.println("  ");

    // // Serial.print("lbdata ");
    // // for (int ch = 0; ch < 32; ch++) {
    // //    //ddg(">   ",testval[ch] );
    // //    Serial.print( localbdata[ch],HEX); Serial.print("  ");
    // // }  
    // Serial.println("  ");
    // Serial.print("fcinp  ");
    // for (int ch = 0; ch < 32; ch++) {
    //    //ddg(">   ",testval[ch] );
    //    Serial.print( fcinputpkt[ch],HEX); Serial.print("  ");
    // }  
    // Serial.println("  ");
    // Serial.print("vals   ");
    // for (int ch = 0; ch < 14; ch++) {
    //    //ddg(">   ",testval[ch] );
    //    Serial.print(testval[ch]); Serial.print("  ");
    // }  
    // Serial.println("  ");
    // Serial.println("  ");
    // Serial.println("  ");
    // delay(2000);

                                        // #ifdef PDEBUG
                                        //     //ddg("FC5ints");
                                        //     for(int i=0 ;i<4;i++){
                                        //       ddg("fc   ",FC5ints[i]);
                                        //     }
                                        //     ddg("________");
                                        //     delay(2000);
                                        // #endif 
                                        // //DEBUG stuff 
                              

















                                                                                  // void loopall(){

                                                                                  //   int mt ;
                                                                                  //   mt =millis();            

                                                                                  //   if (mt > timestd){
                                                                                  //               timestd = millis() + stdperiod ; //set timer 
                                                                                  //                       //BLINK STUFF
                                                                                  //                       #ifdef BLINK
                                                                                  //               //ddg("1sec");
                                                                                  //               blk = !blk;
                                                                                  //               digitalWrite(blkpin, blk);
                                                                                  //                         #endif 
                                                                                  //                       //end BLINK STUFF
                                                                                  //   }




                                                                                  //           byte localbdata[32]; 
                                                                                  //             if (hs1.available() > 0){
                                                                                  //               if (gps.encode(hs1.read())){
                                                                                  //                 if (gps.location.isValid()){ 
                                                                                  //                   gotafix = 1;
                                                                                  //                   lat =  gps.location.lat(); //smallest  first [0]th in array ie little endian!?
                                                                                  //                   lng =  gps.location.lng();
                                                                                  //                 }else{
                                                                                  //                    gotafix = 0;
                                                                                  //                    alert = 10;// change this when have alert list
                                                                                  //                    lat =0.0;
                                                                                  //                    lng =0.0;
                                                                                  //                 }
                                                                                  //               } 
                                                                                  //             }
                                                                                  //             //always make a pakt BUT lat long = 0 if no fix -- dont think will be flying in central atlantic
                                                                                  //           //lora sendpkt
                                                                                  //   if (mt > timetxgps && gotafix){
                                                                                  //               timetxgps = millis() + txgpssendperiod ; //set timer 
                                                                                  //               //make a pakt lat long = 0 if no fix -- dont think will be flying in central atlantic
                                                                                  //               gps2words( gpswords);                  
                                                                                  //               makeintshortpkt( localbdata,  gpswords, 16);
                                                                                  //               txpkt( localbdata, 8 + 4);
                                                                                  //         //other housekeeping
                                                                                  //                       //BLINK STUFF
                                                                                  //                       #ifdef BLINK
                                                                                  //               //ddg("sentGPS",mt);
                                                                                  //               blk = !blk;
                                                                                  //               digitalWrite(blkpin, blk);
                                                                                  //                         #endif 
                                                                                  //                       //end BLINK STUFF
                                                                                        
                                                                                  //   }          
                                                                                  //   if (mt > timetxrssi2){
                                                                                  //           timetxrssi2 = millis() + txrssi2sendperiod ; //set timer 
                                                                                  //           //note no UNIQUE Modesw alert  in plane yet so just send back boomerang to txbase  //DO LATER TWO SETS vars
                                                                                  //           encodeRsalmd( Rsalmd);
                                                                                  //           //makepkt
                                                                                  //           makeRsalmd(  Rsalmd , localbdata  )  ;
                                                                                  //           //lora sendpkt
                                                                                  //           txpkt( localbdata , 6); //later do a "sizeof"
                                                                                  //   }          
                                                                                  //   if (mt > timefc){
                                                                                  //               timefc = mt + FCsendperiod ; //set timer 
                                                                                  //               makeibuspkt( fcinputpkt, FC5ints , 5);
                                                                                  //               //send to FC uart Ibus pkt
                                                                                  //               //ele,ail,rud,MODE
                                                                                  //               hs1.write(fcinputpkt,32);
                                                                                  //           //DEBUG stuff 
                                                                                  //           #ifdef PDEBUG
                                                                                  //               //ddg("FC5ints");
                                                                                  //               for(int i=0 ;i<4;i++){
                                                                                  //                 ddg("fc   ",FC5ints[i]);
                                                                                  //               }
                                                                                  //               ddg("________");
                                                                                  //               delay(2000);
                                                                                  //           #endif 
                                                                                  //           //DEBUG stuff 
                                                                                  
                                                                                  //   }
                                                                                  // }//end loop plane


Serial.print("r0make  "); Serial.println(localbdata[0]);
Serial.print("r1make  "); Serial.println(localbdata[1]);
Serial.print(" Routg ");     Serial.println(Rsalmdout.brssib);
Serial.print(" Moutg ");     Serial.println(Rsalmdout.Modesw);
Serial.print(" Aoutg ");     Serial.println(Rsalmdout.alert);
                                  //makepkt
                                  //makeRsalmd(  Rsalmd , localbdata  )  ;







*/


