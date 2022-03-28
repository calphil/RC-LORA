 /*
    20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
  Explanation
    Protocol length: 20
    Command code: 40 
    Channel 0: DB 5  -> value 0x5DB        each ch has two bytes, low high swapped
    Channel 1: DC 5  -> value 0x5Dc
    Channel 2: 54 5  -> value 0x554
    Channel 3: DC 5  -> value 0x5DC
    Channel 4: E8 3  -> value 0x3E8
    Channel 5: D0 7  -> value 0x7D0
    Channel 6: D2 5  -> value 0x5D2
    Channel 7: E8 3  -> value 0x3E8
    Channel 8: DC 5  -> value 0x5DC
    Channel 9: DC 5  -> value 0x5DC
    Channel 10: DC 5 -> value 0x5DC
    Channel 11: DC 5 -> value 0x5DC
    Channel 12: DC 5 -> value 0x5DC
    Channel 13: DC 5 -> value 0x5DC
    Checksum: DA F3 -> calculated by adding up all previous bytes, total must be FFFF
 */
#include "Arduino.h"

#include <LoRa.h>




//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//RSSI ALERT  MODE STUFF
//Rsalmd STUFF
//8b rssi ... 4b alert .....4b mode
//as an  int:: Rsalmd;



int  Modefreq[8] = {
0, //unused
1100, //mode1
1300, //mode2
1410, //mode3
1550, //mode4
1700, //mode5
1900, //mode6
1950  //mode7 Non standard - as only only 6 items per channel allowed in ardupilot this will set Mode6
};

int mode2FC;  //used for sending mode to 5th channel into FC

//INCOMING
static byte ModeswIN = 0 ; // but carried as 4 bits
static byte alertIN = 0;   // but carried as 4 bits
static int rssiiIN;        // but carried as 1 byte  can / is negative

//OUTGOING
static byte ModeswOUT = 0 ; // but carried as 4 bits
static byte alertOUT = 0;   // but carried as 4 bits
static int rssiiOUT;        //but carried as 1 byte  can / is negative

                     //from least-significant to most-significant bit //The layout of bit fields in a C struct is implementation-defined. 

static struct Rsalmdfield{  
  byte Modesw      : 4;    //eg  Rsalmd.Modesw
  byte alert       : 4;
  byte brssib      : 8;
  //byte dummy        : 8;//maybe necessary as cannot xmit just 2 bytes as conflicts with joys, for present just pad out in the xmit part
}Rsalmdout ={0,0,0};    //this is of type (int)  go figure???????    !!!!!!!!!!!!!!
Rsalmdfield Rsalmdin ={0,0,0};
                     // 2^12 +2^9 + 3 = 4611 

word  rssi2Rsalmd(Rsalmdfield  Rsalmdout){
        word  rr = LoRa.packetRssi() + 160;  //
        Rsalmdout.brssib = rr;
        return rr;
}

word rssi2word(Rsalmdfield * Rsalmdout){
        word  rr =   (word)  Rsalmdout->brssib  - 160;  //
        return rr;
}
//!!@#!@!!#@#@!@##!@!!@@! note changed bdata from word to byte
word makeibuspkt(  byte* bdata, word * val, int ch, int bdatasz = 14) { //bdatasz nos of values for xmit less than standard ibus lenght bdatas
    word checksum = 0xFFFF - 0x20 -0x40;
    bdata[0]  = 0x20;
    bdata[1]  = 0x40;
    for(int i= 0; i< ch;i++){
        word ll,rr;
        // rr = val[i] % 16;
        // ll = val[i] / 16;
        rr = val[i] & 0xFF;
        ll = val[i] >> 8 ;
        bdata[i*2 + 2] =   rr ;
        bdata[i*2 + 3] =   ll  ;
        checksum = checksum - ll -rr ;
    }
    for(int i =  ch +1  ; i < bdatasz +1  ;i++){
        bdata[i*2 ] =   0xDC ;
        bdata[i*2+ 1] =   0x05 ;
        checksum = checksum -0xDC - 0x05;
    }
    //bdata[30] =  checksum % 16 ;
    bdata[30] =  checksum & 0xFF ;
    //bdata[31] =  checksum  / 16;
    bdata[31] =  checksum  >> 8;
// Serial.print(bdata[30],HEX);
// Serial.print(" ");
// Serial.print(bdata[31],HEX);
// Serial.println("<<<<<<<<<<  chksum");



    return true;//success phil  
}
//!!@#!@!!#@#@!@##!@!!@@! note changed bdata from int to byte
word makeintshortpkt(  byte* bdata, word * val, int nosbytes ) {   //for transmitting only NOT AN IBUS TYPE pkt
    word checksum = 0xFFFF - 0x20 -0x40;
    bdata[0]  = 0x20;
    bdata[1]  = 0x40;
    for(byte  i= 0; i< nosbytes ;i++){
        bdata[i + 2] =   val[i] ;
        checksum = checksum - bdata[i + 2] ;
    }
    bdata[nosbytes +3] =  checksum % 16 ;
    bdata[nosbytes +4] =  checksum  / 16;
    return true;//success phil  
}
bool makesbyteshortpkt(  byte* bdata, byte * val, int nosbytes ) {   //for transmitting only NOT AN IBUS TYPE pkt
    word checksum = 0xFFFF - 0x20 -0x40;
    bdata[0]  = 0x20;
    bdata[1]  = 0x40;
    for(byte  i= 0; i< nosbytes ;i++){
        bdata[i + 2] =   val[i] ;
        checksum = checksum - bdata[i + 2] ;
    }
    bdata[nosbytes +3] =  checksum % 16 ;
    bdata[nosbytes +4] =  checksum  / 16;
    return true;//success phil  
}
bool makeRsalmdshortpkt(   Rsalmdfield  *Rsalmdout , byte* bdata  ) {   //for transmitting only NOT AN IBUS TYPE pkt
    word checksum = 0xFFFF - 0x20 -0x40;
    bdata[0]  = 0x20;
    bdata[1]  = 0x40;
    bdata[2]  = Rsalmdout->Modesw  | Rsalmdout->alert << 4;
    bdata[3]  = (byte) rssi2word( Rsalmdout);
    checksum = checksum - bdata[2]- bdata[3];
    bdata[4] =  checksum % 16 ;
    bdata[5] =  checksum  / 16;
    return true;//success phil  
}


//!!@#!@!!#@#@!@##!@!!@@! note changed bdata from word to byte
word makeTwobytepkt(  byte* bdata, word val ) {  //for transmitting only NOT AN IBUS TYPE pkt
    bdata[0]  = 0x20;
    bdata[1]  = 0x40;
    bdata[2]  = (byte) val >> 8;  //  big end
    bdata[3]  = (byte) val & 0xF ;//  little end
    word checksum = 0xFFFF - 0x20 -0x40 - bdata[3]-bdata[4];
    bdata[4] =  checksum % 16 ;
    bdata[5] =  checksum  / 16;
    return true;//success phil  
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
bool ibus_decode( byte* bdata, word *values, int IBUS_INPUT_CHANNELS = 14 ,int IBUS_FRAME_SIZE = 32 ){ //generalised decoder
    word chksum = 96;
    /* check bdata boundary markers to avoid out-of-sync cases */
    if ((bdata[0] != 0x20) || (bdata[1] != 0x40)) {
        return false;
    }
    for (int ch = 0; ch < IBUS_INPUT_CHANNELS; ch++) {
        //values[ch]=bdata[ch * 2 + 2]|(bdata[ch *2 +3] & 0x0F)<<8; //somebody made stupid mistake BUT not ME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        values[ch]=bdata[ch * 2 + 2]|(bdata[ch *2 +3] <<8);
        chksum+=bdata[ch * 2 + 2]+bdata[ch *2 +3];
    }
    chksum += bdata[IBUS_FRAME_SIZE-2]|bdata[IBUS_FRAME_SIZE-1]<<8;
    if (chksum!=0xFFFF) {
        return false;
    }
    return true;
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//returns 4 joy vals in values
bool bytes4tojoy( byte* bdata, word * values ){  //

    for (int ch = 0; ch < 4; ch++) {
        values[ch]= (word) bdata[ch];
        values[ch]= (values[ch] << 2 ) + 1000; //divide by 4 +1000
    }

    // if (false) {
    //     return false;
    // }
    return true;
}

bool bytes2toword( byte* bdata, word * values, int cnt){
    values = (word*) &bdata;
    if (false) {
        return false;
    }
    return true;
}


bool decode2I4byte( byte* bdata, word * values ){  //
    word chksum = 96; 
    if ((bdata[0] != 0x20) || (bdata[1] != 0x40)) {
        return false;
    }
    for (int ch = 2; ch < 6; ch++) {
        values[ch]= (word) bdata[ch];
        chksum += bdata[ch];
    }
    chksum += bdata[6] | ( bdata[7]) << 8;
    if (chksum!=0xFFFF) {
        return false;
    }
    return true;
}

bool decode2I2byte( byte* bdata, word retval){
    word chksum = 96;
    if ((bdata[0] != 0x20) || (bdata[1] != 0x40)) {
        return false;
    }
    retval =  (word) (bdata[2] << 8) & (word)bdata[3];
    chksum +=  bdata[2] + bdata[3];
    chksum += bdata[4] | ( bdata[5]) <<8;
    if (chksum!=0xFFFF) {
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//convert byte to ibus 1000-2000 range for only joy vals to send reduced size
void fourb2send (  byte * bbdata , word * joyint  ){ //ch is position of the array ar ,nos of items to convert defauult 4
            byte joybyt[4];
            for(int i =0; i<4;i++){
                joybyt[i] =  (word) joyint[i] ; //but can typecast easily to byte in returned vals
            }
            makesbyteshortpkt(  bbdata, joybyt , 4) ;

}
void joy2byte4 (  byte * bdata , word * joyint  ){ //ch is position of the array ar ,nos of items to convert defauult 4
            byte joybyt[4];
            for(int i =0; i<4;i++){
                bdata[i] =  (byte) ((joyint[i]-1000) >> 2 ); //but can typecast easily to byte in returned vals
            }

}

//////////////////////////////////////////////////////////////////////////////////////
                                                            // struct Rsalmdfield{  
                                                            //   byte Modesw      : 4;    //eg  Rsalmd.Modesw
                                                            //   byte alert       : 4;
                                                            //   byte brssib      : 8; 
                                                            // }Rsalmd ={0,0,0};    //this is of type (word)  go figure???????    !!!!!!!!!!!!!!

void encodeRsalmd(Rsalmdfield   * Rsalmdout){
          //Rsalmdout->brssib = (LoRa.packetRssi() + 160) & 0xf;  //word2byte
          Rsalmdout->brssib =  (byte) (LoRa.packetRssi() + 160);  //word2byte
          Rsalmdout->Modesw = ModeswOUT;  // assumed to already exist
          Rsalmdout->alert = alertOUT;  // assumed to already exist
}

void makeRsalmd(   Rsalmdfield  Rsalmdout, byte *localbdata){
         makeRsalmdshortpkt(  &Rsalmdout ,localbdata); ///silly have to deref with &
}
//////////////////////////////////////////////////////////////////////////////////////
void decodeRsalmd( word * Rm,  Rsalmdfield   * anRsalmd){ //byte Modesw = 0 ; // but carried as 4 bits
                                                     // byte alert = 0;   // but carried as 4 bits
                                                     // word rssii;        // but carried as 1 byte
        memcpy ( anRsalmd, Rm, 2);
        //rssiiIN = anRsalmd->brssib;  //byte
        rssiiIN = anRsalmd->brssib -160;  //byte
        ModeswIN = anRsalmd->Modesw  ;  // assumed to already exist
        alertIN = anRsalmd->alert  ;  // assumed to already exist

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//NO IBUS ROUTINES
//joy out>>>>>>>>>>>>>>>>>>>>>>>> (in main)
                                //   byte bdata[4];
                                //   fourb2send ( bdata, joyval);
                                //   //lora sendpkt
                                //   txpkt( bdata , 8); //later do a "sizeof"
//joy in >>>>>>>>>>>>>>>>>>>>>>>>>









