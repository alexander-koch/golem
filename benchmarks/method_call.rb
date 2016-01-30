
class Toggle
	def initialize(state)
		@bool = state
	end

	def activate()
		@bool = !@bool
		return @bool
	end
end

def main()
	start = Time.now
	val = 1
	n = 100000
	toggle = Toggle.new(val)

	n.times do
		val = toggle.activate()
	end

	if val then puts "true" else puts "false" end

	puts "elapsed: " + (Time.now - start).to_s
end

main()
