import os
import threading
import subprocess

def compress_file(dec_file):
    compressed_file = '..\\fs\\' + dec_file[:-4]
    subprocess.call(['sly_compressor.exe', dec_file, compressed_file])

dec_files = [f for f in os.listdir('.') if f.endswith('.dec')]

threads = []
for dec_file in dec_files:
    thread = threading.Thread(target=compress_file, args=(dec_file,))
    threads.append(thread)
    thread.start()

for thread in threads:
    thread.join()
