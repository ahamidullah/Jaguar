#pragma once

template <typename T>
struct Array {
	size_t Count = 0;
	size_t Capacity = 0;
	T *Elements = (T *)malloc(0);

	T &operator[](size_t I);
	T &operator[](size_t I) const;
};

template <typename T>
T &Array<T>::operator[](size_t I) {
	Assert(I < Count);
	return Elements[I];
}

template <typename T>
T &Array<T>::operator[](size_t I) const {
	Assert(I < Count);
	return Elements[I];
}

template <typename T>
bool operator==(const Array<T> &A, const Array<T> &B) {
	if (A.Count != B.Count) {
		return false;
	}
	for (size_t I = 0; I < A.Count; I++) {
		if (A.Elements[I] != B.Elements[I]) {
			return false;
		}
	}
	return true;
}

template <typename T>
Array<T> CreateArray(size_t Count, size_t Capacity) {
	return {
		.Count = Count,
		.Capacity = Capacity,
		.Elements = (T *)malloc(sizeof(T) * Capacity),
	};
}

template <typename T>
Array<T> CreateArray(size_t Count) {
	return {
		.Count = Count,
		.Capacity = Count,
		.Elements = (T *)malloc(sizeof(T) * Count),
	};
}

template<typename T>
size_t ArrayArgumentCount(const T&) {
    return 1;
}

template<typename T, typename... ElementPack>
size_t ArrayArgumentCount(const T& first, const ElementPack... rest) {
    return 1 + ArrayArgumentCount(rest...);
}

template<typename T>
void CopyArrayArguments(size_t writeIndex, Array<T> *result, const T &newElement) {
	result->Elements[writeIndex] = newElement;
}

template<typename T, typename... ElementPack>
void CopyArrayArguments(size_t writeIndex, Array<T> *result, const T &first, const ElementPack... rest) {
	result->Elements[writeIndex] = first;
	CopyArrayArguments(writeIndex + 1, result, rest...);
}

template<typename T, typename... ElementPack>
Array<T> CreateInitializedArray(const T &first, const ElementPack... rest) {
	Array<T> result = CreateArray<T>(ArrayArgumentCount(first, rest...));
	CopyArrayArguments(0, &result, first, rest...);
	return result;
}

template <typename T>
Array<T> CreateArray(size_t DataCount, const T *DataPointer) {
	Array<T> A = {
		.Count = DataCount,
		.Capacity = DataCount,
		.Elements = (T *)malloc(DataCount * sizeof(T)),
	};
	Copy_Memory(DataPointer, A.Elements, DataCount * sizeof(T));
	return A;
}

template <typename T>
void SetMinimumCapacity(Array<T> *A, size_t MinimumCapacity) {
	if (A->Capacity >= MinimumCapacity) {
		return;
	}
	while (A->Capacity < MinimumCapacity) {
		A->Capacity = (A->Capacity * 2) + 2;
	}
	A->Elements = (T *)realloc(A->Elements, sizeof(T) * A->Capacity);
#if 0
	if (!A->Elements) {
		InitializeArray(A, MinimumCapacity);
		return;
	}
	ReallocateArrayIfBelowCapacity(A, MinimumCapacity);
#endif
}

template <typename T>
void Resize(Array<T> *A, size_t NewCount) {
	SetMinimumCapacity(A, NewCount);
	A->Count = NewCount;
}

// @TODO: Make this variadic and take any number of new elements.
template <typename T>
void Append(Array<T> *A, const T &NewElement) {
	size_t NewElementIndex = A->Count;
	Resize(A, A->Count + 1);
	Assert(A->Count <= A->Capacity);
	Assert(NewElementIndex < A->Count);
	A->Elements[NewElementIndex] = NewElement;
}

template <typename T>
void SetSize(Array<T> *A, size_t NewCount) {
	A->Count = NewCount;
}

template <typename T>
void Append(Array<T> *Destination, const T *Source, size_t SourceCount) {
	size_t OldDestinationCount = Destination->Count;
	size_t NewDestinationCount = Destination->Count + SourceCount;
	Resize(Destination, NewDestinationCount);
	Copy_Memory(Source, Destination->Elements + OldDestinationCount, SourceCount * sizeof(T)); // @TODO
}

template <typename T>
void Append(Array<T> *Destination, const Array<T> &Source) {
	Append(Destination, Source.Elements, Source.Count);
}

template <typename T>
void Append(Array<T> *Destination, const Array<T> &Source, size_t SourceCount) {
	Append(Destination, Source.Elements, SourceCount);
}

#if 0
template<typename T>
void UnsafeAppend(Array<T> *Destination, const Array<T>& Source) {
	Copy_Memory(Source.Elements, &Destination->Elements[Destination->Count], Source.Count);
	Destination->Count += Source.Count;
}

template<typename T>
void UnsafeAppend(Array<T> *Destination, const Array<T>& Source, size_t SourceCount) {
	Copy_Memory(Source.Elements, &Destination->Elements[Destination->Count], SourceCount);
	Destination->Count += SourceCount;
}

template<typename T>
void UnsafeAppend(Array<T> *Destination, const T *Source, size_t SourceCount) {
	Copy_Memory(Source, &Destination->Elements[Destination->Count], SourceCount);
	Destination->Count += SourceCount;
}

template<typename T, typename... ArrayPack>
void UnsafeAppend(Array<T> *Result, const Array<T> &First, ArrayPack... Rest) {
	UnsafeAppend(Result, First);
	UnsafeAppend(Result, Rest...);
}
#endif

template <typename T>
size_t Length(const Array<T> &A) {
	return A.Count;
}

/*
template <typename T>
T *DataPointer(const Array<T> &A) {
	return &A.Elements;
}
*/

template <typename T>
T *begin(Array<T> &A) {
	return &A.Elements[0];
}

template <typename T>
T *end(Array<T> &A) {
	return &A.Elements[A.Count];
}

template <typename T>
T *begin(Array<T> *A) {
	return &A->Elements[0];
}

template <typename T>
T *end(Array<T> *A) {
	return &A->Elements[A->Count];
}

