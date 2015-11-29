#include "vm.h"

void gc(vm_t* vm);

extern val_t core_print(vm_t* vm);
extern val_t core_println(vm_t* vm);
extern val_t core_getline(vm_t* vm);
extern val_t core_parseFloat(vm_t* vm);
extern val_t core_break(vm_t* vm);
extern val_t core_clock(vm_t* vm);
extern val_t core_sysarg(vm_t* vm);

extern val_t math_abs(vm_t* vm);
extern val_t math_sin(vm_t* vm);
extern val_t math_cos(vm_t* vm);
extern val_t math_tan(vm_t* vm);
extern val_t math_sqrt(vm_t* vm);
extern val_t math_floor(vm_t* vm);
extern val_t math_ceil(vm_t* vm);
extern val_t math_pow(vm_t* vm);
extern val_t math_sinh(vm_t* vm);
extern val_t math_cosh(vm_t* vm);
extern val_t math_tanh(vm_t* vm);
extern val_t math_prng(vm_t* vm);

extern val_t io_readFile(vm_t* vm);
extern val_t io_writeFile(vm_t* vm);

static gvm_c_function system_methods[] = {
	core_print,			// 1
	core_println,		// 2
	core_getline,		// 3
	core_parseFloat,	// 4
	core_break,			// 5
	core_clock,			// 6
	core_sysarg,		// 8

	math_abs,			// 9
	math_sin,			// 9
	math_cos,			// 10
	math_tan,			// 11
	math_sqrt,			// 12
	math_floor,			// 13
	math_ceil,			// 14
	math_pow,			// 15
	math_sinh,			// 16
	math_cosh,			// 17
	math_tanh,			// 18
	math_prng,			// 19

	io_readFile,		// 20
	io_writeFile,		// 21
	0
};

void vm_clear(vm_t* vm);

void vm_init(vm_t* vm)
{
	vm->pc = 0;
	vm->fp = 0;
	vm->sp = 0;
	vm->reserve = 0;
	vm->firstVal = 0;
	vm->numObjects = 0;
	vm->maxObjects = 8;
	vm->errjmp = 0;
}

#define VM_ASSERT(x, msg) \
	if(!(x)) { vm_throw(vm, msg); goto *dispatch_table[OP_HLT]; }

void vm_throw(vm_t* vm, const char* format, ...)
{
    printf("=> Exception thrown: ");
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    printf("\nat: PC(%d), SP(%d), FP(%d)\n", vm->pc, vm->sp, vm->fp);

	vm_clear(vm);
	vm->pc = vm->errjmp;
}

void mark(val_t v)
{
	if(IS_OBJ(v))
	{
		obj_t* obj = AS_OBJ(v);
		if(!obj->marked)
		{
			obj->marked = 1;

			switch(obj->type)
			{
				case OBJ_CLASS:
				{
					obj_class_t* cls = obj->data;
					for(size_t i = 0; i < CLASS_FIELDS_SIZE; i++)
					{
						mark(cls->fields[i]);
					}
					break;
				}
				case OBJ_ARRAY:
				{
					obj_array_t* arr = obj->data;
					for(size_t i = 0; i < arr->len; i++)
					{
						mark(arr->data[i]);
					}
					break;
				}
				default: break;
			}
		}
	}
}

void markAll(vm_t* vm)
{
	for(int i = 0; i < vm->sp; i++)
	{
		mark(vm->stack[i]);
	}
	for(int i = 0; i < LOCALS_SIZE; i++)
	{
		mark(vm->locals[i]);
	}
}

void sweep(vm_t* vm)
{
	obj_t** val = &vm->firstVal;
	while(*val)
	{
		if(!(*val)->marked)
		{
			obj_t* unreached = *val;
			*val = unreached->next;
			obj_free(unreached);
			unreached = 0;
			vm->numObjects--;
		}
		else
		{
			(*val)->marked = 0;
			val = &(*val)->next;
		}
	}
}

void gc(vm_t* vm)
{
// Garbage day!
#ifdef TRACE
	printf("Collecting garbage...\n");
#endif

#ifdef TRACE_STEP
	printf("Beginning objects:%d\n", vm->numObjects);
#endif

	markAll(vm);
	sweep(vm);
	vm->maxObjects = vm->numObjects * 2;

#ifdef TRACE_STEP
	printf("New objects:%d\n", vm->numObjects);
#endif
}

void push(vm_t* vm, val_t val)
{
	if(vm->sp >= STACK_SIZE)
	{
		vm_throw(vm, "Stack overflow");
		return;
	}

	vm->stack[vm->sp] = val;
	vm->sp++;
}

val_t pop(vm_t* vm)
{
	vm->sp--;
	val_t v = vm->stack[vm->sp];
	vm->stack[vm->sp] = 0;
	return v;
}

void val_append(vm_t* vm, val_t v1);

void obj_append(vm_t* vm, obj_t* obj)
{
	if(vm->numObjects >= vm->maxObjects)
	{
		gc(vm);
	}

	obj->marked = 0;
	obj->next = vm->firstVal;
	vm->firstVal = obj;
	vm->numObjects++;

	// If the objects are containers, check their content
	switch(obj->type)
	{
		case OBJ_ARRAY:
		{
			obj_array_t* arr = obj->data;
			/*for(size_t i = 0; i < arr->len; i++) {
				val_append(vm, arr->data[i]);
			}*/

			// faster ?
			for(size_t i = arr->len; i != 0; i--) {
				val_append(vm, arr->data[i-1]);
			}
			break;
		}
		case OBJ_CLASS:
		{
			obj_class_t* cls = obj->data;
			for(size_t i = 0; i < CLASS_FIELDS_SIZE; i++) {
				val_append(vm, cls->fields[i]);
			}
			break;
		}
		default: break;
	}
}

void val_append(vm_t* vm, val_t v1)
{
	if(IS_OBJ(v1))
	{
		obj_append(vm, AS_OBJ(v1));
	}
}

// Push and register in GC, if v1 is a ptr
void vm_register(vm_t* vm, val_t v1)
{
	push(vm, v1);
	val_append(vm, v1);
}

// Just prints out instruction codes
void vm_print_code(vm_t* vm, vector_t* buffer)
{
	vm->pc = 0;
	printf("\nImmediate code:\n");

	instruction_t* instr = vector_get(buffer, vm->pc);
	while(instr->op != OP_HLT)
	{
		printf("  %.2d: %s", vm->pc, op2str(instr->op));
		if(instr->v1 != NULL_VAL)
		{
			printf(", ");
			val_print(instr->v1);
		}
		if(instr->v2 != NULL_VAL)
		{
			printf(", ");
			val_print(instr->v2);
		}
		putchar('\n');

		vm->pc++;
		instr = vector_get(buffer, vm->pc);
	}
	vm->pc = 0;
}

void reserve(vm_t* vm, size_t args)
{
	vm->reserve += 1;
	for(size_t i = 1; i <= args; i++) {
		vm->stack[vm->sp+vm->reserve-i] = vm->stack[vm->sp-i];
	}
	vm->stack[vm->sp+vm->reserve-args-1] = INT32_VAL(vm->reserve-1);

	vm->sp += vm->reserve;
}

void revert_reserve(vm_t* vm)
{
	int undo = AS_INT32(pop(vm));
	for(int i = 0; i < undo; i++) {
		pop(vm);
	}
}

void vm_trace_print(vm_t* vm, instruction_t* instr)
{
	printf("  %.2d (SP:%.2d, FP:%.2d): %s", vm->pc, vm->sp, vm->fp, op2str(instr->op));
	if(instr->v1 != NULL_VAL)
	{
		printf(", ");
		val_print(instr->v1);
	}
	if(instr->v2 != NULL_VAL)
	{
		printf(", ");
		val_print(instr->v2);
	}
	printf(" => STACK [");

	//int begin = vm->sp - 8;
	//if(begin < 0) begin = 0;

	for(int i = 0; i < vm->sp; i++)
	{
		val_print(vm->stack[i]);
		if(i < vm->sp-1) printf(", ");
	}
	printf("]\n");

#ifdef TRACE_STEP
	printf("OBJECTS: %d\n", vm->numObjects);

	char c = getchar();
	if(c == 'c')
	{
		gc(vm);
	}
#endif
}

// Fast, optimized version for vm_register.
// Use if value needs to be copied and pushed onto the stack.
void vm_copy(vm_t* vm, val_t val) {
	if(IS_OBJ(val))
	{
		obj_t* obj = AS_OBJ(val);
		obj_t* newObj = COPY_OBJ(obj);

		push(vm, OBJ_VAL(newObj));
		obj_append(vm, newObj);
	}
	else
	{
		push(vm, val);
	}
}

// Processes a buffer instruction based on instruction / program counter (pc).
void vm_exec(vm_t* vm, vector_t* buffer)
{
	static void* dispatch_table[] =
	{
		&&code_hlt,
		&&code_push,
		&&code_pop,
		&&code_store,
		&&code_load,
		&&code_gstore,
		&&code_gload,
		&&code_ldarg0,
		&&code_setarg0,
		&&code_iadd,
		&&code_isub,
		&&code_imul,
		&&code_idiv,
		&&code_mod,
		&&code_bitl,
		&&code_bitr,
		&&code_bitand,
		&&code_bitor,
		&&code_bitxor,
		&&code_bitnot,
		&&code_iminus,
		&&code_i2f,
		&&code_fadd,
		&&code_fsub,
		&&code_fmul,
		&&code_fdiv,
		&&code_fminus,
		&&code_f2i,
		&&code_not,
		&&code_b2i,
		&&code_syscall,
		&&code_invoke,
		&&code_invokevirtual,
		&&code_reserve,
		&&code_ret,
		&&code_retvirtual,
		&&code_jmp,
		&&code_jmpf,
		&&code_arr,
		&&code_str,
		&&code_ldlib,
		&&code_tostr,
		&&code_beq,
		&&code_ieq,
		&&code_feq,
		&&code_bne,
		&&code_ine,
		&&code_fne,
		&&code_ilt,
		&&code_igt,
		&&code_ile,
		&&code_ige,
		&&code_flt,
		&&code_fgt,
		&&code_fle,
		&&code_fge,
		&&code_band,
		&&code_bor,
		&&code_getsub,
		&&code_setsub,
		&&code_len,
		&&code_append,
		&&code_cons,
		&&code_upval,
		&&code_upstore,
		&&code_class,
		&&code_setfield,
		&&code_getfield
	};

	// Set the jmp position if an error occurs
	vm->errjmp = vector_size(buffer)-1;

	// Create the tmp instruction
	instruction_t* instr = 0;

#ifndef TRACE
	#define FETCH() instr = buffer->data[vm->pc++];
#else
	#define FETCH() instr = buffer->data[vm->pc++]; \
		vm_trace_print(vm, instr)
#endif

	// DISPATCH -> jump to pc and increment pc afterwards
	#define DISPATCH() \
		FETCH(); \
		goto *dispatch_table[instr->op]

	// Dispatch and run
	DISPATCH();
	code_hlt: return;
	code_push:
	{
		vm_copy(vm, instr->v1);
		DISPATCH();
	}
	code_pop:
	{
		pop(vm);
		DISPATCH();
	}
	code_store:
	{
		int offset = AS_INT32(instr->v1);
		if(offset < 0)
		{
			vm->stack[vm->fp+offset] = pop(vm);
		}
		else
		{
			vm->locals[vm->fp+offset] = pop(vm);
		}
		DISPATCH();
	}
	code_load:
	{
		int offset = AS_INT32(instr->v1);
		if(offset < 0)
		{
			val_t v = vm->stack[vm->fp+offset];
			vm_copy(vm, v);
		}
		else
		{
			val_t v = vm->locals[vm->fp+offset];
			vm_copy(vm, v);
		}
		DISPATCH();
	}
	code_gstore:
	{
		int offset = AS_INT32(instr->v1);
		vm->locals[offset] = pop(vm);
		DISPATCH();
	}
	code_gload:
	{
		int offset = AS_INT32(instr->v1);
		val_t v = vm->locals[offset];
		vm_copy(vm, v);
		DISPATCH();
	}
	code_ldarg0:
	{
		size_t args = AS_INT32(vm->stack[vm->fp-3]);
		vm_copy(vm, vm->stack[vm->fp-args-4]);
		DISPATCH();
	}
	code_setarg0:
	{
		size_t args = AS_INT32(vm->stack[vm->fp-3]);
		vm->stack[vm->fp-args-4] = pop(vm);
		DISPATCH();
	}
	code_iadd:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 + v2));
		DISPATCH();
	}
	code_isub:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 - v2));
		DISPATCH();
	}
	code_imul:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 * v2));
		DISPATCH();
	}
	code_idiv:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 / v2));
		DISPATCH();
	}
	code_mod:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 % v2));
		DISPATCH();
	}
	code_bitl:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 << v2));
		DISPATCH();
	}
	code_bitr:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 >> v2));
		DISPATCH();
	}
	code_bitand:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 & v2));
		DISPATCH();
	}
	code_bitor:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 | v2));
		DISPATCH();
	}
	code_bitxor:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(v1 ^ v2));
		DISPATCH();
	}
	code_bitnot:
	{
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(~v1));
		DISPATCH();
	}
	code_iminus:
	{
		int v1 = AS_INT32(pop(vm));
		push(vm, INT32_VAL(-v1));
		DISPATCH();
	}
	code_i2f:
	{
		int v1 = AS_INT32(pop(vm));
		push(vm, NUM_VAL(v1));
		DISPATCH();
	}
	code_fadd:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, NUM_VAL(v1 + v2));
		DISPATCH();
	}
	code_fsub:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, NUM_VAL(v1 - v2));
		DISPATCH();
	}
	code_fmul:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, NUM_VAL(v1 * v2));
		DISPATCH();
	}
	code_fdiv:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, NUM_VAL(v1 / v2));
		DISPATCH();
	}
	code_fminus:
	{
		double v1 = AS_NUM(pop(vm));
		push(vm, NUM_VAL(-v1));
		DISPATCH();
	}
	code_f2i:
	{
		double v1 = AS_NUM(pop(vm));
		push(vm, INT32_VAL((int)v1));
		DISPATCH();
	}
	code_not:
	{
		bool b = AS_BOOL(pop(vm));
		push(vm, BOOL_VAL(!b));
		DISPATCH();
	}
	code_b2i:
	{
		bool b = AS_BOOL(pop(vm));
		push(vm, INT32_VAL(b));
		DISPATCH();
	}
	code_syscall:
	{
		size_t index = AS_INT32(instr->v1);
		val_t ret = system_methods[index](vm);
		vm_register(vm, ret);
		DISPATCH();
	}
	code_invoke:
	{
		// Arguments already on the stack
		int address = AS_INT32(instr->v1);
		int args = AS_INT32(instr->v2);

		// Arg0 -3
		// Arg1 -2
		// Arg2 -1
		// sp (args=3) (reserve=2)
		// ...  +1
		// ...  +2
		reserve(vm, args);

		push(vm, INT32_VAL(args));
		push(vm, INT32_VAL(vm->fp));
		push(vm, INT32_VAL(vm->pc));

		// |   STACK_BOTTOM	  |
		// |...				  |
		// |Arg0			-7|
		// |Arg1			-6|
		// |Arg2			-5|
		// |...				-4|
		// |NUM_ARGS		-3|
		// |FP				-2|	<-- vm->fp - 2
		// |PC				-1|
		// |...				 0|	<-- current position sp / fp
		// |...				+1|
		// |    STACK_TOP     |

		vm->fp = vm->sp;
		vm->pc = address;
		// DISPATCH_RAW();
		DISPATCH();
	}
	code_invokevirtual:
	{
		int address = AS_INT32(instr->v1);
		int args = AS_INT32(instr->v2);

		//if(args > 0) reserve(vm, args+1);
		reserve(vm, args+1);
		push(vm, INT32_VAL(args));
		push(vm, INT32_VAL(vm->fp));
		push(vm, INT32_VAL(vm->pc));
		vm->fp = vm->sp;
		vm->pc = address;
		// DISPATCH_RAW();
		DISPATCH();
	}
	code_reserve:
	{
		// Reserves some memory.
		// Uses this before any function call to maintain stored values.
		// Otherwise, if too many locals are saved in the last scope e.g. from 1 - 11,
		// the new scope overwrites the old values for instance: fp at 5, overwrites 5 - 11.

		vm->reserve = AS_INT32(instr->v1);
		DISPATCH();
	}
	code_ret:
	{
		// Returns to previous instruction pointer,
		// and pushes the return value back on the stack.
		// If you call this function make sure there is a return value on the stack.
		val_t ret = pop(vm);

		vm->sp = vm->fp;
		vm->pc = AS_INT32(pop(vm));
		vm->fp = AS_INT32(pop(vm));
		size_t args = AS_INT32(pop(vm));

		vm->sp -= args;
		revert_reserve(vm);

		push(vm, ret);
		DISPATCH();
	}
	code_retvirtual:
	{
		// Returns from a virtual class function
		val_t ret = pop(vm);

		vm->sp = vm->fp;
		vm->pc = AS_INT32(pop(vm));
		vm->fp = AS_INT32(pop(vm));
		size_t args = AS_INT32(pop(vm));

		vm->sp -= args;
		val_t clazz = pop(vm);
		revert_reserve(vm);

		push(vm, ret);
		push(vm, clazz);
		DISPATCH();
	}
	code_jmp:
	{
		vm->pc = AS_INT32(instr->v1);
		DISPATCH();
	}
	code_jmpf:
	{
		bool result = AS_BOOL(pop(vm));
		if(!result)
		{
			vm->pc = AS_INT32(instr->v1);
		}
		DISPATCH();
	}
	code_arr:
	{
		// Reverse list fetching and inserting.
		// Copying is not needed, because array consumes all the objects.
		// The objects already have to be a copy.
		size_t elsz = AS_INT32(instr->v1);
		val_t* arr = malloc(sizeof(val_t) * elsz);
		for(int i = elsz; i > 0; i--)
		{
			// Get index object
			val_t val = vm->stack[vm->sp - i];
			arr[elsz - i] = COPY_VAL(val);		// <-- vm_register causes all sub objects also to be appended, therefore error
			vm->stack[vm->sp - i] = NULL_VAL;
		}
		vm->sp -= elsz;

		obj_t* obj = obj_array_new(arr, elsz);
		push(vm, OBJ_VAL(obj));
		obj_append(vm, obj);
		DISPATCH();
	}
	code_str:
	{
		size_t elsz = AS_INT32(instr->v1);
		char *str = malloc(sizeof(char) * (elsz+1));

		for(int i = elsz; i > 0; i--)
		{
			val_t val = vm->stack[vm->sp - i];
			str[elsz - i] = (char)AS_INT32(val);
			vm->stack[vm->sp - i] = 0;
		}
		vm->sp -= elsz;
		str[elsz] = '\0';
		obj_t* obj = obj_string_nocopy_new(str);
		push(vm, OBJ_VAL(obj));
		obj_append(vm, obj);
		DISPATCH();
	}
	code_ldlib:
	{
		// TODO: implement
		DISPATCH();
	}
	code_tostr:
	{
		val_t val = pop(vm);
		char* str = val_tostr(val);
		vm_register(vm, STRING_NOCOPY_VAL(str));
		DISPATCH();
	}
	code_beq:
	{
		bool b2 = AS_BOOL(pop(vm));
		bool b1 = AS_BOOL(pop(vm));
		push(vm, BOOL_VAL(b1 == b2));
		DISPATCH();
	}
	code_ieq:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 == v2));
		DISPATCH();
	}
	code_feq:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 == v2));
		DISPATCH();
	}
	code_bne:
	{
		bool b2 = AS_BOOL(pop(vm));
		bool b1 = AS_BOOL(pop(vm));
		push(vm, BOOL_VAL(b1 != b2));
		DISPATCH();
	}
	code_ine:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 != v2));
		DISPATCH();
	}
	code_fne:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 != v2));
		DISPATCH();
	}
	code_ilt:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 < v2));
		DISPATCH();
	}
	code_igt:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 > v2));
		DISPATCH();
	}
	code_ile:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 <= v2));
		DISPATCH();
	}
	code_ige:
	{
		int v2 = AS_INT32(pop(vm));
		int v1 = AS_INT32(pop(vm));
		push(vm, BOOL_VAL(v1 >= v2));
		DISPATCH();
	}
	code_flt:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 < v2));
		DISPATCH();
	}
	code_fgt:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 > v2));
		DISPATCH();
	}
	code_fle:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 <= v2));
		DISPATCH();
	}
	code_fge:
	{
		double v2 = AS_NUM(pop(vm));
		double v1 = AS_NUM(pop(vm));
		push(vm, BOOL_VAL(v1 >= v2));
		DISPATCH();
	}
	code_band:
	{
		bool b2 = AS_BOOL(pop(vm));
		bool b1 = AS_BOOL(pop(vm));
		push(vm, BOOL_VAL(b1 && b2));
		DISPATCH();
	}
	code_bor:
	{
		bool b2 = AS_BOOL(pop(vm));
		bool b1 = AS_BOOL(pop(vm));
		push(vm, BOOL_VAL(b1 || b2));
		DISPATCH();
	}
	code_getsub:
	{
		// Stack:
		// | object |
		// | key 	|
		// | getsub |
		val_t key = pop(vm);
		val_t obj = pop(vm);
		int idx = AS_INT32(key);

		if(IS_STRING(obj))
		{
			char* str = AS_STRING(obj);
			// VM_ASSERT(idx >= 0 && idx < strlen(str), "Array index out of bounds");
			push(vm, INT32_VAL(str[idx]));
		}
		else
		{
			obj_array_t* arr = AS_ARRAY(obj);
			// VM_ASSERT(idx >= 0 && idx < arr->len, "Array index out of bounds");
			vm_copy(vm, arr->data[idx]);
		}
		DISPATCH();
	}
	code_setsub:
	{
		// Stack:
		// | value	|
		// | object |
		// | key 	|
		// | setsub |
		val_t key = pop(vm);
		val_t obj = pop(vm);
		val_t val = pop(vm);
		int idx = AS_INT32(key);

		if(IS_STRING(obj))
		{
			obj = COPY_VAL(obj);
			char* data = AS_STRING(obj);
			// VM_ASSERT(idx >= 0 && idx < strlen(data), "Array index out of bounds");
			data[idx] = (char)AS_INT32(val);
			vm_register(vm, obj);
		}
		else
		{
			// Copy the whole array
			// Upload the new array
			obj = COPY_VAL(obj);

			// Free the copied object at index
			obj_array_t* arr = AS_ARRAY(obj);
			val_free(arr->data[idx]);

			// Try to replace it
			// VM_ASSERT(idx >= 0 && idx < arr->len, "Array index out of bounds");
			arr->data[idx] = val;
			vm_register(vm, obj);
		}
		DISPATCH();
	}
	code_len:
	{
		val_t obj = pop(vm);

		if(IS_STRING(obj))
		{
			char* data = AS_STRING(obj);
			push(vm, INT32_VAL(strlen(data)-1));
		}
		else
		{
			obj_array_t* arr = AS_ARRAY(obj);
			push(vm, INT32_VAL(arr->len));
		}
		DISPATCH();
	}
	code_append:
	{
		val_t val = pop(vm);
		val_t obj = pop(vm);

		if(IS_STRING(obj))
		{
			// Simple string concatenation
			char* str1 = AS_STRING(obj);
			char* str2 = AS_STRING(val);
			size_t len = strlen(str1) + strlen(str2) + 1;
			char* data = malloc(sizeof(char) * len);
			data[0] = '\0';
			strcat(data, str1);
			strcat(data, str2);

			obj_t* obj_ptr = obj_string_nocopy_new(data);
			push(vm, OBJ_VAL(obj_ptr));
			obj_append(vm, obj_ptr);
		}
		else
		{
			// Get the two array
			// Allocate a new val_t array
			// Upload it into a obj_t form
			// register it / push it to the stack
			obj_array_t* arr1 = AS_ARRAY(obj);
			obj_array_t* arr2 = AS_ARRAY(val);

			size_t len = arr1->len + arr2->len;
			val_t* arr3 = malloc(sizeof(val_t) * len);

			size_t i;
			for(i = 0; i < arr1->len; i++)
			{
				arr3[i] = val_copy(arr1->data[i]);
			}
			for(i = 0; i < arr2->len; i++)
			{
				arr3[i+arr1->len] = val_copy(arr2->data[i]);
			}

			obj_t* newObj = obj_array_new(arr3, len);
			push(vm, OBJ_VAL(newObj));
			obj_append(vm, newObj);
		}
		DISPATCH();
	}
	code_cons:
	{
		// Construct a new value on top
		val_t val = pop(vm);
		val_t obj = pop(vm);

		if(IS_STRING(obj))
		{
			// Allocate len + 2 => one for the char and one for the trailing zero
			char* str = AS_STRING(obj);
			size_t len = strlen(str);
			char c = (char)AS_INT32(val);
			char* newStr = malloc(sizeof(char) * (len+2));
			strcpy(newStr, str);
			newStr[len] = c;
			newStr[len+1] = '\0';

			obj_t* obj_ptr = obj_string_nocopy_new(newStr);
			push(vm, OBJ_VAL(obj_ptr));
			obj_append(vm, obj_ptr);
		}
		else
		{
			// Copy the whole array
			obj = COPY_VAL(obj);

			// Get the information
			obj_array_t* arr = AS_ARRAY(obj);
			arr->len += 1;
			size_t allocSz = sizeof(val_t) * arr->len;

			// Reallocate and assign its content
			arr->data = (arr->len == 1) ? malloc(allocSz) : realloc(arr->data, allocSz);
			arr->data[arr->len-1] = COPY_VAL(val);
			vm_register(vm, obj);
		}
		DISPATCH();
	}
	code_upval:
	{
		int scopes = AS_INT32(instr->v1);
		int offset = AS_INT32(instr->v2);
		int fp = vm->fp;
		int sp = vm->sp;

		for(int i = 0; i < scopes; i++)
		{
			vm->fp = AS_INT32(vm->stack[vm->fp - 2]);
		}
		vm->sp = vm->fp;

		val_t val = (offset < 0) ? vm->stack[vm->fp+offset] : vm->locals[vm->fp+offset];
		vm->sp = sp;
		vm->fp = fp;

		vm_copy(vm, val);
		//vm_register(vm, COPY_VAL(val));
		DISPATCH();
	}
	code_upstore:
	{
		val_t newVal = pop(vm);

		int scopes = AS_INT32(instr->v1);
		int offset = AS_INT32(instr->v2);

		int fp = vm->fp;
		int sp = vm->sp;

		for(int i = 0; i < scopes; i++)
		{
			vm->fp = AS_INT32(vm->stack[vm->fp - 2]);
		}
		vm->sp = vm->fp;

		if(offset < 0)
		{
			vm->stack[vm->fp+offset] = newVal;
		}
		else
		{
			vm->locals[vm->fp+offset] = newVal;
		}

		vm->sp = sp;
		vm->fp = fp;
		DISPATCH();
	}
	code_class:
	{
		obj_t* obj = obj_class_new();
		push(vm, OBJ_VAL(obj));
		obj_append(vm, obj);

		//vm_register(vm, OBJ_VAL(obj));
		DISPATCH();
	}
	code_setfield:
	{
		// Stack
		// ---
		// value
		// class

		int index = AS_INT32(instr->v1);
		val_t val = pop(vm);
		val_t class = pop(vm);

		obj_class_t* cls = AS_CLASS(class);
		cls->fields[index] = val;

		push(vm, class);
		DISPATCH();
	}
	code_getfield:
	{
		// Copy val to keep class internal value alive
		int index = AS_INT32(instr->v1);
		val_t class = pop(vm);

		obj_class_t* cls = AS_CLASS(class);
		val_t val = cls->fields[index];
		val = COPY_VAL(val); // <--
		vm_register(vm, val);
		DISPATCH();
	}
}

void vm_clear(vm_t* vm)
{
	// Move stack pointer to zero, -> clears all elements by gc
	// Discard the rest
	// Nullify the locals
	memset64(vm->locals, NULL_VAL, sizeof(val_t) * LOCALS_SIZE);

	vm->sp = 0;
	gc(vm);

	vm->argc = 0;
	vm->argv = 0;
}

void vm_run(vm_t* vm, vector_t* buffer)
{
	vm_run_args(vm, buffer, 0, 0);
}

// Execute a buffer
void vm_run_args(vm_t* vm, vector_t* buffer, int argc, char** argv)
{
	// Reset vm
	vm->argc = argc;
	vm->argv = argv;
	vm->sp = 0;
	vm->pc = 0;
	vm->fp = 0;
	vm->reserve = 0;
	memset64(vm->stack, NULL_VAL, sizeof(val_t) * STACK_SIZE);
	memset64(vm->locals, NULL_VAL, sizeof(val_t) * LOCALS_SIZE);

#ifndef NO_IR
	// Print out bytecodes
	vm_print_code(vm, buffer);
	printf("\nExecution:\n");
#endif

	// Run
#ifndef NO_EXEC
#ifndef NO_TIME
	clock_t begin = clock();
#endif

	vm_exec(vm, buffer);

#ifndef NO_TIME
	clock_t end = clock();
	double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Elapsed time: %.5f (sec)\n", elapsed);
#endif
#endif

	vm_clear(vm);
}
