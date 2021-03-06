#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "myMalloc.h"
#include "printing.h"

/* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 */
#ifdef TEST_ASSERT
  inline static void assert(int e) {
    if (!e) {
      const char * msg = "Assertion Failed!\n";
      write(2, msg, strlen(msg));
      exit(1);
    }
  }
#else
  #include <assert.h>
#endif

/*
 * Mutex to ensure thread safety for the freelist
 */
static pthread_mutex_t mutex;

/*
 * Array of sentinel nodes for the freelists
 */
header freelistSentinels[N_LISTS];

/*
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 */
header * lastFencePost;

/*
 * Pointer to maintian the base of the heap to allow printing based on the
 * distance from the base of the heap
 */ 
void * base;

/*
 * List of chunks allocated by  the OS for printing boundary tags
 */
header * osChunkList [MAX_OS_CHUNKS];
size_t numOsChunks = 0;

/*
 * direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */
static void init (void) __attribute__ ((constructor));

// Helper functions for manipulating pointers to headers
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off);
static inline header * get_left_header(header * h);
static inline header * ptr_to_header(void * p);

// Helper functions for allocating more memory from the OS
static inline void initialize_fencepost(header * fp, size_t left_size);
static inline void insert_os_chunk(header * hdr);
static inline void insert_fenceposts(void * raw_mem, size_t size);
static header * allocate_chunk(size_t size);

// Helper functions for freeing a block
static inline void deallocate_object(void * p);

// Helper functions for allocating a block
static inline header * allocate_object(size_t raw_size);

// Helper functions for verifying that the data structures are structurally 
// valid
static inline header * detect_cycles();
static inline header * verify_pointers();
static inline bool verify_freelist();
static inline header * verify_chunk(header * chunk);
static inline bool verify_tags();

static void init();

static bool isMallocInitialized;


/**
 * @brief Helper function to retrieve a header pointer from a pointer and an 
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where header is located
 *
 * @return a pointer to a header offset bytes from pointer
 */
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off) {
	return (header *)((char *) ptr + off);
}

/**
 * @brief Helper function to get the header to the right of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
header * get_right_header(header * h) {
	return get_header_from_offset(h, get_size(h));
}

/**
 * @brief Helper function to get the header to the left of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
inline static header * get_left_header(header * h) {
  return get_header_from_offset(h, -h->left_size);
}

/**
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 */
inline static void initialize_fencepost(header * fp, size_t left_size) {
	set_state(fp,FENCEPOST);
	set_size(fp, ALLOC_HEADER_SIZE);
	fp->left_size = left_size;
}

/**
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 */
inline static void insert_os_chunk(header * hdr) {
  if (numOsChunks < MAX_OS_CHUNKS) {
    osChunkList[numOsChunks++] = hdr;
  }
}

/**
 * @brief given a chunk of memory insert fenceposts at the left and 
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 */
inline static void insert_fenceposts(void * raw_mem, size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;

  // Insert a fencepost at the left edge of the block
  header * leftFencePost = (header *) mem;
  initialize_fencepost(leftFencePost, ALLOC_HEADER_SIZE);

  // Insert a fencepost at the right edge of the block
  header * rightFencePost = get_header_from_offset(mem, size - ALLOC_HEADER_SIZE);
  initialize_fencepost(rightFencePost, size - 2 * ALLOC_HEADER_SIZE);
}




/**********************************************************************/
/*************************** Chunk Methods ****************************/
/**********************************************************************/

/**
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencpost)
 */
static header * allocate_chunk(size_t size) 
{

  //0. Safety checks
    //a. If size equals 0, return null
      if(size == 0) return NULL;
    //b. Initialize variables
      size_t chunk_count = ((size-1)/ARENA_SIZE);//The amount of chunks of arena_size bytes we need
      void * mem = sbrk(ARENA_SIZE);//To be used to hold the chunks that are being retrieved
      void * temp = NULL;//Used to hold the contiguous chunks

  //1. Grab chunks equal to the chunk count using sbrk
    //a. Call sbrk the required amount of times
      size_t a = 0;
      for(a; a < chunk_count; a++) { temp = sbrk(ARENA_SIZE); }

    /**********************/
    //Place for optimization
      a = (1+chunk_count)*ARENA_SIZE;
    /**********************/

  //2. Set the fenceposts
    //a. Set the fenceposts around the chunk of size ARENA_SIZE*chunk_count
      insert_fenceposts(mem,a);
  //3. Make a new header inside the chunk and set the state and size
    header * new_header = (header *)((char*)mem+ALLOC_HEADER_SIZE);
    set_size_and_state(new_header,a-2*ALLOC_HEADER_SIZE,UNALLOCATED);
    new_header->left_size = ALLOC_HEADER_SIZE; 
    return new_header;
}

/**
 * @brief Retrieves a chunk of the requested size, using allocate chunk to retreive
 * new chunks and then coalescing them
 *
 * @param size The size of the new chunk
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencpost)
 */
static inline header * get_chunk(size_t size)
{
  //1. Grab chunks equal to the chunk count using allocate chunk
    header * new_header = allocate_chunk(size);
  //2. Set up the left_header
    header * left_header = NULL;
  //3. Check for adjacency.
    if(lastFencePost == get_header_from_offset(new_header,-2*ALLOC_HEADER_SIZE))
    {
      //a. If the chunks are adjacent, coalesce the chunks
        //i. Coalesce the chunks
          //b. Set the middle fencepost to UNALLOCATED
            set_state(get_right_header(lastFencePost),UNALLOCATED);
          //a. Set the fencepost size and state
            set_size_and_state(lastFencePost,ALLOC_HEADER_SIZE*2+get_size(new_header),UNALLOCATED);    
          //c. Set the header to the old chunk's fencepost
            new_header = lastFencePost;
    }

  //4. If check fails, just insert the new chunk to the os
    else insert_os_chunk(get_left_header(new_header));
  //5. Replace the lastFencePost  
    lastFencePost = get_right_header(new_header);
  //6. Coelesce the header
    new_header = coalesce(new_header);  
  //7. Return the header  
  return new_header;
}













/**********************************************************************/
/*************************** Header Methods ***************************/
/**********************************************************************/

/**
 * @brief Links two headers in the free list
 *
 * @param p-the header to be the first link, n-the header to be the second link
 *
 * @return void
 */
static inline void link_headers(header * p, header * n) 
{ 
  if(p != NULL) p->next = n; 
  if(n != NULL) n->prev = p; 
}


/**
 * @brief Splits a single header into two seperate headers, one of the requested length and one of
 * leftover size
 *
 * @param h-the header to be split, size-the requested size of the new block of code in bytes
 *
 * @return a pointer to the header 
 */

static inline header * split_header(header * h, size_t new_size)
{
  //Put here for optimization
    size_t delta_size = get_size(h)-new_size;
  //Initialize variables
      header * new_header = NULL;//Used to return the newly split header

    
  //0. Safety checks
    //a. if h is null, return NULL
      if(h==NULL) return h;
    
    //b. if size is greater than or equal to h's size, return h
      if(0 > delta_size) return (header *) NULL;
    
    //c. if the leftover size is less than or equal to the size of a header, return h 
      if(sizeof(header) > delta_size)
      { 
        //remove_from_free(h);
        new_header = h;
      }
      else
      {
        //1. Make the new header
    
          //b. Set the size of h to h's size minus the new_size
            set_size(h,delta_size);
    
          //c. Create a new header at the pointer of the right header minus the size
            new_header = get_header_from_offset(h,delta_size);
      
          //i. Set the size to new_size and state to unallocated
            set_size_and_state(new_header,new_size,UNALLOCATED);
      
          //ii. Set the left size to the size of h's size 
            new_header->left_size = delta_size;
        //2.Get the right header and set it's left size to new_size
              get_right_header(new_header)->left_size = new_size;

        //3. Relist the old header
          bool b = relist_header(h,delta_size+new_size);
      }
  return new_header;
}

/**
 * @brief Helper function that does a forward merge for any header pointer
 *
 * @param prev- a header pointer to be merged in the rightward direction. 
 *
 * @return returns the merged chunk if successful, otherwise it returns NULL
 */
static inline header * merge(header * h)
{
  //Put up top for optimization
    header * right = get_right_header(h);
  //0. Safety checks and intialization  
    //a. if either prev or next is allocated, return false
      if(get_state(h) != UNALLOCATED || get_state(right) != UNALLOCATED) return NULL;
    //b. Initialize variables
      size_t new_size = get_size(right)+get_size(h); 
  //1. Update the prev size
    set_size(h,new_size);
  //2. Update the right header
    get_right_header(h)->left_size = new_size;
    
  return h;
}



/**
 * @brief Formula to get the list position of a header based on it's current size
 *
 * @param h- the header pointer which we request a list position for
 *
 * @return void
 */
static inline size_t header_list_position(header * h)
{
  //Gets the size of the header pointer minus the metadata size
  size_t header_size = get_size(h) - ALLOC_HEADER_SIZE;
  //Convert the size to a list position
  header_size = (header_size>>3)-1;
  //Check for max list index
  if(header_size >= N_LISTS) header_size = N_LISTS-1;
  return header_size;
}






/**********************************************************************/
/************************* Free List Methods **************************/
/**********************************************************************/


/**
 * @brief Helper that attempts to put a header whose size has been
 * altered back into the free list
 *
 * @param h- header to be relisted, og_size- original size of said
 * header that is being relisted
 *
 * @return true if the header is relisted and false if the header
 * doesn't change it's position
 */

static inline bool relist_header(header * h, size_t og_size)
{
  size_t new_index = header_list_position(h);
  if(new_index == proper_list(og_size)) return false;
  //Remove it from it's current place in the free list
  remove_from_free(h);
  //Insert it to the free list
  insert_to_free(h);
  return true;
}


/**
 * @brief Helper that removes a given header from the free list
 *
 * @param h- the header pointer to be removed from the free list
 *
 * @return void
 */
static inline void remove_from_free(header * h)
{
  //If h does not posses a prev or a next, do nothing
  if(h->prev == NULL && h->next == NULL){}
  //If h is not in the free list, do nothing
  else if(h->prev->next != h || h->next->prev != h){}
  //Link the previous and next headers together
  else link_headers(h->prev,h->next);
  return;
}

/**
 * @brief Helper that inserts a given header from the free list
 *
 * @param h- the header pointer to be removed from the free list
 *
 * @return void
 */
static inline void insert_to_free(header * h)
{
  //Check if h is null
  if(h == NULL) return;
  //Find the proper list index relative to h
  size_t h_index = header_list_position(h);
  //Find the proper sentinel to add it to
  header * sentinel = &freelistSentinels[h_index];
  //Insert as the next node
  link_headers(h,sentinel->next);
  link_headers(sentinel,h);
  return;
}

/**
 * @brief Formula that does the code for getting the proper list index where an item for the relative
 * size would be stored
 *
 * @param size-relative size for the requested index
 *
 * @return the list index for relative size
 */
static inline size_t proper_list(size_t size)
{
  //Round to the nearest 8
  size = (((size-ALLOC_HEADER_SIZE)>>3)-1);
  //Check to make sure it doesn't get an out of bound index
  if(size >= N_LISTS) size = N_LISTS-1;
  return size;
}

/**
 * @brief Helper to retrieve a header from free list that has the requested size
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block from the free list satisfying the user's request
 */
static inline header * retrieve_from_free(size_t size)
{
  header * h = NULL;
  header * sentinel;
  //Find the proper list index
  size_t a = proper_list(size);
  //If a is -1, then the size is 0
  if(a == -1) return h;
  //Iterate through all the list lengths
  for(a = a; a < N_LISTS-1; a++)
  {
    //Assign the sentinel with the relative index
    sentinel = &freelistSentinels[a];
    //Check if the sentinel has an item with the appropriate size
    //If so, we would like to return said item
    if(sentinel->next != NULL && sentinel->next != sentinel)
    {
      //Return it
      return sentinel->next;
    }
  }


  /*If it makes it here, it didn't find any header that had the appropriate size*/
  

  //Set the sentinel at the last possible sentinel
  sentinel = &freelistSentinels[N_LISTS-1];
  //Iterate through the list to find a suitable fit
  header * temp = sentinel;
  do
  {
    temp = temp->next;
    //Check the temp size for a proper fit, return it if it is the right size
    if(get_size(temp) >= size) { return temp; }
  //loop else
  } while (temp != sentinel);
  return h;
}





/**
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
 */
static inline header * allocate_object(size_t raw_size) {
  
  
  //0. Safety Checks and Initialize Variables
    header * h = NULL;//Pointer to handle the back and forth between other methods
    size_t h_size; // Size of the total bytes needed to 
    
    //a. Allocation of 0 bytes returns NULL
    if(raw_size == 0) return h;
  //1. Calculate the required block size allocation
    //a. All allocations sizes should be rounded up to the next 8 bytes
    raw_size += 7-((raw_size-1)&7);
    h_size = raw_size + ALLOC_HEADER_SIZE;
    //Check the minimum size requirement for the headers  
    if(h_size < sizeof(header)) h_size = sizeof(header);
  //2. Attempt to retrieve a block from the free list
    h = retrieve_from_free(h_size);
  //3. If failure, allocate a new chunk
    if(h == NULL) h = get_chunk(h_size);
  //4. Split Chunk and then return required data
    h = split_header(h,h_size);
    set_state(h,ALLOCATED);
    remove_from_free(h);
    return (header*)(h->data);
}

/**********************************************************************/
/************************* Coalesce Methods **************************/
/**********************************************************************/

/**
 * @brief Coalescing method that coalesces a given header by attempting to coalesce right and left
 *
 * @param header pointer to be coalesced
 *
 * @return The final block after coalescing
 */
static inline header * coalesce(header * h)
{
   //b. Retrieve the left and the right headers next and prev
      //i. Holds the left and right headers
      header * left = get_left_header(h);
      header * right = get_right_header(h);

      //i. holds the prev and next of the right and left when coalescing
      header * prev = NULL;
      header * next = NULL;

      //i. Grab the states of the headers
      size_t right_state = get_state(right);
      size_t left_state = get_state(left);

      //i. Used to gauruntee a relisting
      size_t original_size = -1;

      //ii. If the the right and left states are not UNALLOCATED, insert to free list
      if(right_state != UNALLOCATED && left_state != UNALLOCATED)
      {
        insert_to_free(h);
        return h;
      }

      //iii. If you can coalesce with the right one, coalesce with the right one
      if(right_state == UNALLOCATED)
      {
        //a. Update the prev and next
          prev = right->prev;
          next = right->next;
        
        //b. Remove the right_header from the free list
          remove_from_free(right);
        
        //c. Set the original size
          original_size = get_size(right);
        
        //d. Merge the headers
          h = merge(h);
      }

      //iv. If you can coalesce with the left one, coalesce with the left one
      if(left_state == UNALLOCATED)
      {
        //a. Update the prev and next
          prev = left->prev;
        next = left->next;
        //b. Remove the item from free
          remove_from_free(left);
        //c. Update original size
          original_size = get_size(left);
        //d. Merge the blocks
          h = merge(left);
      }
  //2. Put the coalesced blocks in the free list
    //a. If the header can be relisted, relist it
      if(header_list_position(h) == proper_list(original_size))
      {
        link_headers(prev,h);
        link_headers(h,next);
      }
      //b. Else just insert it to free
      else
      {
        
        insert_to_free(h);
      }
  return h;

}






/**
 * @brief Helper to get the header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the header of the block
 */
static inline header * ptr_to_header(void * p) {
  return (header *)((char *) p - ALLOC_HEADER_SIZE); //sizeof(header));
}

/**
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 */
static inline void deallocate_object(void * p) 
{ 
  //0. Safety Checks
    //a. Null pointer check
      if((header*)p == NULL) return;
    //b. convert the void pointer to a header pointer
      header * h = ptr_to_header(p);
    //b. Double free
      if(get_state(h) == UNALLOCATED)
      {
        printf("Double Free Detected\n");
        assert(false);
        return;
      }
  //1. Return to freelist
    //a. Set the state of h to unallocated
      set_state(h,UNALLOCATED);
    //b. Retrieve the left and the right headers next and prev
      //i. Holds the left and right headers
      header * left = get_left_header(h);
      header * right = get_right_header(h);
      
      //i. holds the prev and next of the right and left when coalescing
      header * prev = NULL;
      header * next = NULL;

      //i. Grab the states of the headers
      size_t right_state = get_state(right);
      size_t left_state = get_state(left);

      //i. Used to gauruntee a relisting
      size_t original_size = -1;      
      
      //ii. If the the right and left states are not UNALLOCATED, insert to free list
      if(right_state != UNALLOCATED && left_state != UNALLOCATED)
      { 
        insert_to_free(h);
        return;
      }

      //iii. If you can coalesce with the right one, coalesce with the right one
      if(right_state == UNALLOCATED)
      {
        prev = right->prev;
        next = right->next;
        //a. Remove the right_header from the free list
        remove_from_free(right);
        //b. Set the original size
        original_size = get_size(right);
        //b. Merge the headers
        h = merge(h);
      }
      
      //iv. If you can coalesce with the left one, coalesce with the left one
      if(left_state == UNALLOCATED)
      {
        prev = left->prev;
        next = left->next;
        remove_from_free(left);
        original_size = get_size(left);
        h = merge(left);
      }
  //2. Put the coalesced blocks in the free list
    if(header_list_position(h) == proper_list(original_size))
    {
      link_headers(prev,h);
      link_headers(h,next);
    }
    else
    {
      insert_to_free(h);
    }
  //3. Set the state of h
    set_state(h,UNALLOCATED);

  //4. Return
  return;
}

/**
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 */
static inline header * detect_cycles() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * slow = freelist->next, * fast = freelist->next->next; 
         fast != freelist; 
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}

/**
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 */
static inline header * verify_pointers() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}

/**
 * @brief Verify the structure of the free list is correct by checkin for 
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
 */
static inline bool verify_freelist() {
  header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    print_sublist(print_object, cycle->next, cycle);
    return false;
  }

  header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    print_object(invalid);
    return false;
  }

  return true;
}

/**
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid header or NULL if all header's are valid
 */
static inline header * verify_chunk(header * chunk) {
	if (get_state(chunk) != FENCEPOST) {
		fprintf(stderr, "Invalid fencepost\n");
		print_object(chunk);
		return chunk;
	}
	
	for (; get_state(chunk) != FENCEPOST; chunk = get_right_header(chunk)) {
		if (get_size(chunk)  != get_right_header(chunk)->left_size) {
			fprintf(stderr, "Invalid sizes\n");
			print_object(chunk);
			return chunk;
		}
	}
	
	return NULL;
}

/**
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
 */
static inline bool verify_tags() {
  for (size_t i = 0; i < numOsChunks; i++) {
    header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }

  return NULL;
}

/**
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
static void init() {
  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);

#ifdef DEBUG
  // Manually set printf buffer so it won't call malloc when debugging the allocator
  setvbuf(stdout, NULL, _IONBF, 0);
#endif // DEBUG

  // Allocate the first chunk from the OS
  header * block = allocate_chunk(ARENA_SIZE);

  header * prevFencePost = get_header_from_offset(block, -ALLOC_HEADER_SIZE);
  insert_os_chunk(prevFencePost);

  lastFencePost = get_header_from_offset(block, get_size(block));

  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - ALLOC_HEADER_SIZE; //sizeof(header);

  // Initialize freelist sentinels
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }

  // Insert first chunk into the free list
  header * freelist = &freelistSentinels[N_LISTS - 1];
  freelist->next = block;
  freelist->prev = block;
  block->next = freelist;
  block->prev = freelist;
}

/* 
 * External interface
 */
void * my_malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  header * hdr = allocate_object(size); 
  pthread_mutex_unlock(&mutex);
  return hdr;
}

void * my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

void * my_realloc(void * ptr, size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem; 
}

void my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}

bool verify() {
  return verify_freelist() && verify_tags();
}
