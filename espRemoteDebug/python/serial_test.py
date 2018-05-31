# -----------------------------------------------------------------------------
#
#
# -----------------------------------------------------------------------------
import sys
import serial
import random

# -----------------------------------------------------------------------------
if(len(sys.argv) < 3):
	print("python3 serial_test.py PORT_NAME TEST_SIZE")
	sys.exit();

# -----------------------------------------------------------------------------
print("Port: " + sys.argv[1])
sp = serial.Serial(port = sys.argv[1], baudrate = 115200, timeout = 0.1)

# -----------------------------------------------------------------------------
TEST_SIZE = int(sys.argv[2])
print("Test size: " + str(TEST_SIZE))

# -----------------------------------------------------------------------------
txBuf = [0] * TEST_SIZE
testFlag = True
testCounter = 0

# -----------------------------------------------------------------------------
while testFlag:
	for x in range(0,TEST_SIZE):
		txBuf[x] = random.randint(0, 255)

	sp.write(bytearray(txBuf))
	rxBuf = sp.read(TEST_SIZE)

	if(len(rxBuf) == TEST_SIZE):
		for x in range(0,TEST_SIZE):
			if(txBuf[x] != rxBuf[x]):
				testFlag = False
	else:
		testFlag = False
	if(testFlag):
		testCounter += 1
		print("testCounter: " + str(testCounter))

# -----------------------------------------------------------------------------
sp.close()
