#!/usr/bin/env python
# @author Alexander Koch 2016

import sys
import struct

# File format description (GVM)
#
# ---- (header)
# uint32_t [magic	 ] 0xACCE55 (Access)
# uint32_t [num_codes] number of instructions
# ---- (instruction)
# uint8_t  [opcode   ] operation
# uint8_t  [args	 ] argument count
# value*   [values   ] argument values
# ---- (value)
# uint8_t  [tag		 ] type tag
# void*	   [data	 ] direct read as val_t, for objects special condition
# ----

# Introduction:
# Label -> name, ':';
# Instruction -> opcode, {',', value}
# Program -> {Label | Instruction}
#
# Use call to invoke a function / label
# Also don't forget to write hlt at the end or your program.

# Opcode definition:
# Tuple: first=opcode, second=args
opcodes = {
	'hlt': (b'\x00', b'\x00'),
	'push': (b'\x01', b'\x01'),
	'pop': (b'\x02', b'\x00'),
	'store': (b'\x03', b'\x01'),
	'load': (b'\x04', b'\x01'),
	'gstore': (b'\x05', b'\x01'),
	'gload': (b'\x06', b'\x01'),
	'ldarg0': (b'\x07', b'\x00'),
	'setarg0': (b'\x08', b'\x00'),
	'iadd': (b'\x09', b'\x00'),
	'isub': (b'\x0A', b'\x00'),
	'imul': (b'\x0B', b'\x00'),
	'idiv': (b'\x0C', b'\x00'),
	'mod': (b'\x0D', b'\x00'),
	'bitl': (b'\x0E', b'\x00'),
	'bitr': (b'\x0F', b'\x00'),
	'bitand': (b'\x10', b'\x00'),
	'bitor': (b'\x11', b'\x00'),
	'bitxor': (b'\x12', b'\x00'),
	'bitnot': (b'\x13', b'\x00'),
	'iminus': (b'\x14', b'\x00'),
	'i2f': (b'\x15', b'\x00'),
	'fadd': (b'\x16', b'\x00'),
	'fsub': (b'\x17', b'\x00'),
	'fmul': (b'\x18', b'\x00'),
	'fdiv': (b'\x19', b'\x00'),
	'fminus': (b'\x1A', b'\x00'),
	'f2i': (b'\x1B', b'\x00'),
	'not': (b'\x1C', b'\x00'),
	'b2i': (b'\x1D', b'\x00'),
	'syscall': (b'\x1E', b'\x01'),
	'invoke': (b'\x1F', b'\x02'),
	'reserve': (b'\x20', b'\x01'),
	'ret': (b'\x21', b'\x00'),
	'retvirtual': (b'\x22', b'\x00'),
	'jmp': (b'\x23', b'\x01')
}

def check_int(s):
	if s[0] in ('-', '+'):
		return s[1:].isdigit()
	return s.isdigit()

def writeValue(bytecode, arg):
	if check_int(arg):
		bytecode += b'\x01'
		bytecode += struct.pack("<i", int(arg))
		bytecode += struct.pack("<i", 0)
	elif arg == "true" or arg == "false":
		bytecode += b'\x02'
		bval = (arg == "true")
		# TODO: convert to 64-bit NaN-boolean value
	else:
		bytecode += b'\x03'
		# TODO: append string data

	return bytecode

def main():
	# Create the initial array
	bytecode = bytearray()
	bytecode_count = 0

	# Write the header
	bytecode += b'\x55\xCE\xAC\x00'
	bytecode += b'\x00\x00\x00\x00'

	# Initial jump
	bytecode += b'\x24'
	bytecode += b'\x01'
	bytecode += b'\x01'
	bytecode += struct.pack("<q", 0)
	bytecode_count = bytecode_count + 1

	labels = {}

	# Begin decoding the file
	with open(sys.argv[1],'r') as f:
		for linenum, line in enumerate(f):
			# Remove comments
			if line.find(";") != -1:
				line = line[:line.find(";")]

			line = line.strip()
			if line == "":
				continue

			# Split into tokens
			tokens = line.split(" ",1)
			args = []
			if len(tokens) > 1:
				args = tokens[1].split(",")

			# Get the opcode
			opcode = tokens[0]

			# Remove trailing comma
			if opcode[-1:] == ',':
				opcode = opcode[:-1]

			# Test for labels
			elif opcode[-1:] == ':':
				# Get the label name
				label = opcode[:-1]
				labels[label] = bytecode_count
				continue

			# Test for call
			if opcode == "call":
				label = args[0]
				if label in labels:
					# invoke
					bytecode += b'\x1F'
					bytecode += b'\x02'

					# Arg1
					bytecode += b'\x01'
					bytecode += struct.pack("<q", labels[label])
					bytecode_count = bytecode_count + 1

					# Arg2
					argc = int(args[1])
					bytecode += b'\x01'
					bytecode += struct.pack("<q", argc)
					bytecode_count = bytecode_count + 1
				continue

			# Test for jump
			if opcode == "jmp":
				label = args[0]
				if label in labels:
					# jump
					bytecode += b'\x24'
					bytecode += b'\x01'

					# Arg1
					bytecode += b'\x01'
					bytecode += struct.pack("<q", labels[label])
					bytecode_count = bytecode_count + 1
				continue

			# Write one instruction
			if opcode in opcodes:
				ituple = opcodes[opcode]
				bytecode += ituple[0]
				bytecode += ituple[1]
				if ituple[1] == b'\x01':
					bytecode = writeValue(bytecode, args[0])
				elif ituple[1] == b'\x02':
					bytecode = writeValue(bytecode, args[0])
					bytecode = writeValue(bytecode, args[1])

				bytecode_count = bytecode_count + 1
			else:
				print("Opcode: "+opcode+" does not exist.")

	print(labels)

	# Testing for main
	if "main" in labels:
		# Set the initial jump
		bytecode[11:11+8] = struct.pack("<q", labels["main"])
	else:
		print("Must provide a main label")
		sys.exit()

	# Set the number of bytecodes and write
	bytecode[4:8] = struct.pack("<i", bytecode_count)
	with open("out.gvm", 'wb') as f:
		f.write(bytecode)

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print("Must provide an input path.")
		sys.exit()
	else:
		main()
