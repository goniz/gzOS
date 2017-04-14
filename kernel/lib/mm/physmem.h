/* The MIT License (MIT)              
                    
    Copyright (c) 2015 Krystian Bacławski
                    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
        
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER   
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.           
              
    Source code was adapted from https://github.com/cahirwpz/mimiker
*/

#ifndef _PHYS_MEM_H_
#define _PHYS_MEM_H_

#include "vm.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pm_seg pm_seg_t;

/* Platform independant initialization of physical memory manager. */
void pm_init();

/* Calculates the size of data structure that should be allocated to describe
 * segment of @size bytes. */
size_t pm_seg_space_needed(size_t size);

/*
 * Initialize data structure describing physical memory segment.
 *
 * Note that this system manages PHYSICAL PAGES. Therefore start and end,
 * should be physical addresses. @offset determines initial virtual address
 * offset for pages, i.e. all pages in this segment will be visible under
 * addresses (start + offset, end + offset) by default.
 */
void pm_seg_init(pm_seg_t *seg, pm_addr_t start, pm_addr_t end,
                 vm_addr_t offset);
/* After using this function pages in range (start, end) are never going to be
 * allocated. Should be used at start to avoid allocating from text, data,
 * ebss, or any possibly unwanted places. */
void pm_seg_reserve(pm_seg_t *seg, pm_addr_t start, pm_addr_t end);
/* Add physical memory segment to physical memory manager. */
void pm_add_segment(pm_seg_t *seg);

/* Allocates contiguous big page that consists of n machine pages. */
vm_page_t *pm_alloc(size_t n);

void pm_free(vm_page_t *page);
void pm_dump();
vm_page_t *pm_split_alloc_page(vm_page_t *pg);

#ifdef __cplusplus
}
#endif //extern "C"
#endif /* _PHYS_MEM_H */

