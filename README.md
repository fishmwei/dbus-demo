# README


This is a demo for dbus-1

## FIRST

start a dbus-daemon with address unix:abstract=/dbus-session-address

```shell
dbus-daemon --session --print-address --fork --print-pid --address=unix:abstract=/dbus-session-address
```


## SECOND

compile cpp file

```shell
g++ server.cpp -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/include/dbus-1.0/ -ldbus-1 -lpthread -o server
```

## THRID

install  python packags dbus-next first

run dbus_test.py

```shell
python3 dbus_test.py signal
Has 2 param
Type is signal.
send test signal 10
```


 
