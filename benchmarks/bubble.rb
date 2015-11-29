class Array
  def bubblesort1!
    length.times do |j|
      for i in 1...(length - j)
        if self[i] < self[i - 1]
          self[i], self[i - 1] = self[i - 1], self[i]
        end
      end
    end
    self
  end
end

start = Time.now
ary = [5, 6, 1, 2, 9, 14, 2, 15, 6, 7, 8, 97]
ary.bubblesort1!
p ary
puts "elapsed: " + (Time.now - start).to_s
