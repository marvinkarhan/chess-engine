#!/bin/bash
echo "Installing Flask"
pip install flask
echo "Installing Flask-Socket-IO"
pip install flask-socketio
echo "Installing Eventlet"
pip install eventlet
echo "Installing Gunicorn"
pip install gunicorn

# run
gunicorn -k eventlet -w 1 --reload app:app