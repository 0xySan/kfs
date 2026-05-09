#include "../includes/kernel.h"

#define MAX_SIGNAL_CALLBACKS 256
#define MAX_PENDING_SIGNALS 16

typedef struct s_pending_signal
{
	int used;
	uint32_t signal_num;
	void *payload;
} pending_signal_t;

static signal_callback_t g_signal_callbacks[MAX_SIGNAL_CALLBACKS] = {0};
static pending_signal_t g_pending_signals[MAX_PENDING_SIGNALS] = {0};

void register_signal_callback(uint32_t signal_num, signal_callback_t callback)
{
	if (signal_num >= MAX_SIGNAL_CALLBACKS)
	{
		printk("SIGNAL", "signal %u is out of range\n", signal_num);
		return;
	}
	g_signal_callbacks[signal_num] = callback;
}

void schedule_signal(uint32_t signal_num, void *payload)
{
	uint32_t i;

	for (i = 0; i < MAX_PENDING_SIGNALS; i++)
	{
		if (!g_pending_signals[i].used)
		{
			g_pending_signals[i].used = 1;
			g_pending_signals[i].signal_num = signal_num;
			g_pending_signals[i].payload = payload;
			return;
		}
	}
	printk("SIGNAL", "signal queue full, dropping signal %u\n", signal_num);
}

void process_scheduled_signals(void)
{
	uint32_t i;
	uint32_t signal_num;
	void *payload;

	for (i = 0; i < MAX_PENDING_SIGNALS; i++)
	{
		if (!g_pending_signals[i].used)
			continue;
		g_pending_signals[i].used = 0;
		signal_num = g_pending_signals[i].signal_num;
		payload = g_pending_signals[i].payload;
		if (signal_num < MAX_SIGNAL_CALLBACKS && g_signal_callbacks[signal_num])
			g_signal_callbacks[signal_num](signal_num, payload);
		else
			printk("SIGNAL", "no callback registered for signal %u\n", signal_num);
	}
}