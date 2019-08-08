typedef struct Performance_Timer {
	Platform_Time start;
	f64 running_sum;
	u64 iteration;
	const char *name;
} Performance_Timer;

void start_performance_timer(Performance_Timer *timer) {
	timer->start = get_current_platform_time();
}

void print_performance_timer(Performance_Timer *timer) {
	Platform_Time end = get_current_platform_time();
	timer->running_sum = timer->running_sum + platform_time_difference(timer->start, end);
	debug_print("%s: %gams %gms\n", timer->name, timer->running_sum / timer->iteration, platform_time_difference(timer->start, end));
	timer->iteration++;
}

#if 0

void
timer_set(u32 wait_time, Timer *t)
{
	t->wait_time  = wait_time;
	t->start_time = platform_get_time_ms();
}

bool
timer_check_one_shot(Timer *t)
{
	if (t->start_time == 0) {
		return true;
	}

	u32 current_time = platform_get_time_ms();

	if ((current_time - t->start_time) >= t->wait_time) {
		t->start_time = 0;
		return true;
	}

	return false;
}

bool
timer_check_repeating(Timer *t)
{
	u32 current_time = platform_get_time_ms();

	if ((current_time - t->start_time) >= t->wait_time) {
		t->start_time = current_time;
		return true;
	}

	return false;
}
#endif
