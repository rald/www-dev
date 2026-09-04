#!/usr/bin/env python3
import os
import fcntl

COUNTER_FILE = "hits.txt"

# Open (and create if missing) the counter file safely
# Using 'a+' mode ensures we can both read and write
with open(COUNTER_FILE, "a+") as f:
    # Acquire an exclusive lock to prevent race conditions under concurrent requests
    fcntl.flock(f, fcntl.LOCK_EX)
    
    # Move pointer to the beginning to read the current count
    f.seek(0)
    content = f.read().strip()
    
    try:
        count = int(content) if content else 0
    except ValueError:
        count = 0
        
    # Increment count
    count += 1
    
    # Truncate file and write the new count atomically while still locked
    f.seek(0)
    f.truncate()
    f.write(str(count))
    
    # Lock is automatically released when the file is closed

# Output HTTP header and the raw count for the JavaScript fetch call
print("Content-Type: text/plain\n")
print(count)
