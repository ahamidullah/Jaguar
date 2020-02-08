#pragma once

template <typename T>
struct AtomicLinkedList {
	T *head = NULL;
};

template <typename T>
T *PopFromFront(AtomicLinkedList<T> *l) {
	T *currentHead;
	do {
		currentHead = l->head;
		Assert(currentHead);
	} while (PlatformCompareAndSwapPointers((void *volatile *)&l->head, currentHead, l->head->next) != currentHead);
	return currentHead;
}

template <typename T>
void PushToFront(AtomicLinkedList<T> *l, T *newHead) {
	T *currentHead;
	do {
		currentHead = l->head;
		newHead->next = currentHead;
	} while (PlatformCompareAndSwapPointers((void *volatile *)&l->head, currentHead, newHead) != currentHead);
}

