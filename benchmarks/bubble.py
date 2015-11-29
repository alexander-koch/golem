import time

def shortBubbleSort(alist):
    exchanges = True
    passnum = len(alist)-1
    while passnum > 0 and exchanges:
       exchanges = False
       for i in range(passnum):
           if alist[i]>alist[i+1]:
               exchanges = True
               temp = alist[i]
               alist[i] = alist[i+1]
               alist[i+1] = temp
       passnum = passnum-1

alist = [5, 6, 1, 2, 9, 14, 2, 15, 6, 7, 8, 97]
start = time.time()
shortBubbleSort(alist)
print(alist)
end = time.time()
elapsed = end - start
print("Elapsed time %.5f" % (elapsed))
