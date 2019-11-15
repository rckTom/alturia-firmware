import struct
import numpy as np
from matplotlib import pyplot as plt
import scipy as sp
import functools
import itertools
import pandas as pd

tracks = dict()
track_data = dict()

def calcentries(fmt):
    return len(fmt)

with open('/home/thomas/Tools/littlefs-fuse/press_new.dat','rb') as f:
	while True:
		fb = f.read(1)
		if not fb:
			break
		cid = (fb[0] & 0xF0) >> 4
		if cid == 1:
			tid = fb[0] & 0x0F
			by = bytes()
			while True:
				b = f.read(1)
				if b[0] == 0:
					break
				by = by + b
			fmt = by.decode('ascii')
			tracks[tid] = dict()
			track_data[tid] = pd.DataFrame(columns=range(0,calcentries(fmt)))
			tracks[tid]['format'] = fmt
			tracks[tid]['length'] = struct.calcsize(fmt)
		elif cid == 2:
			tid = fb[0] & 0x0F
			by = bytes()
			while True:
				b = f.read(1)
				if b[0] == 0:
					break
				by = by +b
			name = by.decode('ascii')
			names = name.split(',')
			print(names)
			track_data[tid].rename(columns=dict(zip(range(0,len(names)),names)))
		elif cid == 3:
			tid = fb[0] & 0x0F
			data = f.read(tracks[tid]['length'])
			values = struct.unpack(tracks[tid]['format'],data)
			track_data[tid].loc[len(track_data[tid])] = values

track_data[0].plot()
plt.show()