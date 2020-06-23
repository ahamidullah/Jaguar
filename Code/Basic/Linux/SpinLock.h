#pragma once

#include "Code/Common.h"

typedef s64 SpinLock;

void AcquireSpinLock(SpinLock *lock);
void ReleaseSpinLock(SpinLock *lock);
