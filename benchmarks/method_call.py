#!/usr/bin/python
from __future__ import print_function

import sys
import time

class Toggle(object):
    def __init__(self, start_state):
        self.bool = start_state
    def activate(self):
        self.bool = not self.bool
        return self.bool

def main():
    start = time.clock()

    NUM = 100000

    val = True
    toggle = Toggle(val)
    for i in range(0,NUM):
        val = toggle.activate()
    if val:
        print("true")
    else:
        print("false")

    print("elapsed: " + str(time.clock() - start))


main()
