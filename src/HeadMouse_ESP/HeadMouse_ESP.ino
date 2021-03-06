
#include <BleMouse.h>
//Carrega a biblioteca Wire
#include <Wire.h>
#include "filters.h"
#include "MahonyAHRS.h"
#include"mpu6050.h"
//---------------------------------------------------------------------------------------------------
//Definitions
//Mouse
#define SENSITIVITY  35;
//
#define MPU6050_ACC_GAIN 16384.0
#define MPU6050_GYRO_GAIN 131.072 
/*********************************************************************
 * Global variables
 */
float axR, ayR, azR, gxR, gyR, gzR;
float axg, ayg, azg, gxrs, gyrs, gzrs;
//float ax_filtro, ay_filtro, az_filtro, gx_filtro, gy_filtro, gz_filtro;
bool conectado = false;
//float yaw_filtro, pitch_filtro, roll_filtro;
const int MPU_addr=0x68;  // I2C address of the MPU-6050
/*********************************************************************
 * External variables
 */
extern int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
extern float yaw_mahony,pitch_mahony,roll_mahony;
//Objects
BleMouse bleMouse;
filters pbax,pbay,pbaz,pbgx,pbgy,pbgz;

void filtraIMU()
{
  //Filtro 2 ordem
  // ax_filtro= filtro_2PB10Hz(AcX, pbax);
  // ay_filtro= filtro_2PB10Hz(AcY, pbay);
  // az_filtro= filtro_2PB10Hz(AcZ, pbaz);
  // gx_filtro= filtro_2PB10Hz(GyX, pbgx);
  // gy_filtro= filtro_2PB10Hz(GyY, pbgy);
  // gz_filtro= filtro_2PB10Hz(GyZ, pbgz);
  
  //Converte
  axg = (float)(AcX /*- LSM6DSM_AXOFFSET*/) / MPU6050_ACC_GAIN;
  ayg = (float)(AcY /*- LSM6DSM_AYOFFSET*/) / MPU6050_ACC_GAIN;
  azg = (float)(AcZ /*- LSM6DSM_AZOFFSET*/) / MPU6050_ACC_GAIN;
  gxrs = (float)(GyX - (-143)) / MPU6050_GYRO_GAIN * 0.01745329; //degree to radians
  gyrs = (float)(GyY - (245)) / MPU6050_GYRO_GAIN * 0.01745329; //degree to radians
  gzrs = (float)(GyZ - (25)) / MPU6050_GYRO_GAIN * 0.01745329; //degree to radians
  // Degree to Radians Pi / 180 = 0.01745329 0.01745329251994329576923690768489
}
uint8_t mouseHoriz(void)
{
  static float horzZero =0.0f;
  static float horzValue = 0.0f;  // Stores current analog output of each axis
  

  horzValue = (yaw_mahony - horzZero)*SENSITIVITY;
  horzZero = yaw_mahony;
  
  return horzValue;
}
uint8_t mouseVert(void)
{
  static float vertZero =0.0f;
  static float vertValue = 0.0f;  // Stores current analog output of each axis
 
  vertValue = (pitch_mahony - vertZero)*SENSITIVITY;
  vertZero = pitch_mahony;

  return vertValue;
}
#define ACIONADOR 34
void setup() 
{
  Serial.begin(115200);
  //Serial.println("Starting BLE work!");
  bleMouse.begin();
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  MPU6050_Init();
  pinMode(ACIONADOR,INPUT);
}
enum
{
  Estado_LeBotao = 0,
  Estado_Clica,
};
void loop() 
{
  static int printDivider = 10;
  static int estado = 0;
  uint8_t xchg = 0,ychg = 0;
  bool estadoAcionador = false;
  static int contador = 0;

  estadoAcionador = digitalRead(ACIONADOR);
  mpu6050_GetData();
  filtraIMU();
  MahonyAHRSupdateIMU( gxrs,  gyrs,  gzrs , axg,  ayg,  azg);
  getRollPitchYaw_mahony();
  xchg = mouseHoriz();
  ychg = mouseVert();
  
  if(bleMouse.isConnected()) 
  {
    switch (estado)
    {
      case Estado_LeBotao:
        if(estadoAcionador  == false)
        {
          contador++;
          if(contador >= 10)
          {
            estado = Estado_Clica;
            contador = 0;
          }
        }
        else
        {
          contador = 0; 
        }
        break;
      case Estado_Clica:
        if(estadoAcionador  == true)
        {
          bleMouse.click(MOUSE_LEFT);
          estado = Estado_LeBotao;
        }
        break;
      
      default:
        break;
    }
    
   
    bleMouse.move(xchg,ychg,0);
  }
  else
  {
    conectado = false;
  }
 
  if(--printDivider ==0)
  {
    printDivider = 10;
    // Serial.print(pitch_mahony);
    // Serial.print(" ");
    // Serial.print(yaw_mahony);
    Serial.print(AcX);
    Serial.print(" ");
    Serial.print(AcY);
    Serial.print(" ");
    Serial.print(AcZ);
    Serial.print(" ");
    Serial.print(GyX);
    Serial.print(" ");
    Serial.print(GyY);
    Serial.print(" ");
    Serial.print(GyZ);
    Serial.println("");
  }
    
//  delay(10);
}
