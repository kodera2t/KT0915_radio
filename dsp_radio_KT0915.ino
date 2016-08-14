#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 9
Adafruit_SSD1306 display(OLED_RESET);
#define RADIO 0x35
int read_byte,raw_upper,upper,lower,mode,read_byte2;
    int rssi,stereo,infreq;
float freq, old_freq;
unsigned int channel_num,s_upper,s_lower;
unsigned char s_upper2, s_lower2, s_upper3;
unsigned int initial_num;
volatile int encorder_val;
volatile int mode_set=0; /// mode_set=0:AM, mode_set=1:FM
volatile int band_mode = LOW;
float listen_freq;
int terminal_1  = 2;
int terminal_2  = 4;
volatile char old_state = 0;
int ct,pt,event,event2;
int rssi_count;
int rssi_count2=0;


void i2c_write(int device_address, int memory_address, int value, int value2)
{
  Wire.beginTransmission(device_address);
  Wire.write(memory_address);
    delay(5);
  Wire.write(value);
    delay(5);
  Wire.write(value2);
    delay(5);
  Wire.endTransmission();
}

void i2c_read(int device_address, int memory_address)
{
Wire.beginTransmission(device_address);
Wire.write(memory_address);
Wire.endTransmission(false);
Wire.requestFrom(device_address, 2);
read_byte = Wire.read();
//Wire.requestFrom(device_address, 1);
read_byte2 = Wire.read();
Wire.endTransmission(true);
//delay(30);
}



void setup()
{
    unsigned int upper,lower,raw_upper;
    unsigned int mask,mode,mode_set;
    Wire.begin() ;
  attachInterrupt(0,Rotary_encorder,CHANGE);
  attachInterrupt(1,mode_setting,CHANGE);
   delay(100) ;  
////////////    lcd.begin(8,2);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  pinMode(3, INPUT);
  pinMode(terminal_1, INPUT);
  pinMode(terminal_2, INPUT);
//  digitalWrite(terminal_1, HIGH);
//  digitalWrite(terminal_2, HIGH);
//  digitalWrite(3,HIGH);
  
  int temp;
    temp=0;
    i2c_write(RADIO,0x04,0b01100000,0b10110000);
    i2c_write(RADIO,0x05,0b00010000,0b00100000);   
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
    i2c_write(RADIO,0x2E,0b00101000,0b10001100);
encorder_val=0;
display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
//  display.setCursor(32,32);
//  display.println("Hello, world!");
//  display.setCursor(32,40);
//  display.println("Hello2, world!");
//  display.setCursor(32,48);
//  display.println("Hello2, world!");  
//    display.setCursor(32,56);
//  display.println("Hello2, world!"); 
  
  display.display();
}

void loop()
{

rssi_display();
if(event==1){
  if(mode_set==0){
  i2c_write(RADIO,0x16,0b00000000,0b00000010);
//        i2c_write(RADIO,0x0F,0b10001000,0b00010000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
  listen_freq=83.3;  
  initial_num=listen_freq*20.0;
  channel_num=initial_num+encorder_val*2;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  display.clearDisplay();
  display.setCursor(32,32);
  display.print("FM");
  freq=channel_num/20.0;
  display.setCursor(32,40);
 /// display.print(encorder_val);
  
 display.print(freq);
 display.setCursor(70,40);
 display.print("MHz");
   display.display();
  event=0;  
  }
  else if(mode_set==1){
  i2c_write(RADIO,0x16,0b10000000,0b11000011);
  i2c_write(RADIO,0x22,0b01010100,0b00000000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
  listen_freq=810;  
  initial_num=listen_freq;
  channel_num=initial_num+encorder_val*9;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  display.clearDisplay();
  display.setCursor(32,32);
  display.print("AM");
  freq=channel_num;
  display.setCursor(32,40);
  display.print(freq);
  display.setCursor(78,40);
  display.print("kHz ");
   display.display();
  event=0;  
  } else if(mode_set==2){
    SW_rec(2250);
  }else if(mode_set==3){
      SW_rec(3150); 
  }else if(mode_set==4){
      SW_rec(3850);
  }else if(mode_set==5){
      SW_rec(4700);
  }else if(mode_set==6){
      SW_rec(5900);
  }else if(mode_set==7){
      SW_rec(7100);
  }else if(mode_set==8){
      SW_rec(9400);
  }else if(mode_set==9){
      SW_rec(11500);
  }else if(mode_set==10){
      SW_rec(13500);
  }else if(mode_set==11){
      SW_rec(15000);
  }else if(mode_set==12){
      SW_rec(17450);
  }else if(mode_set==13){
      SW_rec(18850);
  }else if(mode_set==14){
      SW_rec(21450);
  }else if(mode_set==15){
      SW_rec(25600);
  }
}
}

void mode_setting(){
int sw,k;
ct=millis();
delay(1);
sw=digitalRead(3);
if(sw==LOW && (ct-pt)>50){
band_mode=HIGH;
mode_set=mode_set+1;
}
pt=ct;
if(mode_set>15){
  mode_set=0;
}
event=1;
encorder_val=0;
k=0;
}
    
void Rotary_encorder(void)
{
  if(!digitalRead(terminal_1)){
    if(digitalRead(terminal_2)){
      old_state = 'R';
    } else {
      old_state = 'L';
    }
  } else {
    if(digitalRead(terminal_2)){
      if(old_state == 'L'){ 
        encorder_val++;
      }
    } else {
      if(old_state == 'R'){
        encorder_val--;
      }
    }
    old_state = 0;
    event=1;
  }

}


void rssi_display(){
  int pll_lock,old_rssi;
  if(mode_set==0){
//// display RSSI  every seconds. 
  rssi_count=millis()-1000*rssi_count2; 
  if(rssi_count>1000){
    rssi_count2++;
  i2c_read(RADIO,0x12);
  old_rssi=rssi;
  rssi=-100+(read_byte2>>3)*3;
  pll_lock=(read_byte&0b00001100);
  stereo=read_byte&0b00000011;
  if(stereo==0b11){
  display.setCursor(32,48);
  display.print("stereo");
  display.display();
  }else{
  display.setCursor(32,48);
  display.print("      ");
  display.display();
  }
  if(pll_lock!=12){
  display.setCursor(32,48);
  display.print("PLL UNLOCK"); 
  display.display(); 
  }
  display.setCursor(32,56);
  display.print("RSSI:");
    display.setCursor(70,56);
      display.setTextColor(BLACK);
  display.print(old_rssi);
  display.display();
       display.setTextColor(WHITE); 
      display.setCursor(70,56);
  display.print(rssi);
  display.display();
    rssi_count=0;
  }
}
}

void SW_rec(float listen_freq)
{
  i2c_write(RADIO,0x16,0b10000000,0b00000011);
  i2c_write(RADIO,0x22,0b01010100,0b00000000);
//  listen_freq=3000;  
  initial_num=listen_freq;
  channel_num=initial_num+encorder_val*5;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  old_freq=freq;
  //freq=channel_num;
  display.clearDisplay();  
  display.setCursor(32,32);
  display.print("Short Wave");
  infreq=channel_num;
  display.setCursor(32,40);
  display.print(infreq);
  display.setCursor(78,40);
  display.print("kHz ");
  display.display();
  event=0; 
  mode=0;
}

