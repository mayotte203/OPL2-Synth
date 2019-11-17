#include <SPI.h>
#include <OPL2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define ce_pin 4 // 15 pin, !CE / чипселект
#define SO  5     // SO-Serial output (Pin 9) of 74165 is connected to the digital pin 2 of Arduino UNO.
#define SH_LD  3  // SH/LD (Pin 1) of 74165 is connected to the digital pin 3 of Arduino UNO.
#define CLK  2    // CLK (Pin 2) of 74165 is connected to the digital pin 4 of Arduino UNO.
#define latchPin 15
#define clockPin 16
#define dataPin 14

bool keyboardStatus[40] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool controlStatus[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte ChannelCount = 6;
OPL2 opl2;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

byte counter = 0;
byte OPLFeedback = 0x0; //0..7
bool OPLSynthType = false; //FM - true/AM - false
byte OPLNoteOffset = 0; //-127..127

byte CarrierAttack = 0x0A;
byte CarrierDecay = 0x04;
byte CarrierSustain = 0x0F;
byte CarrierRelease = 0x02;

byte CarrierWaveform = 0x01;

bool CarrierTremolo = false;
bool CarrierVibrato = false;
bool CarrierSustainVoice = true;
bool CarrierEnvelopeScale = true;

byte CarrierLevel = 0;
byte CarrierFrequencyMultiplication = 0;
byte CarrierKeyScaleLevel = 0;

byte ModulatorAttack = 0x0A;
byte ModulatorDecay = 0x04;
byte ModulatorSustain = 0x0F;
byte ModulatorRelease = 0x02;

byte ModulatorWaveform = 0x01;

bool ModulatorTremolo = false;
bool ModulatorVibrato = false;
bool ModulatorSustainVoice = true;
bool ModulatorEnvelopeScale = true;

byte ModulatorLevel = 0;
byte ModulatorFrequencyMultiplication = 0;
byte ModulatorKeyScaleLevel = 0x01;

byte MenuOption = 0;
bool OperatorSelect = false; //Carrier - false/Modulator - true

bool MajorChord = false;
bool MinorChord = false;

void control(byte button)
{
  switch(button)
  {
    case 19:
    {
      MajorChord = !MajorChord;
      MinorChord = false; 
      break;
    }
    case 8:
    {
      MinorChord = !MinorChord;
      MajorChord = false;
      break;
    }
    case 5:{
      ++MenuOption;
      MenuOption = MenuOption % 5;
      break;
    }
    case 22:{
      if(MenuOption == 0)
      {
        MenuOption = 4;
      }
      else
      {
        --MenuOption;
      }   
      break;   
    }
    case 6:{
      OperatorSelect = false;
      break;
    }
    case 21:{
      OperatorSelect = true;
      break;
    }
    case 7:
    {
      for(int i = 0; i < ChannelCount; i++)
      {
        opl2.setFeedback(i, OPLFeedback);
        opl2.setSynthMode(i, OPLSynthType);
        
        opl2.setAttack    (i, MODULATOR, ModulatorAttack);
        opl2.setDecay     (i, MODULATOR, ModulatorDecay);
        opl2.setSustain   (i, MODULATOR, ModulatorSustain);
        opl2.setRelease   (i, MODULATOR, ModulatorRelease);
        opl2.setAttack    (i, CARRIER, CarrierAttack);
        opl2.setDecay     (i, CARRIER, CarrierDecay);
        opl2.setSustain   (i, CARRIER, CarrierSustain);
        opl2.setRelease   (i, CARRIER, CarrierRelease);
    
        opl2.setWaveForm(i, MODULATOR, ModulatorWaveform);
        opl2.setWaveForm(i, CARRIER, CarrierWaveform);
    
        opl2.setTremolo   (i, MODULATOR, ModulatorTremolo);
        opl2.setVibrato   (i, MODULATOR, ModulatorVibrato);
        opl2.setMaintainSustain(i, MODULATOR, ModulatorSustainVoice);
        opl2.setEnvelopeScaling(i, MODULATOR, ModulatorEnvelopeScale);
        opl2.setTremolo   (i, CARRIER, CarrierTremolo);
        opl2.setVibrato   (i, CARRIER, CarrierVibrato);
        opl2.setMaintainSustain(i, CARRIER, CarrierSustainVoice);
        opl2.setEnvelopeScaling(i, CARRIER, CarrierEnvelopeScale);
    
        opl2.setVolume(i,MODULATOR, ModulatorLevel);
        opl2.setMultiplier(i, MODULATOR, ModulatorFrequencyMultiplication);
        opl2.setScalingLevel(i, MODULATOR, ModulatorKeyScaleLevel);
        opl2.setVolume(i,CARRIER, CarrierLevel);
        opl2.setMultiplier(i, CARRIER, CarrierFrequencyMultiplication);
        opl2.setScalingLevel(i, CARRIER, CarrierKeyScaleLevel);
      }
      break;
    }
  }
  lcd.clear();
  if(OperatorSelect)
  {
      lcd.setCursor(0,1);
      lcd.print(F("M"));
  }
  else
  {  
      lcd.setCursor(0,0);
      lcd.print(F("C"));
  }
  switch(MenuOption)
  {
    case 0:{
      switch(button)
      {
        case 13:
        {
          ++OPLFeedback;
          OPLFeedback %= 8;
          break;
        }
        case 12:
        {
          if(OPLFeedback == 0)
          {
            OPLFeedback = 7;
          }
          else
          {
            --OPLFeedback;
          }
          break;
        }
        case 14:
        {
          OPLSynthType = true;
          break;
        }
        case 11:
        {
          OPLSynthType = false;
          break;
        }
        case 15:
        {
          ++OPLNoteOffset;
          break;
        }
        case 10:
        {
          if(OPLNoteOffset == 0)
          {
            OPLNoteOffset = 255;
          }
          else
          {
            --OPLNoteOffset;
          }
          break;
        }
      }
      lcd.setCursor(0,0);
      lcd.print(F("C"));
      lcd.setCursor(0,1);
      lcd.print(F("M"));
      lcd.setCursor(2,0);
      lcd.print(F("1FB 2ST  3NO"));
      lcd.setCursor(4,1);
      lcd.print(OPLFeedback);
      lcd.setCursor(7,1);
      if(OPLSynthType)
      {
        lcd.print(F("AM"));
      }
      else
      {
        lcd.print(F("FM"));
      }
      lcd.setCursor(11,1);
      lcd.print(OPLNoteOffset);
      break;  
    }
    case 1:{
      switch(button)
      {
        case 13:
        {
          if(OperatorSelect)
          {
            ++ModulatorAttack;
            ModulatorAttack = ModulatorAttack%16;
          }
          else
          {
            ++CarrierAttack;
            CarrierAttack = CarrierAttack%16;
          }
          break;
        }
        case 12:
        {
          if(OperatorSelect)
          {
            if(ModulatorAttack == 0)
            {
              ModulatorAttack = 15; 
            }
            else
            {
              --ModulatorAttack;
            }
          }
          else
          {
            if(CarrierAttack == 0)
            {
              CarrierAttack = 15; 
            }
            else
            {
              --CarrierAttack;
            }
          }
          break;
        }
        case 14:
        {
          if(OperatorSelect)
          {
            ++ModulatorDecay;
            ModulatorDecay = ModulatorDecay%16;
          }
          else
          {
            ++CarrierDecay;
            CarrierDecay = CarrierDecay%16;
          }
          break;
        }
        case 11:
        {
          if(OperatorSelect)
          {
            if(ModulatorDecay == 0)
            {
              ModulatorDecay = 15; 
            }
            else
            {
              --ModulatorDecay;
            }
          }
          else
          {
            if(CarrierDecay == 0)
            {
              CarrierDecay = 15; 
            }
            else
            {
              --CarrierDecay;
            }
          }
          break;
        }
        case 15:
        {
          if(OperatorSelect)
          {
            ++ModulatorSustain;
            ModulatorSustain = ModulatorSustain%16;
          }
          else
          {
            ++CarrierSustain;
            CarrierSustain = CarrierSustain%16;
          }
          break;
        }
        case 10:
        {
          if(OperatorSelect)
          {
            if(ModulatorSustain == 0)
            {
              ModulatorSustain = 15; 
            }
            else
            {
              --ModulatorSustain;
            }
          }
          else
          {
            if(CarrierSustain == 0)
            {
              CarrierSustain = 15; 
            }
            else
            {
              --CarrierSustain;
            }
          }
          break;
        }
        case 18:
        {
          if(OperatorSelect)
          {
            ++ModulatorRelease;
            ModulatorRelease = ModulatorRelease%16;
          }
          else
          {
            ++CarrierRelease;
            CarrierRelease = CarrierRelease%16;
          }
          break;
        }
        case 9:
        {
          if(OperatorSelect)
          {
            if(ModulatorRelease == 0)
            {
              ModulatorRelease = 15; 
            }
            else
            {
              --ModulatorRelease;
            }
          }
          else
          {
            if(CarrierRelease == 0)
            {
              CarrierRelease = 15; 
            }
            else
            {
              --CarrierRelease;
            }
          }
          break;
        }
      }
      lcd.setCursor(2,0);
      lcd.print(F("1A  2D  3S  4R"));
      lcd.setCursor(2,1);
      if(OperatorSelect)
      {
    		if(ModulatorAttack < 10)
    		{
    			lcd.print(0);
    		}
            lcd.print(ModulatorAttack);
    		lcd.setCursor(6,1);
    		if(ModulatorDecay < 10)
    		{
    			lcd.print(0);
    		}
    		lcd.print(ModulatorDecay);
    		lcd.setCursor(10,1);
    		if(ModulatorSustain < 10)
    		{
    			lcd.print(0);
    		}
    		lcd.print(ModulatorSustain);
    		lcd.setCursor(14,1);
    		if(ModulatorRelease < 10)
    		{
    			lcd.print(0);
    		}
    		lcd.print(ModulatorRelease);
      }
      else
      {
       if(CarrierAttack < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierAttack);
        lcd.setCursor(6,1);
        if(CarrierDecay < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierDecay);
        lcd.setCursor(10,1);
        if(CarrierSustain < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierSustain);
        lcd.setCursor(14,1);
        if(CarrierRelease < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierRelease);        
      }
      break;  
    }
    case 2:{
      switch(button)
      {
        case 13:
        {
          if(OperatorSelect)
          {
            ++ModulatorWaveform;
            ModulatorWaveform %= 4;
          }
          else
          {
            ++CarrierWaveform;
            CarrierWaveform %= 4;
          }
          break;
        }
        case 12:
        {
          if(OperatorSelect)
          {
            if(ModulatorWaveform == 0)
            {
             ModulatorWaveform = 3;
            }
            else
            {
              --ModulatorWaveform;
            }
          }
          else
          {
            if(CarrierWaveform == 0)
            {
              CarrierWaveform = 3;
            }
            else
            {
              --CarrierWaveform;
            }
          }
          break;
        }
      }
      lcd.setCursor(2,0);
      lcd.print(F("1Waveform"));
      lcd.setCursor(12,0);
      switch(OperatorSelect ? ModulatorWaveform : CarrierWaveform)
      {
        case 0:
        {
          lcd.printByte(0);
          lcd.printByte(1);
          lcd.setCursor(14,1);
          lcd.printByte(2);
          lcd.printByte(3);
          break;
        }
        case 1:
        {
          lcd.printByte(0);
          lcd.printByte(1);
          lcd.print(F("__"));
          break;
        }
        case 2:
        {
          lcd.printByte(0);
          lcd.printByte(1);
          lcd.printByte(0);
          lcd.printByte(1);
          break;
        }
        case 3:
        {
          lcd.printByte(0);
          lcd.print(F("_"));
          lcd.printByte(0);   
          lcd.print(F("_"));
          break;
        }
      }
      break;  
    }
    case 3:{
      switch(button)
      {
        case 13:
        {
    		  if(OperatorSelect)
    		  {
    			ModulatorTremolo = true;
    		  }
    		  else
    		  {
    			CarrierTremolo = true;
    		  }
          break;
        }
        case 12:
        {
    		  if(OperatorSelect)
    		  {
    			ModulatorTremolo = false;
    		  }
    		  else
    		  {
    			CarrierTremolo = false;
    		  }
          break;
        }
        case 14:
        {
          if(OperatorSelect)
          {
          ModulatorVibrato = true;
          }
          else
          {
          CarrierVibrato = true;
          }
          break;
        }
        case 11:
        {
          if(OperatorSelect)
          {
          ModulatorVibrato = false;
          }
          else
          {
          CarrierVibrato = false;
          }
          break;
        }
        case 15:
        {
          if(OperatorSelect)
          {
          ModulatorSustainVoice = true;
          }
          else
          {
          CarrierSustainVoice = true;
          }
          break;
        }
        case 10:
        {if(OperatorSelect)
          {
          ModulatorSustainVoice = false;
          }
          else
          {
          CarrierSustainVoice = false;
          }
          break;
        }
        case 18:
        {
          if(OperatorSelect)
          {
          ModulatorEnvelopeScale = true;
          }
          else
          {
          CarrierEnvelopeScale = true;
          }
          break;
        }
        case 9:
        {
          if(OperatorSelect)
          {
          ModulatorEnvelopeScale = false;
          }
          else
          {
          CarrierEnvelopeScale = false;
          }
          break;
        }
      }
      lcd.setCursor(2,0);
      lcd.print(F("1T  2V  3S  4E"));
      lcd.setCursor(1,1);
      if(OperatorSelect)
      {
        if(ModulatorTremolo)
        {
          lcd.print(F(" ON"));
        }
        else
        {
          lcd.print(F("OFF"));
        }
        if(ModulatorVibrato)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
        if(ModulatorSustainVoice)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
        if(ModulatorEnvelopeScale)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
      }
      else
      {
        if(CarrierTremolo)
        {
          lcd.print(F(" ON"));
        }
        else
        {
          lcd.print(F("OFF"));
        }
        if(CarrierVibrato)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
        if(CarrierSustainVoice)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
        if(CarrierEnvelopeScale)
        {
          lcd.print(F("  ON"));
        }
        else
        {
          lcd.print(F(" OFF"));
        }
      }
      break;  
    }
    case 4:{
      switch(button)
      {
        case 13:
        {
          if(OperatorSelect)
          {
            ++ModulatorLevel;
            ModulatorLevel %= 64;
          }
          else
          {
            ++CarrierLevel;
            CarrierLevel %= 64;
          }
          break;
        }
        case 12:
        {
          if(OperatorSelect)
          {
            if(ModulatorLevel == 0)
            {
              ModulatorLevel = 63;
            }
            else
            {
              --ModulatorLevel;
            }
          }
          else
          {
            if(CarrierLevel == 0)
            {
              CarrierLevel = 63;
            }
            else
            {
              --CarrierLevel;
            }
          }
          break;
        }
        case 14:
        {
          if(OperatorSelect)
          {
            ++ModulatorFrequencyMultiplication;
            ModulatorFrequencyMultiplication %= 16;
          }
          else
          {
            ++CarrierFrequencyMultiplication;
            CarrierFrequencyMultiplication %= 16;
          }
          break;
        }
        case 11:
        {
          if(OperatorSelect)
          {
            if(ModulatorFrequencyMultiplication == 0)
            {
              ModulatorFrequencyMultiplication = 15;
            }
            else
            {
              --ModulatorFrequencyMultiplication;
            }
          }
          else
          {
            if(CarrierFrequencyMultiplication == 0)
            {
              CarrierFrequencyMultiplication = 63;
            }
            else
            {
              --CarrierFrequencyMultiplication;
            }
          }
          break;
        }
        case 15:
        {
          if(OperatorSelect)
          {
            ++ModulatorKeyScaleLevel;
            ModulatorKeyScaleLevel %= 4;
          }
          else
          {
            ++CarrierKeyScaleLevel;
            CarrierKeyScaleLevel %= 4;
          }
          break;
        }
        case 10:
        {
          if(OperatorSelect)
          {
            if(ModulatorKeyScaleLevel == 0)
            {
              ModulatorKeyScaleLevel = 3;
            }
            else
            {
              --ModulatorKeyScaleLevel;
            }
          }
          else
          {
            if(CarrierKeyScaleLevel == 0)
            {
              CarrierKeyScaleLevel = 3;
            }
            else
            {
              --CarrierKeyScaleLevel;
            }
          }
          break;
        }
      }
      lcd.setCursor(2,0);
      lcd.print(F("1LVL 2FrM 3KSL"));
      lcd.setCursor(4,1);
      if(OperatorSelect)
      {
        if(ModulatorLevel < 10)
        {
          lcd.print(0);
        }
        lcd.print(ModulatorLevel);
        lcd.setCursor(9,1);
        if(ModulatorFrequencyMultiplication < 10)
        {
          lcd.print(0);
        }
        lcd.print(ModulatorFrequencyMultiplication);
        lcd.setCursor(15,1);
        lcd.print(ModulatorKeyScaleLevel);
      }
      else
      {
        if(CarrierLevel < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierLevel);
        lcd.setCursor(9,1);
        if(CarrierFrequencyMultiplication < 10)
        {
          lcd.print(0);
        }
        lcd.print(CarrierFrequencyMultiplication);
        lcd.setCursor(15,1);
        lcd.print(CarrierKeyScaleLevel);
      }
      break;  
    }
  }
}

uint8_t sin1[8]  = 
{
  0b00001,
  0b00011,
  0b00111,
  0b00111,
  0b01111,
  0b01111,
  0b01111,
  0b11111
};
uint8_t sin2[8] =
{
  0b10000,
  0b11000,
  0b11100,
  0b11100,
  0b11110,
  0b11110,
  0b11110,
  0b11111  
};
uint8_t sin3[8]  = 
{
  0b11111,
  0b01111,
  0b01111,
  0b01111,
  0b00111,
  0b00111,
  0b00011,
  0b00001
};
uint8_t sin4[8] =
{
  0b11111,
  0b11110, 
  0b11110,
  0b11110,
  0b11100, 
  0b11100,
  0b11000,
  0b10000
};

void setup() 
{
  opl2.init();
  opl2.setWaveFormSelect(true);
  lcd.init();                
  lcd.backlight();
  lcd.createChar(0, sin1);
  lcd.createChar(1, sin2);
  lcd.createChar(2, sin3);
  lcd.createChar(3, sin4);
  lcd.home();
  lcd.clear();
  pinMode(SH_LD, OUTPUT);
  pinMode(CLK, OUTPUT);  
  pinMode(SO, INPUT); 
  pinMode(ce_pin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(ce_pin, LOW);
  control(6);
  control(7);
}

byte playing[6] = {0,0,0,0,0,0};

void loop()
{
  for(byte i = 0; i < 8; i++)
  {
    digitalWrite(latchPin, LOW);
    byte shift;
    shift = 0b10000000 >> i;
    shift = ~shift;
    shiftOut(dataPin, clockPin, MSBFIRST, shift);
    digitalWrite(latchPin, HIGH);
    digitalWrite(CLK, LOW);   
    digitalWrite(SH_LD, LOW); 
    digitalWrite(SH_LD, HIGH);
    for(byte j = 0;j<3;j++)
    {
      digitalWrite(CLK, LOW);
      digitalWrite(CLK, HIGH);
    }
    for(byte j = 0;j<5;j++)
    {
      if(digitalRead(SO) != keyboardStatus[i + j * 8])
      {
        keyboardStatus[i + j * 8] = digitalRead(SO);
        /*
        if(!keyboardStatus[i + j * 8])
        {
          opl2.playNote(0, 2 + ((i + j * 8 + 4 + OPLNoteOffset)/12), (i + j * 8 + 4 + OPLNoteOffset)%12);
          if(MajorChord)
          {
            opl2.playNote(1, 2 + ((i + j * 8 + 8 + OPLNoteOffset)/12), (i + j * 8 + 8 + OPLNoteOffset)%12);
            opl2.playNote(2, 2 + ((i + j * 8 + 12 + OPLNoteOffset)/12), (i + j * 8 + 12 + OPLNoteOffset)%12); 
          }
          if(MinorChord)
          {
            opl2.playNote(1, 2 + ((i + j * 8 + 7 + OPLNoteOffset)/12), (i + j * 8 + 7 + OPLNoteOffset)%12);
            opl2.playNote(2, 2 + ((i + j * 8 + 12 + OPLNoteOffset)/12), (i + j * 8 + 12 + OPLNoteOffset)%12);
          }
        }
        else
        {
          opl2.setKeyOn(0, false);
          opl2.setKeyOn(1, false);
          opl2.setKeyOn(2, false);
        }
        */
        if(!keyboardStatus[i + j * 8])
        {
          for(int z = 0; z < ChannelCount; ++z)
          {
            if(playing[z] == 0)
            {
              playing[z] = i + j * 8;
              opl2.playNote(z, 2 + ((i + j * 8 + 4 + OPLNoteOffset)/12), (i + j * 8 + 4 + OPLNoteOffset)%12);
              z = ChannelCount;
            }
          }
        }
        else
        {
          for(int z = 0; z < ChannelCount; ++z)
          {
            if(playing[z] == i + j * 8)
            {
              playing[z] = 0;
              opl2.setKeyOn(z, false);
              z = ChannelCount;
            }
          }
        }
      }
      digitalWrite(CLK, LOW);
      digitalWrite(CLK, HIGH);
    }
    for(byte j=0;j<4;j++)
    {
      digitalWrite(CLK, LOW);
      digitalWrite(CLK, HIGH);
    }
    for(byte j=0;j<4;j++)
    {
      if(digitalRead(SO) != controlStatus[i + j * 8])
      {
        controlStatus[i + j * 8] = digitalRead(SO);
        if(!controlStatus[i + j * 8])
        {
          control(i + j * 8);
        }  
        control(7);
      }
      digitalWrite(CLK, LOW);
      digitalWrite(CLK, HIGH);
    }
  }
}
