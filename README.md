# malloc
As the name suggests, this project is a hobbiest allocator written in C. 

## Alloc-what?

Usually when you write code, you would need **memory** to store objects and data that you find useful. This memory can be divided into 2 types:

- Stack based memory
- Heap based memory

In high level languages like C, allocating space on stack could be as simple as: `int c = 4;`. This looks perhaps simple enough to make 
an inexperienced dev think that no memory is even assigned for this, but a compiler would turn this simple line into:

```asm
push rbp          ; store old bp
mov rbp, rsp
sub rsp, 16       ; reserve 16 bytes for the stack frame
mov DWRD PTR [rbp-4], 4 ; store 4 into c (-4 from RBP)
```

Notice how 16 bytes of memory were just "allocated" for our function. 

> NOTE: This is not entirely accurate because if this assignment of `c` was made as a static variable, say at the start of the file, it would likely to end on 
the `.data` segment but that's another story. 

So, why is stack memory not enough? There are a few reasons:
1. Usually threads would get a limited amount of stack reserved space, so large amount of memory can't be worked with.
2. More importantly, stack variables are temporary: the stack grows and shrinks when entering and exiting functions.

The following example shows one problem with working only with stack memory:

```c
int *foo() {
  int x = 42;
  return &x; 
}
```

This code would return a dangling pointer since the memory of x would be "freed" back to the stack once leaving the `foo` function. For these reasons, 
lets see what is the heap meant for:

The heap is a different region of memory that is meant for dynnamic and runtime managed allocations. The heap allows programs to request memory of a certain size,
that would persist beyond the lifetime of a function call. Memory on the heap would be allocated by calling `malloc` in C, or `new` in C++. These functions would return
a pointer to the start of the memory designated for the program. 

The equivalent of our program from before, using the heap, is:

```c
void foo() {
  int *ptr = malloc(sizeof(int));
  *ptr = 42; 
  return ptr;
}
```
> NOTE: Like explained, the developer MUST later call `free(ptr)` to free the memory back for the allocator to reuse.

These functions are implemented by a module called the system allocator which is in charge of doing its best to supply the program a "good" memory chunk of the size requested. There are many important aspects
to the allocator work which are not all explored in this project, but many are. 

An extremely important point about the heap memory is that managing the memory is entirely up to the prgoram (= the developer of that program). When the program finishes using memory
that was allocated with `malloc`, it must call `free` on that memory. 

**The goal of this project is to write a functional allocator module, with some security mitigations**

## Dangers of the Heap Realm
A few problems are inherited with the design of the heap (and also the design of humans):

1. Developers may forget to `free` unused memory. On itself, it may not seem so scary, but remember that freeing a memory is the only way to alert the allocator that this
memory is ready to be regiven to another program or memory request. If enough memory is not freed back, the allocator would be incapable of allocating any more memory for the program,
which will end up crashing it. This scenario is called *memory leak*. 

2. Developers may perform a double `free`. This is more or less a *vulnerability* [https://cwe.mitre.org/data/definitions/415.html] now, not just a memory leak. It should be noted
that performing `free(NULL)` is just fine, and would be ignored by a normal allocator. This is why it is recommended to set `buffer = NULL;` after performing `free(buffer)` once. 

3. Developers may continue to use a `free`-d memory. This is the most dangerous scenario out of these 3. This type of bug (vulnerability) is called *UAF - Use-After-Free*, and could be
exploited to achieve code execution on a program in many cases. 

## Inspiration from iOS XZone malloc

This allocator is inspired by the currently used allocator for user space in iOS, which is called xzone-malloc. XZone is considered a very advacned allocator because, for starters, it does type separation (which means pointer & data are not accomedated one near another) and also does separation by sizeclasses and buckets (Buckets are also calculated based on the caller, which means allocations done by different functions are not in the same bucket).

There's a lot more to XZone. Fortunately, it has been made public: 

[https://github.com/apple-oss-distributions/libmalloc/tree/libmalloc-792.41.1]

## Roadmap

In the future, maybe try :

- MTE
- Guard pages
