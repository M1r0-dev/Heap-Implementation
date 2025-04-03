/*D-ary max-heap realization*/
#include <iostream>
#include <vector>
using namespace std;

// Template class for a d-ary max-heap.
template<typename T>
class MaxHeap {
private:
    // Internal storage for heap elements.
    vector<T> heap;
    // Number of children per node (d-ary heap).
    int d;

    // Swap two elements by reference.
    void swap(T& a, T& b) {
        T tmp = a;
        a = b;
        b = tmp;
    }

    // Heapify down: ensures that the subtree rooted at index 'ind' satisfies the max-heap property.
    void hDown(int ind) {
        int max = ind;
        // Iterate through all d children.
        for (int i = 1; i <= d; i++) {
            int child = d * ind + i;
            // If the child exists and is greater than the current max, update max.
            if (child < heap.size() && heap[child] > heap[max])
                max = child;
        }
        // If a larger child is found, swap and continue heapifying down.
        if (max != ind) {
            swap(heap[ind], heap[max]);
            hDown(max);
        }
    }

    // Heapify up: ensures that the element at index 'ind' is moved up to maintain the max-heap property.
    void hUp(int ind) {
        // Base case: reached the root.
        if (ind == 0)
            return;
        // Calculate parent's index.
        int parent = (ind - 1) / d;
        // If current element is greater than its parent, swap and continue heapifying up.
        if (heap[ind] > heap[parent]) {
            swap(heap[ind], heap[parent]);
            hUp(parent);
        }
    }
public:
    // Default constructor.
    MaxHeap() = default;
    // Constructor to initialize a d-ary heap with given number of children 'd'.
    MaxHeap(int d) : d(d) {}

    // Inserts a new value into the heap.
    void insert(const T& value) {
        heap.push_back(value); // Add the new value at the end.
        hUp(heap.size() - 1);  // Restore heap property by moving it up.
    }

    // Returns a constant reference to the internal heap vector.
    const vector<T>& getHeap() const {
        return heap;
    }

    // Builds a heap from an unsorted array.
    void build(const vector<T>& arr) {
        heap = arr; // Copy the array into the heap.
        // Start heapifying from the last non-leaf node down to the root.
        int start = (heap.size() - 1) / d;
        for (int i = start; i >= 0; i--)
            hDown(i);
    }

    // Checks whether the heap is empty.
    bool empty() {
        return heap.empty();
    }
};

int main() {
    int d, n;
    // Read number of elements and the arity 'd' for the heap.
    cin >> n >> d;
    // Create a max heap with specified d-ary structure.
    MaxHeap<int> mh(d);
    vector<int> tmp(n);
    // Read n elements into a temporary vector.
    for (int i = 0; i < n; i++)
        cin >> tmp[i];
    // Build the heap from the input array.
    mh.build(tmp);
    // Retrieve the internal heap vector.
    vector<int> out = mh.getHeap();
    // Output the elements of the heap.
    for (auto item : out)
        cout << item << " ";
    return 0;
}
