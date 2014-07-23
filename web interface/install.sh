#!/bin/bash

sudo rm /var/www/html/*
sudo cp *.html *.css *.js *.jpg /var/www/html/
sudo chmod a+rx /var/www/html/*
