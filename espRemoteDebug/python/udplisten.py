#!/usr/local/bin/python3
# -----------------------------------------------------------------------------
# Listen for UDP broadcast messages and print them to console as is.
# -----------------------------------------------------------------------------
# ihsan Kehribar - 2016
# -----------------------------------------------------------------------------
import sys
import socket
import select

if(len(sys.argv) < 2):
	print("Provide the port number as argument!")
	sys.exit()

print("Started listening at port " + sys.argv[1])

port = int(sys.argv[1])
bufferSize = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("", port))
s.setblocking(0)

while True:
    result = select.select([s],[],[])
    msg = result[0][0].recv(bufferSize)
    print(msg.decode('utf-8'),)
