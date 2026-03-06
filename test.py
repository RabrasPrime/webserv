#!/usr/bin/env python3
import sys

print("Content-Type: text/plain")
print("Status: 200 OK")
print("")

# On génère 1 Go de données par morceaux de 8 Ko
chunk_size = 8192
total_size = 200 * 200 * 200 # 1 Go
data = "A" * chunk_size

for _ in range(total_size // chunk_size):
    sys.stdout.write(data)

sys.stdout.flush()
