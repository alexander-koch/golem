-- bubblesort algorithm

function bubbleSort(list)
    local len = #list
    local hasChanged = false
    repeat
        hasChanged = false
        len = len - 1
        for i = 1, len do
            if list[i] > list[i + 1] then
                list[i], list[i + 1] = list[i + 1], list[i]
                hasChanged = true
            end
        end
    until hasChanged == false
end

local start = os.clock()
local list = { 5, 6, 1, 2, 9, 14, 2, 15, 6, 7, 8, 97 }
bubbleSort(list)
for i, j in pairs(list) do
    print(j)
end
io.write(string.format("Elapsed: %.5f\n", os.clock() - start))
