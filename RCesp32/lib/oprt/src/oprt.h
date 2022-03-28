
 //#include "display.h" //
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

                  
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
//OOOd displ(SCREEN_WIDTH, SCREEN_HEIGHT, &displ,-1,400000,100000);//reset = -1 not used!!!!!
Adafruit_SSD1306 displ(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);//reset = -1 not used!!!!!

    String sca = "CAL >>>> ";
    String sce = "CEN >>>> ";
    String smd ="mode ";
    String sla= "LAT ";
    String slg= "LNG ";
    String srs ="rsii B/P";
    String  sal ="alrt ";

    int linenos;
    String St; 
    int  vali;


    
  //declarations, definitions under main but move here later  
    void oprtclearline(   int linenos);
    void oprtname( String St , int linenos );
    void oprtnameval( String St, int  vali , int linenos );
    void oprtnameval2( String St, int  vali ,int val, int linenos );  //passing 2 int but usually a byte 
    void oprtgps( long * vali , int linenos );
    void oprtlatlng( double  vlat ,double  vlng , int linenos );
    void oprt4byte3rowCAL(  int linenos );
    void oprt4byteCAL( int row, int linenos );
    void oprtsingle( int val);//for testing
    void oprt4JOY(  int linenos );
    void oprt4byte(  int  valpos, int linenos );  //passing an int but usually a byte 


// If you have text size = 1 character height will be 7 pixel.
// So if you want to clear first row you could do this :
      // for (y=0; y<=6; y++)      {
      //  for (x=0; x<127; x++)       {
      //   OLED.drawPixel(x, y, BLACK); 
      //  }
      // }

//move these later modify cal array
    void oprtclearline(   int linenos){  //
        // displ.setCursor(1,10*linenos -9); 
        // displ.print ( F("                ") ); //16 chars of blank I hope   NOPE need to overwrite all pixels
        // displ.display();
      int lpix = 10*linenos -10;  
      for (int y= lpix; y<= lpix +10; y++)      {
       for (int x=0; x<127; x++)       {
        displ.drawPixel(x, y, BLACK); 
       }
      }
    }

    void oprtname( String St , int linenos ){  //
          //int ls;
          //ls =St.length();
          displ.setCursor(0,10*linenos -10); 
          displ.print (St );
          displ.display();
    }

    void oprtnameval( String St, int  vali , int linenos ){  //passing an int but usually a byte 
          displ.setCursor(0,10*linenos -10); 
          displ.print (St + F(" ") + vali);
          displ.display();
    }
    void oprtnameval2( String St, int  vali ,int val, int linenos ){  //passing an int but usually a byte 
          displ.setCursor(0,10*linenos -10); 
          displ.print (St + F("  ") + vali +F("  ") + val);
          displ.display();
    }
    void oprt4byte3rowCAL( int linenos ){  //passing an int but usually a byte 
          for (int c=0; c< 3;c++){
            displ.setCursor(0,10*linenos -10 + c*10);
            for (int i=0; i< 4;i++){
              displ.setCursor(i*30  ,10*linenos -10 + c*10);
              displ.print (cal[i][c]);
           }             
          }
          displ.display();
    }
    void oprt4byteCAL( int c, int linenos ){  //passing an int but usually a byte 
            displ.setCursor(0,10*linenos -10 );
            for (int i=0; i< 4;i++){
              displ.setCursor(i*30  ,10*linenos -10 );
              displ.print (cal[i][c]);
           }             
    }

    void oprt4JOY(  int linenos ){  //passing an int but usually a byte 
            displ.setCursor(0,10*linenos -10 );
            for (int i=0; i< 4;i++){
              displ.setCursor(i*30  ,10*linenos -10 );
              displ.print (joyval[i]);
           }             
    }
    void oprt4byte(  int  valpos, int linenos ){  //passing an int but usually a byte 
            displ.setCursor(0,10*linenos -10 );
            for (int i=0; i< 4;i++){
              displ.setCursor(i*30  ,10*linenos -10 );
              displ.print (cal[i][valpos]);
           }             
    }
                //
    char flbuf[12]={'a','a','a','a','a','a','a','a','a','a','a','a'};
    char * oprtfloat6( double  along  ,char * buf, int len = 12,int prec=6){
      //int n;
      //n = sprintf (flbuf, "%d plusd %d is %d",1,2,3);
      char * cp;
      cp = dtostrf(along, len, prec, flbuf);
      return cp;
    }  

    void oprtsingle( int val){//for testing
            displ.setCursor(30,30);
            displ.clearDisplay();
            char ddd[] = {'1','1','1','1','1','1','1','1','1'};
            char *dd = &ddd[0];
            itoa(val,dd,0);
            displ.print (dd);
            displ.display();
    }  

    void oprtlatlng( double  vlat ,double  vlng , int linenos ){  
      oprtfloat6(vlat, flbuf); //sign + 3 + . + 6 >   11 bytes the 12th for a String is null,0
      displ.setCursor(0,10*linenos -10); 
      displ.print (sla);
      displ.setCursor(24,10*linenos -10);
      displ.print (flbuf);
      oprtfloat6(vlng, flbuf); //sign + 3 + . + 6 >   11 bytes the 12th for a String is null,0
      displ.setCursor(0,10*linenos ); 
      displ.print (slg);
      displ.setCursor(24,10*linenos );
      displ.print (flbuf);
      displ.display();
    }


    
