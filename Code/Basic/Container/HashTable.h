#include "Array.h"
#include "Log.h"

const auto VACANT_HASH_TABLE_KEY_SENTINEL = -1;
const auto DELETED_HASH_TABLE_KEY_SENTINEL = -2;

// @TODO: Initializer list.

typedef u64 (*HashProcedure)(void *);

template <typename K, typename V>
struct KeyValuePair
{
	K key;
	V value;
};

template <typename K, typename V>
struct HashTable
{
	Array<KeyValuePair> buckets;
	HashProcedure hash;
	s64 occupiedSlotCount;
	f32 loadFactor;
};

template <typename K, typename V>
void CreateHashTable(s64 initialCapacity, HashProcedure hash)
{
	auto result = HashTable<K, V>
	{
		.keys = CreateArray(initialCapacity),
		.values = CreateArray(initialCapacity),
		.hash = hash,
	}
	for (auto b : result.buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
		b.value = {};
	}
	return result;
}

template <typename K, typename V>
void ClearHashTable(HashTable<K, V> *h)
{
	h->occupiedSlotCount = 0;
	for (auto &b : h->buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
	}
}

template <typename K, typename V>
void ClearAndResizeHashTable(HashTable<K, V> *h, s64 newCapacity)
{
	h->occupiedSlotCount = 0;
	ResizeArray(&h->buckets, newCapacity);
	for (auto &b : h->buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
	}
}

template <typename K, typename V>
void ResizeHashTable(HashTable<K, V> *h)
{
	// @TODO
}

template <typename K, typename V>
void DoInsertIntoHashTable(HashTable<K, V> *h, K key, V value, bool overwriteIfExists)
{
	Assert(h->buckets.count > 0);

	auto keyHash = h->hash(&key);
	if (keyHash == VACANT_HASH_TABLE_KEY_SENTINEL)
	{
		keyHash = 0;
	}
	else if (keyHash == DELETED_HASH_TABLE_KEY_SENTINEL)
	{
		keyHash = 1;
	}
	auto startIndex = keyHash % h->buckets.count;
	auto index = startIndex;
	do
	{
		if (h->buckets[index].key == VACANT_HASH_TABLE_KEY_SENTINEL)
		{
			h->buckets[index].key = key;
			h->buckets[index].value = value;
			return;
		}
		else if (h->buckets[index].key == key)
		{
			if (overwriteIfExists)
			{
				h->buckets[index].key = key;
				h->buckets[index].value = value;
			}
			return;
		}
		index = (index + 1) % h->buckets.count;
	} while(index != startIndex);

	h->occupiedSlotCount += 1;
	h->loadFactor = h->occupiedSlotCount / h->buckets.count;
	if (h->loadFactor > 0.75f)
	{
		auto newBuckets = CreateArray<KeyValuePair<K, V>>(h->buckets.count * 2);
		for (auto i = 0; i < h->buckets.count; i++)
		{
			auto index = h->hash(h->keys[i]) % newBuckets.count;
			newBuckets[index].key = h->buckets[i].key;
			newBuckets[index].value = h->buckets[i].value;
		}
		DestroyArray(&h->buckets);
		h->buckets = newBuckets;
	}
}

template <typename K, typename V>
void InsertIntoHashTable(HashTable<K, V> *h, K key, V value)
{
	DoInsertIntoHashTable(h, key, value, false);
}

template <typename V>
void InsertIntoHashTable(HashTable<String, V> *h, const String &key, const V &value)
{
	DoInsertIntoHashTable(h, key, value, false);
}

template <typename K, typename V>
V *InsertIntoHashTableIfNonExistent(HashTable<K, V> *h, const K &key, const V &value)
{
	DoInsertIntoHashTable(h, key, value, true);
}

template <typename K, typename V>
V *LookupInHashTable(HashTable<K, V> *h, const K &key)
{
	if (h->buckets.count == 0)
	{
		return NULL;
	}
	auto keyHash = Hash(key);
	auto index = keyHash % h->buckets.count;
	auto lastIndex = index;
	do
	{
		if (h->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL)
		{
			return NULL;
		}
		else if (h->buckets[index].keyHash == keyHash)
		{
			return &h->buckets[index].value;
		}
		index = (index + 1) % h->buckets.count;
	} while (index != lastIndex);
	return NULL;
}

template <typename K, typename V>
void RemoveFromHashTable(HashTable<K, V> *h, const K &key)
{
	// @TODO
}
