#!/usr/bin/python
import cgi, cgitb, zmq
cgitb.enable()
print "Content-Type: text/html\n\n"

cgi_data = cgi.FieldStorage()

profile = cgi_data['a'].value
minh = cgi_data['b'].value
maxh = cgi_data['c'].value
smoothpass = cgi_data['d'].value

s = str(profile)+','+str(minh)+','+str(maxh)+','+str(smoothpass)

context = zmq.Context()
socket = context.socket(zmq.REQ)

socket.connect("tcp://localhost:5555")

socket.send(s)
