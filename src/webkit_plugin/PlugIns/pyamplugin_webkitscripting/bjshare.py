import pybonjour
import random
import urlparse
import thread
import SocketServer

BONJOUR_REGTYPE="_syncstate._tcp"

class Sync_API:
    def __init__(self):
        print "Sync_object()->0x%x"%id(self)
        
    def __del__(self):
        print "Sync_object(0x%x).__del__()" % id(self)
        
    def transaction(self):
        pass
        
    def commit(self, data):
        print 'State not shared:', data
        
class Sync_server_Handler(SocketServer.BaseRequestHandler):
    def handle(self):
        self.data = self.request.recv(1024).strip()
        print 'Sync_server_Handler: got request', self.data

class Sync_server(Sync_API):
    def __init__(self, (sdRef, fullname, port)):
        self.sdRef = sdRef
        self.port = port
        thread.start_new_thread(self.run, ())
        
    def run(self):
        print 'Sync_server: thread started'
                
        self.server = SocketServer.TCPServer(("localhost", self.port), Sync_server_Handler)
        self.server.serve_forever()
    
class Sync_client(Sync_API):
    def __init__(self, (sdRef, host, port, txtRecord)):
        self.sdRef = sdRef
        self.port = port
        self.sock = socket.create_connection((host, port), 5)
        
    def transaction(self):
        self.sock.write('<transaction>\r\n')
        
    def commit(self, data):
        self.sock.write('<commit>\r\n')
    
def start_sync_client(domain, name):
    resolve_info = [0, '', 0, '']
    def resolve_callback(sdRef, flags, interfaceIndex, errorCode, fullname,
                     hosttarget, port, txtRecord):
        if errorCode == pybonjour.kDNSServiceErr_NoError:
            print 'Found service:'
            print '  fullname    =', fullname
            print '  hosttarget  =', hosttarget
            print '  port        =', port
            print '  txtRecord   =', txtRecord
            resolve_info[1] = hosttarget
            resolve_info[2] = port
            resolve_info[3] = txtRecord
        else:
            print 'DNSServiceRegister: error=', errorCode
    if not domain:
        domain = None
    try:
        sdRef = pybonjour.DNSServiceResolve(0, 0, name, BONJOUR_REGTYPE, "local.", resolve_callback)
        pybonjour.DNSServiceProcessResult(sdRef)
    except pybonjour.BonjourError:
        print 'DNSServiceResolve failed.'
        return None
    if resolve_info[2]:
        resolve_info[0] = sdRef
        return tuple(resolve_info)
    return None
    
def start_sync_server(domain, name):
    resolve_info = [0, name, 0]
    def register_callback(sdRef, flags, errorCode, realName, regtype, domain):
        if errorCode == pybonjour.kDNSServiceErr_NoError:
            print 'Registered service:'
            print '  name    =', realName
            print '  regtype =', regtype
            print '  domain  =', domain
            print '  port    =', port
            if name != realName:
                # If we got a different name someone else is already serving this.
                # Abort, so we can go to client mode.
                print '  ** Got different name than requested', name
                resolve_info[2] = 0
        else:
            print 'DNSServiceRegister: error=', errorCode
            resolve_info[2] = 0
    if not domain:
        domain = None
    port = random.randrange(10000, 20000)
    resolve_info[2] = port
    sdRef = pybonjour.DNSServiceRegister(name = name,
                 regtype = BONJOUR_REGTYPE,
                 port = port,
                 domain = domain,
                 callBack = register_callback)
    pybonjour.DNSServiceProcessResult(sdRef)
    if resolve_info[2]:
        resolve_info[0] = sdRef
        return tuple(resolve_info)
    return None
    
def start_sync(url):
    if url:
        scheme, netloc, path, query, fragment = urlparse.urlsplit(url)
        while path[0] == '/':
            path = path[1:]
        conndata = start_sync_server(netloc, path)
        if conndata:
            return Sync_server(conndata)
        conndata = start_sync_client(netloc, path)
        if conndata:
            return Sync_client(conndata)
    print '** Neither Bonjour server nor client could be initialized. State not shared.'
    return Sync_API()