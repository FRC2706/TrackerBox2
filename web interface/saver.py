#!/usr/bin/python

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
import gtk
import yaml
import collections
import os

_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG

def dict_representer(dumper, data):
    return dumper.represent_mapping(_mapping_tag, data.iteritems())

def dict_constructor(loader, node):
    return collections.OrderedDict(loader.construct_pairs(node))

yaml.add_representer(collections.OrderedDict, dict_representer)
yaml.add_constructor(_mapping_tag, dict_constructor)

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
        outfile.write( yaml.dump(data_file, default_flow_style=False) )
        # Line ending because vim was angry
        outfile.write("\n")

    # Make the file satanic
    os.chmod('/dev/shm/TrackerBox2_parameters.yaml', 0666)


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