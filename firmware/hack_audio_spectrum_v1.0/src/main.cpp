/*********************************************************************************************************
  HackLabs_Audio_Spectrum                                                                               
  ********************************************************************************************************
  Author: HACK实验室                                                                                     
  YouTubeID: HACK实验室, welcome to subscribe：https://www.youtube.com/channel/UCxFY1FcIYK9d7riTvIh6eiA                                                          
  Version 1.0 
  Hack_Audio_Spectrum is a free download and may be used, modified, evaluated and
  distributed without charge provided the user adheres to version hree of the GNU
  General Public License (GPL) and does not remove the copyright notice or this
  text. The GPL V3 text is available on the gnu.org web site

  作者：HACK实验室                                                                                        
  B站ID：HACK实验室  欢迎订阅：https://space.bilibili.com/395145107                                                                             
  微信公众号：HACK实验室，开源资料唯一发布点，定期分享开源硬件以及有价值的技术文章，欢迎关注.                        
  Version 1.0                                                                                           
  Hack_Audio_Spectrum 是一个完全开源的硬件项目，允许用户免费下载使用，但需遵守GPL V3开源协议，协议文本可在开源文件根目录或gnu.org网站上获得.  

  *如果你想调节LED亮度，请Ctrl+F搜索“FastLED.setBrightness(50)”，可将50改为0~255之间的任意数，但请注意电流，过亮可能会导致你的电源供电不足引起死机.
  *如果你想调节频谱的频响曲线，请到data.h中Ctrl+F搜索“fft_freq_boost[32]”，调节数组内的倍数来调节各个频段的增益.
  *如果你想调节频谱整体的增益，请到data.h中Ctrl+F搜索“gain=40”，更改这个值来适应你的音频环境.
*********************************************************************************************************/
#include "data.h"
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>


#define DATA_PIN     27
#define BUTTON_PIN    2

#define MATRIX_WIDTH     32
#define MATRIX_HEIGHT    16
#define NUM_LEDS    (MATRIX_WIDTH * MATRIX_HEIGHT)

cLEDMatrix<-MATRIX_WIDTH, -MATRIX_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX> leds;

uint8_t num_bands = 32;
uint8_t pattern= 0;
uint8_t color_timer = 0;

uint8_t bar_height[]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t peak_height[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t prev_fft_value[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Color Palette
DEFINE_GRADIENT_PALETTE(green_to_red) {
  0,   173,   255,    47,    //green
127,   255,   218,     0,    //yellow
255,   231,     0,     0 };  //red
DEFINE_GRADIENT_PALETTE(purple_to_blue) {
  0,   141,     0,   100,    //purple
127,   255,   192,     0,    //yellow
255,     0,     5,   255 };  //blue
DEFINE_GRADIENT_PALETTE(red_to_mistyrose) {
  0,   255,   228,   225,    //MistyRose
 64,   255,    69,     0,    //OrangeRed
127,   255,     0,     0,    //red
128,   255,     0,     0,    //red
192,   255,    69,     0,    //OrangeRed
255,   255,   228,   225 };  //MistyRose
CRGBPalette16 green_red_palette = green_to_red;
CRGBPalette16 purple_blue_palette = purple_to_blue;
CRGBPalette16 red_mistyrose_palette = red_to_mistyrose;
// end Color Palette

// bars patterns
void green_red_bars(int band, int bar) {
  for (int y = 0; y < bar; y++) {
    leds(band,y) = ColorFromPalette(green_red_palette, y * (255 / bar));
  }
}
void rainbow_bars(uint8_t band, uint8_t bar) {
  for (int y = 0; y <= bar; y++) {
    leds(band,y) = CHSV( (band*(255/num_bands)), 255, 255);
  }
}
void half_rainbow_bars(uint8_t band, uint8_t bar) {
  if ((band%2)==0){
    for (int y = 0; y <= bar; y++) {
    leds(band,y) = CHSV( (band*(255/num_bands)), 255, 255);
    }
  }
}
void center_bars(int band, int bar) {
  if (bar % 2 == 0) bar--;
  int y_start = ((MATRIX_HEIGHT - bar) / 2 );
  for (int y = y_start; y <= (y_start + bar); y++) {
    int color_index = constrain((y - y_start) * (255 / bar), 0, 255);
    leds(band,y) = ColorFromPalette(red_mistyrose_palette, color_index);
  }
}
void changing_bars(int band, int bar) {
  for (int y = 0; y < bar; y++) {
    leds(band,y) = CHSV(y * (255 / MATRIX_HEIGHT) + color_timer, 255, 255); 
  }
}// end bars patterns

// peaks patterns
void yellow_white_peak(int band) {
  leds(band, (int)peak_height[band]) = CRGB::White;
  if (peak_height[band] > 0) 
    leds(band,0) = CRGB::GreenYellow;
}
void white_peak(int band) {
  leds(band, (int)peak_height[band]) = CRGB::White;
}
void half_white_peak(int band) {
  if ((band%2)==0){
    leds(band, (int)peak_height[band]) = CRGB::White;
  }
}
void changing_peak(int band) {
  leds(band,(int)peak_height[band]) = ColorFromPalette(purple_blue_palette, peak_height[band] * (255 / MATRIX_HEIGHT));
}// end peaks patterns


// the setup function runs once when you press reset or power the board
void setup() {

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds[0], NUM_LEDS);
  FastLED.setBrightness(50);  //0~255
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); //Set the button pin pull-up input mode
  audio_receive();

}// end setup 

// the loop function runs over and over again forever
void loop() {

  FastLED.clear();
  
  for (int i = 0; i < 32; i=i+1) {
    uint8_t fft_value;
    fft_value = fft_result[i];
    fft_value = ((prev_fft_value[i] * 3) + fft_value) / 4;   
    bar_height[i] = fft_value / (255 / (MATRIX_HEIGHT-1));       // scale bar height
    if (bar_height[i] > peak_height[i])                          // peak up
      peak_height[i] = min((MATRIX_HEIGHT-1), (int)bar_height[i]);
    prev_fft_value[i] = fft_value;        
  }

  for (int band = 0; band < num_bands; band++) {

    // Draw bars
    switch (pattern) {
      case 0:
        green_red_bars(band, bar_height[band]);
        break;
      case 1:
        rainbow_bars(band, bar_height[band]);
        break;
      case 2:
        half_rainbow_bars(band, bar_height[band]);
        break;
      case 3:
        center_bars(band, bar_height[band]);
        break;
      case 4:
        changing_bars(band, bar_height[band]);
        EVERY_N_MILLISECONDS(10) { color_timer++; }
        break;
    }
    // Draw peaks
    switch (pattern) {
      case 0:
        yellow_white_peak(band);
        break;
      case 1:
        white_peak(band);
        break;
      case 2:
        half_white_peak(band);
        break;
      case 3:
        // without peak
        break;
      case 4:
        changing_peak(band);
        break;
    }
  }

  EVERY_N_MILLISECONDS(100) {
    for (uint8_t band = 0; band < num_bands; band++)
      if (peak_height[band] > 0) peak_height[band] -= 1;
  }

  FastLED.show();
    
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(80);
    if (digitalRead(BUTTON_PIN) == LOW) {   
      pattern++;
      delay(80);
    }
  }
  if (pattern > 4) {
    pattern = 0;
  }

}// end loop