#!/usr/bin/env python

import SimpleHTTPServer

class CustomHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        SimpleHTTPServer.SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
    SimpleHTTPServer.test(HandlerClass=CustomHTTPRequestHandler)
