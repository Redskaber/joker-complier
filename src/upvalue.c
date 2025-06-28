//
// Created by Kilig on 2024/11/21.
//
#include <stdio.h>

#include "object.h"
#include "memory.h"
#include "upvalue.h"
#include "vm.h"


upvalue_info_t new_upvalue_info(uint8_t index, bool is_local) {
    return (uint8_t)index | ((uint8_t)is_local << 7);
}

UpvaluePtr new_upvalue(VirtualMachine *vm, Value* slot) {
	UpvaluePtr upvalue = macro_allocate_object(vm, Upvalue, OBJ_UPVALUE);
	upvalue->location = slot;
	upvalue->closed = macro_val_null;
	upvalue->next = NULL;
	return upvalue;
}

void free_upvalue(UpvaluePtr self) {
    if (self == NULL) return;
    if (self->location != NULL) {
        macro_free(self->base.vm, Upvalue, self);
    }
}

bool upvalue_equal(UpvaluePtr a, UpvaluePtr b) {
    return values_equal(a->closed, b->closed);
}

void print_upvalues(UpvaluePtr self) {
    Upvalue* upv = self;
    while(upv != NULL) {
        print_upvalue(upv);
        upv = upv->next;
    }
}

void print_upvalue(UpvaluePtr self) {
    printf("upvalue[{%p} {%p} {", self, self->location);
    print_value(self->closed);
    printf("}]\n");
}

int snprintf_upvalue(UpvaluePtr self, char* buf, size_t size) {
    return snprintf_value(self->closed, buf, size);
}


/*
* vm->open_upv_ptr: upvalue header pointer
* vm --> root                   -----
*	1.prev_upvalue = NULL			|
*   2.curr_upvalue = root         <--
*
* Cmp{ Upvalue } ´ó --> Ð¡
*   1.Eq	return
*   2.Insert local
*
* foreach link list find 'local' exist? link
*      ---{108} ||   {74}   || {56}------
*	   V(head)         V(mid)      V(back)
*     [{89}, {87}, {83}, {72}, ..., ]
*/
Upvalue* capture_upvalue(VirtualMachine *vm, Value* local) {
	UpvaluePtr prev_upv = NULL;
	UpvaluePtr curr_upv = vm->open_upv_ptr;

	while (curr_upv != NULL && curr_upv->location > local) {
		prev_upv = curr_upv;
		curr_upv = curr_upv->next;
	}
	/* exist curr_upv && eq */
	if (curr_upv != NULL && curr_upv->location == local) {
		return curr_upv;
	}

	/* foreach over (not in link list) */
	UpvaluePtr created_upvalue = new_upvalue(vm, local);
    created_upvalue->next = curr_upv;
	if (prev_upv == NULL) {
        vm->open_upv_ptr = created_upvalue; // header
	}
	else {
		prev_upv->next = created_upvalue;	// mid || back
	}

	return created_upvalue;
}

/*
* root_ref: vm->open_upv_ptr (**Upvalue)
*	Upvalue {
*		Object base;
*		Value* location;    <--- add { closed: Value }, update { location: closed address }
*		struct Upvalue* next;
*	}
*				  V-(2)<-
*              curr_upv | ---------(1)-------->closed
*                 V-(2)->						 |
* {&} root_ref: header -> node -> node -> ...    |
*				  V                              |
*               Value     <--------(1)---------closed
* close prev:
*	Upvalue::location	=> pointer stack slot
* close after:
*	Upvalue::location	=> pointer Upvalue::closed
*	Upvalue::closed		=> stored stack value
*/
void close_upvalues(UpvaluePtr root, Value* last) {
    UpvaluePtr upv = root;
    while (upv != NULL && upv->location > last) {
        upv->closed = *upv->location;
        upv->location = &upv->closed;
        upv = upv->next;
    }
}
