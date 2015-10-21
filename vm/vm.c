#include "vm.h"

void gc(vm_t* vm);

extern val_t core_print(vm_t* vm);
extern val_t core_println(vm_t* vm);
extern val_t core_getline(vm_t* vm);
extern val_t core_parseFloat(vm_t* vm);
extern val_t core_break(vm_t* vm);

extern val_t math_abs(vm_t* vm);
extern val_t math_sin(vm_t* vm);
extern val_t math_cos(vm_t* vm);
extern val_t math_tan(vm_t* vm);
extern val_t math_sqrt(vm_t* vm);
extern val_t math_floor(vm_t* vm);
extern val_t math_ceil(vm_t* vm);
extern val_t math_pow(vm_t* vm);

extern val_t io_readFile(vm_t* vm);
extern val_t io_writeFile(vm_t* vm);

static gvm_c_function system_methods[] = {
	core_print,			// 1
	core_println,		// 2
	core_getline,		// 3
	core_parseFloat,	// 4
	core_break,			// 5

	math_abs,			// 6
	math_sin,			// 7
	math_cos,			// 8
	math_tan,			// 9
	math_sqrt,			// 10
	math_floor,			// 11
	math_ceil,			// 12
	math_pow,			// 13

	io_readFile,		// 14
	io_writeFile,		// 15
	0
};

// Compile in parser to invokedynamic, <address>, <arguments>
// where address refers to the index in the system_methods

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->pc = 0;
	vm->fp = 0;
	vm->sp = 0;
	vm->reserve = 0;
	vm->running = false;
	vm->firstVal = 0;
	vm->numObjects = 0;
	vm->maxObjects = 8;
	return vm;
}

void vm_throw(vm_t* vm, const char* format, ...)
{
    printf("=> Exception thrown: ");
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    printf("\nat: PC(%d), SP(%d), FP(%d)\n", vm->pc, vm->sp, vm->fp);
	vm->running = false;
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

	// Is the objects are containers, check their content
	switch(obj->type)
	{
		case OBJ_ARRAY:
		{
			obj_array_t* arr = obj->data;
			for(size_t i = 0; i < arr->len; i++) {
				val_append(vm, arr->data[i]);
			}
			break;
		}
		case OBJ_CLASS:
		{
			obj_class_t* cls = obj->data;
			for(size_t i = 0; i < CLASS_FIELDS_SIZE; i++)
			{
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

// Fetches the next instruction
instruction_t* vm_fetch(vm_t* vm, vector_t* buffer)
{
	return vector_get(buffer, vm->pc);
}

// Just prints out instruction codes
void vm_print_code(vm_t* vm, vector_t* buffer)
{
	vm->pc = 0;
	printf("\nImmediate code:\n");

	instruction_t* instr = vm_fetch(vm, buffer);
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
		instr = vm_fetch(vm, buffer);
	}
	vm->pc = 0;
}

void reserve(vm_t* vm, size_t args)
{
	//if(vm->reserve > 0)
	//{
		vm->reserve += 1;
		for(size_t i = 1; i <= args; i++)
		{
			vm->stack[vm->sp+vm->reserve-i] = vm->stack[vm->sp-i];
		}
		vm->stack[vm->sp+vm->reserve-args-1] = INT32_VAL(vm->reserve-1);

		vm->sp += vm->reserve;
	//}
}

void revert_reserve(vm_t* vm)
{
	int undo = AS_INT32(pop(vm));
	for(int i = 0; i < undo; i++)
	{
		pop(vm);
	}
}

// Processes a buffer instruction based on instruction / program counter (pc).
void vm_process(vm_t* vm, instruction_t* instr)
{

#ifdef TRACE
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
		if(vm->stack[i] != NULL_VAL)
		{
			val_print(vm->stack[i]);
			if(i < vm->sp-1) printf(", ");
		}
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

#endif

	switch(instr->op)
	{
		case OP_PUSH:
		{
			vm_register(vm, COPY_VAL(instr->v1));
			break;
		}
		case OP_POP:
		{
			pop(vm);
			break;
		}
		case OP_HLT:
		{
			vm->running = false;
			break;
		}
		case OP_STORE:
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
			break;
		}
		case OP_LOAD:
		{
			int offset = AS_INT32(instr->v1);
			if(offset < 0)
			{
				val_t v = vm->stack[vm->fp+offset];
				vm_register(vm, COPY_VAL(v));
				//push(vm, v);
			}
			else
			{
				val_t v = vm->locals[vm->fp+offset];
				vm_register(vm, COPY_VAL(v));
				//push(vm, v);
			}
			break;
		}
		case OP_GSTORE:
		{
			int offset = AS_INT32(instr->v1);
			//val_free(vm->locals[offset]);
			vm->locals[offset] = pop(vm);
			break;
		}
		case OP_GLOAD:
		{
			int offset = AS_INT32(instr->v1);
			val_t v = vm->locals[offset];
			vm_register(vm, COPY_VAL(v));
			break;
		}
		case OP_LDARG0:
		{
			size_t args = AS_INT32(vm->stack[vm->fp-3]);
			//push(vm, vm->stack[vm->fp-args-4]);
			vm_register(vm, COPY_VAL(vm->stack[vm->fp-args-4]));
			break;
		}
		case OP_SETARG0:
		{
			size_t args = AS_INT32(vm->stack[vm->fp-3]);
			vm->stack[vm->fp-args-4] = pop(vm);
			break;
		}
		case OP_UPVAL:
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
			vm_register(vm, COPY_VAL(val));
			break;
		}
		case OP_UPSTORE:
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
				//val_free(vm->locals[vm->fp+offset]);
				vm->locals[vm->fp+offset] = newVal;
			}

			vm->sp = sp;
			vm->fp = fp;
			break;
		}
		case OP_SYSCALL:
		{
			size_t index = AS_INT32(instr->v1);
			val_t ret = system_methods[index](vm);
			vm_register(vm, ret);
			break;
		}
		case OP_INVOKE:
		{
			// Arguments already on the stack
			int address = AS_INT32(instr->v1);
			size_t args = AS_INT32(instr->v2);

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
			return;
		}
		case OP_INVOKEVIRTUAL:
		{
			int address = AS_INT32(instr->v1);
			size_t args = AS_INT32(instr->v2);

			//if(args > 0) reserve(vm, args+1);
			reserve(vm, args+1);
			push(vm, INT32_VAL(args));
			push(vm, INT32_VAL(vm->fp));
			push(vm, INT32_VAL(vm->pc));
			vm->fp = vm->sp;
			vm->pc = address;
			return;
		}
		case OP_RESERVE:
		{
			// Reserves some memory.
			// Uses this before any function call to maintain stored values.
			// Otherwise, if too many locals are saved in the last scope e.g. from 1 - 11,
			// the new scope overwrites the old values for instance: fp at 5, overwrites 5 - 11.

			vm->reserve = AS_INT32(instr->v1);
			break;
		}
		case OP_RET:
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
			break;
		}
		case OP_RETVIRTUAL:
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
			break;
		}
		case OP_JMP:
		{
			vm->pc = AS_INT32(instr->v1);
			return;
		}
		case OP_JMPF:
		{
			bool result = AS_BOOL(pop(vm));
			if(!result)
			{
				vm->pc = AS_INT32(instr->v1);
				return;
			}
			break;
		}
		case OP_ARR:
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
			vm_register(vm, OBJ_VAL(obj));
			break;
		}
		case OP_STR:
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
			vm_register(vm, OBJ_VAL(obj));
			break;
		}
		case OP_LDLIB:
		{
			break;
		}
		case OP_TOSTR:
		{
			val_t val = pop(vm);
			char* str = val_tostr(val);
			vm_register(vm, STRING_NOCOPY_VAL(str));
			break;
		}
		case OP_IADD:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 + v2));
			break;
		}
		case OP_ISUB:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 - v2));
			break;
		}
		case OP_IMUL:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 * v2));
			break;
		}
		case OP_IDIV:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 / v2));
			break;
		}
		case OP_MOD:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 % v2));
			break;
		}
		case OP_BITL:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 << v2));
			break;
		}
		case OP_BITR:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 >> v2));
			break;
		}
		case OP_BITAND:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 & v2));
			break;
		}
		case OP_BITOR:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 | v2));
			break;
		}
		case OP_BITXOR:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(v1 ^ v2));
			break;
		}
		case OP_BITNOT:
		{
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(~v1));
			break;
		}
		case OP_IMINUS:
		{
			int v1 = AS_INT32(pop(vm));
			push(vm, INT32_VAL(-v1));
			break;
		}
		case OP_I2F:
		{
			int v1 = AS_INT32(pop(vm));
			push(vm, NUM_VAL(v1));
			break;
		}
		case OP_FADD:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 + v2));
			break;
		}
		case OP_FSUB:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 - v2));
			break;
		}
		case OP_FMUL:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 * v2));
			break;
		}
		case OP_FDIV:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 / v2));
			break;
		}
		case OP_FMINUS:
		{
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(-v1));
			break;
		}
		case OP_F2I:
		{
			double v1 = AS_NUM(pop(vm));
			push(vm, INT32_VAL((int)v1));
			break;
		}

		case OP_NOT:
		{
			bool b = AS_BOOL(pop(vm));
			push(vm, BOOL_VAL(!b));
			break;
		}
		case OP_BEQ:
		{
			bool b2 = AS_BOOL(pop(vm));
			bool b1 = AS_BOOL(pop(vm));
			push(vm, BOOL_VAL(b1 == b2));
			break;
		}

		case OP_CEQ:
		case OP_IEQ:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 == v2));
			break;
		}
		case OP_FEQ:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 == v2));
			break;
		}

		case OP_CNE:
		case OP_INE:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 != v2));
			break;
		}
		case OP_FNE:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 != v2));
			break;
		}

		case OP_ILT:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 < v2));
			break;
		}
		case OP_IGT:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 > v2));
			break;
		}
		case OP_ILE:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 <= v2));
			break;
		}
		case OP_IGE:
		{
			int v2 = AS_INT32(pop(vm));
			int v1 = AS_INT32(pop(vm));
			push(vm, BOOL_VAL(v1 >= v2));
			break;
		}

		case OP_FLT:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 < v2));
			break;
		}
		case OP_FGT:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 > v2));
			break;
		}
		case OP_FLE:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 <= v2));
			break;
		}
		case OP_FGE:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 >= v2));
			break;
		}

		// Bool
		case OP_BAND:
		{
			bool b2 = AS_BOOL(pop(vm));
			bool b1 = AS_BOOL(pop(vm));
			push(vm, BOOL_VAL(b1 && b2));
			break;
		}
		case OP_BOR:
		{
			bool b2 = AS_BOOL(pop(vm));
			bool b1 = AS_BOOL(pop(vm));
			push(vm, BOOL_VAL(b1 || b2));
			break;
		}

		// Array operators
		case OP_GETSUB:
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
				push(vm, INT32_VAL(str[idx]));
			}
			else
			{
				obj_array_t* arr = AS_ARRAY(obj);
				val_t element = COPY_VAL(arr->data[idx]);
				vm_register(vm, element);
			}
			break;
		}
		case OP_SETSUB:
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
				data[idx] = (char)AS_INT32(val);
				vm_register(vm, obj);
			}
			else
			{
				// Copy the whole array
				// Free the copied object at index
				// Upload the new array
				obj = COPY_VAL(obj);
				obj_array_t* arr = AS_ARRAY(obj);
				val_free(arr->data[idx]);
				arr->data[idx] = val;
				vm_register(vm, obj);
			}
			break;
		}
		case OP_LEN:
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
			break;
		}
		case OP_CONS:
		{
			// Construct a new value on top
			val_t val = pop(vm);
			val_t obj = pop(vm);

			if(IS_STRING(obj))
			{
				char* str = AS_STRING(obj);
				size_t len = strlen(str);
				char c = (char)AS_INT32(val);
				char* newStr = malloc(sizeof(char) * (len+1));
				strcpy(newStr, str);
				newStr[len] = c;
				newStr[len+1] = '\0';

				obj_t* obj_ptr = obj_string_nocopy_new(newStr);
				vm_register(vm, OBJ_VAL(obj_ptr));
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
				arr->data = arr->len == 1 ? malloc(allocSz) : realloc(arr->data, allocSz);
				arr->data[arr->len-1] = val;
				vm_register(vm, obj);
			}
			break;
		}
		case OP_APPEND:
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
				vm_register(vm, OBJ_VAL(obj_ptr));
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
				vm_register(vm, OBJ_VAL(newObj));
			}
			break;
		}

		// Class operations
		case OP_CLASS:
		{
			obj_t* obj = obj_class_new();
			vm_register(vm, OBJ_VAL(obj));
			break;
		}
		case OP_SETFIELD:
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
			break;
		}
		case OP_GETFIELD:
		{
			// Copy val to keep class internal value alive
			int index = AS_INT32(instr->v1);
			val_t class = pop(vm);

			obj_class_t* cls = AS_CLASS(class);
			val_t val = cls->fields[index];
			val = COPY_VAL(val); // <--
			vm_register(vm, val);
			//push(vm, val);
			break;
		}
		default:
		{
			printf("Uknown operator encountered\n");
			break;
		}
	}

	vm->pc++;
}

// Executes a buffer / list of instructions
void vm_run(vm_t* vm, vector_t* buffer)
{
	// Reset vm
	vm->sp = 0;
	vm->pc = 0;
	vm->fp = 0;
	vm->reserve = 0;
	vm->running = true;

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

	while(vm->running)
	{
		vm_process(vm, vm_fetch(vm, buffer));
	}

#ifndef NO_TIME
	clock_t end = clock();
	double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Elapsed time: %.5f (sec)\n", elapsed);
#endif
#endif

	// Move stack pointer to zero, -> clears all elements by gc
	// Discard the rest
	for(int i = 0; i < LOCALS_SIZE; i++)
	{
		vm->locals[i] = NULL_VAL;
	}

	vm->sp = 0;
	gc(vm);
}

// Frees the memory used by the vm
void vm_free(vm_t* vm)
{
	free(vm);
}
