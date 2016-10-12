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

#include <malloc.h>
#include <vm_object.h>
#include <vm_pager.h>
#include <assert.h>
#include "physmem.h"

static vm_object_t *default_pager_alloc() {
  vm_object_t *obj = vm_object_alloc();
  obj->pgr = (pager_t *)default_pager;
  return obj;
}

static void default_pager_free(vm_object_t *obj) {
  vm_object_free(obj);
}

static vm_page_t *default_pager_fault(vm_object_t *obj, vm_addr_t fault_addr,
                                      vm_addr_t vm_offset, vm_prot_t vm_prot) {
  assert(obj != NULL);

  vm_page_t *new_pg = pm_alloc(1);
  new_pg->vm_offset = vm_offset;
  vm_object_add_page(obj, new_pg);
  return new_pg;
}

pager_t default_pager[1] = {{
                                    .pgr_alloc = default_pager_alloc,
                                    .pgr_free = default_pager_free,
                                    .pgr_fault = default_pager_fault,
                            }};
