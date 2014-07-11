#!/usr/bin/python
import cgi, cgitb, dbus
cgitb.enable()
print "Content-Type: text/html\n\n"

cgi_data = cgi.FieldStorage()

profile = cgi_data['a'].value
category = cgi_data['b'].value
value = cgi_data['c'].value

s = str(profile)+','+str(category)+','+str(value)

bus = dbus.SystemBus()
helloservice = bus.get_object('com.cyberfalcons.trackerbox', '/com/cyberfalcons/trackerbox')
hello = helloservice.get_dbus_method('hello', 'com.cyberfalcons.trackerbox')
hello(s)
