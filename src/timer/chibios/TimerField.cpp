
#include "TimerField.h"


void acc_timer_isr(void *arg) {
  TimerField *tf = (TimerField *)arg;
  tf->acc_isr();  
}

void step_timer_isr(void *arg) {
  TimerField *tf = (TimerField *)arg;
  tf->step_isr();  
}

void delay_timer_isr(void *arg) {
  TimerField *tf = (TimerField *)arg;
  tf->delay_isr();  
}

