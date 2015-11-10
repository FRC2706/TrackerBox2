# TODO list going into the 2016 season with Team 2706.

## Web interface tasks [Mike + Matt + Kevin + Others]:


- Replace the "Profile #" slider with a dropdown list

- When changing the Pi's IP address, apache will need sudo power (or at least sudoers priviledge on `ifconfig` and `reboot`)

- On the web page, have a button "Settings" which takes you to a page with "Team Number", and IP addresses for "RPi IP", "AxisCam IP", and "RoboRio IP" (all defaulting to 10.TE.AM.___ but completely editable)

- Have a "Downscale" dropdown menu on the web interface?

- add fps on the interface




## C++ code tasks [Mike / Kevin, Matt / Morgan / Others]:


- clean up code (open-source quality commenting, etc) and remove old or unused files.

- update code to use the libyaml0.5 api, if they take libyaml0.3 off the ubuntu repos, we're in trouble.

- Update all the opencv data structures and function calls to the c++ api. (currently it's a mashup of the c and c++ api's)

- Optimize frame-rate by reducing image resolution?

- hard-code the priority: 1) USB Cam, 2) Axis Cam, 3) Laptop Cam

- Add (smoothed) fps to the particleReport


## With cRIO (to let more teams use it):
- test and update Java wrapper
- write & test c++ wrapper
- write & test LabView wrapper

Notes for Pi image setup:
-needs permissions for cgi-bin scripts
-default user needs permission to own services in dbus SystemBus
-apache user needs permission to run methods over SystemBus
