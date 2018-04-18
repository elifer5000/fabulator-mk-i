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

# class MachineComm:
# 	def __init__(self, name):
# 		self.okcnt = 0
# 		self.linecnt = 0
# 		self.name = name
# 		self.machine = machines[name]
# 		self.tn = telnetlib.Telnet(self.machine['ip'], 23)
# 		time.sleep(1.0)
# 		if self.machine['prompt']:
# 			print('reading prompt for ' + self.name)
# 			self.tn.read_until(self.machine['prompt'])

# 		self.reset()

# 	def reset(self):
# 		print("Homing with G28 for " + self.name)
# 		self.tn.write('G28\n')
# 		self.tn.write('G90\n') #Absolute position
# 		self.tn.write('G92 X0 Y0 Z0 E0\n') #Set origin to current position
# 		for command in self.machine['resetCommands']:
# 			self.tn.write(command + '\n')
# 		rep = self.tn.read_eager()
# 		time.sleep(1.0)
# 		print(self.name + " ready")

# 	def send(self, x, y, f):
# 		line = 'G01 X{} Y{} F{}'.format(x, y, f)
# 		print(self.name + ' ' + line)
# 		self.tn.write(line + '\n')
# 		self.linecnt += 1
# 		rep = self.tn.read_eager()
# 		self.okcnt += rep.count("ok")
# 		if verbose: print("SND " + str(self.linecnt) + ": " + line.strip() + " - " + str(self.okcnt))
			
# 		while self.okcnt < self.linecnt:
# 			print("Waiting for complete...")
# 			rep = self.tn.read_some()
# 			self.okcnt += rep.count("ok")
# 			if verbose: print(str(self.linecnt) + " - " + str(self.okcnt))

# 	def close(self):
# 		print('closing telnet for ' + self.name)
# 		if self.machine['end']:
# 			print('ending with ' + self.machine['end'])
# 			self.tn.write(self.machine['end']+ '\n')
# 		self.tn.write("exit\n") # for 
# 		# self.tn.read_all()
# 		self.tn.read_eager()
# 		self.tn.close()


# class NoteToGCode:
	
# 	def __init__(self, name):
# 		self.name = name
# 		self.machine = machines[name]
# 		self.safemin = self.machine['safemin']
# 		self.safemax = self.machine['safemax']
# 		self.ppu = self.machine['ppu']
# 		self.higherNotesPriority = self.machine['higherNotesPriority']
# 		self.transpose = self.machine['transpose']
# 		if idiomode:
# 			self.higherNotesPriority = not self.higherNotesPriority
		
# 		self.noterange = self.machine['noterange']
# 		self.mach_comm = MachineComm(name)
# 		self.x = 0.0
# 		self.y = 0.0
# 		self.x_dir = 1.0
# 		self.y_dir = 1.0
# 		self.num_axes = 2

# 	def reached_limit(self, current, distance, direction, min, max):

# 	    if ( ( (current + (distance * direction)) < max ) and 
# 	         ( (current + (distance * direction)) > min ) ):
# 	        # Movement in the current direction is within safe limits,
# 	        return False

# 	    elif ( ( (current + (distance * direction)) >= max ) and 
# 	           ( (current - (distance * direction)) >  min ) ):
# 	        # Movement in the current direction violates maximum safe
# 	        # value, but would be safe if the direction is reversed
# 	        return True

# 	    elif ( ( (current + (distance * direction)) <= min ) and 
# 	           ( (current - (distance * direction)) <  max ) ):
# 	        # Movement in the current direction violates minimum safe
# 	        # value, but would be safe if the direction is reversed
# 	        return True

# 	    else:
# 	        # Movement in *either* direction violates the safe working
# 	        # envelope, so abort.
# 	        # 
# 	        print "\n*** ERROR ***"
# 	        print "The current movement cannot be completed within the safe working envelope of"
# 	        print "your machine. Turn on the --verbose option to see which MIDI data caused the"
# 	        print "problem and adjust the MIDI file (or your safety limits if you are confident"
# 	        print "you can do that safely). Aborting."
# 	        exit(2);

# 	def sendGCode(self, notes):
# 		feed_xy = [0, 0]
# 		distance_xy = [0, 0]
# 		filteredNotes = list(filter(lambda x: self.noterange[0] <= x <= self.noterange[1], notes.values()))
# 		sortedNotes = sorted(filteredNotes, reverse=self.higherNotesPriority)

# 		for i in range(0, min(len(notes.values()), self.num_axes)):
# 			note = sortedNotes[i]
# 			note += self.transpose
# 			print('note: {} {}'.format(i, note))
# 			freq = pow(2.0, (note - 69) / 12.0) * 440.0
		
# 			feed_xy[i] = ( freq * 60.0 ) / self.ppu[i]
# 			distance_xy[i] = ( feed_xy[i] * microNoteDuration ) / 60.0 
		
# 		# Turn around BEFORE crossing the limits of the 
# 		# safe working envelope
# 		if self.reached_limit( self.x, distance_xy[0], self.x_dir, self.safemin[0], self.safemax[0] ):
# 			self.x_dir = self.x_dir * -1
		
# 		self.x = (self.x + (distance_xy[0] * self.x_dir))
		
# 		if self.reached_limit( self.y, distance_xy[1], self.y_dir, self.safemin[1], self.safemax[1] ):
# 			self.y_dir = self.y_dir * -1
		
# 		self.y = (self.y + (distance_xy[1] * self.y_dir))
		
# 		combined_feedrate = math.sqrt(feed_xy[0]**2 + feed_xy[1]**2)

# 		self.mach_comm.send(self.x, self.y, combined_feedrate)
		
# 	def close(self):
# 		self.mach_comm.close()

# def write_raw_sequence(tn, seq):
#     sock = tn.get_socket()
#     if sock is not None:
#         sock.send(seq)


# ## parameters
# verbose = True
# microNoteDuration = 0.05 # in seconds
# # idioteque chords
# idiochords = [
# 	{'43': 43, '50': 50, '58': 58, '63': 63},
# 	{'51': 51, '58': 58, '62': 62, '67': 67},
# 	{'50': 50, '55': 55, '63': 63, '70': 70},
# 	{'58': 58, '63': 63, '67': 67, '74': 74}
# ]



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


numMotors = 4
motor1 = True
motor2 = True
motor3 = True
motor4 = True

def getAvailableMotor():
	global motor1, motor2, motor3, motor4
	if motor1:
		motor1 = False
		return 1
	if motor2:
		motor2 = False
		return 2
	if motor3:
		motor3 = False
		return 3
	if motor4:
		motor4 = False
		return 4

	return 0

def setMotorAvailable(val):
	global motor1, motor2, motor3, motor4
	if val == 1:
		motor1 = True
	if val == 2:
		motor2 = True
	if val == 3:
		motor3 = True
	if val == 4:
		motor4 = True

active_notes={}
try:
	timer = time.time()
	while True:
		msg = midiin.get_message()
		
		if msg:
			message, deltatime = msg
			timer += deltatime
			print("[%s] @%0.6f %r" % (port_name, timer, message))
			note = message[1]
			if message[0] & 0xF0 == NOTE_ON:
				freq = pow(2.0, (note - 69) / 12.0) * 440.0   # Hz -> 1/s
				speed = int(freq)   # steps / second
				lobyte = speed & 0xff
				hibyte = (speed & 0xff00) >> 8
				motor = getAvailableMotor() & 0xff
				print(speed)
				if motor > 0:
					values = bytearray([motor, lobyte, hibyte])
					ser.write(values)
					active_notes[note] = motor
					print ser.readline()
			elif message[0] & 0xF0 == NOTE_OFF:
				if active_notes.has_key(note):
					values = bytearray([active_notes[note], 0, 0])
					setMotorAvailable(active_notes[note])
					ser.write(values)
					active_notes.pop(note)
					print ser.readline()
			elif message[0] & 0xF0 == CONTROLLER_CHANGE:
				if message[1] == 21:
					values = bytearray([10, message[2], 0])
				elif message[1] == 22:
					values = bytearray([11, message[2], 0])

				ser.write(values)
				print ser.readline()
				print ser.readline()
				# print ser.readline()
				# print ser.readline()
				# print ser.readline()

		time.sleep(0.005)
except KeyboardInterrupt:
	print('')
finally:
	print("Exit.")
	# Close midi
	midiin.close_port()
	del midiin

# zmorphMachine = NoteToGCode('zmorph')
# monopriceMachine = NoteToGCode('monoprice')

# print("Entering main loop. Press Control-C to exit.")

# active_notes={}
# try:
# 	timer = time.time()
# 	while True:
# 		msg = midiin.get_message()
		
# 		if msg:
# 			message, deltatime = msg
# 			timer += deltatime
# 			print("[%s] @%0.6f %r" % (port_name, timer, message))
# 			note = message[1]
# 			if message[0] & 0xF0 == NOTE_ON:
# 				if idiomode:
# 					if note == 60:
# 						active_notes = idiochords[0]
# 					if note == 62:
# 						active_notes = idiochords[1]	
# 					if note == 64:
# 						active_notes = idiochords[2]
# 					if note == 65:
# 						active_notes = idiochords[3]
# 				else:	
# 					active_notes[note] = note
# 			elif message[0] & 0xF0 == NOTE_OFF:
# 				if idiomode:
# 					if note == 60 and active_notes == idiochords[0]:
# 						active_notes = {}
# 					if note == 62 and active_notes == idiochords[1]:	
# 						active_notes = {}	
# 					if note == 64 and active_notes == idiochords[2]:
# 						active_notes = {}
# 					if note == 65 and active_notes == idiochords[3]:	
# 						active_notes = {}
					
# 				elif active_notes.has_key(note):
# 					active_notes.pop(note)

# 		if len(active_notes) > 0:
# 			zmorphMachine.sendGCode(active_notes)
# 			monopriceMachine.sendGCode(active_notes)	
				
# 		time.sleep(microNoteDuration)
# except KeyboardInterrupt:
# 	print('')
# finally:
# 	print("Exit.")
# 	# Close midi
# 	midiin.close_port()
# 	del midiin
# 	# Close machines
# 	zmorphMachine.close() 
# 	monopriceMachine.close()
	
# 	print("Done")

