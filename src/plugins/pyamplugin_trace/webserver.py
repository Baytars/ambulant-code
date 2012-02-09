from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import os
import sys
import mimetypes
import posixpath
import thread
import urlparse

PORT=8842
DEBUG=True

try:
    modfile = __file__
except NameError:
    dirname = "."
else:
    dirname = os.path.dirname(modfile)

class WebServer(HTTPServer):
    def __init__(self):
        self.tracer = None
        HTTPServer.__init__(self, ('', PORT), MyHandler)
    
    def setTracer(self, tracer):
        print 'TRACER SET', tracer
        self.tracer = tracer
        
    def _still_running(self):
        return True
        
    def _run(self):
        while self._still_running():
            self.handle_request()
            
    def start(self):
        thread.start_new_thread(self._run, ())
        url = "http://localhost:%d/visualize.html" % PORT
        os.system("open %s" % url)
        
class MyHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        _, _, path, _, query, _ = urlparse.urlparse(self.path)
        if path[-1] == '/': path += 'index.html'
        print path
        data = None
        if path == '/data.json':
            print 'TRACER IS NOW', self.server.tracer
            if self.server.tracer:
                data = self.server.tracer.dump_json()
                mimetype = "application/json"
        else:
            try:
                filename = posixpath.split(path)[1]
                mimetype = mimetypes.guess_type(filename)[0]
                filename = os.path.join(dirname, filename)
                fp = open(filename)
                data = fp.read()
                if DEBUG: print 'webserver: read %d bytes' % len(data)
            except IOError:
                pass
        if data is None:
            self.send_error(404, "No data available")
        else:
            self.send_response(200)
            self.send_header('Content-type', mimetype)
            self.send_header('Cache-Control', 'no-cache')
            self.end_headers()
            self.wfile.write(data)
