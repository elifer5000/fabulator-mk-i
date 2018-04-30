#!/usr/bin/env python
"""\
Midi input (using rtmidi) based on:
https://github.com/SpotlightKid/python-rtmidi/blob/master/examples/basic/midiin_poll.py


"""

# C3 note is MIDI note 48
# C4 note is MIDI note 60
# A4 note is MIDI note 69
# C5 note is MIDI note 72

import sys
import math
import time
import random
import serial
import rtmidi


from rtmidi.midiutil import open_midiinput
from rtmidi.midiconstants import NOTE_ON, NOTE_OFF, CONTROLLER_CHANGE

ser = serial.Serial('/dev/tty.usbmodem1411', 115200)
print ser.readline()

# Prompts user for MIDI input port, unless a valid port number or name
# is given as the first argument on the command line.
# API backend defaults to ALSA on Linux.
midiport = sys.argv[1] if len(sys.argv) > 1 else None

try:
    midiin, port_name = open_midiinput(midiport)
except (EOFError, KeyboardInterrupt):
    sys.exit()

try:
	timer = time.time()
	while True:
		msg = midiin.get_message()
		
		if msg:
			message, deltatime = msg
			timer += deltatime
			print("[%s] @%0.6f %r" % (port_name, timer, message))
			note = message[1]
			volume = message[2]
			if message[0] & 0xF0 == NOTE_ON or message[0] & 0xF0 == NOTE_OFF:
				noteEvent = 1 if message[0] & 0xF0 == NOTE_ON else 2
				freq = pow(2.0, (note - 69) / 12.0) * 440.0   # Hz -> 1/s - MOVE THIS TO ARDUINO AS WELL
				speed = int(freq)   # steps / second
				lobyte = speed & 0xff
				hibyte = (speed & 0xff00) >> 8
				volLoByte = volume & 0xff
				volHiByte = (volume & 0xff00) >> 8
				print(speed)
				
				values = bytearray([noteEvent, lobyte, hibyte, volLoByte, volHiByte])
				ser.write(values)
				# print ser.readline()
			elif message[0] & 0xF0 == CONTROLLER_CHANGE:
				if message[1] == 21:
					values = bytearray([message[1], message[2], 0, 0, 0])
				elif message[1] == 22:
					values = bytearray([message[1], message[2], 0, 0, 0])
				elif message[1] == 23:
					values = bytearray([message[1], message[2], 0, 0, 0])

				ser.write(values)
				# print ser.readline()
				# print ser.readline()


		# time.sleep(0.005)
except KeyboardInterrupt:
	print('')
finally:
	print("Exit.")
	# Close midi
	midiin.close_port()
	del midiin


