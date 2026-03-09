#!/usr/bin/env python3
import sys
import time

print("Content-Type: text/plain")
print("Status: 200 OK")
print("")


for i in range(100000): # 15Mo / 32000 ~ 480 itérations
    sys.stdout.write("A" * 500)
    sys.stdout.flush()
    time.sleep(0.01)

