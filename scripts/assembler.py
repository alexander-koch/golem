#!/usr/bin/env python
import sys
import struct

def writeValue(bytecode, arg):
	if arg.isdigit():
		bytecode += b'\x01'
		bytecode += struct.pack("<q", int(arg))

	elif arg == "true" or arg == "false":
		bytecode += b'\x02'
		# TODO: append
	else:
		bytecode += b'\x03'
		# TODO: append

	return bytecode

def main():
	print("Opening file: "+sys.argv[1])

	bytecode = bytearray()
	bytecode_count = 0
	bytecode += b'\x55\xCE\xAC\x00'
	bytecode += b'\x00\x00\x00\x00'

	with open(sys.argv[1],'r') as f:
		for linenum, line in enumerate(f):
			# Remove comments
			if line.find(";") != -1:
				line = line[:line.find(";")]

			line = line.strip()
			if line == "":
				continue

			tokens = line.split(" ",1)
			args = []
			if len(tokens) > 1:
				args = tokens[1].split(",")

			bytecode_count = bytecode_count + 1

			# Get the opcode
			opcode = tokens[0]
			if opcode[len(opcode)-1] == ',':
				opcode = opcode[0:-1]

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

			elif opcode == "syscall":
				bytecode += b'\x1E'
				bytecode += b'\x01'
				bytecode = writeValue(bytecode, args[0])

			else:
				print("Opcode: "+opcode+" is not defined")

			print(tokens)
			print(args)

	bytecode[4:8] = struct.pack("<i", bytecode_count)
	with open("out.gvm", 'wb') as f:
		f.write(bytecode)

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print("Must provide an input path.")
		sys.exit()
	else:
		main()
