typedef struct Performance_Timer {
	Platform_Time start;
	f64 running_sum;
	u64 iteration;
	const char *name;
} Performance_Timer;

#define Start_Performance_Timer(timer_name) Performance_Timer timer_##timer_name = {.name = #timer_name, .start = platform_get_current_time(), .iteration = 1}

#define Print_Performance_Timer(timer_name) Print_Performance_Timer_Actual(&timer_##timer_name)

void _Start_Performance_Timer(Performance_Timer *timer) {
	timer->start = Platform_Get_Current_Time();
}

void Print_Performance_Timer_Actual(Performance_Timer *timer) {
	Platform_Time end = Platform_Get_Current_Time();
	timer->running_sum = timer->running_sum + Platform_Time_Difference(timer->start, end);
	Console_Print("%s: %gms, avg %gms\n", timer->name, Platform_Time_Difference(timer->start, end), timer->running_sum / timer->iteration);
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
