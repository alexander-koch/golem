#include "compiler.h"
datatype_t compiler_eval(compiler_t* compiler, ast_t* node);

// Compiler.init()
void compiler_init(compiler_t* compiler)
{
	compiler->buffer = 0;
	compiler->filename = 0;
	compiler->error = false;
	compiler->scope = 0;
}

// Compiler.throw(node, message)
// If an error occurs, this function can be used as an exception.
// It halts the compilation, and prints an error message to the console.
// The parameter node is needed to get the location in the source-code that is wrong.
void compiler_throw(compiler_t* compiler, ast_t* node, const char* format, ...)
{
	compiler->error = true;
    location_t loc = node->location;
    fprintf(stdout, "[line %d, column %d] (Semantic): ", loc.line, loc.column);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

void push_scope(compiler_t* compiler)
{
	// Create scope
	scope_t* scope = scope_new();
	scope->super = compiler->scope;

	// Register new scope in parent
	list_push(compiler->scope->subscopes, scope);

	// Set child active scope
	compiler->scope = scope;
	compiler->depth++;
	compiler->scope->address = 0;
}

void pop_scope(compiler_t* compiler)
{
	// Get parent scope
	scope_t* super = compiler->scope->super;

	// Set parent scope to active
	compiler->scope = super;
	compiler->depth--;
}

// Symbol.new(node, address, type)
// Stores an indentifier with an address, an AST and a type
// If the scope depth is zero, the  global flag is set to true
symbol_t* symbol_new(compiler_t* compiler, ast_t* node, int address, datatype_t type)
{
	symbol_t* symbol = malloc(sizeof(*symbol));
	symbol->node = node;
	symbol->address = address;
	symbol->type = type;
	symbol->global = compiler->depth == 0 ? true : false;
	symbol->used = 1;
	symbol->ref = 0;
	return symbol;
}

// Symbol.isLocal(string ident, ref int depth)
// Checks if the symbol is stored in the current local scope.
symbol_t* symbol_get_ext(scope_t* scope, char* ident, int* depth)
{
	void* val;
	if(hashmap_get(scope->symbols, ident, &val) != HMAP_MISSING)
	{
		symbol_t* sym = (symbol_t*)val;
		sym->used++;
		return sym;
	}

	if(scope->super)
	{
		(*depth)++;
		return symbol_get_ext(scope->super, ident, depth);
	}
	return 0;
}

// Symbol.get(string ident)
// Tries to find a symbol by the given identifier.
// This function works recursive and searches upwards until
// there is no parent available.
// If the symbol is found it is returned, if not, a null pointer
// is returned.
symbol_t* symbol_get(scope_t* scope, char* ident)
{
	void* val;
	if(hashmap_get(scope->symbols, ident, &val) != HMAP_MISSING)
	{
		symbol_t* sym = (symbol_t*)val;
		sym->used++;
		return sym;
	}

	if(scope->super)
	{
		return symbol_get(scope->super, ident);
	}
	return 0;
}

symbol_t* class_find(scope_t* scope, unsigned long id)
{
	hashmap_iterator_t* iter = hashmap_iterator_create(scope->classes);
	while(!hashmap_iterator_end(iter))
	{
		symbol_t* symbol = hashmap_iterator_next(iter);
		if(symbol)
		{
			if(symbol->type.id == id)
			{
				hashmap_iterator_free(iter);
				return symbol;
			}
		}
	}
	hashmap_iterator_free(iter);

	if(scope->super)
	{
		return class_find(scope->super, id);
	}
	return 0;
}

// Symbol.exists(node, name)
// Test if given identifier exists in the symbol-table.
// If so, an exception is thrown and true returned.
bool symbol_exists(compiler_t* compiler, ast_t* node, char* ident)
{
	symbol_t* symbol = symbol_get(compiler->scope, ident);
	if(symbol)
	{
		location_t loc = symbol->node->location;
		compiler_throw(compiler, node,
			"Redefinition of symbol '%s'.\nPrevious definition: [line %d column %d]",
			ident, loc.line, loc.column);
		return true;
	}
	return false;
}

void symbol_replace(compiler_t* compiler, symbol_t* symbol)
{
	int depth = 0;
	symbol_get_ext(compiler->scope, symbol->node->vardecl.name, &depth);

	if(depth == 0 || symbol->global)
	{
		emit_store(compiler->buffer, symbol->address, symbol->global);
	}
	else
	{
		emit_store_upval(compiler->buffer, depth, symbol->address);
	}
}

// Eval.block(List<Block>)
// The function evaluates a list of Abstract syntax trees.
// The datatype of the last entry is returned.
// If there is no entry, datatype_new(DATA_NULL) is returned.
datatype_t eval_block(compiler_t* compiler, list_t* block)
{
	datatype_t ret = datatype_new(DATA_NULL);
	list_iterator_t* iter = list_iterator_create(block);
	while(!list_iterator_end(iter))
	{
		ret = compiler_eval(compiler, list_iterator_next(iter));
	}
	list_iterator_free(iter);
	return ret;
}

// Eval.declfunc(node)
// Compiles a function.
// Therefore it creates a jump at the beginning to prevent runtime execution.
// A new scope is created as well as the parameter symbols and
// the function signature. Finally the body gets analysed.
datatype_t eval_declfunc(compiler_t* compiler, ast_t* node)
{
	// Check if existing
	if(symbol_exists(compiler, node, node->funcdecl.name)) return datatype_new(DATA_NULL);

	// External sentinel
	if(node->funcdecl.external)
	{
		symbol_t* sym = symbol_new(compiler, node, -1, node->funcdecl.rettype);
		hashmap_set(compiler->scope->symbols, node->funcdecl.name, sym);
		return datatype_new(DATA_NULL);
	}

	// Emit jump
	value_t* addr = emit_jmp(compiler->buffer, 0);

	// Register a symbol for the function, save the bytecode address
	int byte_address = vector_size(compiler->buffer);
	symbol_t* fnSymbol = symbol_new(compiler, node, byte_address, datatype_new(DATA_LAMBDA));
	hashmap_set(compiler->scope->symbols, node->funcdecl.name, fnSymbol);

	// Create a new scope
	push_scope(compiler);

	// Treat each parameter as a local variable, with no type or value
	list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
	int i = -(list_size(node->funcdecl.impl.formals) + 3);
	while(!list_iterator_end(iter))
	{
		// Create parameter in symbols list
	 	ast_t* param = list_iterator_next(iter);

		if(symbol_exists(compiler, param, param->vardecl.name)) return datatype_new(DATA_NULL);
		symbol_t* symbol = symbol_new(compiler, param, i, param->vardecl.type);
		hashmap_set(compiler->scope->symbols, param->vardecl.name, symbol);
		i++;
	}
	list_iterator_free(iter);

	// Body analysis
	iter = list_iterator_create(node->funcdecl.impl.body);
	bool hasReturn = false;
	while(!list_iterator_end(iter))
	{
		ast_t* sub = list_iterator_next(iter);
		if(sub->class == AST_RETURN && !list_iterator_end(iter))
		{
			compiler_throw(compiler, node, "Return statement declared before end was reached");
			break;
		}

		if(sub->class == AST_RETURN)
		{
			hasReturn = true;
			if(sub->returnstmt && node->funcdecl.rettype.type == DATA_VOID)
			{
				compiler_throw(compiler, node, "Functions with type void do not return a value");
				break;
			}
			else if(!sub->returnstmt && node->funcdecl.rettype.type != DATA_VOID)
			{
				compiler_throw(compiler, node, "Return statement without a value");
				break;
			}
		}
		compiler_eval(compiler, sub);
	}

	list_iterator_free(iter);
	pop_scope(compiler);

	// Handle void return
	if(node->funcdecl.rettype.type == DATA_VOID)
	{
		// Just return 0 if void
		emit_int(compiler->buffer, 0);
		emit_return(compiler->buffer);
	}
	else
	{
		if(hasReturn == false)
		{
			compiler_throw(compiler, node, "Warning: Function without return statement");
		}
	}

	// Set beginning jump address to end
	byte_address = vector_size(compiler->buffer);
	value_set_int(addr, byte_address);
	return datatype_new(DATA_LAMBDA);
}

// Eval.declvar(node)
// The function evaluates a variable declaration.
// This registers a new symbol with the name
// and the current available address.
// The return value is NULL.
datatype_t eval_declvar(compiler_t* compiler, ast_t* node)
{
	if(symbol_exists(compiler, node, node->vardecl.name)) return datatype_new(DATA_NULL);

	// First eval initializer to get type
	// Then test for non-valid types
	datatype_t vartype = compiler_eval(compiler, node->vardecl.initializer);
	if(vartype.type == DATA_VOID)
	{
		compiler_throw(compiler, node, "Variable initializer is of type VOID");
		return datatype_new(DATA_NULL);
	}
	else if(vartype.type == DATA_NULL)
	{
		compiler_throw(compiler, node, "Variable initializer is NULL");
		return datatype_new(DATA_NULL);
	}
	else if(vartype.type == DATA_LAMBDA)
	{
		compiler_throw(compiler, node, "Trying to assign a function to a value (Currently not supported)");
		return datatype_new(DATA_NULL);
	}

	// Store the symbol
	symbol_t* symbol = symbol_new(compiler, node, compiler->scope->address, vartype);
	symbol->node->vardecl.type = vartype;
	hashmap_set(compiler->scope->symbols, node->vardecl.name, symbol);

	// Emit bytecode and increase compiler address
	emit_store(compiler->buffer, symbol->address, symbol->global);
	compiler->scope->address++;

	// Debug variables if flag is set
#ifdef DB_VARS
	console("Created %s variable '%s' of data type <%s>\n", node->vardecl.mutate ? "mutable" : "immutable", node->vardecl.name, datatype2str(vartype));
#endif
	return datatype_new(DATA_NULL);
}

// Eval.number(node)
// The function evaluates a number.
// The number can only be of type float or integer.
// Emits only one push instruction
datatype_t eval_number(compiler_t* compiler, ast_t* node)
{
	if(node->class == AST_FLOAT)
	{
		emit_float(compiler->buffer, node->f);
		return datatype_new(DATA_FLOAT);
	}

	emit_int(compiler->buffer, node->i);
	return datatype_new(DATA_INT);
}

datatype_t eval_bool(compiler_t* compiler, ast_t* node)
{
	emit_bool(compiler->buffer, node->b);
	return datatype_new(DATA_BOOL);
}

// Eval.binary(node)
// This function evaluates a binary node.
// A binary node consists of two seperate nodes
// connected with an operator.
// The contents will be optimized.
// ---------------
// Example:
// bin(a, b, +) -> a + b
// bin(a, bin(b, c, *), +) -> a + (b * c)
datatype_t eval_binary(compiler_t* compiler, ast_t* node)
{
	ast_t* lhs = node->binary.left;
	ast_t* rhs = node->binary.right;
	token_type_t op = node->binary.op;

	// Integer check -> first optimization pass
	if(lhs->class == AST_INT && rhs->class == AST_INT)
	{
	    switch(op)
	    {
	        case TOKEN_ADD:
	        {
	            lhs->i = lhs->i + rhs->i;
	            break;
	        }
	        case TOKEN_SUB:
	        {
	            lhs->i = lhs->i - rhs->i;
	            break;
	        }
	        case TOKEN_MUL:
	        {
	            lhs->i = lhs->i * rhs->i;
	            break;
	        }
	        case TOKEN_DIV:
	        {
	            lhs->i = lhs->i / rhs->i;
	            break;
	        }
	        case TOKEN_MOD:
	        {
	            lhs->i = lhs->i % rhs->i;
	            break;
	        }
	        case TOKEN_BITLSHIFT:
	        {
	            lhs->i = lhs->i << rhs->i;
	            break;
	        }
	        case TOKEN_BITRSHIFT:
	        {
	            lhs->i = lhs->i >> rhs->i;
	            break;
	        }
	        case TOKEN_BITAND:
	        {
	            lhs->i = lhs->i & rhs->i;
	            break;
	        }
	        case TOKEN_BITOR:
	        {
	            lhs->i = lhs->i | rhs->i;
	            break;
	        }
	        case TOKEN_BITXOR:
	        {
	            lhs->i = lhs->i ^ rhs->i;
	            break;
	        }
	        case TOKEN_EQUAL:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i == rhs->i);
	            break;
	        }
	        case TOKEN_NEQUAL:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i != rhs->i);
	            break;
	        }
	        case TOKEN_LEQUAL:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i <= rhs->i);
	            break;
	        }
	        case TOKEN_GEQUAL:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i >= rhs->i);
	            break;
	        }
	        case TOKEN_LESS:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i < rhs->i);
	            break;
	        }
	        case TOKEN_GREATER:
	        {
	            lhs->class = AST_BOOL;
	            lhs->b = (lhs->i > rhs->i);
	            break;
	        }
	        default:
	        {
				compiler_throw(compiler, node, "Invalid operator. Operator might not be available for integers");
	            return datatype_new(DATA_NULL);
	        }
	    }

		return compiler_eval(compiler, lhs);
	}

	// Float checking -> second optimization pass
	if(lhs->class == AST_FLOAT && rhs->class == AST_FLOAT)
	{
		switch(op)
		{
			case TOKEN_ADD:
			{
				lhs->f = lhs->f + rhs->f;
				break;
			}
			case TOKEN_SUB:
			{
				lhs->f = lhs->f - rhs->f;
				break;
			}
			case TOKEN_MUL:
			{
				lhs->f = lhs->f * rhs->f;
				break;
			}
			case TOKEN_DIV:
			{
				lhs->f = lhs->f / rhs->f;
				break;
			}
			case TOKEN_EQUAL:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f == rhs->f);
				break;
			}
			case TOKEN_NEQUAL:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f != rhs->f);
				break;
			}
			case TOKEN_LEQUAL:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f <= lhs->f);
				break;
			}
			case TOKEN_GEQUAL:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f >= lhs->f);
				break;
			}
			case TOKEN_LESS:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f < lhs->f);
				break;
			}
			case TOKEN_GREATER:
			{
				lhs->class = AST_BOOL;
				lhs->b = (lhs->f > lhs->f);
				break;
			}
			default:
			{
				compiler_throw(compiler, node, "Invalid operator. Operator might not be available for floats");
				return datatype_new(DATA_NULL);
			}
		}

		return compiler_eval(compiler, lhs);
	}

	// Assignment operator: special case
	if(op == TOKEN_ASSIGN)
	{
		if(lhs->class == AST_IDENT)
		{
			symbol_t* symbol = symbol_get(compiler->scope, lhs->ident);
			if(symbol)
			{
				if(symbol->node->class == AST_DECLVAR)
				{
					if(!symbol->node->vardecl.mutate)
					{
						compiler_throw(compiler, node, "Invalid statement, trying to modifiy an immutable variable");
						return datatype_new(DATA_NULL);
					}

					if(symbol->ref)
					{
						emit_op(compiler->buffer, OP_LDARG0);
					}

					datatype_t dt = compiler_eval(compiler, rhs);
					if(!datatype_match(dt, symbol->node->vardecl.type))
					{
						compiler_throw(compiler, node, "Warning: Change of types is not permitted");
						return datatype_new(DATA_NULL);
					}

					if(symbol->ref)
					{
						symbol_t* classRef = symbol->ref;
						ast_t* classNode = classRef->node;
						void* val = 0;
						if(hashmap_get(classNode->classstmt.fields, lhs->ident, &val) == HMAP_MISSING)
						{
							compiler_throw(compiler, node, "No such class field");
							return datatype_new(DATA_NULL);
						}

						symbol = (symbol_t*)val;
						emit_class_setfield(compiler->buffer, symbol->address);
					}

					symbol_replace(compiler, symbol);
				}
				else
				{
					// Left-hand side must be a variable
					// Example 5 = 4 does not make any sense
					// Also you can't replace a function <- function is stored in a variable
					compiler_throw(compiler, node, "Left hand side value must be a variable");
					return datatype_new(DATA_NULL);
				}
			}
			else
			{
				// If the symbol is not found, throw an error
				compiler_throw(compiler, node, "Warning: Implicit declaration of field '%s'", lhs->ident);
				return datatype_new(DATA_NULL);
			}
		}
		// If it's not an identifier
		// e.g.: myArray[5] = 28
		else if(lhs->class == AST_SUBSCRIPT)
		{
			// Subscript: expr, key: myArray[key]
			// expr must be an identifier
			ast_t* expr = lhs->subscript.expr;
			ast_t* key = lhs->subscript.key;

			if(expr->class != AST_IDENT)
			{
				// [1,2,3,4][0] = 8
				// can be valid too, optimize or throw error
				// TODO: assignment return a value
				compiler_throw(compiler, node, "Warning: Identifier for index access expected");
			}
			else
			{
				symbol_t* symbol = symbol_get(compiler->scope, expr->ident);
				if(symbol)
				{
					ast_t* symNode = symbol->node;
					if(symNode->class != AST_DECLVAR)
					{
						compiler_throw(compiler, node, "Subscripts are only allowed for variables");
					}
					else
					{
						if(!symNode->vardecl.mutate)
						{
							compiler_throw(compiler, node, "The field '%s' is immutable", expr->ident);
						}
						else
						{
							// Example:
							// let var = "Hello World"
							// var[0] = "B"

							// If we found a subscript, it has to be an array
							// Evaluate the rhs and lhs
							datatype_t rhsType = compiler_eval(compiler, rhs);
							datatype_t lhsType = compiler_eval(compiler, expr);
							datatype_t arrType = datatype_new(lhsType.type & ~DATA_ARRAY);

							if(!datatype_match(arrType, rhsType))
							{
								compiler_throw(compiler, node, "Assignment value has the wrong type");
								return datatype_new(DATA_NULL);
							}

							// rhs -> int / string / anyhing but null and void
 							// lhs -> string / array otherwise error
							// lhs -> vardecl / namespace varaible declaration

							compiler_eval(compiler, key);
							emit_op(compiler->buffer, OP_SETSUB);
							//emit_store(compiler->buffer, symbol->address, symbol->global);
							symbol_replace(compiler, symbol);
						}
					}
				}
				else
				{
					compiler_throw(compiler, node, "Warning: Implicit declaration of field '%s'", expr->ident);
				}
			}
			return datatype_new(DATA_NULL);
		}
		else
		{
			compiler_throw(compiler, node, "TODO: Unknown assignment operation");
			return datatype_new(DATA_NULL);
		}
	}
	else
	{
		// Simple binary instruction
		// Can be evaluated easier.

		// Emit node op-codes
		datatype_t lhs_type = compiler_eval(compiler, lhs);
		datatype_t rhs_type = compiler_eval(compiler, rhs);

		if(!datatype_match(lhs_type, rhs_type))
		{
			compiler_throw(compiler, node, "Cannot perform operation '%s' on the types '%s' and '%s'", tok2str(op), datatype2str(lhs_type), datatype2str(rhs_type));
			return datatype_new(DATA_NULL);
		}

		// Emit operator and test if allowed
		bool valid = emit_tok2op(compiler->buffer, op, lhs_type);
		if(!valid)
		{
			compiler_throw(compiler, node, "Cannot perform operation '%s' on the types '%s' and '%s'", tok2str(op), datatype2str(lhs_type), datatype2str(rhs_type));
			return datatype_new(DATA_NULL);
		}

		// Get the type for the compiler
		switch(op)
		{
			case TOKEN_EQUAL:
			case TOKEN_NEQUAL:
			case TOKEN_LESS:
			case TOKEN_GREATER:
			case TOKEN_LEQUAL:
			case TOKEN_GEQUAL:
			case TOKEN_AND:
			case TOKEN_OR: return datatype_new(DATA_BOOL);
			default:
			{
				return lhs_type;
			}
		}
	}
	return datatype_new(DATA_NULL);
}

// Eval.ident(node)
// This function evaluates an occuring symbol and loads its content.
datatype_t eval_ident(compiler_t* compiler, ast_t* node)
{
	int depth = 0;
	symbol_t* ptr = symbol_get_ext(compiler->scope, node->ident, &depth);
	if(ptr)
	{
		if(ptr->node->class == AST_DECLVAR)
		{
			// If it is a field of a class
			if(ptr->ref)
			{
				symbol_t* classRef = ptr->ref;
				ast_t* classNode = classRef->node;
				void* val = 0;
				if(hashmap_get(classNode->classstmt.fields, node->ident, &val) == HMAP_MISSING)
				{
					compiler_throw(compiler, node, "No such class field");
					return datatype_new(DATA_NULL);
				}

				ptr = (symbol_t*)val;

				emit_op(compiler->buffer, OP_LDARG0);
				emit_class_getfield(compiler->buffer, ptr->address);
				return ptr->type;
			}


			// If local or global
			if(depth == 0 || ptr->global)
			{
				emit_load(compiler->buffer, ptr->address, ptr->global);
			}
			// Otherwise in upper scope
			else
			{
				emit_load_upval(compiler->buffer, depth, ptr->address);
			}
		}
		return ptr->type;
	}

	compiler_throw(compiler, node, "Warning: Implicit declaration of field '%s'", node->ident);
	return datatype_new(DATA_NULL);
}

// Eval.compareAndCall(func, node, address)
// @param func Function declaration node
// @param node Node that causes the call
// @param address Address to call, if the function is internal
// -----
// Helper function for eval_call
// Checks the given values and compares them with the function parameters.
// If they match, the bytecode is emitted.
bool eval_compare_and_call(compiler_t* compiler, ast_t* func, ast_t* node, int address)
{
	size_t argc = list_size(node->call.args);
	ast_t* call = node->call.callee;

	list_t* formals = 0;
	bool external = false;
	if(func->class == AST_DECLFUNC)
	{
		formals = func->funcdecl.impl.formals;
		external = func->funcdecl.external;
	}
	else if(func->class == AST_CLASS)
	{
		formals = func->classstmt.formals;
	}
	size_t paramc = list_size(formals);

	// Param checking
	if(argc > paramc)
	{
		compiler_throw(compiler, node, "Too many arguments for function '%s'. Expected: %d", call->ident, paramc);
		return false;
	}
	else if(argc < paramc)
	{
		compiler_throw(compiler, node, "Too few arguments for function '%s'. Expected: %d", call->ident, paramc);
		return false;
	}
	else
	{
		if(paramc > 0)
		{
			// Valid, test parameter types
			list_iterator_t* iter = list_iterator_create(formals);
			list_iterator_t* args_iter = list_iterator_create(node->call.args);
			int i = 1;
			while(!list_iterator_end(iter))
			{
				// Get the datatypes
				ast_t* param = list_iterator_next(iter);
				datatype_t argType = compiler_eval(compiler, list_iterator_next(args_iter));
				datatype_t paramType = param->vardecl.type;

				// Do template test
				if((paramType.type & DATA_GENERIC) == DATA_GENERIC)
				{
					type_t tp = paramType.type & ~DATA_GENERIC;
					if((argType.type & tp) == tp) continue;
				}

				// Test datatypes
				if(!datatype_match(argType, paramType))
				{
					compiler_throw(compiler, node,
						"Parameter %d has the wrong type.\nFound: %s, expected: %s",
						i, datatype2str(argType), datatype2str(paramType));
					break;
				}
				i++;
			}
			list_iterator_free(args_iter);
			list_iterator_free(iter);
		}
	}

	// Emit invocation
	if(external)
	{
		emit_syscall(compiler->buffer, func->funcdecl.name, argc);
	}
	else
	{
		emit_invoke(compiler->buffer, address, argc);
	}

	return true;
}

// Eval.call(node)
// Evaluates a call:
// Tries to find the given function,
// compares the given parameters with the requested ones and
// returns the functions return type.
datatype_t eval_call(compiler_t* compiler, ast_t* node)
{
	ast_t* call = node->call.callee;
	if(call->class == AST_IDENT)
	{
		symbol_t* symbol = symbol_get(compiler->scope, call->ident);
		if(symbol)
		{

			// If variable, set function to reference
			if(symbol->node->class == AST_DECLVAR && symbol->node->vardecl.type.type == DATA_LAMBDA)
			{
				symbol = symbol->ref;
			}

			if(symbol->node->class == AST_DECLFUNC)
			{
				// Normal function call
				if(eval_compare_and_call(compiler, symbol->node, node, symbol->address))
				{
					return symbol->node->funcdecl.rettype;
				}
			}
			else if(symbol->node->class == AST_CLASS)
			{
				// Class constructor call
				if(eval_compare_and_call(compiler, symbol->node, node, symbol->address))
				{
					return symbol->type;
				}
			}
			else
			{
				compiler_throw(compiler, node, "Identifier '%s' is not a function", call->ident);
			}

			return datatype_new(DATA_NULL);
		}

		compiler_throw(compiler, node, "Implicit declaration of function '%s'", call->ident);
	}
	else if(call->class == AST_SUBSCRIPT)
	{
		// Calling a class function

		// Get the contents
		ast_t* expr = call->subscript.expr;
		ast_t* key = call->subscript.key;

		// Evaluate the lhs
		datatype_t dt = compiler_eval(compiler, expr);

		// EXPERIMENTAL
		// if((dt.type & DATA_ARRAY) == DATA_ARRAY)
		// {
		// 	if(!strcmp(key->ident, "length"))
		// 	{
		// 		if(list_size(call->formals) != 0)
		// 		{
		// 			compiler_throw(compiler, node, "Expected zero arguments");
		// 			return datatype_new(DATA_NULL);
		// 		}
		//
		// 		// Length operation
		// 		//emit_op(compiler->buffer, OP_LEN);
		// 		return datatype_new(DATA_INT);
		// 	}
		// 	else if(!strcmp(key->ident, "cons"))
		// 	{
		// 		if(list_size(call->formals) != 1)
		// 		{
		// 			compiler_throw(compiler, node, "Expected one argument of type array");
		// 			return datatype_new(DATA_NULL);
		// 		}
		//
		// 		return dt;
		// 	}
		// 	else if(!strcmp(key->ident, "insert"))
		// 	{
		// 		if(list_size(call->formals) != 2)
		// 		{
		// 			compiler_throw(compiler, node, "Expected two arguments");
		// 			return datatype_new(DATA_NULL);
		// 		}
		//
		//
		// 	}
		// 	else if(!strcmp(key->ident, "tail"))
		// 	{
		// 		// List tail
		// 		return dt;
		// 	}
		// }

		if(dt.type != DATA_CLASS)
		{
			compiler_throw(compiler, node, "Identifier is not a class");
			return datatype_new(DATA_NULL);
		}

		// Retrive the id and the corresponding class
		unsigned long id = dt.id;
		symbol_t* class = class_find(compiler->scope, id);
		if(!class)
		{
			compiler_throw(compiler, node, "Class does not exist");
			return datatype_new(DATA_NULL);
		}

		// Test if the field of the class is valid
		ast_t* cls = class->node;
		void* val = 0;
		if(hashmap_get(cls->classstmt.fields, key->ident, &val) == HMAP_MISSING)
		{
			compiler_throw(compiler, node, "Class field '%s' does not exist in class '%s'", key->ident, cls->classstmt.name);
			return datatype_new(DATA_NULL);
		}

		// Retrive the field
		class = (symbol_t*)val;
		if(eval_compare_and_call(compiler, class->node, node, class->address))
		{
			// Get the last instruction (invoke)
			// and increase the arguments by one
			instruction_t* ins = vector_top(compiler->buffer);
			value_set_int(ins->v2, value_int(ins->v2)+1);

			// Return the type
			return class->node->funcdecl.rettype;
		}
	}
	else
	{
		// lamdba(x,y,z)- > void { body} (6,2,3) will never occur due to parsing (maybe in future versions)
		compiler_throw(compiler, node, "Callee has to be an identifier or a lambda");
	}

	return datatype_new(DATA_NULL);
}

datatype_t eval_string(compiler_t* compiler, ast_t* node)
{
	emit_string(compiler->buffer, node->string);
	return datatype_new(DATA_STRING);
}

datatype_t eval_char(compiler_t* compiler, ast_t* node)
{
	emit_char(compiler->buffer, node->ch);
	return datatype_new(DATA_CHAR);
}

datatype_t eval_array(compiler_t* compiler, ast_t* node)
{
	datatype_t dt = datatype_new(DATA_NULL);
	list_iterator_t* iter = list_iterator_create(node->array.elements);
	dt = compiler_eval(compiler, list_iterator_next(iter));
	node->array.type = dt;

	if(dt.type == DATA_VOID || dt.type == DATA_NULL)
	{
		compiler_throw(compiler, node, "Invalid: Array is composed of NULL elements");
		return datatype_new(DATA_NULL);
	}
	else if((dt.type & DATA_ARRAY) == DATA_ARRAY)
	{
		compiler_throw(compiler, node, "Multidimensional arrays are not permitted");
		return datatype_new(DATA_NULL);
	}

	// Create an index and iterate through the elements.
	// Typechecks ensure that the array is valid.
	// The index is mainly used for debugging reasons.
	// We already evaluated one element, so set the index to 2
	size_t idx = 2;
	while(!list_iterator_end(iter))
	{
		datatype_t tmp = compiler_eval(compiler, list_iterator_next(iter));
		if(!datatype_match(tmp, dt))
		{
			compiler_throw(compiler, node, "An array can only hold one type of elements (@element %d)", idx);
			list_iterator_free(iter);
			return datatype_new(DATA_NULL);
		}

		if(tmp.type == DATA_NULL)
		{
			compiler_throw(compiler, node, "Invalid array. (@element %d is NULL)", idx);
			list_iterator_free(iter);
			return tmp;
		}
		idx++;
	}
	list_iterator_free(iter);

	// | STACK_BOTTOM
	// | ...
	// | OP_ARR, element size
	// | STACK_TOP
	size_t ls = list_size(node->array.elements);
	insert_v1(compiler->buffer, OP_ARR, value_new_int(ls));
	datatype_t ret = datatype_new(DATA_ARRAY | node->array.type.type);
	ret.id = node->array.type.id;
	return ret;
}

// Eval.if(node)
// The function evaluates ifclauses
// by emitting jumps around the instructions.
// ---------------
// Example:
// if(x == 5) {
// 	println(5)
// } else if(x == 4) {
// 	println(4)
// } else {
//  println(x)
// }
//
// Bytecode compilation:
// 01: load x
// 02: push 5
// 03: ieq			<-- condition 1
// 04: jmpf 8		<-- jump to condition 2
// 05: push 5
// 06: syscall, println, 1
// 07: jmp 12		<-- end block, jump to end
// 08: load x 		<-- condition 2
// 09: push 5
// 10: ieq
// 11: jmpf ..		<-- jump to condition 3 and so on
datatype_t eval_if(compiler_t* compiler, ast_t* node)
{
	// Eval the sub-ifclauses
	list_t* jmps = list_new();
	list_iterator_t* iter = list_iterator_create(node->ifstmt);
	while(!list_iterator_end(iter))
	{
		// Get sub-ifclause
		ast_t* subnode = list_iterator_next(iter);

		// Test if not an else-statement
		value_t* instr = 0;
		if(subnode->ifclause.cond)
		{
			// Eval the condition and generate an if-false jump
			compiler_eval(compiler, subnode->ifclause.cond);
			instr = emit_jmpf(compiler->buffer, 0);
		}

		// Eval execution block code
		eval_block(compiler, subnode->ifclause.body);

		// Optimization
		// If not an else statement and more ifclauses than one
		// Generate a jump to end
		if(list_size(node->ifstmt) > 1 && subnode->ifclause.cond)
		{
			value_t* pos = emit_jmp(compiler->buffer, 0);
			list_push(jmps, pos);
		}

		// If not an else statement
		// Generate jump to next clause
		if(subnode->ifclause.cond)
		{
			instr->v.i = vector_size(compiler->buffer);
		}
	}

	list_iterator_free(iter);

	// Set the jump points to end after whole if block
	iter = list_iterator_create(jmps);
	while(!list_iterator_end(iter))
	{
		value_t* pos = list_iterator_next(iter);
		pos->v.i = vector_size(compiler->buffer);
	}
	list_iterator_free(iter);
	list_free(jmps);
	return datatype_new(DATA_NULL);
}

// Eval.ifclause(node)
// Helper function for ifclause-evaluation.
// Evaluates a simple if-statement
datatype_t eval_ifclause(compiler_t* compiler, ast_t* node)
{
	// helper function for if
	compiler_eval(compiler, node->ifclause.cond);
	value_t* instr = emit_jmpf(compiler->buffer, 0);
	eval_block(compiler, node->ifclause.body);
	value_set_int(instr, vector_size(compiler->buffer));
	return datatype_new(DATA_NULL);
}

// Eval.while(node)
// Evaluates a while statement.
// Example:
// 01: push condition
// 02: jmpf 5
// 03: <instruction block>
// 04: jmp 2
datatype_t eval_while(compiler_t* compiler, ast_t* node)
{
	size_t start = vector_size(compiler->buffer);
	compiler_eval(compiler, node->whilestmt.cond);
	value_t* instr = emit_jmpf(compiler->buffer, 0);
	eval_block(compiler, node->ifclause.body);
	emit_jmp(compiler->buffer, start);
	value_set_int(instr, vector_size(compiler->buffer));
	return datatype_new(DATA_NULL);
}

// Eval.return(node)
// Simply compiles the return data and emits the bytecode
datatype_t eval_return(compiler_t* compiler, ast_t* node)
{
	compiler_eval(compiler, node->returnstmt);
	emit_return(compiler->buffer);
	return datatype_new(DATA_NULL);
}

// Eval.subscript(node)
// Compiles a subscript node.
// Currently only allowed for strings and arrays.
datatype_t eval_subscript(compiler_t* compiler, ast_t* node)
{
	ast_t* expr = node->subscript.expr;
	ast_t* key = node->subscript.key;
	datatype_t exprType = compiler_eval(compiler, expr);

	if(exprType.type == DATA_CLASS)
	{
		compiler_throw(compiler, node, "Field access of classes is not permitted");
		return datatype_new(DATA_NULL);
	}

	datatype_t keyType = compiler_eval(compiler, key);
	if(keyType.type != DATA_INT)
	{
		compiler_throw(compiler, node, "Key must be of type integer");
		return datatype_new(DATA_NULL);
	}

	if((exprType.type & DATA_ARRAY) == DATA_ARRAY)
	{
		// We got an array and want to access an element.
		// Remove the array flag to get the return type.
		emit_op(compiler->buffer, OP_GETSUB);
	 	datatype_t ret = datatype_new(exprType.type & ~DATA_ARRAY);
		ret.id = exprType.id;
		return ret;
	}
	else
	{
		compiler_throw(compiler, node, "Invalid subscript operation");
		return datatype_new(DATA_NULL);
	}

	return datatype_new(DATA_NULL);
}

// Eval.unary(node)
// Compiles prefix operators.
datatype_t eval_unary(compiler_t* compiler, ast_t* node)
{
	datatype_t type = compiler_eval(compiler, node->unary.expr);
	if(node->unary.op == TOKEN_ADD) return type;

	// ADD; SUB; NOT; BITNOT
	// OPCODES:
	// 1. F / I / B -> float / integer / boolean
	// 2. U -> unary

	switch(type.type)
	{
		case DATA_INT:
		{
			// SUB, BITNOT
			if(node->unary.op == TOKEN_NOT)
			{
				compiler_throw(compiler, node, "Logical negation can only be used with objects of type boolean");
			}
			else if(node->unary.op == TOKEN_SUB)
			{
				emit_op(compiler->buffer, OP_IMINUS);
			}
			else // Can only be Bitnot
			{
				emit_op(compiler->buffer, OP_BITNOT);
			}
			break;
		}
		case DATA_FLOAT:
		{
			// SUB

			if(node->unary.op == TOKEN_BITNOT || node->unary.op == TOKEN_NOT)
			{
				compiler_throw(compiler, node, "Bit operations / logical negation can only be used with objects of type int");
			}
			else // Can only be float negation
			{
				emit_op(compiler->buffer, OP_FMINUS);
			}
			break;
		}
		case DATA_BOOL:
		{
			if(node->unary.op != TOKEN_NOT)
			{
				compiler_throw(compiler, node, "Arithmetic operations can not be applied on objects of type boolean");
			}
			else
			{
				emit_op(compiler->buffer, OP_NOT);
			}
			break;
		}
		case DATA_CLASS:
		case DATA_CHAR:
		case DATA_VOID:
		case DATA_NULL:
		default:
		{
			compiler_throw(compiler, node, "Invalid unary instruction, only applicable to numbers or booleans");
			break;
		}
	}

	return type;
}

// Eval.class(node)
// Concept compilation of a class structure (WIP)
datatype_t eval_class(compiler_t* compiler, ast_t* node)
{
	// Approach:
	// Convert the constructor to a function that returns a 'class'-object value
	// Class declaration is the main function, instantiating the classes values and functions
	// functions and values are saved in class value + symbol

	// Get the class data
	char* name = node->classstmt.name;
	list_t* body = node->classstmt.body;

	// Generate the datatype with an id
	unsigned long id = djb2((unsigned char*)name);
	datatype_t dt;
	dt.type = DATA_CLASS;
	dt.id = id;

	// Emit a jump
	value_t* addr = emit_jmp(compiler->buffer, 0);

	// Register a symbol for the class, save the bytecode address
	size_t byte_address = vector_size(compiler->buffer);

	void* tmp = 0;
	if(hashmap_get(compiler->scope->classes, name, &tmp) != HMAP_MISSING)
	{
		compiler_throw(compiler, node, "Class already exists");
		return datatype_new(DATA_NULL);
	}
	symbol_t* symbol = symbol_new(compiler, node, byte_address, dt);
	hashmap_set(compiler->scope->classes, name, symbol);
	hashmap_set(compiler->scope->symbols, name, symbol);

	// Emit new class operator
	emit_op(compiler->buffer, OP_CLASS);

	// Create a new scope
	push_scope(compiler);

	// Treat each parameter as a local variable, with no type or value
	list_iterator_t* iter = list_iterator_create(node->classstmt.formals);
	int i = -(list_size(node->classstmt.formals) + 3);
	while(!list_iterator_end(iter))
	{
		// Create parameter in symbols list
		ast_t* param = list_iterator_next(iter);

		if(symbol_exists(compiler, param, param->vardecl.name)) return datatype_new(DATA_NULL);
		symbol_t* symbol = symbol_new(compiler, param, i, param->vardecl.type);
		hashmap_set(compiler->scope->symbols, param->vardecl.name, symbol);
		i++;
	}
	list_iterator_free(iter);

	// Iterate through body, emit the variables
	iter = list_iterator_create(body);
	while(!list_iterator_end(iter))
	{
		ast_t* sub = list_iterator_next(iter);
		if(sub->class == AST_DECLVAR)
		{
			size_t addr = compiler->scope->address;
			compiler_eval(compiler, sub);
			emit_load(compiler->buffer, addr, false);

			// Create new symbol and new addr to register in class
			symbol_t* sym = symbol_get(compiler->scope, sub->vardecl.name);
			sym->ref = symbol;

			hashmap_set(node->classstmt.fields, sub->vardecl.name, sym);
			emit_class_setfield(compiler->buffer, compiler->scope->address-1);
		}
		else if(sub->class == AST_DECLFUNC)
		{
			compiler_eval(compiler, sub);
			symbol_t* sym = symbol_get(compiler->scope, sub->funcdecl.name);
			hashmap_set(node->classstmt.fields, sub->funcdecl.name, sym);
		}
		else
		{
			compiler_throw(compiler, node, "Statements are not allowed as a direct field of a class");
			break;
		}
	}
	list_iterator_free(iter);
	pop_scope(compiler);

	// Return the class object
	emit_return(compiler->buffer);

	// Set the beggining byte address; end
	byte_address = vector_size(compiler->buffer);
	value_set_int(addr, byte_address);
	return datatype_new(DATA_NULL);
}

// Compiler.eval(node)
// Evaluates a node according to its class.
datatype_t compiler_eval(compiler_t* compiler, ast_t* node)
{
	if(compiler->error) return datatype_new(DATA_NULL);

#ifdef DB_EVAL
	console("Evaluating: %s\n", ast_classname(node->class));
#endif

	switch(node->class)
	{
		case AST_TOPLEVEL: return eval_block(compiler, node->toplevel);
		case AST_DECLVAR: return eval_declvar(compiler, node);
		case AST_DECLFUNC: return eval_declfunc(compiler, node);
		case AST_RETURN: return eval_return(compiler, node);
		case AST_FLOAT: return eval_number(compiler, node);
		case AST_INT: return eval_number(compiler, node);
		case AST_BOOL: return eval_bool(compiler, node);
		case AST_STRING: return eval_string(compiler, node);
		case AST_CHAR: return eval_char(compiler, node);
		case AST_ARRAY: return eval_array(compiler, node);
		case AST_BINARY: return eval_binary(compiler, node);
		case AST_IDENT: return eval_ident(compiler, node);
		case AST_CALL: return eval_call(compiler, node);
		case AST_IF: return eval_if(compiler, node);
		case AST_WHILE: return eval_while(compiler, node);
		case AST_UNARY: return eval_unary(compiler, node);
		case AST_SUBSCRIPT: return eval_subscript(compiler, node);
		case AST_CLASS: return eval_class(compiler, node);
		default: break;
	}

	return datatype_new(DATA_NULL);
}

// Compiler.dump(node)
// 'Dumps' the node to the console.
// Just a printing function for abstract syntax trees
void compiler_dump(ast_t* node, int level)
{
	if(!node) return;

	if(level > 0) console("  ");
	for(int i = 0; i < level; i++) console("#");

	switch(node->class)
	{
		case AST_TOPLEVEL:
		{
			list_iterator_t* iter = list_iterator_create(node->toplevel);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_DECLVAR:
		{
			fprintf(stdout, ":decl %s->%s<", node->vardecl.name, datatype2str(node->vardecl.type));
			compiler_dump(node->vardecl.initializer, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_DECLFUNC:
		{
			fprintf(stdout, ":func<%s::(", node->funcdecl.name);

			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
			while(!list_iterator_end(iter))
			{
				ast_t* param = list_iterator_next(iter);
				fprintf(stdout, "%s: %s->%lu", param->vardecl.name, datatype2str(param->vardecl.type), param->vardecl.type.id);

				if(!list_iterator_end(iter))
				{
					fprintf(stdout, ", ");
				}
			}
			fprintf(stdout, ") => ret: %s->%lu>\n", datatype2str(node->funcdecl.rettype),node->funcdecl.rettype.id);

			list_iterator_free(iter);
			iter = list_iterator_create(node->funcdecl.impl.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_IDENT:
		{
			fprintf(stdout, ":ident = %s", node->ident);
			break;
		}
		case AST_FLOAT:
		{
			fprintf(stdout, ":num = %f", node->f);
			break;
		}
		case AST_INT:
		{
			fprintf(stdout, ":num = %li", (long int)node->i);
			break;
		}
		case AST_STRING:
		{
			fprintf(stdout, ":str = '%s'", node->string);
			break;
		}
		case AST_CHAR:
		{
			fprintf(stdout, ":char = '%c'", node->ch);
			break;
		}
		case AST_BOOL:
		{
			fprintf(stdout, ":bool = '%s'", node->b == true ? "true" : "false");
			break;
		}
		case AST_BINARY:
		{
			fprintf(stdout, ":bin<");
			compiler_dump(node->binary.left, 0);
			compiler_dump(node->binary.right, 0);
			fprintf(stdout, ":op = %s>", tok2str(node->binary.op));
			break;
		}
		case AST_UNARY:
		{
			fprintf(stdout, ":unary<%s, ", tok2str(node->unary.op));
			compiler_dump(node->unary.expr, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_SUBSCRIPT:
		{
			fprintf(stdout, ":subscript<(key)");
			compiler_dump(node->subscript.key, 0);
			fprintf(stdout, "; (expr)");
			compiler_dump(node->subscript.expr, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_IF:
		{
			fprintf(stdout, ":if<>\n");
			list_iterator_t* iter = list_iterator_create(node->ifstmt);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_IFCLAUSE:
		{
			if(node->ifclause.cond != 0)
			{
				fprintf(stdout, ":ifclause<");
				compiler_dump(node->ifclause.cond, 0);
				fprintf(stdout, ">");
			}
			else
			{
				fprintf(stdout, ":else<>");
			}

			if(list_size(node->ifclause.body) > 0)
			{
				fprintf(stdout, "\n");
				list_iterator_t* iter = list_iterator_create(node->ifclause.body);
				while(!list_iterator_end(iter))
				{
					ast_t* next = list_iterator_next(iter);
					compiler_dump(next, level+1);
					fprintf(stdout, "\n");
				}
				list_iterator_free(iter);
			}
			break;
		}
		case AST_WHILE:
		{
			fprintf(stdout, ":while<");
			compiler_dump(node->whilestmt.cond, 0);
			fprintf(stdout, ">\n");

			list_iterator_t* iter = list_iterator_create(node->whilestmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_IMPORT:
		{
			fprintf(stdout, ":import<%s>", node->import);
			break;
		}
		case AST_ARRAY:
		{
			fprintf(stdout, ":array<");

			list_iterator_t* iter = list_iterator_create(node->array.elements);
			while(!list_iterator_end(iter))
			{
				compiler_dump(list_iterator_next(iter), 0);
			}
			list_iterator_free(iter);
			fprintf(stdout, ">");
			break;
		}
		case AST_CLASS:
		{
			fprintf(stdout, ":class<%s->%lu>\n", node->classstmt.name, djb2((unsigned char*)node->classstmt.name));

			list_iterator_t* iter = list_iterator_create(node->classstmt.formals);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);

			iter = list_iterator_create(node->classstmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_RETURN:
		{
			fprintf(stdout, ":return<");
			compiler_dump(node->returnstmt, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_CALL:
		{
			fprintf(stdout, ":call<");
			compiler_dump(node->call.callee, 0);
			fprintf(stdout, ">");
			break;
		}
		default: break;
	}
}

void symbols_track_usage(compiler_t* compiler, scope_t* scope)
{
	hashmap_iterator_t* iter = hashmap_iterator_create(scope->symbols);
	while(!hashmap_iterator_end(iter))
	{
		hashmap_iterator_next(iter);
	}
	hashmap_iterator_free(iter);

	// list_iterator_t* l_iter = list_iterator_create(scope->subscopes);
	// while(!list_iterator_end(l_iter))
	// {
	// 	scope_t* subscope = list_iterator_next(l_iter);
	// 	symbols_track_usage(compiler, subscope);
	// }
	// list_iterator_free(l_iter);
}

// Compiler.compileBuffer(string code)
// Compiles code into bytecode instructions
vector_t* compile_buffer(compiler_t* compiler, const char* source)
{
	// Reset compiler
	compiler_clear(compiler);
	compiler->buffer = vector_new();
	compiler->error = false;
	compiler->depth = 0;
	compiler->scope = scope_new();

	// Run the parser
	parser_init(&compiler->parser);
	ast_t* root = parser_run(&compiler->parser, source);
	if(root)
	{
#ifndef NO_AST
		console("Abstract syntax tree:\n");
		compiler_dump(root, 0);
#endif
		//symbols_track_usage(compiler, compiler->scope);

		compiler_eval(compiler, root);
		ast_free(root);
	}

	// Free filename and scope and parser
	if(compiler->filename) free(compiler->filename);
	parser_free(&compiler->parser);
	scope_free(compiler->scope);

	// Return bytecode if valid
	if(root && !compiler->error)
	{
		return compiler->buffer;
	}
	else
	{
		compiler_clear(compiler);
		return 0;
	}
}

// Compiler.compileFile(string filename)
// Compiles a file into an instruction set
vector_t* compile_file(compiler_t* compiler, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if(!file) return 0;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	char* source = (char*)malloc(size+1);
	fread(source, sizeof(char), size, file);
 	source[size] = '\0';
	fclose(file);

	compiler->filename = strdup(filename);

	// Run vm with compiler
	vector_t* buffer = compile_buffer(compiler, source);
	free(source);
	return buffer;
}

// Compiler.clear()
// Clears the current instruction buffer of the compiler
void compiler_clear(compiler_t* compiler)
{
	if(compiler->buffer)
	{
		for(size_t i = 0; i < vector_size(compiler->buffer); i++)
		{
			instruction_t* instr = vector_get(compiler->buffer, i);
			if(instr->v1) value_free(instr->v1);
			if(instr->v2) value_free(instr->v2);
			free(instr);
		}

		vector_free(compiler->buffer);
		compiler->buffer = 0;
	}
}
