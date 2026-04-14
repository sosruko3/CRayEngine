#include "cre_time.h"
#include "engine/core/cre_types.h"
#include "cre_sys.h"

void timeSystem_Init(TimeContext* time) {
    time->realDt = 0.0f;
    time->gameDt = 0.0f;
    time->lastTime = Platform_GetTime();
    time->accumulator = 0.0f;
    time->timeScale = 1.0f;
}
void timeSystem_Update(TimeContext* time) {
    double current = Platform_GetTime();
    time->realDt = static_cast<float>(current - time->lastTime);
    time->lastTime = current;

    if (time->realDt > 0.1f) 
        time->realDt = 0.1f;
    
    time->gameDt = time->realDt * time->timeScale;
    time->accumulator += time->gameDt;
}

bool timeSystem_ConsumeFixedStep(TimeContext* time, float fixedDt) {
    if (time->accumulator >= fixedDt) {
        time->accumulator -= fixedDt;
        return true;
    }
    return false;
}