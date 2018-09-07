#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include "rotary.h"
#include <EEPROM.h>
#include "si5351.h"
#include <Wire.h>

// Установка энкодера
#define ENCODER_A    3                      // Encoder pin A
#define ENCODER_B    2                      // Encoder pin B
Rotary r = Rotary(ENCODER_A, ENCODER_B);
Si5351 si5351;
// Установка дисплея
LiquidCrystal lcd(9, 8, 7, 6, 5, 4); // I used an odd pin combination because I need pin 2 and 3 for the interrupts.

// Task
void TaskBlink( void *pvParameters );

volatile uint32_t vfo_round=0; 
volatile uint32_t vfo2=1; // variable to hold the updated frequency
volatile uint32_t increment = 100000; // starting VFO update increment in KHz.
int buttonstate = 0;
byte press_counter = 0;
String hertz = "               ";
byte ones,tens,hundreds,thousands,tenthousands,hundredthousands,millions;  //Placeholders
String freq; // string to hold the frequency
volatile uint32_t timepassed = millis(); // int to hold the arduino miilis since startup
int memstatus = 1;  // value to notify if memory is current or old. 0=old, 1=current.

int border1 = 300;
int border2 = 75;
int border3 = 25;

int ForceFreq = 0;  // "1" - Задать начальную частоту 10000 кГц, "0" - Режим хранения последней частоты
boolean inc_flag = false;  // Сброс флага определения вращения энкодера 
byte Start_Reset_Flg_EEPROM10  = 0;
// Переменные для идентификации вращения энкодера
unsigned long time_old = 0;
unsigned long time_new = 0;
unsigned long delta_time = 0;
unsigned long abs_delta_time = 0;

volatile uint32_t FREQ_Si5351 = 0ULL;
volatile uint32_t FREQ_Si5351_Temp = 0ULL;
volatile uint32_t bfo = 0ULL;
volatile uint32_t vfo = 1000000000ULL / SI5351_FREQ_MULT;       //стартовая частота

long Correct_EEPROM20  = 215000; //  204000;
int in_case_STEP = 0;
int in_case_BEND ;
int in_case_MODE = 0;
int in_case_SETUP = 0;
int in_Key;                     // код нажатой кнопки
int in_Timer = 0;

byte by_Flg_Key = 0;          // код флага Нажатой Кнопки
byte by_Flg_SETUP = 0 ;
int  in_in_KeyA = 0;     // А временная переменная для анализа данных с кнопки
long lo_in_KeyB = 0;     // В временная переменная для анализа данных с кнопки



// Индикация частоты
void showFreq()
{
   if (in_case_SETUP == 0 ){
    millions = int(vfo/1000000);
    hundredthousands = ((vfo/100000)%10);
    tenthousands = ((vfo/10000)%10);
    thousands = ((vfo/1000)%10);
    hundreds = ((vfo/100)%10);
    tens = ((vfo/10)%10);
    ones = ((vfo/1)%10);
    lcd.setCursor(0,0);
    lcd.print(' ');
    lcd.print(' ');
   if (millions > 9){lcd.setCursor(1,0);}
   else{lcd.setCursor(2,0);}
    lcd.print(millions);
    lcd.print(".");
    lcd.print(hundredthousands);
    lcd.print(tenthousands);
    lcd.print(thousands);
    lcd.print(".");
    lcd.print(hundreds);
    lcd.print(tens);
    lcd.print(ones);
    lcd.print(" MHz");
     lcd.print(' ');
    timepassed = millis();
    memstatus = 0; // Trigger memory write
   }
}

// Запись частоты в EEPROM
void storeMEM(){
  //Write each frequency section to a EPROM slot.  Yes, it's cheating but it works!
   EEPROM.put(0, vfo);
  memstatus = 1;  // Let program know memory has been written
}

void OUT_Si5351() {
    if (vfo >100000000){vfo=vfo2;}; // Ограничение макс. частоты
    if (vfo <100000){vfo=vfo2;};   // Ограничение мин. частоты
    si5351.set_freq((vfo * SI5351_FREQ_MULT), SI5351_PLL_FIXED, SI5351_CLK0); 
}

void KEYBOARD() {
    //************* подпрограмма измерения состояния кнопок ===============
  in_in_KeyA = analogRead(A0); // считываем с аналогового порта А0
  in_in_KeyA = analogRead(A0); // повторно считываем с аналогового порта А0
  delay (10);                  // ждём
  lo_in_KeyB = analogRead(A0); // ещё раз считываем с аналогового порта А0 в другую переменную
  lo_in_KeyB = analogRead(A0); // ещё раз считываем с аналогового порта А0 в другую переменную

  if ( in_in_KeyA > lo_in_KeyB - 4 & in_in_KeyA < lo_in_KeyB + 4 ) 
  {
    in_Key = in_in_KeyA; // если они одинаковы то сохраняем значение в in_Key для обработки
  }
   //============= Вход в SETUP корректировку основной и опорных частот ========== 

    if (in_Key > 530 && in_Key < 650 )         
  {           
           lcd.setCursor(0, 0);
           lcd.print("Corr.PLL Si5351  ");   
           lcd.setCursor(1, 1);
           lcd.print("  ");
           lcd.print(Correct_EEPROM20);
           lcd.print("            "); 
           in_case_SETUP = 1;   
    }
    else
    {
      in_case_SETUP = 0;
    }
}

void LCD_Step(){
    
      // Назначаем действия по каждому состоянию счетчика нажатий
      lcd.setCursor(3,1); lcd.print("Step:");
      switch (press_counter) {
      case 0:
        {increment = 1000000; hertz="   1 MHz"; lcd.setCursor(8,1); lcd.print(hertz); }
     
    
        break;   
      case 1:
        {increment = 100000; hertz=" 100 KHz"; lcd.setCursor(8,1); lcd.print(hertz); }
     
     
        break;
       
      case 2:
          {increment = 10000; hertz="  10 KHz"; lcd.setCursor(8,1); lcd.print(hertz); }
      
     
        break;
      case 3:
          {increment = 1000; hertz="   1 KHz"; lcd.setCursor(8,1); lcd.print(hertz);}
        break;

      case 4:
        {increment = 100; hertz="  100 Hz"; lcd.setCursor(8,1); lcd.print(hertz);}
        break;
          case 5:
           {increment = 10;   hertz = "   10 Hz"; lcd.setCursor(8,1); lcd.print(hertz);}
        break; 

          case 6:
           {increment = 1;   hertz = "   1  Hz"; lcd.setCursor(8,1); lcd.print(hertz);}
        break; 
          case 7:
           {increment = 10;   hertz = "    Auto"; lcd.setCursor(8,1); lcd.print(hertz);}
        break; 
     
      default: 
{increment = 1000; hertz="   1 KHz"; lcd.setCursor(8,1); lcd.print(hertz);  }

 }
 EEPROM.put(30, press_counter);
//  Serial.println(press_counter);
    }

//=======================================================================
void setup() {

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );



  //Serial.begin(57600);
  lcd.begin(16, 2);       // настройуа LC
  lcd.clear();
  Wire.begin();
  
  pinMode(A0,INPUT); // Connect to a button that goes to GND on push
  digitalWrite(A0,HIGH);

  PCICR |= (1 << PCIE2);          
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();

// --------  Start screen    --------
  lcd.clear();
  lcd.setCursor(0,0);   
  lcd.print("Si5351  Autostep");  
  lcd.setCursor(0,1);     
  lcd.print("100KHz....100MHz");  
 delay(200);
  lcd.setCursor(0,1);     
     
    delay(600);  
// Бегущая строка - экран приветствия
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) { lcd.scrollDisplayLeft(); delay(50);  }

//  Очистим экран после приветствия
lcd.clear(); 

if ( EEPROM.get(10, Start_Reset_Flg_EEPROM10) != 250 || analogRead(A0) > 500 && analogRead(A0)< 700)  
 { 
 EEPROM.put(20, Correct_EEPROM20);
 EEPROM.put(10, 250); 
 EEPROM.put(0, vfo);
 EEPROM.put(30, press_counter);
 }
// Загружаем из EEPROM сохраненную там частоту  
  if (ForceFreq == 0) {
 EEPROM.get(0, vfo);

  }
 press_counter = EEPROM.get(30, press_counter);
// Задаем начальный режим
 // hertz = "  100KHz"; 
//  lcd.setCursor(0,1); lcd.print(press_counter); 
  lcd.setCursor(3,1); lcd.print("Step:");
  lcd.setCursor(8,1); lcd.print(hertz);
  
// Запоминаем опорное время
  time_old=millis();  
  abs_delta_time = 100; // Фиксируем шаг 100Гц
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 27000000 ); 
  // Установка на выходе CLK0 начальную частоту 10 MHz 
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(vfo * SI5351_FREQ_MULT, SI5351_PLL_FIXED, SI5351_CLK0);
  Correct_EEPROM20 = EEPROM.get(20, Correct_EEPROM20);

si5351.set_correction( Correct_EEPROM20); // 204000
si5351.init(SI5351_CRYSTAL_LOAD_8PF, 27000000); //поумолчанию кварц 25Mhz   
si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);

si5351.output_enable(SI5351_CLK0, 1);
si5351.output_enable(SI5351_CLK1, 0);
si5351.output_enable(SI5351_CLK2, 0);
si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_8MA); //you can set this to 2MA, 4MA, 6MA or 8MA
}  // Конец инициализации void setup()


void TaskBlink(void *pvParameters)  // This is a task.
{
  for (;;) // A Task shall never return or exit.
  {
  }

}

// ************  Главный цикл **********
void loop() {
   //Serial.println(by_Flg_SETUP); 
 if( in_case_SETUP == 0)  {  showFreq(); LCD_Step();}
 KEYBOARD();
 
// Обработка режима авт. изменения инкремента
  if (press_counter == 7)
    {      
          if (abs_delta_time > border1)
            { increment = 10; abs_delta_time = 1000; lcd.setCursor(0,1); lcd.print("   ");}
            
          if ((abs_delta_time <= border1)&&(abs_delta_time > border2))
            { increment = 100; abs_delta_time = (border1 + border2)/2; lcd.setCursor(0,1); lcd.print(">  ");}            

          if ((abs_delta_time <= border2)&&(abs_delta_time > border3))
            { increment = 1000; abs_delta_time = (border2 + border3)/2; lcd.setCursor(0,1); lcd.print(">> ");}            
            
           if (abs_delta_time <= border3)
            { increment = 25000; abs_delta_time = 20; lcd.setCursor(0,1); lcd.print(">>>");}      
    }  
 
// Если частота изменилась в последнем цикле, то  
  if (vfo != vfo2 )
   {    
      OUT_Si5351() ;
      vfo2 = vfo;
   }
      
// Пишем частоту в EEPROM, если не записана, и прошли 1,5 секунды со времени последнего изменения частоты.
    if(memstatus == 0)
      {   
        if(millis() >= timepassed+1500)  { storeMEM(); abs_delta_time = 1000;}        
      }   

// Не нажата ли головка энкодера?      
  buttonstate = digitalRead(A0);  
 if(buttonstate == LOW)  // Если нажата
  {
      // Организуем счетчик нажатий кнопки
      press_counter = press_counter+1;
      if (press_counter >= 8){press_counter = 0;}

LCD_Step();
// Пауза при циклической обработке кнопки энкодера.
  delay(200);
      lcd.setCursor(0,1); lcd.print("> ");
  delay(200); 
      lcd.setCursor(0,1); lcd.print("  ");
}
}
// Закончили главный цикл
// ************  Конец цикла **********

  
ISR(PCINT2_vect) {
  unsigned char result = r.process();
  
  if (result) // Если начали вращать энкодер
    {  
      // Определяем скорость вращения
        
    time_new=millis(); // Запоминаем время импульса
    delta_time=(time_new - time_old); // Вычисляем интервал времени между импульсами
    abs_delta_time=abs(delta_time);   // Абс. величина интервала
    time_old=time_new;  // Перезапоминаем опорное время

    // В зависимости от направления вращения "+" или "-" инкремент 
    if (result == DIR_CW)
    {  if (in_case_SETUP == 0 ) {vfo=vfo+increment;  vfo = ( vfo / increment)*increment; storeMEM();} 
    if (in_case_SETUP == 1 ){ EEPROM.put(20, Correct_EEPROM20 = EEPROM.get(20, Correct_EEPROM20) + increment); }
    }
    else                 
    { if (in_case_SETUP == 0 ) {  vfo=vfo-increment;vfo = ( vfo / increment)*increment; storeMEM();}
    if (in_case_SETUP == 1 ){ EEPROM.put(20, Correct_EEPROM20 = EEPROM.get(20, Correct_EEPROM20) - increment);  if (Correct_EEPROM20 <=0){Correct_EEPROM20 = 0; EEPROM.put(20, Correct_EEPROM20);}  }
    }     
    if (vfo >100000000){vfo=vfo2;}; // Ограничение макс. частоты
    if (vfo <100000){vfo=vfo2;};   // Ограничение мин. частоты
    Correct_EEPROM20 = EEPROM.get(20, Correct_EEPROM20);

  } // Конец обработки вращения энкодера
}  //  Конец ловушки прерывания