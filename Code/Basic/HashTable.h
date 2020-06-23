constexpr auto VACANT_HASH_TABLE_KEY_SENTINEL = -1;
constexpr auto DELETED_HASH_TABLE_KEY_SENTINEL = -2;
constexpr auto DEFAULT_INITIAL_HASH_TABLE_LENGTH = 16;

// @TODO: Initializer list.
// @TODO: Handle zero type.
//        How to handle empty hash? Maybe try to guess the hash function based on the type of key and abort if we fail.

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
	Array<KeyValuePair<K, V>> buckets;
	HashProcedure hash;
	s64 occupied;
	f32 loadFactor;
};

template <typename K, typename V>
void NewHashTable(s64 len, HashProcedure h)
{
	auto r = HashTable<K, V>
	{
		.buckets = NewArray<KeyValuePair<K, V>>(len),
		.hash = h,
	};
	for (auto &b : r.buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
		b.value = {};
	}
	return r;
}

template <typename K, typename V>
void NewHashTableIn(s64 len, HashProcedure h, AllocatorInterface a)
{
	auto r = HashTable<K, V>
	{
		.buckets = NewArrayIn<KeyValuePair<K, V>>(len, a),
		.hash = h,
	};
	for (auto &b : r.buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
		b.value = {};
	}
	return r;
}

template <typename K, typename V>
void ClearHashTable(HashTable<K, V> *h)
{
	h->occupied = 0;
	for (auto &b : h->buckets)
	{
		b.key = VACANT_HASH_TABLE_KEY_SENTINEL;
	}
}

template <typename K, typename V>
void ClearAndResizeHashTable(HashTable<K, V> *h, s64 len)
{
	h->occupied = 0;
	ResizeArray(&h->buckets, len);
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
void DoInsertIntoHashTable(HashTable<K, V> *h, K k, V v, bool overwrite)
{
	Assert(h->hash); // The user has to set the hash procedure, even if the HashTable is zero-initialized.
	if (h->buckets.count > 0)
	{
		*h = NewHashTable(DEFAULT_INITIAL_HASH_TABLE_LENGTH, h->hash);
	}
	auto keyHash = h->hash(&k);
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
			h->buckets[index].key = k;
			h->buckets[index].value = v;
			return;
		}
		else if (h->buckets[index].key == k)
		{
			if (overwrite)
			{
				h->buckets[index].key = k;
				h->buckets[index].value = v;
			}
			return;
		}
		index = (index + 1) % h->buckets.count;
	} while(index != startIndex);
	h->occupied += 1;
	h->loadFactor = h->occupied / h->buckets.count;
	if (h->loadFactor > 0.75f)
	{
		auto newBuckets = CreateArray<KeyValuePair<K, V>>(h->buckets.count * 2);
		for (auto i = 0; i < h->buckets.count; i++)
		{
			auto ni = h->hash(h->keys[i]) % newBuckets.count;
			newBuckets[ni].key = h->buckets[i].key;
			newBuckets[ni].value = h->buckets[i].value;
		}
		FreeArray(&h->buckets);
		h->buckets = newBuckets;
	}
}

template <typename K, typename V>
void InsertIntoHashTable(HashTable<K, V> *h, K k, V v)
{
	DoInsertIntoHashTable(h, k, v, false);
}

template <typename V>
void InsertIntoHashTable(HashTable<String, V> *h, const String &k, const V &v)
{
	DoInsertIntoHashTable(h, k, v, false);
}

template <typename K, typename V>
V *InsertIntoHashTableIfNonExistent(HashTable<K, V> *h, const K &k, const V &v)
{
	DoInsertIntoHashTable(h, k, v, true);
}

template <typename K, typename V>
V *LookupInHashTable(HashTable<K, V> *h, const K &k)
{
	if (h->buckets.count == 0)
	{
		return NULL;
	}
	auto keyHash = h->hash(k);
	if (keyHash == VACANT_HASH_TABLE_KEY_SENTINEL)
	{
		keyHash = 0;
	}
	else if (keyHash == DELETED_HASH_TABLE_KEY_SENTINEL)
	{
		keyHash = 1;
	}
	auto index = keyHash % h->buckets.count;
	auto lastIndex = index;
	do
	{
		if (h->buckets[index].key == VACANT_HASH_TABLE_KEY_SENTINEL)
		{
			return NULL;
		}
		else if (h->buckets[index].key == k)
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
