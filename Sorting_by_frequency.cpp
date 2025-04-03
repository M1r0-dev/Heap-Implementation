#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <functional>
#include <type_traits>
using namespace std;

// A key-value node structure for use in various data structures.
template<typename K, typename V>
struct KVNode {
    K key;
    V value;
    // Constructor that initializes key and value.
    KVNode(K key, V value) : key(key), value(value) {}
};

// Custom hash functor that supports integral types and strings.
// For other types, std::hash is used.
template<typename Key>
struct Hash {
    size_t operator()(const Key& key) const {
        if constexpr (is_integral<Key>::value) {
            // Multiply by a large prime constant for better distribution.
            return static_cast<size_t>(key * 2654435761UL);
        }
        else if constexpr (is_same<Key, string>::value) {
            // Simple hash for strings using a multiplier of 31.
            size_t hash = 0;
            for (char c : key) {
                hash = hash * 31 + static_cast<size_t>(c);
            }
            return hash;
        }
        else {
            // Fallback to the standard hash for other types.
            return std::hash<Key>()(key);
        }
    }
};

// UnorderedMap: a simple hash table implementation using separate chaining.
template<typename Key, typename Value, typename Hash = Hash<Key>>
class UnorderedMap {
private:
    // Internal node structure for each bucket.
    struct Node {
        Key key;
        Value value;
    };
    // Buckets: a vector of vectors of nodes.
    vector<vector<Node>> buckets;
    Hash hashFunc; // Hash function object.
    size_t numElements; // Total number of stored elements.
    static constexpr float loadFactor = 0.75f; // Threshold for rehashing.

    // Rehash: doubles the bucket count and redistributes all nodes.
    void rehash() {
        size_t newBucketCount = buckets.size() * 2;
        vector<vector<Node>> newBuckets(newBucketCount);
        // Move each node to its new bucket.
        for (auto& bucket : buckets) {
            for (auto& node : bucket) {
                size_t newIndex = hashFunc(node.key) % newBucketCount;
                newBuckets[newIndex].push_back(node);
            }
        }
        buckets = std::move(newBuckets);
    }
public:
    // Constructor with an optional initial bucket count (default 16).
    UnorderedMap(size_t bucketCount = 16) : buckets(bucketCount), numElements(0) {}

    // Insert a key-value pair into the map. If key exists, update its value.
    void insert(const Key& key, const Value& value) {
        // Rehash if load factor threshold exceeded.
        if (numElements > buckets.size() * loadFactor)
            rehash();
        size_t index = hashFunc(key) % buckets.size();
        // Check if the key already exists in the bucket.
        for (auto& node : buckets[index]) {
            if (node.key == key) {
                node.value = value;
                return;
            }
        }
        // If key not found, add new node.
        buckets[index].push_back(Node{ key, value });
        ++numElements;
    }

    // Erase the element with the given key.
    bool erase(const Key& key) {
        size_t index = hashFunc(key) % buckets.size();
        auto& bucket = buckets[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                --numElements;
                return true;
            }
        }
        return false;
    }

    // Find element by key (non-const version). Returns pointer to value or nullptr.
    Value* find(const Key& key) {
        size_t index = hashFunc(key) % buckets.size();
        for (auto& node : buckets[index]) {
            if (node.key == key)
                return &node.value;
        }
        return nullptr;
    }

    // Find element by key (const version).
    const Value* find(const Key& key) const {
        size_t index = hashFunc(key) % buckets.size();
        for (const auto& node : buckets[index]) {
            if (node.key == key)
                return &node.value;
        }
        return nullptr;
    }

    // Overload operator[]: if key exists, return reference to its value;
    // otherwise, insert a default-constructed value and return reference to it.
    Value& operator[](const Key& key) {
        size_t index = hashFunc(key) % buckets.size();
        for (auto& node : buckets[index]) {
            if (node.key == key)
                return node.value;
        }
        // If rehashing is needed, perform it and recalculate the index.
        if (numElements > buckets.size() * loadFactor) {
            rehash();
            index = hashFunc(key) % buckets.size();
        }
        buckets[index].push_back(Node{ key, Value() });
        ++numElements;
        return buckets[index].back().value;
    }

    // Return all entries as a vector of KVNode for external use.
    vector<KVNode<Key, Value>> getEntries() {
        vector<KVNode<Key, Value>> out;
        for (const auto& bucket : buckets) {
            for (const auto& node : bucket) {
                out.push_back(KVNode<Key, Value>(node.key, node.value));
            }
        }
        return out;
    }
};

// MaxHeap: a d-ary max heap implementation using KVNode for storage.
template<typename K, typename V>
class MaxHeap {
private:
    // The heap is stored as a vector of KVNode objects.
    vector<KVNode<K, V>> heap;
    int d = 2; // d-ary heap (default binary heap)

    // Swap two nodes.
    void swapNodes(KVNode<K, V>& a, KVNode<K, V>& b) {
        KVNode<K, V> tmp = a;
        a = b;
        b = tmp;
    }

    // Heapify down: ensures max-heap property from index 'ind' downward.
    void hDown(int ind) {
        int maxIdx = ind;
        // Check all d children.
        for (int i = 1; i <= d; i++) {
            int child = d * ind + i;
            if (child < heap.size() && heap[child].key > heap[maxIdx].key)
                maxIdx = child;
        }
        // If a larger child is found, swap and continue heapifying down.
        if (maxIdx != ind) {
            swapNodes(heap[ind], heap[maxIdx]);
            hDown(maxIdx);
        }
    }

    // Heapify up: move the element at index 'ind' up to restore heap property.
    void hUp(int ind) {
        if (ind == 0)
            return;
        int parent = (ind - 1) / d;
        if (heap[ind].key > heap[parent].key) {
            swapNodes(heap[ind], heap[parent]);
            hUp(parent);
        }
    }

    // Heapify helper for heapSort: maintains heap property for array 'arr' up to heapSize.
    void heapify(vector<KVNode<K, V>>& arr, int i, int heapSize) {
        int largest = i;
        for (int j = 1; j <= d; j++) {
            int child = d * i + j;
            if (child < heapSize && arr[child].key > arr[largest].key)
                largest = child;
        }
        if (largest != i) {
            swapNodes(arr[i], arr[largest]);
            heapify(arr, largest, heapSize);
        }
    }

public:
    // Default constructor.
    MaxHeap() = default;
    // Constructor to set the arity 'd' for the heap.
    MaxHeap(int d) : d(d) {}

    // Insert a KVNode into the heap.
    void insert(const KVNode<K, V>& value) {
        heap.push_back(value);
        hUp(heap.size() - 1);
    }

    // Returns a constant reference to the internal heap vector.
    const vector<KVNode<K, V>>& getHeap() const {
        return heap;
    }

    // Returns the maximum value (at the root of the heap).
    V getMax() const {
        return heap[0].value;
    }

    // Extracts and removes the maximum element from the heap.
    V extractMax() {
        V maxVal = heap[0].value;
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty())
            hDown(0);
        return maxVal;
    }

    // Build the heap from an unsorted array of KVNodes.
    void build(const vector<KVNode<K, V>>& arr) {
        heap = arr;
        int start = (heap.size() - 1) / d;
        for (int i = start; i >= 0; i--)
            hDown(i);
    }

    // Check if the heap is empty.
    bool empty() {
        return heap.empty();
    }

    // Perform heap sort and return a vector of sorted values.
    vector<V> heapSort() {
        // Create a copy of the heap.
        vector<KVNode<K, V>> arr = heap;
        int n = arr.size();
        // Build a max heap from the array.
        for (int i = (n - 1) / d; i >= 0; i--) {
            heapify(arr, i, n);
        }
        // Extract elements one by one.
        for (int i = n - 1; i > 0; i--) {
            swapNodes(arr[0], arr[i]);
            heapify(arr, 0, i);
        }
        // Extract the sorted values.
        vector<V> sorted;
        for (const auto& node : arr) {
            sorted.push_back(node.value);
        }
        return sorted;
    }
};

int main() {
    int n;
    // Read number of elements.
    cin >> n;
    
    // Create an unordered map to count frequencies.
    UnorderedMap<int, int> mp;
    vector<int> in(n);
    // Read input numbers and increment their count.
    for (int i = 0; i < n; i++) {
        cin >> in[i];
        mp[in[i]]++;
    }
    
    // Get all key-value entries from the map.
    vector<KVNode<int, int>> entries = mp.getEntries();
    // Swap key and value for heap usage (e.g., frequency as key).
    for (int i = 0; i < entries.size(); i++) {
        swap(entries[i].key, entries[i].value);
    }
    
    // Build a max heap from the entries.
    MaxHeap<int, int> mh;
    mh.build(entries);
    
    // Perform heap sort on the heap.
    vector<int> sorted = mh.heapSort();
    // Output sorted values.
    for (auto x : sorted)
        cout << x << endl;
    
    return 0;
}
