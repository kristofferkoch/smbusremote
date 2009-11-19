#!python

from ctypes import *
from time import time, sleep
from socket import AF_UNIX, SOCK_STREAM
import threading, socket, sys, os

LD="/usr/local/lib/libsmbctrl.so"

class SMBus:
	def __init__(self):
		l = CDLL(LD)
		fn = c_char_p("/dev/i2c/0")
		addr = c_uint8(42);
		x = l.init(fn,addr)
		if not x:
			raise IOError
		self.d = c_void_p(x);
		self.l = l
	def setled(self, setting):
		return self.l.setled(self.d, c_uint8(11), c_int(setting))
	def getcode(self):
		r = self.l.getcode(self.d, c_uint8(42));
		if r in (-1, 0xff00):
			return None
		code = r>>8;
		repeat = r & 0xff
		return (code, repeat)
	def __del__(self):
		self.l.deinit(self.d)

class Server(threading.Thread):
	def __init__(self, sock):
		threading.Thread.__init__(self)
		self.sock = sock
		self.l = threading.Lock()
		self.clients = []
		self.setDaemon(True)
	def bcast(self, code, repeat, remote, button):
		self.l.acquire()
		s = ("%016x %02x %s %s\n" % 
			(code, repeat, button, remote))
		print s,
		sys.stdout.flush();
		toremove = []
		for i, c in enumerate(self.clients):
			try:
				c.send(s)
			except socket.error:
				toremove.append(i)
		toremove.reverse()
		for c in toremove:
			print "Disconnect!"
			del self.clients[c]

		self.l.release()
	def run(self):
		try:
			while True:
				conn, addr = self.sock.accept()
				print "Connection!"
				self.l.acquire()
				self.clients.append(conn);
				self.l.release()
		except:
			sys.exit(1)

sm = SMBus();
sock = socket.socket(AF_UNIX, SOCK_STREAM)
SOCKNAME = "/dev/lircd"
if os.path.exists(SOCKNAME):
	os.unlink(SOCKNAME)
sock.bind(SOCKNAME)
os.chmod(SOCKNAME, 0666)
sock.listen(1)
serv = Server(sock)
serv.start()
i = 0

	
codes={	0x65: "TV_PWR",
	0x0c: "PC_PWR",
	0x17: "RECORD",
	0x19: "STOP",
	0x18: "PAUSE",
	0x15: "REW",
	0x16: "PLAY",
	0x14: "FWD",
	0x1b: "REPLAY",
	0x1a: "SKIP",
	0x23: "BACK",
	0x0f: "MORE",
	0x1e: "UP",
	0x1f: "DOWN",
	0x20: "LEFT",
	0x21: "RIGHT",
	0x10: "VOL_UP",
	0x11: "VOL_DOWN",
	0x0d: "START",
	0x22: "OK",
	0x12: "CH_UP",
	0x13: "CH_DOWN",
	0x0e: "MUTE",
	0x1d: "STAR",
	0x1c: "HASH",
	0x0a: "CLEAR",
	0x0b: "ENTER",
	0x48: "RECORDED_TV",
	0x26: "GUIDE",
	0x25: "LIVE_TV",
	0x24: "DVD_MENU",
	0:"0",
	1:"1",
	2:"2",
	3:"3",
	4:"4",
	5:"5",
	6:"6",
	7:"7",
	8:"8",
	9:"9"}

lastcode = None
lastrepeat = None
last = time()
import math
w = 4/(2*math.pi)
but = 0
while 1:
	i+=1;
	if but:
		if but & 1:
			x = 255
			y = 0
		else:
			x = 0
			y = 255
		but -= 1
	else:
		x = int(127-127*math.sin(w*i))
		y = int(127-127*math.cos(w*i))
	sm.setled((x<<8) | (y&0xFF))
	sleep(0.04)
	c = sm.getcode()
	if c is not None:
		now = time()
		delta = now - last
		if delta > 0.16:
			lastrepeat = 0;
		print delta
		code, repeat = c
		if code == lastcode:
			repeat += lastrepeat
		serv.bcast(code, repeat-1, "MCE", codes[code])
		but = 3
		lastcode = code
		lastrepeat = repeat
		last = now
	else:
		sleep(0.04)

