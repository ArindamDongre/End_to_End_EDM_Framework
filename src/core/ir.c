#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

IR* ir_create() {
    IR *ir = (IR*)malloc(sizeof(IR));
    ir->capacity = 32;
    ir->size = 0;
    ir->instructions = (IRInstr*)malloc(sizeof(IRInstr) * ir->capacity);
    return ir;
}

void ir_emit(IR *p, IRInstr instr) {
    if (p->size >= p->capacity) {
        p->capacity *= 2;
        p->instructions = (IRInstr*)realloc(p->instructions, sizeof(IRInstr) * p->capacity);
    }
    p->instructions[p->size++] = instr;
}

IRInstr make_instr(IROp op, int value, const char* name, int line) {
    IRInstr instr;
    memset(&instr, 0, sizeof(IRInstr));
    instr.op = op;
    instr.value = value;
    instr.line = line;
    if (name) {
        strncpy(instr.name, name, 31);
    }
    return instr;
}

void ir_dump(IR *p) {
    if (!p || p->size == 0) {
        printf("  (No instructions generated)\n");
        return;
    }
    for (int i = 0; i < p->size; i++) {
        IRInstr instr = p->instructions[i];
        printf("[%03d] (L%d) ", i, instr.line);
        switch(instr.op) {
            case IR_LOAD_CONST: printf("LOAD_CONST %d\n", instr.value); break;
            case IR_LOAD_VAR:   printf("LOAD_VAR   %s\n", instr.name); break;
            case IR_STORE_VAR:  printf("STORE_VAR  %s\n", instr.name); break;
            case IR_ADD:        printf("ADD\n"); break;
            case IR_SUB:        printf("SUB\n"); break;
            case IR_MUL:        printf("MUL\n"); break;
            case IR_DIV:        printf("DIV\n"); break;
            case IR_EQ:         printf("CMP_EQ\n"); break;
            case IR_NE:         printf("CMP_NE\n"); break;
            case IR_LT:         printf("CMP_LT\n"); break;
            case IR_GT:         printf("CMP_GT\n"); break;
            case IR_LE:         printf("CMP_LE\n"); break;
            case IR_GE:         printf("CMP_GE\n"); break;
            case IR_JMP:   printf("JMP L%d\n", instr.value); break;
            case IR_JZ:    printf("JZ L%d\n", instr.value); break;
            case IR_LABEL: printf("L%d:\n", instr.value); break;
            default:            printf("UNKNOWN_OP\n"); break;
        }
    }
}

void ir_free(IR *p) {
    if (p) {
        free(p->instructions);
        free(p);
    }
}

/* =========================
   Variable store
   ========================= */

typedef struct Var {
    char name[32];
    int value;
    struct Var *next;
} Var;

static Var *var_table = NULL;

static Var* find_var(const char *name) {
    for (Var *v = var_table; v; v = v->next) {
        if (strcmp(v->name, name) == 0)
            return v;
    }
    return NULL;
}

void ir_reset_vars() {
    while (var_table) {
        Var *tmp = var_table;
        var_table = var_table->next;
        free(tmp);
    }
    var_table = NULL;
}

int ir_get_var(const char *name) {
    Var *v = find_var(name);
    if (!v) {
        printf("Runtime Error: Variable '%s' not found\n", name);
        exit(1);
    }
    return v->value;
}

void ir_set_var(const char *name, int value) {
    Var *v = find_var(name);
    if (!v) {
        v = malloc(sizeof(Var));
        strcpy(v->name, name);
        v->value = value;
        v->next = var_table;
        var_table = v;
    } else {
        v->value = value;
    }
}

void ir_print_vars() {
    printf("\n--- VM Final State ---\n");
    for (Var *v = var_table; v; v = v->next) {
        printf("%s = %d\n", v->name, v->value);
    }
    printf("----------------------\n");
}

void ir_resolve_labels(IR *ir) {
    if (!ir) return;

    // 1. Find the highest Label ID to size our map
    int max_label = -1;
    for (int i = 0; i < ir->size; i++) {
        if (ir->instructions[i].op == IR_LABEL) {
            if (ir->instructions[i].value > max_label) {
                max_label = ir->instructions[i].value;
            }
        }
    }

    if (max_label < 0) return; // No labels to resolve

    // 2. Create a map: Label ID -> Instruction Index
    int *label_map = (int*)malloc(sizeof(int) * (max_label + 1));
    for (int i = 0; i <= max_label; i++) label_map[i] = -1;

    for (int i = 0; i < ir->size; i++) {
        if (ir->instructions[i].op == IR_LABEL) {
            label_map[ir->instructions[i].value] = i;
        }
    }

    // 3. Update Jump Instructions
    for (int i = 0; i < ir->size; i++) {
        IROp op = ir->instructions[i].op;
        if (op == IR_JMP || op == IR_JZ) {
            int lbl_id = ir->instructions[i].value;
            
            if (lbl_id >= 0 && lbl_id <= max_label && label_map[lbl_id] != -1) {
                // Replace Label ID with actual Instruction Index
                ir->instructions[i].value = label_map[lbl_id];
            } else {
                printf("Linker Error: Jump to undefined label L%d\n", lbl_id);
            }
        }
    }

    free(label_map);
}