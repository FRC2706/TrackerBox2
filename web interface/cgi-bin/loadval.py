#!/usr/bin/python
import cgi, cgitb, yaml
cgitb.enable()
print "Content-Type: text/plain\n\n"

cgi_data = cgi.FieldStorage()

profile_to_load = str(cgi_data['a'].value)

values = ['minH', 'maxH', 'noiseFilterSize']

stream = open("/dev/shm/TrackerBox2_parameters.yaml", 'r')
data_file = yaml.load(stream)

string_to_print = ""

for i in range(0,2):
	 string_to_print += str(data_file["Profile"+profile_to_load][values[i]])
	 string_to_print += ","

string_to_print += str(data_file["ActiveProfileIdx"])
string_to_print += ','

print string_to_print