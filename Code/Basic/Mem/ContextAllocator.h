#pragma once

#include "Allocator.h"

namespace mem
{

void PushContextAllocator(Allocator *a);
void PopContextAllocator();
Allocator *ContextAllocator();

}
