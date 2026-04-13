//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <LSM6DSO32Sensor.h>
#include <SPI.h>

#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK 12
#define SPI_CS 15
#define IMU_INT1 10


SPIClass dev_spi(HSPI);
LSM6DSO32Sensor IMU(&dev_spi, SPI_CS);

void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]);
void IRAM_ATTR fifo_interrupt();

volatile bool interrupt_flag = false;

typedef struct {
  int32_t accelerometer[3]; 
  int32_t gyroscope[3];
  uint8_t timestamp[6];
} FIFO_Sample;

typedef struct {
  FIFO_Sample fifo_batch[148]; //3072 bytes / () = 114
  int index = 0; 
} FIFO_BATCH;


FIFO_BATCH fifo;

void restart_fifo();

void setup()
{
  pinMode(IMU_INT1, INPUT);
  
  Serial.begin(115200);

  dev_spi.begin(SPI_MISO, SPI_MOSI, SPI_SCK);
  
  IMU.begin();	
  IMU.Set_X_FS(LSM6DSO32_32g);
  IMU.Set_G_FS(LSM6DSO32_2000dps); 

  IMU.Enable_X();  
  IMU.Enable_G();

  IMU.Set_X_ODR(6667.0f); //max
  IMU.Set_G_ODR(6667.0f); 

  //fifo
  IMU.Set_FIFO_X_BDR(6667.0f); //max
  IMU.Set_FIFO_G_BDR(6667.0f);
  IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE); 

  //interrupt
  //IMU.Set_FIFO_INT1_FIFO_Full(true);
  IMU.Set_FIFO_Watermark_Level(300); //num da capire meglio 

  IMU.Write_Reg(LSM6DSO32_INT1_CTRL, 0x08); // Watermark interrput
  IMU.Write_Reg(LSM6DSO32_CTRL10_C, 0x20); // Timestamp fifo

  attachInterrupt(digitalPinToInterrupt(IMU_INT1), fifo_interrupt, RISING);
    
}

void loop()
{
  if (interrupt_flag){
    interrupt_flag = false;
    fifo.index = 0; //reset batch index

    uint16_t nSamples;
    IMU.Get_FIFO_Num_Samples(&nSamples);

    bool X_received{false}, G_received{false}, timestamp_received{false};

    for (int i = 0; i < nSamples; i++){
      uint8_t tag = 0;    
      IMU.Get_FIFO_Tag(&tag);
      
      if (tag == LSM6DSO32_XL_NC_TAG){
        IMU.Get_FIFO_X_Axes(fifo.fifo_batch[fifo.index].accelerometer);
        X_received = true;
        
        if (G_received && timestamp_received){ //se è l'ultimo af essere ricevuto incrementa indice 
          fifo.index++;
          X_received = false;
          G_received = false;
          timestamp_received = false;
        }
      
      }
      else if (tag == LSM6DSO32_GYRO_NC_TAG){
        IMU.Get_FIFO_G_Axes(fifo.fifo_batch[fifo.index].gyroscope);
        G_received = true;

        if (X_received && timestamp_received){
          fifo.index++;
          X_received = false;
          G_received = false;
          timestamp_received = false;
        }
      }
      else if (tag == LSM6DSO32_TIMESTAMP_TAG){
        IMU.Get_FIFO_Data(fifo.fifo_batch[fifo.index].timestamp);  

        timestamp_received = true;
      
        if (X_received && G_received){
          fifo.index++;
          X_received = false;
          G_received = false;
          timestamp_received = false;
        }
      
      }
      //every three samples (the ones listed) proceed to next batch

    }

    //print only the first sample
    print_data(fifo.fifo_batch[0].accelerometer, fifo.fifo_batch[0].gyroscope, fifo.fifo_batch[0].timestamp);
    
  }


  
  //appena finisce di bruciare il motore
  if (false){
    IMU.Set_X_FS(LSM6DSO32_4g);
    IMU.Set_G_FS(LSM6DSO32_500dps); 
    restart_fifo();
  }
  


}

void IRAM_ATTR fifo_interrupt(){
  interrupt_flag = true;
}


void restart_fifo(){
    IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE); 

    IMU.Disable_X();  
    IMU.Disable_G();

    IMU.Enable_X();  
    IMU.Enable_G();

    IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE); 
}

void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]){
  Serial.print("| Acc[mg]: ");
  Serial.print(accelerometer[0]);
  Serial.print(" ");
  Serial.print(accelerometer[1]);
  Serial.print(" ");
  Serial.print(accelerometer[2]);
  Serial.print(" | Gyr[mdps]: ");
  Serial.print(gyroscope[0]);
  Serial.print(" ");
  Serial.print(gyroscope[1]);
  Serial.print(" ");
  Serial.print(gyroscope[2]);

  uint32_t ts = (timestamp[3] << 24) | (timestamp[2] << 16) | (timestamp[1] << 8) | timestamp[0];

  Serial.print("Timestamp: ");
  Serial.println((float)ts / 1000 * 25); //converti in ms 

  Serial.println(" |");
}
