#!/usr/bin/env python
# @author Alexander Koch 2016

import sys
import struct

# File format description (GVM)
#
# ---- (header)
# uint32_t [magic	] 0xACCE55 (Access)
# uint32_t [num_codes] number of instructions
# ---- (instruction)
# uint8_t  [opcode   ] operation
# uint8_t  [args	 ] argument count
# value*   [values   ] argument values
# ---- (value)
# uint8_t  [tag	  ] type tag
# void*	[data	 ] direct read as val_t, for objects special condition
# ----

# Opcode definition
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
	'syscall': (b'\x1E', b'\x01')
}

def writeValue(bytecode, arg):
	if arg.isdigit():
		bytecode += b'\x01'
		bytecode += struct.pack("<q", int(arg))

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
			if opcode[len(opcode)-1] == ',':
				opcode = opcode[0:-1]

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
