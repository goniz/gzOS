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

#ifndef _PAGER_H_
#define _PAGER_H_

#include <vm.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef vm_object_t *pgr_alloc_t();

typedef vm_page_t *pgr_fault_t(vm_object_t *, vm_addr_t fault_addr,
                               vm_addr_t offset, vm_prot_t prot);

typedef void pgr_free_t(vm_object_t *);

typedef struct pager {
    pgr_alloc_t *pgr_alloc;
    pgr_free_t *pgr_free;
    pgr_fault_t *pgr_fault;
} pager_t;

extern pager_t default_pager[1];

#ifdef __cplusplus
}
#endif
#endif /* _PAGER_H_ */
