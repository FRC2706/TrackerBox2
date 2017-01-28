# TODO list going into the 2017 season with Team 2706.

## Web interface tasks [Mike + Matt + Kevin + Others]:


- Replace the "Profile #" slider with a dropdown list

- When changing the Pi's IP address, apache will need sudo power (or at least sudoers priviledge on `ifconfig` and `reboot`)

- On the web page, have a button "Settings" which takes you to a page with "Team Number", and IP addresses for "RPi IP", "AxisCam IP", and "RoboRio IP" (all defaulting to 10.TE.AM.___ but completely editable)

- Have a "Downscale" dropdown menu on the web interface?

- add fps on the interface




## C++ code tasks [Mike / Kevin, Matt / Morgan / Others]:



## With cRIO (to let more teams use it):
- write & test c++ wrapper
- write & test LabView wrapper

Notes for Pi image setup:
-needs permissions for cgi-bin scripts
-default user needs permission to own services in dbus SystemBus
-apache user needs permission to run methods over SystemBus

## Stuff That's Done:

- [Done] update code to use the libyaml0.5 api, if they take libyaml0.3 off the ubuntu repos, we're in trouble.
