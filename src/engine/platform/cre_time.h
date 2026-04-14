#ifndef CRE_TIME_H
#define CRE_TIME_H

struct TimeContext;

void timeSystem_Init(TimeContext* time);
void timeSystem_Update(TimeContext* time);
bool timeSystem_ConsumeFixedStep(TimeContext* time, float fixedDt);

#endif