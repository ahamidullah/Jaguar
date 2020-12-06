#pragma once

#include "../PCH.h"
#include "Common.h"

namespace cpu
{

const auto CacheLineSize = 64;

void SpinWaitHint();
s64 ProcessorCount();
s64 PageSize();

}
