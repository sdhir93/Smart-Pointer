/*
    Design a Shared Pointer Class

    gc_details.h

    This class defines an element that is stored
    in the garbage collection information list.

*/

template <class T>
class PtrDetails
{
public:
    unsigned refCount;  // Current reference count
    T* memPtr;          // Pointer to allocated memory
    bool isArray;       // True if pointing to array
    unsigned arraySize; // Size of array

    PtrDetails(T* mPtr, unsigned arrSize = 0)
    {
        memPtr    = mPtr;
        arraySize = arrSize;
        isArray   = (arrSize == 0);
        refCount++;
    }
};

// Overloading the equal to comparison operator
// Both objects will be same if they point to same
// allocated memory
template <class T>
bool operator==(const PtrDetails<T> &ob1, const PtrDetails<T> &ob2)
{
    return ( ob1.memPtr == ob2.memPtr );
}