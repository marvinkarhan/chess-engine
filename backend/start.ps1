Write-Output "Installing Flask"
pip install flask
Write-Output "Installing Flask-Socket-IO"
pip install flask-socketio
# Set the enivroment variables for the api server.
$env:FLASK_APP = "api.py"
$env:FLASK_ENV = "development"
# Start the flask instance.
python -m flask run