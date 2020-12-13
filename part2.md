# Part 2 #

<p>A systems programming concept that I personally found quite challening to not only understand but also to implement was memory allocation and management, i.e. malloc, calloc, realloc, and free. Memory allocation / freeing is easy enough to use in code but when it comes to implementing the internals, things can get very complex and tangled-up very quickly. Here are some of the main challenges I faced when I first started implementing memory management, and what I did to work my way through them by the time I did regrades.<p>

1.  How to keep track of things
    <p>This most important part of not getting completely in over your head when implementing memory management is understading how you will keep track of heap memory allocated and freed, and this is where I definitely stumbled the most my first time around. A critical point to understand is that you do NOT want increase the heap size (using sbrk) everytime allocation occurs as the heap can run out of space and calling sbrk too much can also negatively affect performance. This is where the metadata struct enters in, as a way to keep track of memory that has been allocated via sbrk'ed and/or freed. <p>

2.  Understanding metadata
    <p>You can think of metadata as the block of memory you asked for (allocated) plus some extra bookkeeping stuff. You'll also probably want to visualize a metadata block as a node in a linked list, though what exactly that list will represent will be up to you. But I'll go over that in a bit.<p>
    For example, this is what my metadata struct looked like:
        
        typedef struct meta_data { 
            void *ptr;
            size_t size;
            bool free;
            struct meta_data* next;
            struct meta_data* prev;
        } meta_data;

    Let's go over each of the struct's different variables!
    -   void* ptr:  you can think of this as the "memory" chunk. If you asked for 10 bytes of memory, this pointer would contain that memory space.
    -   size_t size: this is the size of the previous field; again, if 10 bytes were allocated, then size would = 10.
    -   bool free: this is whether the memory is usable or not. If 10 bytes were alloc'd, then free = false. However, if you then freed that pointer, free = true. This is where individual implementation can vary because you may want to only keep track of memory blocks that have been allocated AND freed.
    -   struct meta_data* next: again, visualize meta_data as a node in a list. Whether you think of the list as singly- or doubly-linked is also an implementation choice, but for my implementation this variable simply represented the next node in the list. NULL if at the end of the list.
    -   struct meta_data* prev: the same thing as next, but represents the previous node in the list. NULL if at the head of the list.


3.  Understanding sbrk()
    Like I said earlier, you will have to sometimes ask the heap for more memory if what you have previously allocated and freed isn't big enough. What exactly does this mean? Well, say I do the following:
        void* ptr = malloc(20);
        free(ptr);
        size_t x = ???
        void* anotherptr = malloc(x);
    <p>Look at the first line. If this was the first time memory was allocated, then sbrk probably needs to be called. This is because there are currently no metadata structs keeping track of any previously allocated memory. So now we have a metadata with size = 20 and free = false. Then we free ptr, but we keep the metadata struct and just change free to true! Now we allocate again but with size `x`. If x <= 20, then we don't have to sbrk again. This is because we have the previously mentioned metadata, and its size is sufficient enough for us to reuse it for anotherptr. However, if x > 20 then we need to sbrk so we can get sufficient memory.
    How does sbrk work? This is something that I found to be quite confusing at first. Sbrk() takes an int as its only parameter and returns a void* pointing to the memory it allocated. Sounds simple, but there are a few confusing bits:
        1.  sbrk(0) - if you sbrk(0), you get the current top of the heap. No new memory is handed over.
        2.  sbrk(size) - this will... also return the current top of the heap. BUT it also asks the heap for `size` amount of memory. ALSO, (and this is what confused me a little), if you call sbrk(0) AGAIN after calling sbrk(size), you will get the NEW top of the heap. So, the following chunks of code are NOT equivalent:<p>
        
        void* p;
        p = sbrk(0);
        p = sbrk(size);     // p points to old top of heap

        - and -

        void* p;
        p = sbrk(size);     
        p = sbrk(0);        // p points to new top of heap

4.  Pointer arithmetic

5.  Dealing with fragmentation (merging and splitting)

