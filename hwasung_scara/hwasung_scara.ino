//아두이노 씨발 마지막코드
#include <Servo.h>
#define EN 8
#define C_EN 26
#define X_STP 2
#define Y_STP 3
#define Z_STP 4
#define W_STP 12
#define C1_STP 40
#define C2_STP 42

#define X_DIR 5
#define Y_DIR 6
#define Z_DIR 7
#define W_DIR 13
#define C1_DIR 41
#define C2_DIR 43

#define X_photo 9
#define X_photo_init A2  
#define Y_photo 10
#define Z_photo 11
#define W_photo A3
#define servoPin A0

#define TRIG 22 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 24 //ECHO 핀 설정 (초음파 받는 핀)
#define TRIG_start 28 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO_start 30 //ECHO 핀 설정 (초음파 받는 핀)
//근접센서
#define stop_sensor 32 // 시그널 회로핀을 sensor 변수에 저장

Servo myservo;

// 아날로그 0번핀 사용        
int angle = 0;
//serial_num받아오는 변수
int serial_num = 0;
int serial_box_num = 0;
int serial_stop = 0;
int mode_select_flag = 0;
int init_flag = 0;
//스피드 설정
int x_speed = 700;
int y_speed = 800;
int z_speed = 700;
int w_speed = 800;
int c_speed = 1000;

void setup()
{
    //connect(영준수정)
    Serial.begin(9600);
    Serial.setTimeout(50);
    /////////limit switch///////////
    pinMode(X_photo, INPUT);
    pinMode(Y_photo, INPUT);
    pinMode(Z_photo, INPUT);
    pinMode(W_photo, INPUT);
    ////////step motor//////////
    pinMode(X_DIR, OUTPUT); pinMode(X_STP, OUTPUT);
    pinMode(Y_DIR, OUTPUT); pinMode(Y_STP, OUTPUT);
    pinMode(Z_DIR, OUTPUT); pinMode(Z_STP, OUTPUT);
    pinMode(W_DIR, OUTPUT); pinMode(W_STP, OUTPUT);
    pinMode(C1_DIR, OUTPUT); pinMode(C1_STP, OUTPUT);
    pinMode(C2_DIR, OUTPUT); pinMode(C2_STP, OUTPUT);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    pinMode(TRIG_start, OUTPUT);
    pinMode(ECHO_start, INPUT);
    pinMode(EN, OUTPUT);
    pinMode(C_EN, OUTPUT);
    digitalWrite(EN, LOW);
    digitalWrite(C_EN, LOW);
    ////////servo motor
    myservo.attach(servoPin, 500, 2500);
    //근접센서
    pinMode(stop_sensor, INPUT);
    init_flag = 1;
}
int count =0;
void loop()
{
           serial_num = Serial.read();
           if(serial_num == 10)
           {
              mode_select_flag = 1;
           }
           else if(serial_num ==20)
           {
              mode_select_flag = 2;
              init_flag = 1;
           }
           switch (mode_select_flag){
            case 1:
              if( init_flag == 1)
              {
                To_init();
                init_flag = 0;
              }
              AutoMode();
              break;
            case 2:
              if(serial_num == 11)
              {
                x_cw_run();
              }
              else if(serial_num == 12)
              {
                x_ccw_run();
              }
              else if(serial_num == 21)
              {
                y_cw_run();
              }
              else if(serial_num == 22)
              {
                y_ccw_run();
              }
              else if(serial_num == 31)
              {
                z_up_run();
              }
              else if(serial_num == 32)
              {
                z_down_run();
              }
              else if(serial_num == 41)
              {
                w_cw_run();
              }
              else if(serial_num == 42)
              {
                w_ccw_run();
              }
              else if(serial_num == 51)
              {
                gripper_open();
              }
              else if(serial_num == 52)
              {
                gripper_close();
              }
           }
  }
  //오토모드
  //집고오는 형식
  void AutoMode()
  {
  
      if(getDistanceSensor_start() <= 10)
      {
        Serial.println(5);
        conveyor_sensor();
        To_DarkBox_From_init();    // 포토인터럽트까지 가게
        serial_box_num = Serial.read();
        switch (serial_box_num)
        {
        case 1:
             To_Box2_From_DarkBox();
            break;
        case 2:
            To_Box1_From_DarkBox();
            break;
        case 3:
            To_Box3_From_DarkBox();
            break;
        }
      }
}

//오토모드 도와주는 함수
void To_DarkBox_From_init()
{
    To_DarkBox_Z();
    To_DarkBox_Y();
    To_DarkBox_X();
    To_DarkBox_W();
    gripper_open();
    delay(500);
    z_down(2500);  // 컨베이어로 바꿔서 조절해야함!
    delay(500);
    gripper_close();
    delay(500);
    To_DarkBox_Z();
}
void To_Box1_From_DarkBox()
{
    x_cw(950);
    y_cw(300);
    w_ccw(350);
    gripper_open();
}
void To_Box2_From_DarkBox()
{
    x_cw(1350);
    y_cw(200);
    w_ccw(163);
    gripper_open();
}
void To_Box3_From_DarkBox()
{
    x_cw(350);
    y_cw(300);
    w_ccw(250);
    gripper_open();
}
void To_init()
{
    To_DarkBox_X();
    To_DarkBox_Z();
    To_DarkBox_Y();
    To_DarkBox_W();
    gripper_open();
}
//PHOTO
//포토인터럽트 감지시 멈추는 함수
void To_Xinit()
{
    run_set(true, X_DIR);
    while (1)
    {
        if (analogRead(X_photo_init) < 500)
        {
            break;
        }
        run(X_STP, x_speed);
    }
}
void To_DarkBox_X()
{
    run_set(false, X_DIR);
    while (1)
    {
        if (digitalRead(X_photo) == LOW)
        {
            break;
        }
        run(X_STP, x_speed);
    }
}
void To_DarkBox_Y()
{
    run_set(false, Y_DIR);

    while (1)
    {
        if (digitalRead(Y_photo) == LOW)
        {
            break;
        }
        run(Y_STP, y_speed);
    }
}
void To_DarkBox_Z()
{
    run_set(true, Z_DIR);

    while (1)
    {
        if (digitalRead(Z_photo) == HIGH) //걸릴때가 high!!
        {
            break;
        }
        run(Z_STP, z_speed);
    }
}
void To_DarkBox_W()
{
    run_set(false, W_DIR);

    while (1)
    {
        if (digitalRead(W_photo) == HIGH)
        {
            break;
        }
        run(W_STP, w_speed);
    }
}

////////////////스텝모터 동작함수//////////////////////////////
void step(boolean dir, byte dirPin, byte stepperPin, int steps, int speed)
{
    digitalWrite(EN, LOW);
    digitalWrite(dirPin, dir);
    delay(50);

    for (int i = 0; i < steps; i++)
    {
        digitalWrite(stepperPin, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepperPin, LOW);
        delayMicroseconds(speed);
    }
}
void conveyor_sensor()
{
    digitalWrite(C1_DIR, false);
    digitalWrite(C2_DIR, true);
    while(1)
    {
      if ( getDistanceSensor() > 12) 
      {        
         digitalWrite(C_EN, LOW);
         conveyor_run(200);
      }
      else
      {
         digitalWrite(C_EN, HIGH);
         break;
      }
    }
    }

void conveyor_run()
{
digitalWrite(C1_DIR, true);
digitalWrite(C2_DIR, false);
  while (1) //회전 시작
  {
      digitalWrite(C1_STP, HIGH);
      digitalWrite(C2_STP, HIGH);
      delayMicroseconds(c_speed);
      digitalWrite(C1_STP, LOW);
      digitalWrite(C2_STP, LOW);
      delayMicroseconds(c_speed);
  }
}
void conveyor_run(int angle) 
{
    digitalWrite(C1_DIR, true);
    digitalWrite(C2_DIR, false);
    for (int i = 0; i < angle; i++) //회전 시작
    {
        digitalWrite(C1_STP, HIGH);
        digitalWrite(C2_STP, HIGH);
        delayMicroseconds(c_speed);
        digitalWrite(C1_STP, LOW);
        digitalWrite(C2_STP, LOW);
        delayMicroseconds(c_speed);
    }
}

//To 우인
//수동조작 : 버튼누를 때 동작 안누르면 동작X
void x_cw_run()
{
    run_set(true, X_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();
        if (digitalRead(X_photo_init) == LOW)
        {
            break;
        }//센서 접촉시

        else if (serial_num == 4) //버튼 뗄때
        {
            digitalWrite(EN, HIGH);
            break;
        }// 멈춤

        else
        {
            run(X_STP, x_speed);
        }// 작동

    }//while

}// x_cw_run
void x_ccw_run()
{
    run_set(false, X_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();
        if (digitalRead(X_photo) == LOW)
        {
            break;
        }
        else if (serial_num == 4) //버튼 뗄때
        {
            digitalWrite(EN, HIGH);
            break;
        }
        else
        {
            run(X_STP, x_speed);
        }
    }
}

void y_cw_run()
{
    run_set(true, Y_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();


        if (serial_num == 4) //버튼 뗄때
        {
            digitalWrite(EN, HIGH);
            break;
        }// 멈춤

        else
        {
            run(Y_STP, y_speed);
        }// 작동

    }
}

void y_ccw_run()
{
    run_set(false, Y_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();
        if (digitalRead(Y_photo) == LOW)
        {
            break;
        }

        else if (serial_num == 4)//버튼 뗄때
        {
            digitalWrite(EN, HIGH); //모든 스텝모터 비활성화
            break;
        }

        else
        {
            run(Y_STP, y_speed);
        }
    }

}
void z_up_run()
{
    run_set(true, Z_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();
        if (digitalRead(Z_photo) == HIGH)
        {
            break;
        }//센서 접촉시
        else if (serial_num == 4) //버튼 뗄때
        {
            digitalWrite(EN, HIGH);
            break;
        }// 멈춤

        else
        {
            run(Z_STP, z_speed);
        }// 작동
    }
}
void z_down_run()
{
    run_set(false, Z_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();

        if (serial_num == 4) //버튼 뗄때
        {
            break;
        }// 멈춤

        else
        {
            run(Z_STP, z_speed);
        }// 작동

    }
}
void w_cw_run()
{
    run_set(false, W_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();

        if (digitalRead(W_photo) == HIGH)
        {
            break;
        }
        else if (serial_num == 4) //버튼 뗄때
        {
            break;
        }// 멈춤
        else
        {
            run(W_STP, w_speed);
        }// 작동
    }
}
void w_ccw_run()
{
    run_set(true, W_DIR);
    while (1) //회전 시작
    {
        serial_num = Serial.read();

        if (serial_num == 4) //버튼 뗄때
        {
            break;
        }// 멈춤
        else
        {
            run(W_STP, w_speed);
        }// 작동
    }
}

/////////////////그리퍼조작//////////////////////
void gripper_open()
{
    myservo.write(50);
}

void gripper_close()
{
    myservo.write(170);
}
////////////////동작 // 정지//////////////////////////////
void run_set(boolean dir, byte dirPin)
{
    digitalWrite(EN, LOW); //스텝모터 활성화
    digitalWrite(dirPin, dir);  // 방향: 정회전
}
void run(byte stepperPin, int speed)
{
    digitalWrite(stepperPin, HIGH);
    delayMicroseconds(speed);
    digitalWrite(stepperPin, LOW);
    delayMicroseconds(speed);
}
void stop()
{
    digitalWrite(EN, HIGH);
}
/////////////////////x, y, z동작함수////////////////////////
void x_cw(int angle)
{
    step(true, X_DIR, X_STP, angle, x_speed);
}
void x_ccw(int angle)
{
    step(false, X_DIR, X_STP, angle, x_speed);
}
void y_cw(int angle)
{
    step(true, Y_DIR, Y_STP, angle, y_speed);
}
void y_ccw(int angle)
{
    step(false, Y_DIR, Y_STP, angle, y_speed);
}
void z_up(int angle)
{
    step(true, Z_DIR, Z_STP, angle, z_speed);
}
void z_down(int angle)
{
    step(false, Z_DIR, Z_STP, angle, z_speed);
}
void w_cw(int angle)
{
    step(false, W_DIR, W_STP, angle, w_speed);
}
void w_ccw(int angle)
{
    step(true, W_DIR, W_STP, angle, w_speed);
}

//초음파센서
int Distance() {
    int i;
    int index = 0;
    int max = 0;
    int array1[10];
    int array2[10];

    for (i = 0; i < 10; i++) {
        array1[i] = getDistanceSensor();
    }

    for (i = 0; i < 10; i++) {
        array2[i] = getCount(array1, array1[i]);
    }

    for (i = 0; i < 9; i++) {
        if (array2[i] > max) {
            max = array2[i];
            index = i;
        }
    }
    return array1[index];
}
int getDistanceSensor()
{
    long start_time, end_time;
    int distance;

    digitalWrite(TRIG, LOW);
    delayMicroseconds(10);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(100);
    digitalWrite(TRIG, LOW);
    while (digitalRead(ECHO) == 0);
    start_time = micros();
    while (digitalRead(ECHO) == 1);
    end_time = micros();
    distance = (end_time - start_time) / 58;
    return distance;
}
int getDistanceSensor_start()
{
    long start_time, end_time;
    int distance;

    digitalWrite(TRIG_start, LOW);
    delayMicroseconds(10);
    digitalWrite(TRIG_start, HIGH);
    delayMicroseconds(100);
    digitalWrite(TRIG_start, LOW);
    while (digitalRead(ECHO_start) == 0);
    start_time = micros();
    while (digitalRead(ECHO_start) == 1);
    end_time = micros();
    distance = (end_time - start_time) / 58;
    return distance;
}
int getCount(int* array, int value) {
    int i;
    int count = 0;
    for (i = 0; i < 10; i++) {
        if (array[i] == value) {
            count++;
        }
    }
    return count;
}
