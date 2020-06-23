#include "../SpinLock.h"
#include "../Assert.h"
#include "../CPU.h"
#include "../Atomic.h"

void AcquireSpinLock(SpinLock *l)
{
	for (;;)
	{
		if (AtomicCompareAndSwap(l, 0, 1) == 1)
		{
			return;
		}
		while (*l != 0)
		{
			CPUHintSpinWaitLoop();
		}
	}
}

void ReleaseSpinLock(SpinLock *l)
{
	Assert(*l == 1);
	*l = 0;
}
