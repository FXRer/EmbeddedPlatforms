#!/bin/bash
#nohup x11vnc -forever -usepw -create &
nohup x11vnc -forever -create &
cd uracoli
nohup python -m SimpleHTTPServer &
