/**
 * vector
 * CS 241 - Fall 2020
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>

/**
 * 'INITIAL_CAPACITY' the initial size of the dynamically.
 */
const size_t INITIAL_CAPACITY = 8;
/**
 * 'GROWTH_FACTOR' is how much the vector will grow by in automatic reallocation
 * (2 means double).
 */
const size_t GROWTH_FACTOR = 2;

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void** array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector* vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
   
    vector* myvector = malloc(sizeof(vector));
    
    myvector->copy_constructor = copy_constructor;
    myvector->destructor = destructor;
    myvector->default_constructor = default_constructor;

    myvector->capacity = INITIAL_CAPACITY;
    myvector->size = 0;

    myvector->array = (void**)malloc(sizeof(void*) * INITIAL_CAPACITY);
    
    // is this needed?
    unsigned int i;
    for (i = 0; i<INITIAL_CAPACITY; i++) {
        myvector->array[i] = NULL;
    }

    (void) get_new_capacity;        //TODO:     REMOVE THIS
    return myvector;
}

void vector_destroy(vector *this) {
    assert(this);
    
    this->default_constructor();
    free(this->array);
    this->array = NULL;
    free(this);
    this = NULL;
}

void** vector_begin(vector *this) {
    return this->array + 0;
}

void **vector_end(vector *this) {
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    assert(this);

    return this->size;
}

// Resizes the container so that it contains 'n' elements.
void vector_resize(vector *this, size_t n) {
    assert(this);

    if (n == this->size) return;

    // If 'n' is smaller than the current container size,
    // the content is reduced to its first n elements,
    // removing those beyond (and destroying them).
    if (n < this->size) {
        for (unsigned int i = n; i < this->size; i++) {
             //vector_erase(this, i);
             this->destructor(this->array[i]);
        }
       
        // this->capacity doesn't change

    /** If 'n' is greater than the current container size, the content is expanded by
    * inserting at the end as many elements as needed to reach a size of n.  These
    * new elements are created using the user defined default-constructor.
    */ 
    } else {
        if (n <= this->capacity) {
            for (unsigned int i = this->size; i < n; i++) {
                //insert elements to array at these positions
                this->array[i] = this->default_constructor();
            }

        /* If 'n' is also greater than the current container capacity,
        * an automatic reallocation of the allocated storage space takes place.
        */
        } else {
            size_t new_capacity = get_new_capacity(n);
            this->array = realloc(this->array, new_capacity * sizeof(void*));
            for (unsigned int i = this->size; i < this->capacity; i++) {
                //insert elements to array at these positions
                this->array[i] = this->default_constructor();
            }
            this->capacity = new_capacity;
        }

    }
    this->size = n;
    
}

size_t vector_capacity(vector *this) {
    assert(this);
    
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    
    return (this->size == 0);
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    if (n > this->capacity) {
        size_t new_cap = get_new_capacity(n);
        this->capacity = new_cap;
    }
    return;
}

void** vector_at(vector *this, size_t position) {
    assert(this);
    assert(position < this->size);

    void** ptr = this->array;
    for (unsigned int i = 0; i < position; i++) {
        ptr++;
    }

    return ptr;
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    assert(element);
    assert(position < this->size);
    
    this->array[position] = element;

    return;

}

void* vector_get(vector *this, size_t position) {
    assert(this);
    assert(position < this->size);

    return this->array[position];
}

// Returns a pointer to the first element in the vector.
void **vector_front(vector *this) {
    assert(this);
    
    return this->array;
}

void **vector_back(vector *this) {
    assert(this);
    assert(this->size != 0);

    void** back = this->array;
    for (unsigned int i = 0; i < this->size-1; i++) {
        back++;
    }
    return back;
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    assert(element);
    if (this->size == 0) vector_insert(this, 0, element);
    else vector_insert(this, (this->size), element);


    return;
}

void vector_pop_back(vector *this) {
    assert(this);
    
    vector_erase(this, (this->size)-1);
}

void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    assert(element);
    assert((long)position >=0);
    assert(position <= this->capacity);    //Why would we need to check this?

    // if inserting within the current array
    if (position < this->size) {
        // increment the size by 1
        size_t old_size = this->size;

        vector_resize(this, (this->size) + 1);  
        // please solve the code thankyou very much.                     
        // move each element from after the position to the end back
        // this should work bc resize() will add an empty element to end
        for (size_t i = old_size; i >= position ; i--) {   // element in front of position
           
            this->array[i] = this->array[i-1];
        }  

        this->array[position] = element;

    } else {
        vector_resize(this, position+1);
        this->array[position] = element;

    }
      
}

void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));

    for (size_t i = position; i < this->size; i++) {
        if (i == this->size-1) {
            this->array[i] = this->default_constructor();
            break;
        }
        this->array[i] = this->array[i+1];
    }
    vector_resize(this, this->size-1);
}

void vector_clear(vector *this) {
    for (size_t i = 0; i < this->size; i++) {
        vector_erase(this, i);
    }
}

// The following is code generated:
vector *shallow_vector_create() {
    return vector_create(shallow_copy_constructor, shallow_destructor,
                         shallow_default_constructor);
}
vector *string_vector_create() {
    return vector_create(string_copy_constructor, string_destructor,
                         string_default_constructor);
}
vector *char_vector_create() {
    return vector_create(char_copy_constructor, char_destructor,
                         char_default_constructor);
}
vector *double_vector_create() {
    return vector_create(double_copy_constructor, double_destructor,
                         double_default_constructor);
}
vector *float_vector_create() {
    return vector_create(float_copy_constructor, float_destructor,
                         float_default_constructor);
}
vector *int_vector_create() {
    return vector_create(int_copy_constructor, int_destructor,
                         int_default_constructor);
}
vector *long_vector_create() {
    return vector_create(long_copy_constructor, long_destructor,
                         long_default_constructor);
}
vector *short_vector_create() {
    return vector_create(short_copy_constructor, short_destructor,
                         short_default_constructor);
}
vector *unsigned_char_vector_create() {
    return vector_create(unsigned_char_copy_constructor,
                         unsigned_char_destructor,
                         unsigned_char_default_constructor);
}
vector *unsigned_int_vector_create() {
    return vector_create(unsigned_int_copy_constructor, unsigned_int_destructor,
                         unsigned_int_default_constructor);
}
vector *unsigned_long_vector_create() {
    return vector_create(unsigned_long_copy_constructor,
                         unsigned_long_destructor,
                         unsigned_long_default_constructor);
}
vector *unsigned_short_vector_create() {
    return vector_create(unsigned_short_copy_constructor,
                         unsigned_short_destructor,
                         unsigned_short_default_constructor);
}
