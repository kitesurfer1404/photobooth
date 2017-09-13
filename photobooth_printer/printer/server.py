#!/usr/bin/env python

import SimpleHTTPServer
import os
import subprocess

class CustomHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        SimpleHTTPServer.SimpleHTTPRequestHandler.end_headers(self)

    def do_GET(self):
        if "printimage" in self.path:
            filename=os.path.basename(self.path)
            print("PRINTING... " + filename)
            subprocess.Popen(['./print.sh', filename])
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

if __name__ == '__main__':
    SimpleHTTPServer.test(HandlerClass=CustomHTTPRequestHandler)
