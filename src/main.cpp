#include <Arduino.h>
#include <FastLED.h>

#define data_pin 9
#define num_leds 3

CRGB leds[num_leds];

uint8_t no_flash[3], flash[3];
uint8_t flicker[3], flicker_time[3], flicker_threshold;
uint8_t led_seq, led_no;  
uint8_t sequence[6][num_leds] ={{0,1,2},
                    {1,2,0},
                    {2,0,1},
                    {0,2,1},
                    {1,0,2},
                    {2,1,0}};
uint8_t flicker_mode[num_leds]; // 0 steady state
                  // 1 ramp up
                  // 2 ramp down
                  // 3 ramp up then down
                  // 4 ramp down then up
                  // 5 ramp up hold ramp down
                  // 6 ramp down hold ramp up
                  // 7 flucuate
                  // 8 ????
uint32_t millis_old, millis_delta;
uint8_t intensity, transistion;
//uint32_t total_flash;
//bool multi_flash_done;
int end_of_seq_delay;
int color_shift[num_leds][3], Red, Green, Blue;
int no_of_strikes, distance_from_strike, disipated_delay, randomized_distance;


void polled_ms_delay(uint32_t time);

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812B, data_pin, GRB>(leds, num_leds);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,500);
  //FastLED.clear();
  //FastLED.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  flicker_threshold = 35;
  led_seq = (random(1,6)) - 1;
  no_of_strikes = random(1,num_leds);
  randomized_distance = distance_from_strike + random(-7, 7);
  distance_from_strike = random(1,(10 - randomized_distance/2));
  disipated_delay = random(1,no_of_strikes);
  
  for (int i=0;i<num_leds;i++){
    no_flash[i]=random(50, 3000);
    flash[i]=random(10, 250);
    if(flash[i] > flicker_threshold){
      flicker[i]=random(2,16);
      flicker_time[i]=random(4,16);
      flicker_mode[i] = random(0,7);
      if((flicker_mode[i] > 2) && (flicker[i] == 2)){
        if(flicker_mode[i] == 3 || flicker_mode[i] == 4) flicker[i]=random(3,16);
        if(flicker_mode[i] > 4 && flicker[i] < 4) flicker[i]=random(4,16);
      }
    }
    color_shift[i][0]=random(1,10) * distance_from_strike;
    color_shift[i][1]=random(1,6) * distance_from_strike;
    color_shift[i][2]=random(1,3) * distance_from_strike;
  }
  end_of_seq_delay = random(500,3000) * disipated_delay;
    
 for (int l=0;l<no_of_strikes;l++){
    led_no = sequence[led_seq][l];
    //delay(no_flash[sequence[led_no]);
    polled_ms_delay(no_flash[led_no]);
    FastLED.clear();
    Red = 255/color_shift[l][0];
    Green = 255/color_shift[l][1];
    Blue = 255/color_shift[l][2];
    leds[led_no] = CRGB(Red,Green,Blue);
    FastLED.show();
    if(flash[led_no] > flicker_threshold){
      for (int i=0;i<flicker[led_no];i++){
        //delay(flicker_time[led_no]);
        polled_ms_delay(flicker_time[led_no]);
        FastLED.clear();
        leds[led_no] = CRGB(0,0,0);
        FastLED.show();
        //delay(flicker_time[led_no]);
        polled_ms_delay(flicker_time[led_no]);
        switch (flicker_mode[led_no]){
          case 0:                                                            // Fixed brightness flash
          Red = 255/color_shift[l][0];
          Green = 255/color_shift[l][1];
          Blue = 255/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 1:                                                           // increasing brightness
          intensity = ((255/flicker[led_no]) * (1+i)) - 1;
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 2:                                                           // decreasing brightness
          intensity = 255 - ((255/flicker[led_no]) * (i));
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 3:                                                           // increasing then decreasing brightness
          transistion = flicker[led_no]/2;
          if(i+1 <= transistion){
            intensity = ((255/transistion) * (1+i)) - 1;
          }
          if(i+1 > transistion){
            intensity = 255 - ((255/transistion) * (transistion-i));
          }
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 4:                                                           // decreasing then increasing brightness 
          transistion = flicker[led_no]/2;
          if(i+1 <= transistion){
            intensity = 255 - ((255/transistion) * (transistion-i));
          }
          if(i+1 > transistion){
            intensity = ((255/transistion) * (1+i)) - 1;
          }
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 5:                                                         // increasing, hold, then decreasing brightnes 
          transistion = flicker[led_no]/3;
          if(i+1 <= transistion){
            intensity = ((255/transistion) * (1+i)) - 1;
          }
          if((i+1 >= transistion) && (i+1 <= transistion*2)){
            intensity = 255 - ((255/transistion) /3);
          }
          if(i+1 >= transistion*2){
            intensity = 255 - ((255/transistion) * (transistion-i));
          }
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          case 6:                                                         // decreasing, hold, then increasing brightnes 
          transistion = flicker[led_no]/3;
          if(i+1 <= transistion){
            intensity = 255 - ((255/transistion) * (transistion-i));
          }
          if((i+1 >= transistion) && (i+1 <= transistion*2)){
            intensity = ((255/transistion) /3);
          }
          if(i+1 >= transistion*2){
            intensity = ((255/transistion) * (1+i)) - 1;
          }
          Red = intensity/color_shift[l][0];
          Green = intensity/color_shift[l][1];
          Blue = intensity/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
          default:                                                       // Fixed brightness flash 
          Red = 255/color_shift[l][0];
          Green = 255/color_shift[l][1];
          Blue = 255/color_shift[l][2];
          leds[led_no] = CRGB(Red,Green,Blue);
          break;
        }        
        FastLED.show();
      }
      //delay(flicker_time[led_no]);
      polled_ms_delay(flicker_time[led_no]);
    }
    if(flash[led_no] < flicker_threshold) {
      //delay(flash[led_no]);
      polled_ms_delay(flash[led_no]);
    }
    FastLED.clear();
    leds[led_no] = CRGB(0,0,0);    
    FastLED.show();
  }
  polled_ms_delay(end_of_seq_delay);

  /*FastLED.clear();
  leds[0] = CRGB(255,0,0);
  leds[1] = CRGB(0,255,0);
  leds[2] = CRGB(0,0,255);
  FastLED.show();*/
  
  /*leds[2] = CRGB(0,0,0);
  leds[0] = CRGB(255,255,255);
  FastLED.show();
  delay(10);
  leds[0] = CRGB(0,0,0);
  leds[1] = CRGB(255,255,255);
  FastLED.show();
  delay(10);
  leds[1] = CRGB(0,0,0);
  leds[2] = CRGB(255,255,255);
  FastLED.show();
  delay(10);*/

  /*for(int t=0;t<3;t++){
    
    leds[sequence[led_seq][t]] = CRGB(255,255,255);
    FastLED.show();
    delay(2000);
    leds[sequence[led_seq][t]] = CRGB(0,0,0);
  }*/

  /*millis_old = millis();
  leds[0] = CRGB(255,255,255);
  FastLED.show();
  while((millis() - millis_old) < 100);
  millis_old = millis();
  leds[0] = CRGB(0,0,0);
  FastLED.show();
  while((millis() - millis_old) < 100);*/

 /* for(int i=0;i<num_leds;i++){
    if(no_flash[i] + flash[i] > total_flash) total_flash = no_flash[i] + flash[i];
  }
  multi_flash_done =0;
  millis_old = millis();  
  while(!multi_flash_done){
    //led_no = sequence[led_seq][l];
    if((millis() < millis_old + no_flash[1]) || (millis() > millis_old + no_flash[1] + flash[1])){
      leds[1] = CRGB(0,0,0);
      //FastLED.show();
      //if(millis() > total_flash) multi_flash_done =1;
    }
    if((millis() < millis_old + no_flash[0]) || (millis() > millis_old + no_flash[0] + flash[0])){
      leds[0] = CRGB(0,0,0);
      //FastLED.show();
      //if(millis() > total_flash) multi_flash_done =1;
    }
    if((millis() < millis_old + no_flash[2]) || (millis() > millis_old + no_flash[2] + flash[2])){
      leds[2] = CRGB(0,0,0);
      //FastLED.show();
      //if(millis() > total_flash) multi_flash_done =1;
    }
    //FastLED.show();
    if((millis() >= millis_old + no_flash[1]) && (millis() <= millis_old + no_flash[1] + flash[1])){
      leds[1] = CRGB(255,255,255);
      //FastLED.show();
    }
    if((millis() >= millis_old + no_flash[0]) && (millis() <= millis_old + no_flash[0] + flash[0])){
      leds[0] = CRGB(255,255,255);
      //FastLED.show();
    }
    if((millis() >= millis_old + no_flash[2]) && (millis() <= millis_old + no_flash[2] + flash[2])){
      leds[2] = CRGB(255,255,255);
      //FastLED.show();
    }
    FastLED.show();
    if(millis() > total_flash) multi_flash_done =1;
  }*/

/*leds[0] = CRGB(0,0,0);
leds[1] = CRGB(0,0,0);
leds[2] = CRGB(0,0,0);
FastLED.show();
polled_ms_delay(750);
leds[1] = CRGB(255,255,255);
leds[2] = CRGB(255,255,255);
FastLED.show();
polled_ms_delay(500);
leds[0] = CRGB(255,255,255);
leds[1] = CRGB(0,0,0);
leds[2] = CRGB(255,255,255);
FastLED.show();
polled_ms_delay(2500);
leds[0] = CRGB(255,255,255);
leds[2] = CRGB(0,0,0);
leds[1] = CRGB(255,255,255);
FastLED.show();
polled_ms_delay(1250);
leds[0] = CRGB(255,255,255);
leds[1] = CRGB(0,0,0);
leds[2] = CRGB(255,255,255);
FastLED.show();
polled_ms_delay(2500);*/



}

void polled_ms_delay(uint32_t time){
  millis_old = millis();
  while(millis() < millis_old + time);  
}