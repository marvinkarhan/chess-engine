#!/bin/bash
echo "Installing Flask"
pip3 install flask
echo "Installing Flask-Socket-IO"
pip3 install flask-socketio
echo "Installing Pexpect"
pip3 install pexpect
echo "Installing Eventlet"
pip3 install eventlet==0.30.2
echo "Installing Gunicorn"
pip3 install gunicorn

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# run
python3 -m gunicorn -k eventlet -w 1 --chdir $SCRIPT_DIR --reload app:app