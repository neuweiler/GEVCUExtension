/*
 * TickHandler.h
 *
 */

#ifndef TICKHANDLER_H_
#define TICKHANDLER_H_

#include <DueTimer.h>
#include "Logger.h"

#define NUM_TIMERS 9

class TickObserver
{
public:
    virtual void handleTick();
};

class TickHandler
{
public:
    static TickHandler *getInstance();
    void attach(TickObserver *observer, uint32_t interval);
    void detach(TickObserver *observer);
    void handleInterrupt(int timerNumber);  // must be public when from the non-class functions
    void cleanBuffer();
    void process();

protected:

private:
    struct TimerEntry
    {
        long interval; // interval of timer
        TickObserver *observer[CFG_TIMER_NUM_OBSERVERS]; // array of pointers to observers with this interval
    };
    TimerEntry timerEntry[NUM_TIMERS]; // array of timer entries (9 as there are 9 timers)
    TickObserver *tickBuffer[CFG_TIMER_BUFFER_SIZE];
    volatile uint16_t bufferHead, bufferTail;

    TickHandler();
    int findTimer(long interval);
    int findObserver(int timerNumber, TickObserver *observer);
};

void timer0Interrupt();
void timer1Interrupt();
void timer2Interrupt();
void timer3Interrupt();
void timer4Interrupt();
void timer5Interrupt();
void timer6Interrupt();
void timer7Interrupt();
void timer8Interrupt();

#endif /* TICKHANDLER_H_ */
