#pragma once

#include "Allocator.h"

namespace Memory
{

void PushContextAllocator(Allocator *a);
void PopContextAllocator();
Allocator *ContextAllocator();

}
