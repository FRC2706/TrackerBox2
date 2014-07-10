#!/usr/bin/python

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
import gtk
import yaml

def save(da):
    message = da.split(',')

    values = ['minH', 'maxH', 'noiseFilterSize']

    stream = open("/dev/shm/TrackerBox2_parameters.yaml", 'r')
    data_file = yaml.load(stream)

    if (message[1]!=3):
        data_file["Profile"+message[0]][values[int(message[1])]] = int(message[2])
    else:
        data_file["ActiveProfileIdx"] = int(message[2])

    with open('/dev/shm/TrackerBox2_parameters.yaml', 'w') as outfile:
        outfile.write( yaml.dump(data_file, default_flow_style=True) )

class MyDBUSService(dbus.service.Object):
    def __init__(self):
        bus_name = dbus.service.BusName('com.cyberfalcons.trackerbox', bus=dbus.SystemBus())
        dbus.service.Object.__init__(self, bus_name, '/com/cyberfalcons/trackerbox')

    @dbus.service.method('com.cyberfalcons.trackerbox')
    def hello(self, s):
    	save(s)
        return "Hail Gaben!"

    @dbus.service.method('com.cyberfalcons.trackerbox')
    def Quit(self):
        """removes this object from the DBUS connection and exits"""
        self.remove_from_connection()
        gtk.main_quit()
        return

DBusGMainLoop(set_as_default=True)
myservice = MyDBUSService()
gtk.main()