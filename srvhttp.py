from http.server import HTTPServer, BaseHTTPRequestHandler
import socket
import random


program_mode = 1


def create_response_rand():
    x = random.randint(1, 5)
    print(x)
    response = str(x).encode()
    return response


def create_response():
    response = str(program_mode).encode()
    return response


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    # def __init__(self):
    #     program_mode = 1
    def do_POST(self):
        global program_mode
        print('post')
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        print(body)
        self.send_response(200)
        number = int(body[7])-48
        print(number)
        program_mode = number
        print('blink mode : {}'.format(program_mode))
        return

    def do_GET(self):
        print('get')
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        print(body)
        content = create_response()
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.send_header('Content-Length', len(content))
        self.end_headers()
        self.wfile.write(content)
        return

    def do_PUT(self):
        print('put')
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        print(body)
        content = create_response()
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.send_header('Content-Length', len(content))
        self.end_headers()
        self.wfile.write(content)
        return


print('test')
httpd = HTTPServer(('', 8000), SimpleHTTPRequestHandler)
print("Your IP address is: ", socket.gethostbyname(socket.gethostname()))
httpd.serve_forever()
