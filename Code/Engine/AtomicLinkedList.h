#pragma once

#include "Code/Basic/Assert.h"

template <typename T>
struct AtomicLinkedList
{
	T *head = NULL;
};

template <typename T>
T *PopFromFrontOfAtomicLinkedList(AtomicLinkedList<T> *list)
{
	auto head = (T *){};
	do
	{
		head = list->head;
		Assert(head); // @TODO: Handle empty list?
	} while (AtomicCompareAndSwap((void *volatile *)&list->head, head, head->next) != head);
	return head;
}

template <typename T>
void PushToFrontOfAtomicLinkedList(AtomicLinkedList<T> *list, T *newHead)
{
	auto head = (T *){};
	do
	{
		head = list->head;
		newHead->next = head;
	} while (AtomicCompareAndSwap((void *volatile *)&list->head, head, newHead) != head);
}
