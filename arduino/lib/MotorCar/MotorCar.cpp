#include "Arduino.h"
#include "MotorCar.h"
/*
MotorCar�ຯ��
����:��������
����:
       _slpin,��������
       _dlpin,������
       _srpin,��������
       _drpin,�ҷ�����
*/
MotorCar::MotorCar(int _slpin,int _dlpin,int _srpin,int _drpin)
{
  _speedLeftPin=_slpin;
  _speedRightPin=_srpin;
  _dirLeftPin=_dlpin;
  _dirRightPin=_drpin;
  pinMode(_speedLeftPin,OUTPUT);
  pinMode(_speedRightPin,OUTPUT);
  pinMode(_dirLeftPin,OUTPUT);
  pinMode(_dirRightPin,OUTPUT);
}
/*
forward�˺���---ǰ��
����:ǰ��
   ���� :_speed--ǰ���ٶ�,��Χ0~255
*/
void MotorCar::forward(int _speed)
{
  digitalWrite(_dirRightPin,HIGH);//
  digitalWrite(_dirLeftPin,HIGH);//
  analogWrite(_speedRightPin,_speed);//
  analogWrite(_speedLeftPin,_speed);//
}
 /*
back�˺���---����
����:����
   ���� :_speed--�����ٶ�,��Χ0~255
*/
void MotorCar::back(int _speed)
{
  digitalWrite(_dirRightPin,LOW);//
  digitalWrite(_dirLeftPin,LOW);//
  analogWrite(_speedRightPin,_speed);//
  analogWrite(_speedLeftPin,_speed);//
}
  /*
turnLeft�˺���--��ǰת
����:��ת
   ���� :_speed--��ת�ٶ�,��Χ0~255
*/
void MotorCar::turnLeft(int _speed)
{
  digitalWrite(_dirRightPin,HIGH);//
  analogWrite(_speedRightPin,_speed);//
  
  analogWrite(_speedLeftPin,0);//
}
  /*
turnLeft2�˺���--���ת
����:��ת
   ���� :_speed--��ת�ٶ�,��Χ0~255
*/
void MotorCar::turnLeft2(int _speed)
{
  digitalWrite(_dirRightPin,LOW);//
  analogWrite(_speedRightPin,_speed);//
  
  analogWrite(_speedLeftPin,0);//
}
   /*
turnRight�˺���--��ǰת
����:��ת
    ���� :_speed--��ת�ٶ�,��Χ0~255
*/
void MotorCar::turnRight(int _speed)
{
  digitalWrite(_dirLeftPin,HIGH);//
  analogWrite(_speedLeftPin,_speed);//
  
  analogWrite(_speedRightPin,0);//
}
   /*
turnRight2�˺���--�Һ�ת
����:��ת
    ���� :_speed--��ת�ٶ�,��Χ0~255
*/
void MotorCar::turnRight2(int _speed)
{
  digitalWrite(_dirLeftPin,LOW);//
  analogWrite(_speedLeftPin,_speed);//
  
  analogWrite(_speedRightPin,0);//
}
   /*
turnLeftOrigin�˺���---ԭ����ת
����:ԭ����ת
    ���� :_speed--ԭ����ת�ٶ�,��Χ0~255
*/
void MotorCar::turnLeftOrigin(int _speed)
{
  digitalWrite(_dirRightPin,HIGH);//
  digitalWrite(_dirLeftPin,LOW);//
  analogWrite(_speedLeftPin,_speed);//
  analogWrite(_speedRightPin,_speed);//
}
   /*
turnRightOrigin�˺���---ԭ����ת
����:ԭ����ת
   ���� :_speed--ԭ����ת�ٶ�,��Χ0~255
*/
void MotorCar::turnRightOrigin(int _speed)
{
  digitalWrite(_dirRightPin,LOW);//
  digitalWrite(_dirLeftPin,HIGH);//
  analogWrite(_speedLeftPin,_speed);//
  analogWrite(_speedRightPin,_speed);//
}
   /*
stop�˺���---ֹͣ
����:ֹͣ
    ���� :��
*/
void MotorCar::stop()
{
  analogWrite(_speedLeftPin,0);//
  analogWrite(_speedRightPin,0);//
} 