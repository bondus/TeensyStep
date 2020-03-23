#pragma once

#include "ChRt.h"

#include "../TF_Handler.h"

class TimerField
{
public:
  inline TimerField(TF_Handler *handler);
  inline ~TimerField();

  inline bool begin();
  inline void end();

  inline void stepTimerStart();
  inline void stepTimerStop();
  inline bool stepTimerIsRunning() const;
  inline void setStepFrequency(unsigned f);

  inline void accTimerStart();
  inline void accTimerStop();
  inline void setAccUpdatePeriod(unsigned period);

  inline void setPulseWidth(unsigned delay);
  inline void triggerDelay();

//private:

  TF_Handler *handler;

  virtual_timer_t step_vt;
  virtual_timer_t acc_vt;
  virtual_timer_t delay_vt;

  // ISRs
  inline void step_isr();
  inline void acc_isr();
  inline void delay_isr();

  sysinterval_t stepPeriod; // 0 if disabled
  sysinterval_t accUpdatePeriod;
  sysinterval_t delayTime;  
};

void acc_timer_isr(void *arg);
void step_timer_isr(void *arg);
void delay_timer_isr(void *arg);

TimerField::TimerField(TF_Handler *_handler)
    : handler(_handler)
{
  chVTObjectInit(&step_vt);
  chVTObjectInit(&acc_vt);
  chVTObjectInit(&delay_vt);
}

TimerField::~TimerField()
{  
  end(); 
}

bool TimerField::begin()
{
  return true;
}

void TimerField::end()
{
    chVTReset(&step_vt);
    chVTReset(&acc_vt);
    chVTReset(&delay_vt);
}


// Step Timer ------------------------------------------------------

void TimerField::step_isr()
{
  if (stepPeriod != 0) {
    handler->stepTimerISR();    
    chVTSetI(&step_vt, stepPeriod, step_timer_isr, this); // order next timer
  }
}

void TimerField::stepTimerStart()
{
  chSysLockFromISR();
  step_timer_isr(this);
  chSysUnlockFromISR();
}

void TimerField::stepTimerStop()
{
  stepPeriod = 0;
}


void TimerField::setStepFrequency(unsigned f)
{

  if (f != 0)
  {
    stepPeriod = TIME_S2I(1) / f; // Hz => period
    if (chVTIsArmed(&step_vt))
    {
      // TODO timer is already running, we should do something clever...
      /*
      uint32_t ldval = stepTimer.getLDVAL();

      if (ldval < F_BUS / 250 ) // normal step freqency (> 250 Hz) -> set new period for following periods
      {       
        stepTimer.setNextReload(F_BUS / f);
        return;
      }
      uint32_t newReload = F_BUS / f;
      uint32_t cyclesSinceLastStep = ldval - stepTimer.channel->CVAL;
      if (cyclesSinceLastStep <= newReload) // time between last pulse and now less than required new period -> wait
      {       
        stepTimer.setThisReload(newReload - cyclesSinceLastStep);
        stepTimer.setNextReload(newReload);
      }
      else
      {       
        stepTimer.setThisReload(newReload);
        handler->stepTimerISR();
      }
      */
      chVTSetI(&step_vt, stepPeriod, step_timer_isr, this); // order next timer
      chSysLockFromISR();
      handler->stepTimerISR();      
      chSysUnlockFromISR();
    }
    else // not running
    {
      chSysLockFromISR();
      handler->stepTimerISR(); // Why is this called here?
      chSysUnlockFromISR();
      chVTSetI(&step_vt, stepPeriod, step_timer_isr, this); // order next timer
    }
  }
  else //f==0
  {    
    chVTReset(&step_vt);
  }
}

bool TimerField::stepTimerIsRunning() const
{
  return stepPeriod > 0;
}


// Acceleration Timer ------------------------------------------------------

void TimerField::acc_isr()
{
  if (accUpdatePeriod == 0)
    return;

  chVTSetI(&acc_vt, accUpdatePeriod,acc_timer_isr, this); // order next one timer
  // TODO: ordering times like this may slow things down if timers IRQs are delayed. Using absolute time would be better

  handler->accTimerISR();
}

void TimerField::accTimerStart()
{
  chSysLockFromISR();
  acc_isr();  // odd way of doing this, but thats how it's done on Teensy 3
  chSysUnlockFromISR();
}

void TimerField::setAccUpdatePeriod(unsigned p)
{
  accUpdatePeriod = TIME_US2I(p);
}

void TimerField::accTimerStop()
{
  accUpdatePeriod = 0;
}



// Delay Timer ------------------------------------------------------

void TimerField::delay_isr()
{
  handler->pulseTimerISR();
}


void TimerField::setPulseWidth(unsigned _delay)
{
  delayTime = TIME_US2I(_delay);
}

void TimerField::triggerDelay()
{
  chVTSetI(&delay_vt, delayTime, delay_timer_isr, this); // order a timer
  // this is typically very short
}
