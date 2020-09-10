const auto HashTableVacantHashSentinel = (u64)-1;
const auto HashTableDeletedHashSentinel = (u64)-2;
const auto HashTableDefaultInitialLength = 16;
const auto HashTableMaxLoadFactor = 0.75f;

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
	Array<u64> hashes;
	Array<KeyValuePair<K, V>> buckets;
	HashProcedure hashProcedure;
	s64 count;
	f32 loadFactor;

	void Insert(K k, V v);
	V Lookup(K k, V notFound);
	V *LookupPointer(K k);
	void Remove(K k);
	void Clear();
	void ClearAndResize(s64 len);
	void Reserve(s64 count);
	void Resize();
};

template <typename K, typename V>
HashTable<K, V> NewHashTableIn(Allocator *a, s64 len, HashProcedure hp)
{
	auto ht = HashTable<K, V>
	{
		.hashes = NewArrayIn<u64>(a, len),
		.buckets = NewArrayIn<KeyValuePair<K, V>>(a, len),
		.hashProcedure = hp,
	};
	for (auto &h : ht.hashes)
	{
		h = HashTableVacantHashSentinel;
	}
	return ht;
}

template <typename K, typename V>
HashTable<K, V> NewHashTable(s64 len, HashProcedure hp)
{
	return NewHashTableIn<K, V>(ContextAllocator(), len, hp);
}

template <typename K, typename V>
void HashTable<K, V>::Insert(K k, V v)
{
	// The user has to set the hash procedure, even if the HashTable is zero-initialized.
	Assert(this->hashProcedure);
	this->Reserve(this->count + 1);
	// @TODO: Would a two-pass solution be faster? First pass figure which keys need to be inserted
	// and their indices, then resize the table if necessary, then second pass does the insertions.
	auto hash = this->hashProcedure(&k);
	if (hash == HashTableVacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedHashSentinel)
	{
		hash = 1;
	}
	auto startIndex = hash % this->buckets.count;
	auto index = startIndex;
	do
	{
		if (this->hashes[index] == HashTableVacantHashSentinel)
		{
			this->hashes[index] = hash;
			this->buckets[index].key = k;
			this->buckets[index].value = v;
			break;
		}
		else if (this->hashes[index] == hash && this->buckets[index].key == k)
		{
			break;
		}
		index = (index + 1) % this->buckets.count;
		// The table should have expanded before we ever run out of space. Thus, we should never
		// wrap around to the start index.
		Assert(index != startIndex);
	} while (index != startIndex);
	this->count += 1;
	this->loadFactor = (f32)this->count / (f32)this->buckets.count;
}

template <typename K, typename V>
V *DoLookup(HashTable<K, V> *h, K k, V *notFound)
{
	if (h->buckets.count == 0)
	{
		return notFound;
	}
	auto hash = h->hashProcedure(&k);
	if (hash == HashTableVacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedHashSentinel)
	{
		hash = 1;
	}
	auto find = hash % h->buckets.count;
	auto lastFind = find;
	do
	{
		if (h->hashes[find] == HashTableVacantHashSentinel)
		{
			return notFound;
		}
		else if (h->hashes[find] == hash && h->buckets[find].key == k)
		{
			return &h->buckets[find].value;
		}
		find = (find + 1) % h->buckets.count;
	} while (find != lastFind);
	return NULL;
}

template <typename K, typename V>
V HashTable<K, V>::Lookup(K k, V notFound)
{
	return *DoLookup(this, k, &notFound);
}

template <typename K, typename V>
V *HashTable<K, V>::LookupPointer(K k)
{
	auto notFound = (V *){};
	return DoLookup(this, k, &notFound);
}

template <typename K, typename V>
void HashTable<K, V>::Remove(K k)
{
	// @TODO
}

template <typename K, typename V>
void HashTable<K, V>::Clear()
{
	this->count = 0;
	for (auto &hash : this->hashes)
	{
		hash = HashTableVacantHashSentinel;
	}
}

template <typename K, typename V>
void HashTable<K, V>::ClearAndResize(s64 len)
{
	this->count = 0;
	this->buckets.Resize(len);
	for (auto &hash : this->hashes)
	{
		hash = HashTableVacantHashSentinel;
	}
}

template <typename K, typename V>
void HashTable<K, V>::Reserve(s64 reserve)
{
	if (this->buckets.count > reserve)
	{
		return;
	}
	if (this->buckets.count == 0)
	{
		this->hashes.Resize(reserve * 2);
		for (auto &h : this->hashes)
		{
			h = HashTableVacantHashSentinel;
		}
		this->buckets.Resize(reserve * 2);
		return;
	}
	if ((f32)(this->count + reserve) / (f32)this->buckets.count > HashTableMaxLoadFactor)
	{
		auto newHashes = NewArrayIn<u64>(this->hashes.allocator, (this->buckets.count + reserve) * 2);
		for (auto &h : newHashes)
		{
			h = HashTableVacantHashSentinel;
		}
		auto newBuckets = NewArrayIn<KeyValuePair<K, V>>(this->buckets.allocator, newHashes.count);
		for (auto i = 0; i < this->buckets.count; i++)
		{
			auto newHash = this->hashProcedure(&this->buckets[i].key);
			auto ni = newHash % newBuckets.count;
			newHashes[ni] = newHash;
			newBuckets[ni].key = this->buckets[i].key;
			newBuckets[ni].value = this->buckets[i].value;
		}
		this->hashes.Free();
		this->buckets.Free();
		this->buckets = newBuckets;
		this->hashes = newHashes;
		this->loadFactor = (f32)this->count / (f32)this->buckets.count;
	}
}

u64 HashU32(void *u)
{
	auto x = *(u32 *)u;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
