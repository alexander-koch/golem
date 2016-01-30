
Toggle = {}

function Toggle:activate ()
	self.state = not self.state
	return self.state
end

function Toggle:new(start_state)
	local o = {state = start_state}
	self.__index =self
	setmetatable(o, self)
	return o
end

function main ()
	local start = os.clock()
	local N = 100000

	local val = 1
  	local toggle = Toggle:new(val)
  	for i=1,N do
		val = toggle:activate()
	end

	print(val and "true" or "false")
	io.write(string.format("elapsed: %.8f\n", os.clock() - start))
end

main()
