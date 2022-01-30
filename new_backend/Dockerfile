FROM gcc:9.4.0-buster as builder
RUN apt-get update && apt-get -y upgrade && apt install -y cmake
COPY ./ /backend/
WORKDIR /backend/
RUN rm -fr build
RUN mkdir build && cd build && cmake .. -G "Unix Makefiles" && make

FROM python:3.9.10
RUN pip install gunicorn eventlet==0.30.2 pexpect flask-socketio flask
COPY --from=builder /backend/build/chess-server-uci-exe /app/chess-server-uci
COPY ./src/flask/app.py /app/app.py
EXPOSE 8000
WORKDIR /app/
CMD ["python", "-m", "gunicorn", "-k", "eventlet", "-w", "1", "--reload", "-b", "0.0.0.0:8000", "app:app"]