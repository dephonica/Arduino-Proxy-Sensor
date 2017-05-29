#include <TimerOne.h>

const long carrierFrequency = 50000;  // 50 kHz carrier
const short maxAdcCount = 500;        // Averaging cycles for ADC
const byte thresholdSwitchCount = 5;  // Switch state after thresholdSwitchCount detection cycles

const byte onThreshold = 21;
const byte offThreshold = 17;

void TimerOneInterrupt();

volatile static bool isKeyPressed[2] = {false, false};
volatile static bool isSwitched[2] = {false, false};

static bool flipFlop = false;
static long int adcSumm[2] = {0, 0};
static short adcCount = 0;
static byte thresholdSwitchCounter[2] = {0,0};
static byte activeKeyIndex = 0;

class ProxySensor
{

  void setupADC()
  {
    //clear ADCSRA and ADCSRB registers
    ADCSRA = 0;
    ADCSRB = 0;

    ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR);
    //ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX0) | (1 << MUX1);
  
    ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
    ADCSRA |= (1 << ADATE); //enabble auto trigger
    ADCSRA |= (1 << ADEN); //enable ADC
    ADCSRA |= (1 << ADSC); //start ADC measurements  
  }

public:
  ProxySensor()
  {
    pinMode(A1, OUTPUT);
    digitalWrite(A1, LOW);

    setupADC();

    pinMode(A2, OUTPUT);

    Timer1.initialize(1000000L / carrierFrequency);
    Timer1.attachInterrupt(&TimerOneInterrupt);
  }

  inline static void detectProxyKeypress()
  {
    if (isKeyPressed[activeKeyIndex])
    {
      if (adcSumm[activeKeyIndex] < offThreshold)
      {
        thresholdSwitchCounter[activeKeyIndex]++;
        if (thresholdSwitchCounter[activeKeyIndex] >= thresholdSwitchCount)
        {
          isSwitched[activeKeyIndex] = true;
          isKeyPressed[activeKeyIndex] = false;
          thresholdSwitchCounter[activeKeyIndex] = 0;
        }
      } else
      {
        if (thresholdSwitchCounter[activeKeyIndex] > 0)
        {
          thresholdSwitchCounter[activeKeyIndex]--;
        }
      }
    } else
    {
      if (adcSumm[activeKeyIndex] > onThreshold)
      {
        thresholdSwitchCounter[activeKeyIndex]++;
        if (thresholdSwitchCounter[activeKeyIndex] >= thresholdSwitchCount)
        {
          isSwitched[activeKeyIndex] = true;
          isKeyPressed[activeKeyIndex] = true;
          thresholdSwitchCounter[activeKeyIndex] = 0;
        }
      } else
      {
        if (thresholdSwitchCounter[activeKeyIndex] > 0)
        {
          thresholdSwitchCounter[activeKeyIndex]--;
        }
      }
    }

    switchActiveKey();
  }

  inline static void switchActiveKey()
  {
    activeKeyIndex = 1 - activeKeyIndex;

    if (activeKeyIndex == 0)
    {
      ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR);
    } else
    {
      ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX0) | (1 << MUX1);
    }
  }
};

void TimerOneInterrupt()
{
  if (flipFlop)
  {
    PORTC |= (1 << 2);
  }
  else
  {
    PORTC ^= (1 << 2);    
  }

  flipFlop = !flipFlop;
  
  if (flipFlop == false)
  {
    adcSumm[activeKeyIndex] += ADCH;
    adcCount++;

    if (adcCount >= maxAdcCount)
    {
      adcSumm[activeKeyIndex] /= adcCount;
      adcCount = 0;
      //Serial.println(adcSumm[activeKeyIndex]);
      ProxySensor::detectProxyKeypress();
    }
  }
}

