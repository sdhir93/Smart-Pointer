#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"

/*
    Class Pointer implements a pointer type that uses
    garbage collection to release unused memory.

    A Pointer must only be used to point to memory
    that was dynamically allocated using new.

    When used to refer to an allocated array,
    specify the array size.
*/

template <class T, int size = 0>
class Pointer
{
private:
    // RefContainer maintains the garbage collection list
    // It is a doubled linked list of PtrDetails
    static std::list<PtrDetails<T> > refContainer;

    // Address points to the allocated memory to which
    // this Pointer pointer currently points
    T* addr;

    // True if pointing to an allocated array
    // False otherwise
    bool isArray;

    unsigned arraySize; // Size of the array
    static bool first;  // True when first Pointer is created

    // Return an iterator to pointer details in refContainer
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T* ptr);

public:
    // Define an iterator type for Pointer<T>
    typedef Iter<T> GCiterator;

    // Empty constructor
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer()
    {
        Pointer(nullptr);
    }

    Pointer(T*);

    // Copy constructor
    Pointer(const Pointer &);

    // Destructor for Pointer
    ~Pointer();

    // Collect garbage. Returns true if at least
    // one object was freed
    static bool collect();

    // Overload assignment of pointer to Pointer
    T* operator=(T* t);

    // Overload assignment of Pointer to Pointer
    Pointer &operator=(Pointer &rv);

    // Return a reference to the object pointed
    // to by this Pointer
    T& operator*()
    {
        return *addr;
    }

    // Return the address being pointed to
    T* operator->()
    {
        return addr;
    }

    // Return a reference to the object at the
    // index specified by i
    T& operator[](int i)
    {
        return addr[i];
    }

    // Conversion function to T*
    operator T* ()
    {
        return addr;
    }

    // Return an Iter to the start of the allocated memory
    Iter<T> begin()
    {
        int _size = 0;
        if (isArray) _size = arraySize;

        return Iter<T>(addr, addr, addr + _size);
    }

    // Return an Iter to one past the end of an allocated array
    Iter<T> end()
    {
        int _size = 0;
        if (isArray) _size = arraySize;

        return Iter<T>(addr + _size, addr, addr + _size);
    }

    // Return the size of refContainer for this type of Pointer
    static int refContainerSize()
    {
        return refContainer.size();
    }

    // A utility function that displays refContainer
    static void showlist();

    // Clear refContainer when program exits
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;

template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T* t)
{
    // Register shutdown() as an exit function
    if (first) atexit(shutdown);

    first     = false;
    addr      = t;
    arraySize = size;
    isArray   = (arraySize > 0) ? true : false;

    PtrDetails<T> ptrDetails(t);
    ptrDetails.refCount     = 1;
    ptrDetails.isArray      = isArray;
    ptrDetails.arraySize    = arraySize;

    // Save the pointer details
    refContainer.push_back(ptrDetails);
}

// Copy constructor
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &obj)
{
    addr      = obj.addr;
    arraySize = obj.arraySize;
    isArray   = obj.isArray;
    auto p    = findPtrInfo(obj.addr);
    p->refCount++;
}

// Collect garbage. Returns true if at least
// one object was freed
template <class T, int size>
bool Pointer<T, size>::collect()
{
    bool memFreed = false;
    typename std::list<PtrDetails<T> > :: iterator p;
    do
    {
        for(p = refContainer.begin(); p != refContainer.end(); p++)
        {
            if(p->refCount > 0)
            {
                continue;
            }

            memFreed = true;
            refContainer.remove(*p);

            if(p->memPtr)
            {
                if(p->isArray) delete [] p->memPtr;
                else delete p->memPtr;

            }

            break;
        }
    } while (p != refContainer.end());

    return memFreed;
}

// Destructor for Pointer
template <class T, int size>
Pointer<T, size>::~Pointer()
{
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(addr);
    if(p->refCount) p->refCount--;

    collect();
}

// Overload assignment of pointer to Pointer
template <class T, int size>
T* Pointer<T, size>::operator=(T* t)
{
    auto p = findPtrInfo(addr);
    p->refCount--;

    p = findPtrInfo(t);

    if(p != refContainer.end()) p->refCount++;

    else refContainer.push_back(PtrDetails<T>(t,size));

    addr = t;
    return t;
}

// Overload assignment of Pointer to Pointer
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv)
{
    auto p = findPtrInfo(addr);
    p->refCount--;

    p = findPtrInfo(rv.addr);
    p->refCount++;

    addr = rv.addr;
    return rv;
}

// A utility function that displays refContainer
template <class T, int size>
void Pointer<T, size>::showlist()
{
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Find a pointer in refContainer
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T* ptr)
{
    typename std::list<PtrDetails<T> >::iterator p;

    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
       if (p->memPtr == ptr) return p;
    }

    return p;
}

// Clear refContainer when program exits
template <class T, int size>
void Pointer<T, size>::shutdown()
{
    if (refContainerSize() == 0) return; // list is empty

    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refCount = 0;
    }

    collect();
}