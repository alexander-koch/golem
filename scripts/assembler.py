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

			bytecode_count = bytecode_count + 1

			# Format:
			# Opcode
			# Args
			if opcode == "hlt":
				bytecode += b'\x00'
				bytecode += b'\x00'

			elif opcode == "push":
				bytecode += b'\x01'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			elif opcode == "pop":
				bytecode += b'\x02'
				bytecode += b'\x00'

			elif opcode == "store":
				bytecode += b'\x03'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			elif opcode == "load":
				bytecode += b'\x04'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			elif opcode == "gstore":
				bytecode += b'\x05'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			elif opcode == "gload":
				bytecode += b'\x06'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			elif opcode == "ldarg0":
				bytecode += b'\x07'
				bytecode += b'\x00'

			elif opcode == "setarg0":
				bytecode += b'\x08'
				bytecode += b'\x00'

			elif opcode == "iadd":
				bytecode += b'\x09'
				bytecode += b'\x00'

			elif opcode == "isub":
				bytecode += b'\x0a'
				bytecode += b'\x00'

			elif opcode == "imul":
				bytecode += b'\x0b'
				bytecode += b'\x00'

			elif opcode == "syscall":
				bytecode += b'\x1E'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			else:
				print("Opcode: "+opcode+" is not defined")

			print(tokens)
			print(args)

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
