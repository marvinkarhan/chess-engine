Write-Output "Installing Flask"
pip install flask
Write-Output "Installing Flask-Socket-IO"
pip install flask-socketio
# Set the enivroment variables for the api server.
$env:FLASK_APP = "api.py"
# Start the flask instance.
flask run