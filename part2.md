# Part 2 #

<p>A systems programming concept that I personally found quite challening to not only understand but also to implement was memory allocation and management, i.e. malloc, calloc, realloc, and free. Memory allocation / freeing is easy enough to use in code but when it comes to implementing the internals, things can get very complex and tangled-up very quickly. I know that for myself, and for many other programmers, organization is not a strong suit. We like to just write code and tests without keeping track of our variables, functions, sequences of events, and logic and before we know it we're competely lost trying to debug stuff we wrote two days ago. This is especially a problem for memory management implementation, where there's so much to keep track of! And especially since a lot of the implementation relies on personal decisions, there's quite a bit of room for error. But fear not! Here are some of the main challenges I faced when I first started implementing memory management, and what I did to work my way through them by the time I did regrades.<p>

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
    How does sbrk work? This is something that I found to be quite confusing at first. Sbrk() takes an int as its only parameter and returns a void* pointing to the memory it allocated. Sounds simple, but there are a few confusing bits: <p>
        1.  sbrk(0) - if you sbrk(0), you get the current top of the heap. No new memory is handed over.
        2.  sbrk(size) - this will... also return the current top of the heap. BUT it also asks the heap for `size` amount of memory. ALSO, (and this is what confused me a little), if you call sbrk(0) AGAIN after calling sbrk(size), you will get the **new** top of the heap. So, the following chunks of code are NOT equivalent:
        
        void* p;
        p = sbrk(0);
        p = sbrk(size);     // p points to old top of heap

        - and -

        void* p;
        p = sbrk(size);     
        p = sbrk(0);        // p points to new top of heap
    <p>Here's an example of how I used sbrk() and its features in my malloc implementation:<p>

        meta_data* chosen = sbrk(sizeof(meta_data));
        chosen->ptr = sbrk(0);      // remember ptr is a void* and points to the chunk of memory the user wants!

        if (sbrk(size) == (void*)-1) {      // then get the amount of memory that the user wants and check for error
            return NULL; 
        }

    Kinda confusing? Maybe not, but that's probably because I just explained it really well :3 Just remember, do NOT call sbrk() everytime you alloc!<p>

4.  Pointer arithmetic
    <p>Ugh... this was a bit of a pain to deal with, and Prof. Angrave WARNED us about it!! But with the help of some lovely TAs I was able to get through this issue! I quickly realized that the **critical** part to remember is that calling ++ on a pointer will not necessarily increment its address by 1! It depends on what the size of its type is. Here's some code to demonstrate (ran on the 241 VM):<p>

        int main() {
            // sizeof(void) returns 1
            // sizeof(int) returns 4

            void* vp = malloc(1);       
            printf("vp: %p\n", vp);     // vp address = 0010
            vp++;
            printf("vp: %p\n", vp);     // vp address = 0011
            
            int* ip = malloc(1);        
            printf("ip: %p\n", ip);     // ip address = 0020
            ip++;
            printf("ip: %p\n", ip);     // ip address = 0024
            
            void* avp = (int*) vp + 1;
            printf("avp: %p\n", avp);   // avp address = 0015! even though (void*)++ will increment address by 1, 
                                        // casting it to an (int*) makes it so that ++ will increment by 4.
            
            return 0;
        }

    I used pointer arithmetic quite frequently in my different memory management applications since using the metadata struct often means a metadata's void* ptr variable, or vice versa (having a void* and using it to get a metadata*). For example:

        meta_data* p = (meta_data*)ptr - 1;
        if (p->free) return;

    This is a bit of code from my free() implementation! Void* ptr was the argument passed to free, but notice how I cast ptr to a metadata* before I decremented it to get the actual metadata* p. If I wanted to, I could have done:
    
        meta_data* p = ptr - sizeof(metadata);
        if (p->free) return;

5.  Dealing with internal fragmentation (merging)
    <p>The final part of memory management, and also one of the biggest challenges, was dealing with internal fragmentation! If you have no clue what theis means, the best way to describe it is probably with another example. Say I have a free (available to be alloc'd) metadata block with its size = 50. Then say that the user calls malloc(46). No problem! I have a previously alloc'd block with sufficient size, so no need to call sbrk! Well, yes, that definitely is true. But an initially small problem that begins to add up is the fact that 4 of the 50 bytes remain unused after the second allocation, and it's unlikely they'll ever be used since they're so small. If several allocation calls are made where there are small chunks of memory left behind and unused, that unused memory will eventually add up. And it likely will get to the point where it seems like there is no memory left at all because it is all splintered into small pieces!<p>
    <p>The way I dealt with this was simple enough on the surface, but can actually be a little tricky to implement. You have to merge blocks! If the problem is caused by blocks being split into unusably-small pieces, then the solution is to re-merge those small pieces into a usuably-sized block! Now, where you choose to merge is an implementation design. I chose to do it a few places: after splitting a block, after freeing a block, and when reallocing a block to a bigger-sized block.<p>
    <p>The merge_blocks() implementation is simple enough though! The way I did it was to merge a block with its previous block. In fact the whole thing was written in four lines!<p>

        void merge_blocks(meta_data* block) {
            block->size += (block->prev->size + sizeof(meta_data));     // remember to include the metadata size since that will be      
                                                                        // "swallowed up" too!
            block->prev = block->prev->prev;
            
            if (!block->prev) head = block;     // you are merging the second block in the list with the first (head)
                
            else block->prev->next = block;
            
        }


<p>Well, that's it! It's by no means an exhaustive explanation of a memory management implementation but it's all the parts I found to be most challenging, plus some tips to help clear them up! Remember, the main issues come from being disorganized. For many programmers, myself included, staying organized is not our strong suit. But keeping track of things like when/where you add in new metadata blocks, when you decide to merge and split blocks, and what data types different variables are can help immensely!!

