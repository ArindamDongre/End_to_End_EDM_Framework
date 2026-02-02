// vm_ir.h
#ifndef VM_IR_H
#define VM_IR_H

#include "../core/ir.h"   // adjust path if needed

#ifdef __cplusplus
extern "C" {
#endif

void vm_execute(IR *ir);

#ifdef __cplusplus
}
#endif

#endif
