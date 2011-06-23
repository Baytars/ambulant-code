# Simple echo-server that waits a predefined number of seconds before replying.

import sys
import SocketServer
import time

# Tunable constants
PORT=12345
DELAY=5
QUEUESIZE=20

class MyTCPHandler(SocketServer.BaseRequestHandler):

    def handle(self):
        data = self.request.recv(1024)
        print 'Request:', data
        time.sleep(DELAY)
        self.request.send(data)
        
class MyTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    allow_reuse_adddress=True
    request_queue_size=QUEUESIZE
    pass
        
def main():
    server = MyTCPServer(("localhost", PORT), MyTCPHandler)
    server.serve_forever()
        
if __name__ == "__main__":
    main()
    