# Trackerbox 2 Raspberry Pi Setup Guide

## Preamble
Welcome to Team 2706’s TrackerBox2 Setup guide on Raspberry Pi! 

A Demo video, and video version of this guide will soon be available on YouTube. We will be showing you how to install and configure trackerbox in three different camera modes: using an image from the hard drive (which is useful for testing), with an ethernet camera (this is the mode we use at 2706), and with a USB camera (this mode also works for a linux laptop with a built-in webcam).

## First-time setup

### Installing Libraries

First thing, make sure your Raspberry Pi has internet access because we are going to install some system libraries that we will need. In the terminal type 
    
    $ sudo apt-get install libyaml-cpp-dev libboost-dev libopencv-dev

You should see the libraries get successfully installed.

### Cloning the source code

Next, download the newest version of the Trackerbox 2 source code using git.

    $ git clone  https://github.com/FRC2706/TrackerBox2
    
and that should pull down the source code.

### Building the source code

You should now be able to build and run trackerbox2 by using `cd` to change directory to the folder `trackerbox2/network\ tracker`. 

    $ cd trackerbox2/network\ tracker
    
now build the code by typing

    $ make
    
If the build failed, you probably need to go back to the library installing step.

### Running trackerbox2

Now run it.

    $ ./trackerbox2

We can see that it’s running. On my ubuntu laptop this pops up a couple of images which you will probably not see on a Pi because of graphics problems, but that’s OK, the tracker is still running.

### Setting your Pi to a static IP

In order for your roboRIO to talk to your Raspberry Pi, you have to set your Pi to a static IP.

Make a backup of the important file in case you make a mistake.

    $ sudo cp /etc/network/interfaces /etc/network/interfaces.bkup
    
Open `/etc/network/interfaces` in your favorite editor, we will be using 

    $ sudo nano
    
In the file, find the line with: iface eth0 inet dhcp
edit the line so it says: iface eth0 inet static
Add these lines under the text
    address 10.27.6.231 (the address you want,10.XX.XX.Y where XXXX is your team number)
    netmask 255.0.0.0
  5. Save the document
  6. Reboot
  7. Check that you have the correct IP by typing `$ ifconfig` (note that this will not show anything if there is no ethernet cable attached).

### Setting Trackerbox2 to run at boot.

Finally, the last piece of setup before competition is to set the trackerbox2 program to run at boot. There are several ways to do this, and plenty of guides online. The method we used was to add it to the global profile `/etc/profile` because this way the console output shows up on the screen, and we can hook up a monitor and see what it's doing without needing to type anything.

Open `/etc/profile`

    $ sudo nano /etc/profile
  
add the following lines anywhere in the file:

    # run trackerbox2 at startup
    # in case it crashes, let's just call it again!
    while true
    do
	    (cd /home/pi/trackerbox2/network\ tracker; ./trackerbox2)
    done


## Using Trackerbox 2 with a picture from disk (mode 0)

1. navigate to the trackerbox2 code folder 

    $ cd trackerbox2/network\ tracker

2. Open the file called “networkTracker.cpp”

    $ nano networkTracker.cpp

There are several #define lines that are important:

    #define SHOW_GUI

turns off the GUI output of trackerbox which saves you some frame rates.

    #define PRINT_FPS
    
controls whether or not it prints output at each frame.

    #define CAMERA_TYPE 
    
is the important one. We will leave it on 0 for reading an image from disk, and we’ll get to the other two in a minute.

5. The picture we process is “camera.jpg” in the same folder. We have a silly robot as an example in the github repo, but replace this with the photo you want to process.

7. Since we modified a code file, type “make” first then wait for it to finish then type “./trackerbox2” into command line and click enter

8. The Output should look somewhat like this 

    Listening on port 1182 for requests to transmit data.<br>
    Waiting for new connection.<br>
    FPS: 11.0706<br>
    <br>
    Found 1 targets. 

## Using Trackerbox 2 with an Ethernet or USB Camera

1. navigate to the trackerbox2 code folder, if you are not already there

    $ cd trackerbox2/network\ tracker

2. Open the file called “networkTracker.cpp”

    $ nano networkTracker.cpp

3. find `#define CAMERA_TYPE` and enter either 1 or 2 depending on the type of camera you want to use.

If you are using an IP camera, open the file `TrackerBox2_parameters.yaml`

    $ nano TrackerBox2_parameters.yaml

find the line that says `ipCamAddr` and set it to the url of the image stream you want to fetch.

Since we modified a code file, type `make` first then wait for it to finish then type `./trackerbox2` into command line and hit enter.





## Changing Parameters of the tracker

Open the parameters file “TrackerBox2_parameters.yaml”
These are the parameters that affect the vision tracking system. Note that we are using and HSV threshold tracker. You can set a minimum and maximum value for each. Hue is the rainbow colour that you are searching for, while saturation and value are the richness and brightness of the colour. minTargetArea is a noise filtering technique to ignore any targets that are smaller than a certain size (in pixels). Depending on how clean you can get your mask, you may be able to lower this to track a target from farther away. erodeDilateSize is another noise smoother to reduce small pixely noise, and to fill in holes in your target.


## Viewing output when GUI is turned off

Because Raspbian doesn't support OpenGL (yet), trackerbox2's GUI doesn't work, making it difficult to train a tracker without seeing your images. For this reason, we save a copy of the output images at each frame to the system ram disk, `/dev/shm`. We are saving to ram disk rather than the SD card, because writing jpgs at 30 frames per second will wear out the SD card pretty quickly! In the Raspberry Pi’s GUI mode, go to the folder `/dev/shm`, and you should see `output.jpg` and `mask.jpg`.





We hope you found TrackerBox2 useful, and we hope to see you at the competition!
