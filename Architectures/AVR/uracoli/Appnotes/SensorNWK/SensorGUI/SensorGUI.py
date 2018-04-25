#import pylab
import serial
import pprint
import Tkinter
import Tix
import UserList

class SerialHost(serial.Serial):
	def __init__(self, *args, **kwargs):
		serial.Serial.__init__(self, *args, **kwargs)
	def tuple(self, s):
		tp = s.split('=')
		key = str(tp[0].strip())
		try:
			val = float(tp[1].strip())
		except:
			val = str(tp[1].strip())
		return {key:val}
	def parse(self, ln):
		d = {}
		[d.update(self.tuple(tpl)) for tpl in ln.strip().split(',')]
		return d
	def rxloop(self):
		while True:
			print self.parse(self.readline().strip())

class SensorProtoHost(SerialHost):
	def __init__(self, *args, **kwargs):
		SerialHost.__init__(self, *args, **kwargs)

class DataB(list):
	def __init__(self):
		pass
	def get(self, start=0, stop= -1):
		return self.data[start, stop]

def emudata():
	db = DataB()
	sh = SerialHost()
	[db.append(sh.parse(ln)) for ln in open('test.log')]
	return db

class NodePool(Tkinter.Canvas):
	def __init__(self, parent, *args, **kwargs):
		Tkinter.Canvas.__init__(self, parent, *args, **kwargs)
		[self.addnode(addr) for addr in range(1,5)]
	def flash(self, addr):
		[self.itemconfigure(i, outline='red', width=2) for i in self.find_withtag(addr)]
		self.after(50, self.unflash, addr)
	def unflash(self, addr):
		[self.itemconfigure(i, outline='black', width=1) for i in self.find_withtag(addr)]
		self.after(2000, self.flash, addr)

	def addnode(self, addr):
		radius=16
		x=10 + 30*addr
		y=20
		tag=addr
		self.create_oval(x,y,x+radius,y+radius, fill='grey', tags=tag)
		self.tag_bind(tag, '<Enter>', func=lambda x: self.enter(tag))
		self.tag_bind(tag, '<Leave>', func=lambda x: self.leave(tag))

	def enter(self, tag):
		self.itemconfigure(tag, fill='lightgrey')
		x0,y0,x1,y1 = self.coords(self.find_withtag(tag)[0])
		self.balloon = self.create_rectangle(x0+20,y0+20,x0+120,y0+120,fill='lightyellow')
	def leave(self, tag):
		self.itemconfigure(tag, fill='grey')
		self.delete(self.balloon)

class Plot(Tkinter.Canvas):
	def __init__(self, parent, *args, **kwargs):
		Tkinter.Canvas.__init__(self, parent, *args, **kwargs)
		
	def plot(self, xs, ys):		
		scale_x = 400 / (max(xs) - min(xs))
		scale_y = 200 / (max(ys) - min(ys))

		scaled_xs, scaled_ys = zip(*[(scale_x*(x - min(xs)), scale_y*(y - min(ys))) for x,y in zip(xs, ys)])
		[self.create_line(x, y,x+1,y+1) for x,y in zip(scaled_xs, scaled_ys)]
		self.create_line(min(scaled_xs), 200, max(scaled_xs), 200)

class ValField(Tkinter.Frame):
	def __init__(self, master, name, unit, val, *args, **kwargs):
		Tkinter.Frame.__init__(self, master, *args, **kwargs)
		Tkinter.Label(master=self, text="%s [%s]: "%(name,unit)).pack(side=Tkinter.LEFT)
		Tkinter.Label(master=self, textvariable=val).pack(side=Tkinter.RIGHT)

class Data_muse231(Tkinter.Frame):
	""" Widget to display Data frames for BOARD_TYPE=muse231 """
	def __init__(self, parent, *args, **kwargs):
		Tkinter.Frame.__init__(self, parent, *args, **kwargs)
		self.title = Tkinter.Label(master=self, text="muse231")
		self.title.pack(side=Tkinter.TOP)
		
		#self.img=Tkinter.PhotoImage(file="pictures/muse231.gif")
		#Tkinter.Label(master=self, image=self.img).pack(side=Tkinter.LEFT)
		
		
		self.valframe=Tkinter.Frame(master=self, relief=Tkinter.RAISED, borderwidth=1)
		self.valframe.pack(side=Tkinter.TOP)
		Tkinter.Label(master=self.valframe, text="Measurement Data").pack(side=Tkinter.TOP)
		
		self.sht_t = Tkinter.DoubleVar(value=25.0)
		ValField(master=self.valframe, name="SHT T", unit="degC", val=self.sht_t).pack(side=Tkinter.BOTTOM)

		self.sht_rh = Tkinter.DoubleVar(value=0.5)
		ValField(master=self.valframe, name="SHT RH", unit="vH", val=self.sht_rh).pack(side=Tkinter.BOTTOM)

		self.vmcu = Tkinter.DoubleVar(value=2.85)
		ValField(master=self.valframe, name="VMCU", unit="mV", val=self.vmcu).pack(side=Tkinter.BOTTOM)

		self.cfgframe=Tkinter.Frame(master=self, relief=Tkinter.RAISED, borderwidth=1)
		self.cfgframe.pack(side=Tkinter.TOP)
		Tkinter.Label(master=self.cfgframe, text="Configuration").pack(side=Tkinter.TOP)
		
		self.cfg_accenable = Tkinter.BooleanVar()
		Tkinter.Checkbutton(master=self.cfgframe, text="ACC enable", variable=self.cfg_accenable).pack(side=Tkinter.TOP)
		self.cfg_shtenable = Tkinter.BooleanVar()
		Tkinter.Checkbutton(master=self.cfgframe, text="SHT21 enable", variable=self.cfg_shtenable).pack(side=Tkinter.TOP)
		self.cfg_blink = Tkinter.BooleanVar()
		Tkinter.Checkbutton(master=self.cfgframe, text="Blink on sample", variable=self.cfg_blink).pack(side=Tkinter.TOP)
		
		rates=[0.1, 1.0, 8.0, 60.0]
		self.cfg_samplerate = Tkinter.DoubleVar()
		Tkinter.Label(master=self.cfgframe, text='Sample Rate [Hz]').pack(side=Tkinter.TOP)
		cb=Tix.ComboBox(master=self.cfgframe, variable=self.cfg_samplerate)
		cb.pack(side=Tkinter.TOP)
		bal=Tix.Balloon(self.cfgframe)
		bal.bind_widget(cb, balloonmsg="Sample rate of measurement")
		[cb.insert(Tix.END, '%.2f'%(1/i)) for i in rates]
		
		#[Tkinter.Radiobutton(master=self.cfgframe, text='%.2f'%(1/i), variable=self.cfg_samplerate, value=i).pack(side=Tkinter.TOP) for i in rates]
		
class GUIApp(object):
	def __init__(self, parent):
		self.appframe = Tkinter.Frame(parent)
		self.appframe.pack()
		self.label = Tkinter.Label(self.appframe, text="Sensor NWK")
		self.label.pack(side=Tkinter.TOP)
		
		self.pool = NodePool(self.appframe, bg='white', width=200, height=200)
		self.pool.pack(side=Tkinter.LEFT)
		self.pool.flash(2)
		
		#self.plot = Plot(self.appframe, bg='white')
		#self.plot.pack()
		#self.db = emudata()
		#xs, ys = zip(*[(i['TSTAMP'], i['SHT_T[degC]']) for i in self.db if i.has_key('FRAME') and i['FRAME']=='DATA' and i['ADDR']==7])
		#self.plot.plot(xs, ys)
		
		self.muse231 = Data_muse231(self.appframe, relief=Tkinter.RAISED, borderwidth=1)
		self.muse231.pack(side=Tkinter.RIGHT)
		
def plot():
	keys = ['SHT_T[degC]', 'SHT_RH[vH]', 'VMCU[mV]', 'RSSI[dBm]']
	fig, axs = pylab.subplots(nrows=len(keys), ncols=1, sharex=True)
	for key, ax in zip(keys, axs):
		xs, ys = zip(*[(i['TSTAMP'], i[key]) for i in data])
		ax.plot(xs, ys)
		ax.set_ylabel(key)
		ax.set_xlabel('time')
		ax.grid()
	pylab.show()
		
if __name__ == '__main__':
	
	pp = pprint.PrettyPrinter()
	#host=SensorProtoHost(port='COM9',baudrate=38400)
	#tkroot = Tkinter.Tk()
	tkroot = Tix.Tk()
	gui = GUIApp(tkroot)
	tkroot.mainloop()
	#host.rxloop()
	
# EOF
