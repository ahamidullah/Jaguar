#pragma once

#include "PCH.h"
#include "Common.h"

const auto CPUCacheLineSize = 64;

void CPUSpinWaitHint();
s64 CPUProcessorCount();
s64 CPUPageSize();
