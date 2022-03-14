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
*********************************************************************************************************/

#include <arduinoFFT.h>
#include <driver/i2s.h>


#define I2S_WS    33
#define I2S_SD    25
#define I2S_SCK   26

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int SAMPLE_BLOCK = 64;
const int SAMPLE_FREQ = 10240;

int gain=40;  //adjust it to set the gain   
uint16_t audio_data;                            

const uint16_t samples = 512;
unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[samples];
double vImag[samples];
double fft_bin[samples];

double fft_data[32];
int fft_result[32]; 

//adjust single frequency curves.
double fft_freq_boost[32] = {1.02,1.03,1.04,1.06,1.08,1.10,1.10,1.11,1.12,1.13,1.15,1.16,1.17,1.18,1.20,1.21,1.30,1.51,2.11,2.22,3.25,3.26,3.52,3.55,4.22,4.24,5.52,5.55,6.53,6.55,8.82,8.88};

TaskHandle_t fft_task;

arduinoFFT FFT = arduinoFFT( vReal, vImag, samples, SAMPLE_FREQ);

double fft_add( int from, int to) {
  int i = from;
  double result = 0;
  while ( i <= to) {
    result += fft_bin[i++];
  }
  return result;
}

void fft_code( void * parameter) {

  for(;;) {

    delay(1);          

    microseconds = micros();

    for(int i=0; i<samples; i++) {
      int32_t digitalSample = 0;
      int bytes_read = i2s_pop_sample(I2S_PORT, (char *)&digitalSample, portMAX_DELAY); // no timeout
      if (bytes_read > 0) {
        audio_data = abs(digitalSample >> 16);
      }
      vReal[i] = audio_data; 
      vImag[i] = 0;
      microseconds += sampling_period_us;
    }

    FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_FORWARD ); 
    FFT.Compute( FFT_FORWARD ); 
    FFT.ComplexToMagnitude(); 

    for (int i = 0; i < samples; i++) {   
      double t = 0.0;
      t = abs(vReal[i]);
      t = t / 16.0;    
      fft_bin[i] = t;
    }

    fft_data[0]  = (fft_add(3,4))/2;       // 60-100Hz
    fft_data[1]  = (fft_add(4,5))/2;       // 80-120Hz
    fft_data[2]  = (fft_add(5,6))/2;       // 100-140Hz
    fft_data[3]  = (fft_add(6,7))/2;       // 120-160Hz
    fft_data[4]  = (fft_add(7,8))/2;       // 140-180Hz
    fft_data[5]  = (fft_add(8,9))/2;       // 160-200Hz
    fft_data[6]  = (fft_add(9,10))/2;      // 180-220Hz
    fft_data[7]  = (fft_add(10,11))/2;     // 200-240Hz
    fft_data[8]  = (fft_add(11,12))/2;     // 220-260Hz
    fft_data[9]  = (fft_add(12,13))/2;     // 240-280Hz
    fft_data[10] = (fft_add(13,14))/2;     // 260-300Hz
    fft_data[11] = (fft_add(14,16))/3;     // 280-340Hz
    fft_data[12] = (fft_add(16,18))/3;     // 320-380Hz
    fft_data[13] = (fft_add(18,20))/3;     // 360-420Hz
    fft_data[14] = (fft_add(20,24))/5;     // 400-500Hz
    fft_data[15] = (fft_add(24,28))/5;     // 480-580Hz
    fft_data[16] = (fft_add(28,32))/5;     // 560-660Hz
    fft_data[17] = (fft_add(32,36))/5;     // 640-740Hz
    fft_data[18] = (fft_add(36,42))/7;     // 720-860Hz
    fft_data[19] = (fft_add(42,48))/7;     // 840-980Hz
    fft_data[20] = (fft_add(48,56))/9;     // 960-1140Hz
    fft_data[21] = (fft_add(56,64))/9;     // 1120-1300Hz
    fft_data[22] = (fft_add(64,74))/11;    // 1280-1500Hz
    fft_data[23] = (fft_add(74,84))/11;    // 1480-1700Hz
    fft_data[24] = (fft_add(84,97))/14;    // 1680-1960Hz
    fft_data[25] = (fft_add(97,110))/14;   // 1940-2240Hz
    fft_data[26] = (fft_add(110,128))/19;  // 2200-2580Hz
    fft_data[27] = (fft_add(128,146))/19;  // 2560-2940Hz
    fft_data[28] = (fft_add(146,170))/25;  // 2920-3420Hz
    fft_data[29] = (fft_add(170,194))/25;  // 3400-3900Hz
    fft_data[30] = (fft_add(194,224))/31;  // 3880-4500Hz
    fft_data[31] = (fft_add(224,255))/32;  // 4520-5120Hz

    //adjust single frequency curves.
    for (int i=0; i < 32; i++) {
      fft_data[i] = fft_data[i] * fft_freq_boost[i];
    }

    //adjust overall frequency curves.
    for (int i=0; i < 32; i++) {
      fft_data[i] = fft_data[i] * gain / 50;
    }

    //constraint function
    for (int i=0; i < 32; i++) {
      fft_result[i] = constrain((int)fft_data[i],0,255);
    }
  }
}

void audio_receive() {
    
  esp_err_t err;
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), 
    .sample_rate = SAMPLE_FREQ * 2,                       // 10240*2=20480Hz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, 
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8, 
    .dma_buf_len = SAMPLE_BLOCK
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1, 
    .data_in_num = I2S_SD 
  };
  
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");
  delay(100);
  
  sampling_period_us = round(1000000*(1.0/SAMPLE_FREQ));
  
  xTaskCreatePinnedToCore(
        fft_code,                    //Task function
        "FFT",                       //Task name
        10000,                       //Task stack size(units:byte)
        NULL,                        //Parameters passed to the task function
        1,                           //Task priority
        &fft_task,                   //Task handle
        0);                          //Specifies the core to run the task
}

