#ifndef CRE_PROFILERSYSTEM_H
#define CRE_PROFILERSYSTEM_H

#include <stdint.h>

#define CRE_ENABLE_PROFILER 1

typedef enum {
  PROF_TOTAL_ACTIVE = 0,
  PROF_SCENE,
  PROF_ECS_SYS,
  PROF_PHYSICS,
  PROF_ANIMATION,
  PROF_CAMERA,
  PROF_RENDER,
  PROF_CLEANUP,
  PROF_MAX_BUCKETS
} ProfilerBucket;

#if CRE_ENABLE_PROFILER
void Profiler_StartBucket(ProfilerBucket bucket);
void Profiler_EndBucket(ProfilerBucket bucket);
void Profiler_UpdateAndPrint(float dt);

#define PROFILE_START(bucket) Profiler_StartBucket((bucket))
#define PROFILE_END(bucket) Profiler_EndBucket((bucket))
#else
#define PROFILE_START(bucket) ((void)0)
#define PROFILE_END(bucket) ((void)0)

static inline void Profiler_UpdateAndPrint(float dt) { (void)dt; }
#endif

#endif
