# Simple threaded echo-server client
import sys
import socket
import time
import thread

PORT=12345
HOST="localhost"
DATA_COUNT=1
THREAD_COUNT=10

worker_count=0

def worker(num):
    global worker_count
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    for i in range(DATA_COUNT):
        data = "Hello World %d %d\r\n" % (num, i)
        s.send(data)
        d = s.recv(1024)
        assert d == data
    s.close()
    worker_count -= 1
    
def main():
    global worker_count
    for i in range(THREAD_COUNT):
        worker_count += 1
        thread.start_new_thread(worker, (worker_count,))
    while worker_count > 0:
        time.sleep(0.1)
    
if __name__ == "__main__":
    main()
    