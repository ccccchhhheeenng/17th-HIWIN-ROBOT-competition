//0821 final 

//0818

//0817辨識
const int IN1 = 5;
const int IN2 = 6;
const int ENA = 9;
const int pinA = 2;
const int pinB = 3;
const int trig1Pin = 13;
const int echo1Pin = 12;

volatile long encoderCount = 0;
float distance1, distance2;

void setup() {
  pinMode(4, INPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);  // 腳位設定
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(trig1Pin, OUTPUT);
  pinMode(echo1Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinA), readEncoder, CHANGE);  // 中斷設定
  Serial.begin(9600);
}
void loop() {
  int input1 = analogRead(A0);
  int input2 = analogRead(A1);
  if (input1>512){
    if (input2>512){
      initialize();
      //馬達全功率運行
      analogWrite(IN1, 255);
      digitalWrite(IN2, LOW);
      int tmp = 0;
      int a[3] = {-150,-150,-150};
      int encoderResult=0;    //連續三次encoder辨識結果相同則輸出辨識結果
      while (true) {
        Serial.println(encoderCount);
        a[tmp % 3] = encoderCount;
        int averageval = average(a, 3);
        if (abs(averageval-encoderCount)<30 && encoderCount!=0) {
            //輸出encoder辨識結果
            encoderResult=encoderCount;
            Serial.print("Result:");
            Serial.println(encoderResult);
            break;
          }
        delay(100);
        tmp++;
      }
      //超音波辨識部分 辨識五次取中間三次
      
      delay(200);
      int times;
      float UltrasonicDistance_result;
      if (encoderResult > 1170 && encoderResult < 1300){
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        delay(200);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        delay(100);
        times=30;
        float ult_result=0;
        for(int j=0;j<3;j++){
          float UltrasonicDistance[times]={0};
          for(int i=0;i<times;i++){
            UltrasonicDistance[i]=readUltrasonicDistance(trig1Pin,echo1Pin);
            Serial.println(UltrasonicDistance[i]);
            delay(50);
          }
          UltrasonicDistance_result=getBestUltrasonicValue(UltrasonicDistance,times);
          ult_result+=UltrasonicDistance_result;
        }
        UltrasonicDistance_result=ult_result;
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        delay(250);
        if (UltrasonicDistance_result > 30.2 && UltrasonicDistance_result < 30.50){
          send(2);
          Serial.println("2");
        }else if(UltrasonicDistance_result<25){
          send(0);
          Serial.println("0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ");
        }else{
          send(4);
          Serial.println("4");
        }
        Serial.print("UltrasonicDistance_result:");
        Serial.println(UltrasonicDistance_result);
        Serial.println("finish");
      }else{
        times=10;
        float UltrasonicDistance[times]={0};
        for(int i=0;i<times;i++){
          UltrasonicDistance[i]=readUltrasonicDistance(trig1Pin,echo1Pin);
          Serial.println(UltrasonicDistance[i]);
          delay(50);
        }
        UltrasonicDistance_result=trimmedAverage(UltrasonicDistance,times);
        Serial.print("UltrasonicDistance_result:");
        Serial.println(UltrasonicDistance_result);
        Serial.println("finish");
        identify(encoderResult,UltrasonicDistance_result);
      }
      //輸出超音波結果

      while(true){
        analogWrite(IN1, 40);
        digitalWrite(IN2,LOW);
        input1 = analogRead(A0);
        if(input1<512){
          break;
        }
        delay(50);
      }
      while (true){
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        if (digitalRead(4)==LOW){
          delay(100);
          break;
        }
      }
      digitalWrite(7,LOW);
      digitalWrite(8,LOW);
      digitalWrite(9,LOW);
      digitalWrite(10,LOW);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      delay(1000);
    }else{
      analogWrite(IN1, 255);
      digitalWrite(IN2, LOW);
      int tmp = 0;
      int a[3] = {-150,-150,-150};
      int encoderResult=0;
      while (true) {
        Serial.println(encoderCount);
        a[tmp % 3] = encoderCount;
        int averageval = average(a, 3);
        if (abs(averageval-encoderCount)<5 && encoderCount!=0) {
            //輸出encoder辨識結果
            encoderResult=averageval;
            Serial.print("Result:");
            Serial.println(encoderResult);
            break;
          }
        delay(50);
        tmp++;
      }
      delay(100);
      send(0);
      analogWrite(IN1, 40);
      digitalWrite(IN2,LOW);
      while(true){
        input1 = analogRead(A0);
        if(input1<512){
          break;
        }
        delay(50);
      }
      while (true){
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        if (digitalRead(4)==LOW){
          delay(100);
          break;
        }
      }
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(7,LOW);
      digitalWrite(8,LOW);
      digitalWrite(9,LOW);
      digitalWrite(10,LOW);
    }
  }
  delay(20);
}

// 中斷副程式
void readEncoder() {
  bool a = digitalRead(pinA);
  bool b = digitalRead(pinB);
  if (a == b) encoderCount++;
  else encoderCount--;
}
float readUltrasonicDistance(int trigPin, int echoPin) {
  float Ult_Duration,Ult_Distance,inf_Distance;
  digitalWrite(trigPin,HIGH); 
  digitalWrite(trigPin,LOW);
  Ult_Duration = pulseIn(echoPin,HIGH); 
  Ult_Distance = Ult_Duration*0.034/2; 
  return Ult_Distance;
}
float average(int values[], int size) {
  long sum = 0;
  for (int i = 0; i < size; i++) {
    sum += values[i];
  }
  return (float)sum / size;
}
//排序取中間三個並平均
float trimmedAverage(float arr[], int size) {
  if (size <= 4) return 0.0; // 無法去掉最大最小

  float temp[size];
  for (int i = 0; i < size; i++) {
    temp[i] = arr[i];
  }

  // 排序
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (temp[j] > temp[j + 1]) {
        float t = temp[j];
        temp[j] = temp[j + 1];
        temp[j + 1] = t;
      }
    }
  }

  // 去掉最大與最小，取平均
  float sum = 0.0;
  for (int i = 3; i < size - 3; i++) {
    sum += temp[i];
  }

  return sum / (size - 6);
}

void initialize(){
  while (true){
    analogWrite(IN1, 50);
    digitalWrite(IN2,LOW);
    if (digitalRead(4)==HIGH){
      break;
    }
  }
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(500);
  encoderCount=0;
}
void identify(int encoder,float ult){
  int result=0;
  if ((encoder > 400 && encoder < 670)){
    //大正方
    result=1;
  }
  if ((encoder > 900 && encoder < 1200)){
      result=2;
  }

  if ((encoder > 1650 && encoder < 2165)){
    //三角柱
    result=5;
  }
  if (encoder > 1750 && encoder < 1800){
    //圓柱
    result=6;
  }
  if ((encoder > 2165)){
    //小正方
    result=3;
  }
  if ((encoder > 1300 && encoder < 1450) && (ult > 7 && ult < 8)){
    //大長方
    result=0;
  }
  Serial.println(result);
  Serial.println("...end...");
  send(result);
}
void send(int value){
   if (value==0){
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    digitalWrite(9,LOW);
  }else if (value==1){
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    digitalWrite(9,HIGH);
  }else if (value==2){
    digitalWrite(7,LOW);
    digitalWrite(8,HIGH);
    digitalWrite(9,LOW);
  }else if (value==3){
    digitalWrite(7,LOW);
    digitalWrite(8,HIGH);
    digitalWrite(9,HIGH);
  }else if (value==4){
    digitalWrite(7,HIGH);
    digitalWrite(8,LOW);
    digitalWrite(9,LOW);
  }else if (value==5){
    digitalWrite(7,HIGH);
    digitalWrite(8,LOW);
    digitalWrite(9,HIGH);
  }else if (value==6){
    digitalWrite(7,HIGH);
    digitalWrite(8,HIGH);
    digitalWrite(9,LOW);
  }else{
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    digitalWrite(9,LOW);
  }
  delay(200);
  digitalWrite(10,HIGH);

}
float getBestUltrasonicValue(float arr[], int size) {
  if (size < 6) return -1; // 保險檢查，至少需要 6 個值

  float minVariance = 999999.0;
  float bestMean = 0;

  // 掃描所有連續的 6 個子集
  for (int i = 0; i <= size - 6; i++) {
    float sum = 0;
    for (int j = 0; j < 6; j++) {
      sum += arr[i + j];
    }
    float mean = sum / 6;

    // 計算該子集的變異數
    float variance = 0;
    for (int j = 0; j < 6; j++) {
      float diff = arr[i + j] - mean;
      variance += diff * diff;
    }
    variance /= 6;

    // 如果這組變異數更小，就更新最佳解
    if (variance < minVariance) {
      minVariance = variance;
      bestMean = mean;
    }
  }

  return bestMean;
}
