import socketserver
import struct
import os
import sys

class MyTCPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        while True:
            len = struct.unpack("I", self.request.recv(4))[0]
            data = self.request.recv(len)

            if data == b"q":
                sys.exit()

            print(data.decode("utf8"))

if __name__ == "__main__":
    HOST, PORT = "0.0.0.0", 1337

    os.startfile(r".\build\bin\cpp\app\codeinjector.exe")

    with socketserver.TCPServer((HOST, PORT), MyTCPHandler) as server:
        server.serve_forever()
