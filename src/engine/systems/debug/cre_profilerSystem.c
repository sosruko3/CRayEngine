#include "cre_profilerSystem.h"

#if CRE_ENABLE_PROFILER

#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	double start_time[PROF_MAX_BUCKETS];
	double sum_seconds[PROF_MAX_BUCKETS];
	uint32_t sample_count[PROF_MAX_BUCKETS];
	bool is_open[PROF_MAX_BUCKETS];
	double print_accumulator_seconds;
	char line_buffer[256];
} ProfilerState;

static ProfilerState s_profiler = {0};

static bool Profiler_IsBucketValid(ProfilerBucket bucket) {
	return bucket >= 0 && bucket < PROF_MAX_BUCKETS;
}

void Profiler_StartBucket(ProfilerBucket bucket) {
	if (!Profiler_IsBucketValid(bucket)) {
		return;
	}

#ifndef NDEBUG
	assert(!s_profiler.is_open[bucket] && "PROFILE_START called before PROFILE_END");
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
	assert(s_profiler.is_open[bucket] && "PROFILE_END called without PROFILE_START");
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

void Profiler_UpdateAndPrint(float dt) {
	double actv = 0.0;
	double scn = 0.0;
	double ecs = 0.0;
	double phy = 0.0;
	double ani = 0.0;
	double cam = 0.0;
	double ren = 0.0;
	double cln = 0.0;

	s_profiler.print_accumulator_seconds += (double)dt;
	if (s_profiler.print_accumulator_seconds < 1.0) {
		return;
	}

	actv = (s_profiler.sum_seconds[PROF_TOTAL_ACTIVE] /
					(double)(s_profiler.sample_count[PROF_TOTAL_ACTIVE] != 0U
											 ? s_profiler.sample_count[PROF_TOTAL_ACTIVE]
											 : 1U)) *
				 1000.0;
	scn = (s_profiler.sum_seconds[PROF_SCENE] /
				 (double)(s_profiler.sample_count[PROF_SCENE] != 0U
											? s_profiler.sample_count[PROF_SCENE]
											: 1U)) *
				1000.0;
	ecs = (s_profiler.sum_seconds[PROF_ECS_SYS] /
				 (double)(s_profiler.sample_count[PROF_ECS_SYS] != 0U
											? s_profiler.sample_count[PROF_ECS_SYS]
											: 1U)) *
				1000.0;
	phy = (s_profiler.sum_seconds[PROF_PHYSICS] /
				 (double)(s_profiler.sample_count[PROF_PHYSICS] != 0U
											? s_profiler.sample_count[PROF_PHYSICS]
											: 1U)) *
				1000.0;
	ani = (s_profiler.sum_seconds[PROF_ANIMATION] /
				 (double)(s_profiler.sample_count[PROF_ANIMATION] != 0U
											? s_profiler.sample_count[PROF_ANIMATION]
											: 1U)) *
				1000.0;
	cam = (s_profiler.sum_seconds[PROF_CAMERA] /
				 (double)(s_profiler.sample_count[PROF_CAMERA] != 0U
											? s_profiler.sample_count[PROF_CAMERA]
											: 1U)) *
				1000.0;
	ren = (s_profiler.sum_seconds[PROF_RENDER] /
				 (double)(s_profiler.sample_count[PROF_RENDER] != 0U
											? s_profiler.sample_count[PROF_RENDER]
											: 1U)) *
				1000.0;
	cln = (s_profiler.sum_seconds[PROF_CLEANUP] /
				 (double)(s_profiler.sample_count[PROF_CLEANUP] != 0U
											? s_profiler.sample_count[PROF_CLEANUP]
											: 1U)) *
				1000.0;

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