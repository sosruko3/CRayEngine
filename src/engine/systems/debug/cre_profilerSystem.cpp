#include "cre_profilerSystem.h"

#if CRE_ENABLE_PROFILER

#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

struct ProfilerState {
  double start_time[PROF_MAX_BUCKETS];
  double sum_seconds[PROF_MAX_BUCKETS];
  uint32_t sample_count[PROF_MAX_BUCKETS];
  bool is_open[PROF_MAX_BUCKETS];
  double print_accumulator_seconds;
  char line_buffer[256];
};

static ProfilerState s_profiler = {};

static bool Profiler_IsBucketValid(ProfilerBucket bucket) {
  return bucket >= 0 && bucket < PROF_MAX_BUCKETS;
}

void Profiler_StartBucket(ProfilerBucket bucket) {
  if (!Profiler_IsBucketValid(bucket)) {
    return;
  }

#ifndef NDEBUG
  assert(!s_profiler.is_open[bucket] &&
         "PROFILE_START called before PROFILE_END");
#endif

  s_profiler.start_time[bucket] = GetTime();
  s_profiler.is_open[bucket] = true;
}

void Profiler_EndBucket(ProfilerBucket bucket) {
  double now = 0.0;
  double elapsed = 0.0;

  if (!Profiler_IsBucketValid(bucket)) {
    return;
  }

#ifndef NDEBUG
  assert(s_profiler.is_open[bucket] &&
         "PROFILE_END called without PROFILE_START");
#endif

  if (!s_profiler.is_open[bucket]) {
    return;
  }

  now = GetTime();
  elapsed = now - s_profiler.start_time[bucket];

  if (elapsed < 0.0) {
    elapsed = 0.0;
  }

  s_profiler.sum_seconds[bucket] += elapsed;
  s_profiler.sample_count[bucket] += 1U;
  s_profiler.is_open[bucket] = false;
}
static double GetBucketAvgMs(ProfilerBucket bucket) {
  const uint32_t count = s_profiler.sample_count[bucket];
  const double divisor = static_cast<double>(count > 0 ? count : 1U);
  return (s_profiler.sum_seconds[bucket] / divisor) * 1000.0;
}
void Profiler_UpdateAndPrint(float dt) {
  s_profiler.print_accumulator_seconds += static_cast<double>(dt);

  if (s_profiler.print_accumulator_seconds < 1.0) {
    return;
  }
  const double actv = GetBucketAvgMs(PROF_TOTAL_ACTIVE);
  const double scn = GetBucketAvgMs(PROF_SCENE);
  const double ecs = GetBucketAvgMs(PROF_ECS_SYS);
  const double phy = GetBucketAvgMs(PROF_PHYSICS);
  const double ani = GetBucketAvgMs(PROF_ANIMATION);
  const double cam = GetBucketAvgMs(PROF_CAMERA);
  const double ren = GetBucketAvgMs(PROF_RENDER);
  const double cln = GetBucketAvgMs(PROF_CLEANUP);

  snprintf(s_profiler.line_buffer, sizeof(s_profiler.line_buffer),
           "\r[PROF] Actv: %.2f | Scn: %.2f | ECS: %.2f | Phy: %.2f | "
           "Ani: %.2f | Cam: %.2f | Ren: %.2f | Cln: %.2f     ",
           actv, scn, ecs, phy, ani, cam, ren, cln);

  fputs(s_profiler.line_buffer, stdout);
  fflush(stdout);

  for (int i = 0; i < PROF_MAX_BUCKETS; ++i) {
    s_profiler.sum_seconds[i] = 0.0;
    s_profiler.sample_count[i] = 0U;
  }

  s_profiler.print_accumulator_seconds -= 1.0;
}

#endif
