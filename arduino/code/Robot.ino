/*
Copyright (c) 2013 WTFRobot

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * The name of the copyright holders may not be used to endorse or promote products
    derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall the Intel Corporation or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/
#include "Arduino.h"
#include <RobotParams.h>
#include <TimeInfo.h>
#include <OdometricLocalizer.h>
#include <SpeedController.h>
#include <BatteryMonitor.h>
#include <NewMotorCar.h> 
#include <Messenger.h>

#include <digitalWriteFast.h>  // library for high performance reads 
//  and writes by jrraines
// see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
// and http://code.google.com/p/digitalwritefast/

// It turns out that the regular digitalRead() calls are too slow and bring the arduino down when
// used in the interrupt routines while the motor is running at full speed creating more than
// 40000 encoder ticks per second per motor.




#define c_UpdateInterval 20      // update interval in milli seconds                         ϵͳʱ�������ֵ  20ms
//#define c_MaxMotorCV   30      // range is 0 ... 89 (half servo range)                   �����ת���Ƕ�  30��

#define MotorWorkingTime   20

// container for robot params wheel diameter [m], trackwidth [m], ticks per revolution     ����������ֱ�����־࣬ÿȦ�������
RobotParams _RobotParams = RobotParams();   //create robot params

TimeInfo _TimeInfo = TimeInfo();

NewMotorCar LMotor(13,12,11);
NewMotorCar RMotor(8,10,9);
//5,LPWM
//4,D
//6,RPWM
//7,D

//Servo _RightServo;  // create servo object to control right motor
//Servo _LeftServo;   // create servo object to control left motor

/////////////////////###############ʹ���ж�0��1������������ݶ�ȡ���庯������pin13,12,3,2��ȡ����AB����ֵ


// Quadrature encoders
// Left encoder
#define c_LeftEncoderInterrupt 0         //�жϺ�
#define c_LeftEncoderPinA 2              //����
#define c_LeftEncoderPinB 5
//#define LeftEncoderIsReversed            //�ж��Ƿ�����
volatile bool _LeftEncoderBSet;          //PinA�����أ������жϺ󣬼��PinB��ƽ����СҪôΪ0��ҪôΪ1
volatile long _LeftEncoderTicks = 0;     //��������

// Right encoder
#define c_RightEncoderInterrupt 1
#define c_RightEncoderPinA 3
#define c_RightEncoderPinB 6
#define RightEncoderIsReversed            //�ж��Ƿ�����
volatile bool _RightEncoderBSet;
volatile long _RightEncoderTicks = 0;    

////////////////////////////////////////////////###############�����˷�λ���ٶ�

OdometricLocalizer _OdometricLocalizer(&_RobotParams, &_TimeInfo);                   //��λ����

SpeedController _SpeedController(&_OdometricLocalizer, &_RobotParams, &_TimeInfo);                  //�ٶȵ���   ��Ҫ��λ��������ʱ��


#define c_ScaledBatteryVInPin A0// analog input pin for the battery voltage divider  //PIN A0  �ɼ���ص�ѹ


#define c_VInToVBatteryRatio 2.921/////////////////////////A6�ϲ�ĵ�ѹ����ʵ��Դ��ѹ�ı�ֵ

BatteryMonitor _BatteryMonitor(c_ScaledBatteryVInPin, c_VInToVBatteryRatio);

// Instantiate Messenger object with the message function and the default separator (the space character)
Messenger _Messenger = Messenger();


/////////////////////////////////////###################################���ñ�����
void SetupEncoders()
{
  // Quadrature encoders
  // Left encoder
  pinMode(c_LeftEncoderPinA, INPUT);      // sets pin A as input
  digitalWrite(c_LeftEncoderPinA, LOW);   // turn on pullup resistors
  pinMode(c_LeftEncoderPinB, INPUT);      // sets pin B as input
  digitalWrite(c_LeftEncoderPinB, LOW);   // turn on pullup resistors
  attachInterrupt(c_LeftEncoderInterrupt, HandleLeftMotorInterruptA, RISING);

  // Right encoder
  pinMode(c_RightEncoderPinA, INPUT);      // sets pin A as input
  digitalWrite(c_RightEncoderPinA, LOW);   // turn on pullup resistors
  pinMode(c_RightEncoderPinB, INPUT);      // sets pin B as input
  digitalWrite(c_RightEncoderPinB, LOW);   // turn on pullup resistors
  attachInterrupt(c_RightEncoderInterrupt, HandleRightMotorInterruptA, RISING); 
}



bool _IsInitialized = false;                  ///############��ʼ����0

void setup()
{

  Serial.begin(115200);                       //��������ͨ��

  SetupEncoders();                            //��������������

  _Messenger.attach(OnMssageCompleted);       //������Ϣ��ȡ  ����
  ///############
  // S����  ���ٶ���ת���ٶȿ�������

  ///############������ʼ������

  ///DriveGeometry
  ///SpeedControllerParams
  ///BatteryMonitorParams

  //_RightServo.attach(10);                     // attaches the servo on specified pin to the servo object   Servo ָ������PWM��� 
  //_LeftServo.attach(11);                      // attaches the servo on specified pin to the servo object   Servo ָ������PWM��� 

  /*
/////////////////////##########################��������ת��90��
   _RightServo.write(90);                      //��ת����90�ȡ�
   _LeftServo.write(90);
   */
  delay(100);                                 ////////��ʱ100ms         
  _TimeInfo.Update();                       //ϵͳ���θ���ʱ��
}

void loop()
{

  ReadSerial();                                       ////�������´�����Ϣ

  unsigned long milliSecsSinceLastUpdate = millis() - _TimeInfo.LastUpdateMillisecs;////�˿�ʱ���ȥϵͳ�ϴ�ʱ����£����뼶��
  if(milliSecsSinceLastUpdate >= c_UpdateInterval)    /////�ж�ʱ���Ƿ�ﵽ������ֵ
  {
    //Serial.println(milliSecsSinceLastUpdate);     

    // time for another update
    _TimeInfo.Update();                              ///////�ٴθ���ʱ��  

    if (_IsInitialized)                              ///////�ж�ϵͳ�Ƿ��ʼ�� ��ʼ��������������PC�����ʼ��
    {                                                ///////��һ��ϵͳδ��ʼ��
      DoWork();                                      ///////���� ʵʱ���� ��ִ���˶�����ٴη���ʵʱ���� 
    }
    else
    {
      RequestInitialization();                       //////��ϵͳδ��ʼ���ɹ����ٴ�����
    }
  }
}
///////////////////////////////////###################��������############################//////////////////////////////////////////////




/////////////###########################1111���·�λ����ص������ٶ�����

/////////////###########################2222�õ�����  ����С���˶����������źţ�

/////////////###########################3333�������ݣ���ص�������ǰλ�ã�x��y����ȫ�����꣩������Heading���Ƕȣ���ǰ���ٶ�Forword V��ת�ȽǶ�Omega

void DoWork()                       
{
  ///////////////####################��������
  _OdometricLocalizer.Update(_LeftEncoderTicks, _RightEncoderTicks);////Ticks�������ⲿ�жϵ��������ȡ

  _BatteryMonitor.Update();

  _SpeedController.Update(false);

  //_SpeedController.Update(_BatteryMonitor.VoltageIsTooLow);     ////�ٶȸ���
  ////��ص�����������ٶȵ��ڣ�����stop



  ///////////////####################ִ�п�������
  IssueCommands();



  ///////////////####################��ʼ��������   
  Serial.print("o"); // o indicates odometry message    ####    ͨ��Э�飺o������̼Ƶ�������

  Serial.print("\t");                                           //// ����֮��ո�

  ////С�������3λ��Ч����
  Serial.print(_OdometricLocalizer.X, 3);                       ////��x��y������ 
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.Y, 3);
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.Heading, 3);                 ////����
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.V, 3);                       ////ƽ���ٶ� 
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.Omega, 3);                   ////ת����ٶ�

  Serial.print("\n");

  Serial.print("b\t"); // b indicates battery info message ## ͨ��Э�飺b �����ص�������
  Serial.print(_BatteryMonitor.BatteryVoltage, 3);            ////С�������3λ��Ч����
  //Serial.print("\t");
  //Serial.print(_BatteryMonitor.VoltageIsTooLow);
  Serial.print("\n");
  /*
  Serial.print(_LeftEncoderTicks);
   Serial.print("\t");
   Serial.print(_RightEncoderTicks);
   Serial.print("\n");
   */
}


//////////////////////////////#######################�����ʼ��ϵͳ
//////////////////////////////��̲�����ʼ��
//////////////////////////////�ٶȲ�����ʼ��
//////////////////////////////��ز�����ʼ��
//////////////////////////////PC�˸���print�������ַ������Ӧ��ʼ��


void RequestInitialization()
{
  _IsInitialized = true;

  if (!_RobotParams.IsInitialized)
  {
    _IsInitialized = false;

    Serial.print("InitializeDriveGeometry"); // ##requesting initialization of the parameters of the differential drive needed for odometry calculations
    Serial.print("\n");
  }

  if (!_SpeedController.IsInitialized)
  {
    _IsInitialized = false;

    Serial.print("InitializeSpeedController"); //## requesting initialization of the speed controller
    Serial.print("\n");
  }

  if (!_BatteryMonitor.IsInitialized)
  {
    _IsInitialized = false;

    Serial.print("InitializeBatteryMonitor"); // ##requesting initialization of the battery monitor
    Serial.print("\n");
  }
}

///////////////////////////////#############################ִ������    �����������ת��
///////////////////////////////#############################��PI����õ��ĵ���˶�����ִ��
void IssueCommands()                 
{
  float normalizedRightMotorCV, normalizedLeftMotorCV;

  normalizedRightMotorCV = _SpeedController.NormalizedLeftCV;          ///PI���ں�õ�������������--ת��
  normalizedLeftMotorCV  = _SpeedController.NormalizedRightCV;



  // Serial.print(_SpeedController.CommandedVelocity - 0.2 * _SpeedController.CommandedAngularVelocity);
  // Serial.print("\t");
  // Serial.print(_SpeedController.CommandedVelocity + 0.2 * _SpeedController.CommandedAngularVelocity);
  //  Serial.print("\t");

  /*
  Serial.print(_SpeedController.LeftError);
   Serial.print("\t");
   Serial.print(_SpeedController.RightError);
   Serial.print("\t");
   Serial.print(_SpeedController.TurnError);
   Serial.print("\t");
   Serial.print(_LeftEncoderTicks);
   Serial.print("\t");
   Serial.print(_RightEncoderTicks);
   Serial.print("\t");
   Serial.print(_OdometricLocalizer._PreviousLeftEncoderCounts);
   Serial.print("\t");
   Serial.print(_RobotParams.DistancePerCount);
   Serial.print("\t");
   Serial.print(_TimeInfo.SecondsSinceLastUpdate);
   
   Serial.print("\n");
   */

  /////////#####################ӳ��        ��������ӳ�䵽���ת��ָ��ֵ    60�㵽120��
  float leftMotorValue = mapFloat(normalizedLeftMotorCV, -1.0, 1.0, -200.0, 200.0); 
  float rightMotorValue = mapFloat(normalizedRightMotorCV, -1.0, 1.0, -160.0, 160.0); 
  if(leftMotorValue>250||leftMotorValue<-250)
  leftMotorValue=0;
  if(rightMotorValue>250||rightMotorValue<-250)
  rightMotorValue=0;
  Serial.print("Speed: ");
  Serial.print(_SpeedController.CommandedVelocity);
  Serial.print("\t");
  Serial.print(_SpeedController.CommandedAngularVelocity);
  Serial.print("\t");
  Serial.print(_SpeedController.angularVelocityOffset);
  Serial.print("\t");
  Serial.print(_SpeedController.LeftError);
  Serial.print("\t");
  Serial.print(_SpeedController.RightError);
  Serial.print("\t");
  Serial.print(_SpeedController.TurnError);
  Serial.print("\t");
  Serial.print(normalizedLeftMotorCV);
  Serial.print("\t");
  Serial.print(normalizedRightMotorCV);
  Serial.print("\t");
  Serial.print(leftMotorValue);
  Serial.print("\t");
  Serial.print(rightMotorValue);
  Serial.print("\t");
  Serial.print(_LeftEncoderTicks);
  Serial.print("\t");
  Serial.print(_RightEncoderTicks);
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.VLeft,3);
  Serial.print("\t");
  Serial.print(_OdometricLocalizer.VRight,3);
  Serial.print("\t");
  Serial.print(abs(_OdometricLocalizer.VLeft)-abs(_OdometricLocalizer.VRight),3);
  Serial.print("\n");

   
  LMotor.work(leftMotorValue);
  RMotor.work(rightMotorValue);


  // _RightServo.write(rightServoValue);     // sets the servo position according to the scaled value (0 ... 179)    ִ�п���  ���ת��
  // _LeftServo.write(leftServoValue);       // sets the servo position according to the scaled value (0 ... 179)
}


// Interrupt service routines for the left motor's quadrature encoder      ####PinA�����أ������жϣ������������������ PinB
void HandleLeftMotorInterruptA()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  _LeftEncoderBSet = digitalReadFast(c_LeftEncoderPinB);   // read the input pin

  // and adjust counter + if A leads B
#ifdef LeftEncoderIsReversed
  _LeftEncoderTicks -= _LeftEncoderBSet ? -1 : +1;
#else
  _LeftEncoderTicks += _LeftEncoderBSet ? -1 : +1;
#endif
}


// Interrupt service routines for the right motor's quadrature encoder    ####PinA�����أ������жϣ������������������ PinB
void HandleRightMotorInterruptA()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  _RightEncoderBSet = digitalReadFast(c_RightEncoderPinB);   // read the input pin             

  // and adjust counter + if A leads B
#ifdef RightEncoderIsReversed
  _RightEncoderTicks -= _RightEncoderBSet ? -1 : +1;
#else
  _RightEncoderTicks += _RightEncoderBSet ? -1 : +1;
#endif
}


////////////////////////####################�����ڻ�ȡ�˶�ָ��  
void ReadSerial()
{
  while (Serial.available())
  {
    _Messenger.process(Serial.read());
  }
}


// Define messenger function     #############       ����ͨ�ź���      ��ȡ��ʼ������ ��ʼ�������� 
void OnMssageCompleted()       /////////////////#####��� ����PC�˹�������Ϣ
{                                            ///////////////�ȼ��ؼ���
  if (_Messenger.checkString("s"))
  {
    SetSpeed();
    ClearOutMessenger();    
    return;
  }

  if (_Messenger.checkString("DriveGeometry"))
  {
    InitializeDriveGeometry();
    ClearOutMessenger();    
    return;
  }

  if (_Messenger.checkString("SpeedControllerParams"))
  {
    InitializeSpeedControllerParams();
    ClearOutMessenger();
    return;
  }

  if (_Messenger.checkString("BatteryMonitorParams"))
  {
    InitializeBatteryMonitor();
  }

  // clear out unrecognized content               
  ClearOutMessenger();
}

void ClearOutMessenger()
{
  while (_Messenger.available())
  {
    _Messenger.readChar();
  }
}


////////////////###################�ٶ��趨    ��С������ת���ٶ�
void SetSpeed()
{
  float commandedVelocity         = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());  ////��PC�� ��ȡ�ٶ�������� ƽ���ٶȣ�ת����ٶ�
  float commandedAngularVelocity  = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());

  ///_OdometricLocalizer.CommandVelocity(commandedVelocity, commandedAngularVelocity);
  //////////////////////////PID���ڿ�������
  _SpeedController.CommandVelocity(commandedVelocity, commandedAngularVelocity); 
}
//////////////######################  ��PC�˻�ȡ�������������ֱ��0.0762m   �־�0.37m  ÿȦ�ļ���   19.5*500*2=19500 
/////////////set robot params wheel diameter [m], trackwidth [m], ticks per revolution
void InitializeDriveGeometry()
{
  float wheelDiameter       = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  float trackWidth          = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  int   countsPerRevolution = _Messenger.readInt();

  _RobotParams.Initialize(wheelDiameter, trackWidth, countsPerRevolution);     //��ʼ��
  

   
}

////#######################################��ʼ���ٶȿ���ϵͳ����        ��PC��ͨ�����ڣ���Messenger��ȡ������õ�PID������Ҫ�Ĳ���
void InitializeSpeedControllerParams()
{
  float velocityPParam = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  float velocityIParam = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  float turnPParam     = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  float turnIParam     = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  float commandTimeout = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());

  _SpeedController.Initialize(velocityPParam, velocityIParam, turnPParam, turnIParam, commandTimeout);////  ���ò���  ��ʼ���ٶȿ���ϵͳ


}

////#######################################��ʼ����ؼ���   �õ����Ƶ�ص�ѹ
void InitializeBatteryMonitor()
{
  float voltageTooLowlimit = GetFloatFromBaseAndExponent(_Messenger.readInt(), _Messenger.readInt());
  _BatteryMonitor.InitializeLowVoltageLimit(voltageTooLowlimit);

  /*
  Serial.print("battery monitor Params: ");
   Serial.print(voltageTooLowlimit);
   Serial.print("\n");
   */
}

//////#############################
float GetFloatFromBaseAndExponent(int base, int exponent)
{
  return base * pow(10, exponent);         /////////////#############������ʽ�õ�10Ϊ�ף�exponentΪ�ݣ��ٳ�base��float
}
/////////////////////////##################ӳ�亯��

long mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; /////////////#############������ʽ
}





