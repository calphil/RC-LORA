#include <SPI.h>
#include <LoRa.h>
#include <philibus.h>

#define GPSD
#define lorad


                      //GPS STUFF
                      #ifdef GPSD
//gps
int gotafix = 0; //make this global
//word gpswords[4]; // 4 word =4 int =16bytes
//double lat ; //  8 bytes    using a STRUCT NOW
//double lng ; // //check ordering 

// void gpsrecvW(word * gpswords){//
//     lat = (double) gpswords[0] ;  //cast int to double
//     lng = (double) gpswords[8] ;
//   if(lat != 0.0 && lng != 0.0 ){  //lat lng = 0.0 indicates no fix
//       gotafix = 1;
//   }else{
//       gotafix = 0;
//   }
// }
// void gpsrecvB(byte * gpsb){//
//     lat = (double) gpsb[0] ;  //cast int to double
//     lng = (double) gpsb[16] ;
//   if(lat != 0.0 && lng != 0.0 ){  //lat lng = 0.0 indicates no fix
//       gotafix = 1;
//   }else{
//       gotafix = 0;
//   }
// }

// void gps2words(word * gpswords){//
//         double D[2]= {lat,lng};     //snazzy stuff!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!dfnetjeryjeryjwertjhertjh
//         double* pToDouble = &D[0];
//         gpswords = reinterpret_cast<word*>(pToDouble);
// }
// void gps2bytes(byte * gpsb){//
//         double D[2]= {lat,lng};     //snazzy stuff!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!dfnetjeryjeryjwertjhertjh
//         double* pToDouble = &D[0];
//         gpsb = reinterpret_cast<byte*>(pToDouble);
// }


//Geeps stuff
static      struct Gll{
            double lat;
            double lng;    
        }sgllin,sgllout;

byte  gbb[16]={1};

void Gll2bytes(Gll * gs, byte * gbb){//
      //byte *b = (byte *) &gbb[0];
      //byte *b = (byte *) &gs;
			//memcpy(gbb,gs, 16* sizeof(byte));
			//memcpy(gbb,gs, sizeof(*gs));
			///memcpy(&gbb[0],gs, 16);
			memcpy(gbb,gs, 16);

}

void b2Gll(byte * b ,Gll din){// this doesnt work uet the memcpy alone does--- could fix but not using anyway
			memcpy(&din, b, sizeof(din));
}

// static struct Gll sgll0, sgll1;
// byte  gbb[16]={1};
// byte* Gll2bytes(Gll * gs, byte * gbb);
// void b2Gll(byte * b ,Gll din);

                      //GPS STUFF
                       #endif
//////////////////////////////////////////////////////////////////////////////////////



                      //LORA  STUFF
                      #ifdef lorad
//#define ss 5  //
                //unused          // #define rst 33 //for doit
                                 // #define dio0 32 //for doit
#define ss 5 //
#define  rst 2 // UNUSED   not connected ..... checkout constructor to see if need to define later
#define dio0 4 //  REQUIRED CONNECTED USED AS INTERUPT for onrecv()


                                            // void Sprint(){//display and Serial print for debugging > use https://github.com/Chris--A libray later but too hard at present
                                            // }
 void readpkt( byte *inpkt ,int packetSize) {
      // read packet
      for (int i = 0; i < packetSize; i++) {
        inpkt[i] =(byte)LoRa.read();
      }
 }

void setupLORA() {
    LoRa.setPins(ss, rst, dio0);// note not using rst  Dio
    LoRa.setSyncWord(0xF3);
    if (LoRa.begin(433E6)){ 
      //LoRa.onReceive(onRecd);
    }else{
      Serial.println("LoRa failed!");
      //todo alert()
      while (1);
  }
}

//////////////////////////////INTERRUPTING ///////////////////////////////////////////////////////////  
                        // void onRecv(int psize) {     //eventually the onReceive = callback function for LoRa via function in txrx.h;
                        // }  
//////////////////////////////POLLING ///////////////////////////////////////////////////////////  

void PollRecv(byte * inpkt,  int psize) {     //eventually the onReceive = callback function for LoRa via function in txrx.h;
  word inval[16]; //max recvd ints is 8+8 //and shared for each receive case

//         Serial.println(psize); 
// Serial.print("<<<<<<<psize "); 

      //  for (int i=0; i<psize ;i++){
      //    //Serial.print("what psize           "); 
      //    Serial.print(inpkt[i]);
      //    Serial.print(" ");
         
      //  }

        // for (int i=0;i<2;i++){
        //   Serial.print(inpkt[i]); 
        //   Serial.println("  "); 
        // }


  switch (psize){
    
    case 16:{//plane2base       8+8 byte  packet ( lat and long) // double = 64bit ,int ABD word  in esp is 32bit , arduino uno 16

              // static struct Gll sgllin, sgllout;
              // byte  gbb[16]={1};
              // byte* Gll2bytes(Gll * gs, byte * gbb);
              // void b2Gll(byte * b ,Gll din);
// for (int i=0;i<16;i++){
//   Serial.print(inpkt[i]); 
//   Serial.print("  "); 
// }
// Serial.println("<<<<     <<<  inpkt  "); 

                                                                                      //      b2Gll(inpkt ,sgllin);
                 memcpy(&sgllin, &inpkt[0], 16);
// Serial.println(sgllin.lat); 
// Serial.println(sgllin.lng); 

//                                   // for (int i=0;i<16;i++){
//                                   //   Serial.print(gbb[i]); 
//                                   //   Serial.print("  "); 
//                                   // }
// Serial.println("<<<< lat lng ");  //   Serial.println(rssiiOUT);

      //Serial.print("gpsIN  "); Serial.print(sgllin.lat); Serial.print("  "); Serial.print(sgllin.lng);

      //do something 


      break;
    }

    case 4:{  //base2plane
    //Serial.println("in CASE4  ");
        //NO IBUS used for joys 4bytes, no crc, no preamble as LoRa already incorporates
        if(bytes4tojoy( inpkt , FC5ints )){  //NOTE the 5th PX5ints holds mode and must be protected
        //!!!!EXPAND BYTES TO INTS FOR JOYvals IE 0-250 >>> 1000-2000
              //FC5ints now has the 4 joyvals init
              //assuming a mode pkt has been received put this into the fifth  member ie [4] of  array
              FC5ints[4] =   Modefreq [mode2FC];
        }
      break;
    }
    case 2:{//bothways

        byte * rpi; 
        rpi = (byte *) &Rsalmdin;
        rpi[0]= inpkt[0];
        rpi[1]= inpkt[1];
// Serial.print("ir0IN  "); Serial.println(inpkt[0]);
// Serial.print("ir1IN  "); Serial.println(inpkt[1]);
// Serial.print("r0IN  "); Serial.println(rpi[0]);
// Serial.print("r1IN  "); Serial.println(rpi[1]);

    rssiiIN =    Rsalmdin.brssib -160 ;  //-160 to get it in decible

    ModeswIN  =    Rsalmdin.Modesw ;  
    mode2FC = ModeswIN; //only used in plane  can eliminate dupname later

    alertIN =     Rsalmdin.alert ;  

//Serial.print(" Rin ");     Serial.println(rssiiIN);
//Serial.print(" Min ");     Serial.println(ModeswIN);
// Serial.print(" Ain ");     Serial.println(alertIN);
                        //               //Modesw from plane can be used as information NOT YET
                        //              //do something  ALERTING MODE CHANGE egALERTING MODE CHANGE
                        //              //do something  ALERT CHANGE
                        //              //do something  LOW rssii
         
      break;
    }

  }
}

//size_t LoRaClass::write(const uint8_t *buffer, size_t size)
void txpkt(byte * buffer, int size ){
      LoRa.beginPacket();
      LoRa.write( buffer, size);
      LoRa.endPacket();
      LoRa.receive();
}

                      #endif
                      //endif lorad
//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------------------------------------------
//use this debug function at end of statement so not in the way
void ddg(String s, int val = 0){ // replace ddg with //ddg to globally cancel
  Serial.print(s);
  Serial.print(" ");
  Serial.println(val);
}
// able to change statments if place modifying statement at end of ddg() so //ddg cancels all out
//----------------------------------------------------------------------------------------------------------------------------

















/*

        // Serial.println(" inpkt 2  "); 
        // for (int i=0;i<2;i++){
        //   Serial.print(inpkt[i]); 
        //   Serial.println("  "); 
        // }
        //   Serial.println("  "); 
        //   Serial.println("  "); 


//Serial.print(" in1 ");     Serial.println(inpkt[0]);
//Serial.print(" in2 ");     Serial.println(inpkt[1]);

        //
        //DANGER WILL R ... Be carefulll whether base2plane or viceversa
        //not now have a RSalmd OUT and IN 

        // NO NO NO   WHY this not works ??????????????  memcpy ( &Rsalmdin, &inpkt, 2); 

        // int * rpi; 
        // rpi = (int *) &Rsalmdin;
        // rpi[0]= inpkt[0];
        // rpi[1]= inpkt[1];











//void (*onReceive)(int);
//in Lora.h:
//  void onReceive(void(*callback)(int));
//  void receive(int size = 0);
void (*onRecd)(int);//declaration       note phil   DOUBLE INDIRECTION

void onRec(void(*callb)(int)){
  onRecd  =  callb;
}



        word tempi[4] ={1,2,3,4} ;
        if (bytes2toword (inpkt,  tempi, 2)){    
              decodeRsalmd(tempi,  &Rsalmdout);
              //At this time only process the Modesw to plane 
              //others can be used as information BUT NOT YET







*/
