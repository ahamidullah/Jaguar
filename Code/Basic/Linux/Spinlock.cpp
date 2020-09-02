#include "../Spinlock.h"
#include "../Assert.h"
#include "../CPU.h"
#include "../Atomic.h"

void Spinlock::Lock()
{
	for (;;)
	{
		if (AtomicCompareAndSwap64(&this->handle, 0, 1) == 0)
		{
			return;
		}
		while (this->handle != 0)
		{
			CPUHintSpinWaitLoop();
		}
	}
}

void Spinlock::Unlock()
{
	Assert(this->handle == 1);
	this->handle = 0;
}
