#pragma once

// @TODO: Change the default values to 0 and 1 for easier intitialization.
const auto HashTableVacantHashSentinel = (u64)-1;
const auto HashTableDeletedHashSentinel = (u64)-2;
const auto HashTableDefaultInitialLength = 16;
const auto HashTableMaxLoadFactor = 0.75f;

template <typename K, typename V>
struct KeyValuePair
{
	K key;
	V value;
};

template <typename K, typename V> struct HashTableIterator;

template <typename K, typename V>
struct HashTable
{
	typedef u64 (*HashProcedure)(K);
	Array<u64> hashes;
	Array<KeyValuePair<K, V>> buckets;
	HashProcedure hashProcedure;
	s64 count;
	f32 loadFactor;

	HashTableIterator<K, V> begin();
	HashTableIterator<K, V> end();
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
HashTable<K, V> NewHashTableIn(Allocator *a, s64 cap, typename HashTable<K, V>::HashProcedure hp)
{
	auto ht = HashTable<K, V>
	{
		.hashes = NewArrayIn<u64>(a, cap),
		.buckets = NewArrayIn<KeyValuePair<K, V>>(a, cap),
		.hashProcedure = hp,
	};
	for (auto &h : ht.hashes)
	{
		h = HashTableVacantHashSentinel;
	}
	return ht;
}

template <typename K, typename V>
HashTable<K, V> NewHashTable(s64 cap, typename HashTable<K, V>::HashProcedure hp)
{
	return NewHashTableIn<K, V>(ContextAllocator(), cap, hp);
}

template <typename K, typename V>
HashTableIterator<K, V> HashTable<K, V>::begin()
{
	auto itr = HashTableIterator<K, V>
	{
		.hashTable = this,
		.index = -1,
	};
	return ++itr;
}

template <typename K, typename V>
HashTableIterator<K, V> HashTable<K, V>::end()
{
	return
	{
		.hashTable = this,
		.index = this->buckets.count,
	};
}

template <typename K, typename V>
void DoHashTableInsert(HashTable<K, V> *ht, u64 hash, K k, V v)
{
	if (hash == HashTableVacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedHashSentinel)
	{
		hash = 1;
	}
	auto i = hash % ht->buckets.count;
	auto end = i;
	do
	{
		if (ht->hashes[i] == HashTableVacantHashSentinel)
		{
			ht->hashes[i] = hash;
			ht->buckets[i].key = k;
			ht->buckets[i].value = v;
			ht->count += 1;
			ht->loadFactor = (f32)ht->count / (f32)ht->buckets.count;
			return;
		}
		else if (ht->hashes[i] == hash && ht->buckets[i].key == k)
		{
			return;
		}
		i = (i + 1) % ht->buckets.count;
		// The table should expand before we run out of space, so we never wrap around to the start index.
		Assert(i != end);
	} while (i != end);
}

template <typename K, typename V>
void HashTable<K, V>::Insert(K k, V v)
{
	// The user has to set the hash procedure, even if the HashTable is zero-initialized.
	Assert(this->hashProcedure);
	this->Reserve(this->count + 1);
	auto h = this->hashProcedure(k);
	DoHashTableInsert(this, h, k, v);
}

template <typename K, typename V>
V *DoHashTableLookup(HashTable<K, V> *h, K k, V *notFound)
{
	if (h->buckets.count == 0)
	{
		return notFound;
	}
	auto hash = h->hashProcedure(k);
	if (hash == HashTableVacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedHashSentinel)
	{
		hash = 1;
	}
	auto i = hash % h->buckets.count;
	auto end = i;
	do
	{
		if (h->hashes[i] == HashTableVacantHashSentinel)
		{
			return notFound;
		}
		else if (h->hashes[i] == hash && h->buckets[i].key == k)
		{
			return &h->buckets[i].value;
		}
		i = (i + 1) % h->buckets.count;
	} while (i != end);
	return NULL;
}

template <typename K, typename V>
V HashTable<K, V>::Lookup(K k, V notFound)
{
	return *DoHashTableLookup(this, k, &notFound);
}

template <typename K, typename V>
V *HashTable<K, V>::LookupPointer(K k)
{
	auto notFound = (V *){};
	return DoHashTableLookup(this, k, notFound);
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
	if ((f32)reserve / (f32)this->buckets.count <= HashTableMaxLoadFactor)
	{
		return;
	}
	auto newHT = NewHashTableIn<K, V>(this->buckets.allocator, reserve * 2, this->hashProcedure);
	for (auto i = 0; i < this->buckets.count; i += 1)
	{
		if (this->hashes[i] == HashTableVacantHashSentinel || this->hashes[i] == HashTableDeletedHashSentinel)
		{
			continue;
		}
		DoHashTableInsert(&newHT, this->hashes[i], this->buckets[i].key, this->buckets[i].value);
	}
	*this = newHT;
	/*
	auto newHashes = NewArrayIn<u64>(this->hashes.allocator, (this->buckets.count + reserve) * 2);
	for (auto &h : newHashes)
	{
		h = HashTableVacantHashSentinel;
	}
	auto newBuckets = NewArrayIn<KeyValuePair<K, V>>(this->buckets.allocator, newHashes.count);
	for (auto &b : newBuckets)
	{
		b = {};
	}
	for (auto i = 0; i < this->buckets.count; i += 1)
	{
		if (this->hashes[i] == HashTableVacantHashSentinel || this->hashes[i] == HashTableDeletedHashSentinel)
		{
			continue;
		}
		auto newHash = this->hashProcedure(this->buckets[i].key);
		auto ni = newHash % newBuckets.count;
		Assert(newHashes[ni] == HashTableVacantHashSentinel);
		newHashes[ni] = newHash;
		newBuckets[ni].key = this->buckets[i].key;
		newBuckets[ni].value = this->buckets[i].value;
	}
	this->buckets = newBuckets;
	this->hashes = newHashes;
	this->loadFactor = (f32)this->count / (f32)this->buckets.count;
	*/
}

template <typename K, typename V>
struct HashTableIterator
{
	HashTable<K, V> *hashTable;
	s64 index;

	HashTableIterator<K, V> operator++();
	KeyValuePair<K, V> operator*();
	bool operator==(HashTableIterator<K, V> itr);
};

template <typename K, typename V>
HashTableIterator<K, V> HashTableIterator<K, V>::operator++()
{
	this->index += 1;
	for (; this->index < this->hashTable->buckets.count; this->index += 1)
	{
		if (this->hashTable->hashes[this->index] != HashTableVacantHashSentinel && this->hashTable->hashes[this->index] != HashTableDeletedHashSentinel)
		{
			break;
		}
	}
	return
	{
		.hashTable = this->hashTable,
		.index = this->index,
	};
}

template <typename K, typename V>
KeyValuePair<K, V> HashTableIterator<K, V>::operator*()
{
	return this->hashTable->buckets[this->index];
}

template <typename K, typename V>
bool HashTableIterator<K, V>::operator==(HashTableIterator<K, V> itr)
{
	return this->hashTable == itr.hashTable && this->index == itr.index;
}
