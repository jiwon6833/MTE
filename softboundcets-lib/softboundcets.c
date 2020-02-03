//=== softboundcets.c - Creates the main function for SoftBound+CETS Runtime --*- C -*===// 
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.

// Developed by: Santosh Nagarakatte,
//               Department of Computer Science, Rutgers University
//               https://github.com/santoshn/softboundcets-34/
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/
//
//               in collaboration with 
//
//               Milo M.K. Martin, Jianzhou Zhao, Steve Zdancewic
//               Department of Computer and Information Sciences,
//               University of Pennsylvania


// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.

//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.

//   3. Neither the names of Santosh Nagarakatte, Milo M. K. Martin,
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania, nor
//      the names of its contributors may be used to endorse or promote
//      products derived from this Software without specific prior
//      written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.
//===---------------------------------------------------------------------===//


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if defined(__linux__)
#include <malloc.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/mman.h>
#if !defined(__FreeBSD__)
#include <execinfo.h>
#endif
#include "softboundcets.h"

__softboundcets_trie_entry_t** __softboundcets_trie_primary_table;

size_t* __softboundcets_free_map_table = NULL;

size_t* __softboundcets_shadow_stack_ptr = NULL;

size_t* __softboundcets_lock_next_location = NULL;
size_t* __softboundcets_lock_new_location = NULL;
size_t __softboundcets_key_id_counter = 2;

/* key 0 means not used, 1 is for  globals*/
size_t __softboundcets_deref_check_count = 0;
size_t* __softboundcets_global_lock = 0;

size_t* __softboundcets_temporal_space_begin = 0;
size_t* __softboundcets_stack_temporal_space_begin = NULL;

void* malloc_address = NULL;

char* __mte_tag_mem = NULL;

__SOFTBOUNDCETS_NORETURN void __softboundcets_abort()
{
  fprintf(stderr, "\nSoftboundcets: Memory safety violation detected\n\nBacktrace:\n");

  // Based on code from the backtrace man page
  size_t size;
  void *array[100];
  
#if !defined (__FreeBSD__)
  size = backtrace(array, 100);
  backtrace_symbols_fd(array, size, fileno(stderr));
#endif
  
  fprintf(stderr, "\n\n");

  abort();
}

static int softboundcets_initialized = 0;

__NO_INLINE void __softboundcets_stub(void) {
  return;
}
void __softboundcets_init(void) 
{
  if (softboundcets_initialized != 0) {
    return;  // already initialized, do nothing
  }
  
  softboundcets_initialized = 1;

  if (__SOFTBOUNDCETS_DEBUG) {
    __softboundcets_printf("Initializing softboundcets metadata space\n");
  }

  
  assert(sizeof(__softboundcets_trie_entry_t) >= 16);

  size_t shadow_stack_size = __SOFTBOUNDCETS_SHADOW_STACK_ENTRIES * sizeof(size_t);
  __softboundcets_shadow_stack_ptr = mmap(0, shadow_stack_size, 
                                          PROT_READ|PROT_WRITE, 
                                          SOFTBOUNDCETS_MMAP_FLAGS, -1, 0);
  assert(__softboundcets_shadow_stack_ptr != (void*)-1);

  *((size_t*)__softboundcets_shadow_stack_ptr) = 0; /* prev stack size */
  size_t * current_size_shadow_stack_ptr =  __softboundcets_shadow_stack_ptr +1 ;
  *(current_size_shadow_stack_ptr) = 0;

  #ifndef METALLOC

  size_t length_trie = (__SOFTBOUNDCETS_TRIE_PRIMARY_TABLE_ENTRIES) * sizeof(__softboundcets_trie_entry_t*);
  
  __softboundcets_trie_primary_table = mmap(0, length_trie, 
					    PROT_READ| PROT_WRITE, 
					    SOFTBOUNDCETS_MMAP_FLAGS, -1, 0);
  assert(__softboundcets_trie_primary_table != (void *)-1);  
  
  int* temp = malloc(1);
  __softboundcets_allocation_secondary_trie_allocate_range(0, (size_t)temp);
  #endif
  __mte_tag_mem = mmap(0, 0x0000100000000000 /* 8TB */, PROT_READ | PROT_WRITE, SOFTBOUNDCETS_MMAP_FLAGS, -1, 0);
}

static void softboundcets_init_ctype(){  
#if defined(__linux__)

  char* ptr;
  char* base_ptr;

  ptr = (void*) __ctype_b_loc();
  base_ptr = (void*) (*(__ctype_b_loc()));
  __softboundcets_allocation_secondary_trie_allocate(base_ptr);

#ifdef __SOFTBOUNDCETS_SPATIAL
  __softboundcets_metadata_store(ptr, ((char*) base_ptr - 129), 
                                 ((char*) base_ptr + 256));

#elif __SOFTBOUNDCETS_TEMPORAL
  __softboundcets_metadata_store(ptr, 1, __softboundcets_global_lock);

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL
  __softboundcets_metadata_store(ptr, ((char*) base_ptr - 129), 
                                 ((char*) base_ptr + 256), 1, 
                                 __softboundcets_global_lock);

#else  
  __softboundcets_metadata_store(ptr, ((char*) base_ptr - 129), 
                                 ((char*) base_ptr + 256), 1, 
                                 __softboundcets_global_lock);
  
#endif

#endif // __linux ends 
}

#if 0
struct tag_info_struct tag_info[NUM_MTE_TAGS] = {
  { 0, 0, 0 }, /* tag 0 unused */
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 },
  { 1, 0, 0 }
};
#endif

struct tag_info_struct tag_info[NUM_MTE_TAGS];

struct tag_info_stack_struct tag_info_stack[TAG_INFO_STACK_DEPTH] = {
  {0, 0, 0xFFFFFFFFFFFFFFFF, 0, 0}  // special value for the first entry
};

/* int stack_depth = 0; */
unsigned long cur_lru = 0;
int tag_info_stack_ptr = 1;

int color_count = 0;
int uncolor_count = 0;
int restore_count = 0;
int mte_color_tag_count = 0;
int mte_restore_tag_count = 0;

long mte_color_tag_main(char *base, char *bound, int tag_num, void * cur_sp) {

    if (tag_info[tag_num].base) {
      char *old_base = tag_info[tag_num].base;
      char *old_bound = tag_info[tag_num].bound;
      _MTE_DEBUG(uncolor_count++);
#if 1
      tag_info_stack[tag_info_stack_ptr].base = old_base;
      tag_info_stack[tag_info_stack_ptr].bound = old_bound;
      /* tag_info_stack[tag_info_stack_ptr].used = tag_info[tag_num].used; */
      /* tag_info_stack[tag_info_stack_ptr].orig_lru = tag_info[tag_num].lru; */
      tag_info_stack[tag_info_stack_ptr].sp = cur_sp;
      tag_info_stack[tag_info_stack_ptr].orig_tag = tag_num;
      tag_info_stack_ptr++;
      if (tag_info_stack_ptr > TAG_INFO_STACK_DEPTH)
        abort();
#endif
      char *tag_start = __mte_tag_mem + ((long)old_base >> 4);
      char *tag_end = __mte_tag_mem + ((long)(old_bound-1) >> 4);
      for (char *cur = tag_start; cur <= tag_end; cur++)
        *cur=0;
      /* *tag_start=0; // JSSHIN */
    }

  char * start = __mte_tag_mem + ((long)base >> 4);
  char * end = __mte_tag_mem + ((long)(bound-1) >> 4);
    int i = tag_num;
    /* abort(); */
  /* } */
    _MTE_DEBUG(color_count++);
    for (char *cur = start; cur <= end; cur++)
      *cur=i;
    /* *start = i; // JSSHIN */

  /* tag_info[i].used++; */
  tag_info[i].base = base;
  tag_info[i].bound = bound;
  /* tag_info[i].sp = cur_sp; */
  /* tag_info[i].lru = cur_lru; */

  return i;
}

#if 0
void mte_uncolor_tag() {
  /* if (base==NULL) */
  /*   return; */

  cur_lru--;

  /* char * start = __mte_tag_mem + ((long)base >> 4); */
  /* char * end = __mte_tag_mem + ((long)(bound-1) >> 4); */

  /* if (tag_info[*start].used <= 0) */
  /*   abort(); */

  /* tag_info[*start].used--; */
  /* if (tag_info[*start].used) */
  /*   return; */

  /* tag_info[*start].base = 0; */
  /* tag_info[*start].bound = 0; */

  /* for (char *cur = start; cur <= end; cur++) */
  /*   *cur=0; */
}
#endif

void mte_restore_tag_main(void * cur_sp) {
  int tmp_stack_ptr = tag_info_stack_ptr-1;
#if 0
  int tmp_lru = tag_info_stack[tmp_stack_ptr].lru;
  for (int i = tmp_stack_ptr; i >= 0; i--) {
    if (tag_info_stack[tmp_stack_ptr].lru > tmp_lru)
      abort();
    tmp_lru = tag_info_stack[tmp_stack_ptr].lru;
  }
#endif
  // tag_info_stack[0].sp has MAX value so don't need to check tmp_stack_ptr >= 0
  while (tag_info_stack[tmp_stack_ptr].sp <= cur_sp) {
    int orig_tag = tag_info_stack[tmp_stack_ptr].orig_tag;

    char *base = tag_info[orig_tag].base;
    char *bound = tag_info[orig_tag].bound;

    char *tag_start =  __mte_tag_mem + ((long)base >> 4);
    char *tag_end =  __mte_tag_mem + ((long)(bound-1) >> 4);

    for (char *cur = tag_start; cur <= tag_end; cur++)
      *cur=0;
    /* *tag_start = 0; // JSSHIN */

    char *old_base = tag_info_stack[tmp_stack_ptr].base;
    char *old_bound = tag_info_stack[tmp_stack_ptr].bound;

    tag_start = __mte_tag_mem + ((long)old_base >> 4);
    tag_end = __mte_tag_mem + ((long)(old_bound-1) >> 4);

    for (char *cur = tag_start; cur <= tag_end; cur++)
      *cur=orig_tag;
    /* *tag_start = orig_tag; // JSSHIN */

    tag_info[orig_tag].base = old_base;
    tag_info[orig_tag].bound = old_bound;
    /* tag_info[orig_tag].lru = tag_info_stack[tmp_stack_ptr].orig_lru; */

    tmp_stack_ptr--;
    _MTE_DEBUG(restore_count++);
  }

  tag_info_stack_ptr = tmp_stack_ptr+1;
}

void __softboundcets_printf(const char* str, ...)
{
  va_list args;
  
  va_start(args, str);
  vfprintf(stderr, str, args);
  va_end(args);
}

extern int softboundcets_pseudo_main(int argc, char **argv);

#ifdef MTE_DEBUG
long load_deref_cnt = 0;
long store_deref_cnt = 0;
void atexit_hook() {
  printf("load_deref_cnt = %ld\n", load_deref_cnt);
  printf("store_deref_cnt = %ld\n", store_deref_cnt);
  printf("color_count = %ld\n", color_count);
  printf("uncolor_count = %ld\n", uncolor_count);
  printf("restore count = %ld\n", restore_count);
  printf("mte_color_tag count = %ld\n", mte_color_tag_count);
  printf("mte_restore_tag count = %ld\n", mte_restore_tag_count);
  printf("=== object access count ===\n");
  printAccessCount();
}
#endif

int main(int argc, char **argv){

#if __WORDSIZE == 32
  exit(1);
#endif
  
  char** new_argv = argv;
  int i;
  char* temp_ptr;
  int return_value;
  size_t argv_key;
  void* argv_loc;

  int* temp = malloc(1);
  malloc_address = temp;
  __softboundcets_allocation_secondary_trie_allocate_range(0, (size_t)temp);

#if defined(__linux__)
  mallopt(M_MMAP_MAX, 0);
#endif
#ifndef METALLOC
  for(i = 0; i < argc; i++) { 

#ifdef __SOFTBOUNDCETS_SPATIAL

    __softboundcets_metadata_store(&new_argv[i], 
                                   new_argv[i], 
                                   new_argv[i] + strlen(new_argv[i]) + 1);
    
#elif __SOFTBOUNDCETS_TEMPORAL
    //    printf("performing metadata store\n");
    __softboundcets_metadata_store(&new_argv[i],  
                                   argv_key, argv_loc);
    
#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

    __softboundcets_metadata_store(&new_argv[i], 
                                   new_argv[i], 
                                   new_argv[i] + strlen(new_argv[i]) + 1, 
                                   argv_key, argv_loc);

#else

    __softboundcets_metadata_store(&new_argv[i], 
                                   new_argv[i], 
                                   new_argv[i] + strlen(new_argv[i]) + 1, 
                                   argv_key, argv_loc);

#endif


  }

  //  printf("before init_ctype\n");
  softboundcets_init_ctype();
#endif
  /* Santosh: Real Nasty hack because C programmers assume argv[argc]
   * to be NULL. Also this NUll is a pointer, doing + 1 will make the
   * size_of_type to fail
   */
  temp_ptr = ((char*) &new_argv[argc]) + 8;

  /* &new_argv[0], temp_ptr, argv_key, argv_loc * the metadata */

  __softboundcets_allocate_shadow_stack_space(4);

#ifdef __SOFTBOUNDCETS_SPATIAL

  __softboundcets_store_base_shadow_stack(&new_argv[0], 1);
  __softboundcets_store_bound_shadow_stack(temp_ptr, 1);
  __softboundcets_store_base_shadow_stack(0, 2);
  __softboundcets_store_bound_shadow_stack(0xfffffffffffffff0, 2);

#elif __SOFTBOUNDCETS_TEMPORAL

  //  printf("before writing to shadow stack\n");
  __softboundcets_store_key_shadow_stack(argv_key, 1);
  __softboundcets_store_lock_shadow_stack(argv_loc, 1);

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

  __softboundcets_store_base_shadow_stack(&new_argv[0], 1);
  __softboundcets_store_bound_shadow_stack(temp_ptr, 1);
  __softboundcets_store_key_shadow_stack(argv_key, 1);
  __softboundcets_store_lock_shadow_stack(argv_loc, 1);

#else

  __softboundcets_store_base_shadow_stack(&new_argv[0], 1);
  __softboundcets_store_bound_shadow_stack(temp_ptr, 1);
  __softboundcets_store_key_shadow_stack(argv_key, 1);
  __softboundcets_store_lock_shadow_stack(argv_loc, 1);

#endif
  
  //  printf("before calling program main\n");
  return_value = softboundcets_pseudo_main(argc, new_argv);
  __softboundcets_deallocate_shadow_stack_space();

#ifdef MTE_DEBUG
  atexit(atexit_hook);
#endif
  return return_value;
}

void * __softboundcets_safe_mmap(void* addr, 
                                 size_t length, int prot, 
                                 int flags, int fd, 
                                 off_t offset){
  return mmap(addr, length, prot, flags, fd, offset);
}

void* __softboundcets_safe_calloc(size_t nmemb, size_t size){

  return calloc(nmemb, size);
}

void* __softboundcets_safe_malloc(size_t size){

  return malloc(size);
}
void __softboundcets_safe_free(void* ptr){

  free(ptr);
}
