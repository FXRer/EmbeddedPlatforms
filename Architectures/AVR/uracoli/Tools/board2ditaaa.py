#
# Parse board definition files and generate ascii block diagrams, readily
# prepared for Ditaa
#
#

# Python distribution
import glob
import string
import os

# ./
import boardcfg

signals_trx = [
	'PORT_TRX_RESET',
	'MASK_TRX_RESET',
	'PORT_TRX_SLPTR',
	'MASK_TRX_SLPTR',
	'TRX_IRQ_vect',
	'PORT_SPI',
	'SPI_SS',
	'SPI_SCK',
	'SPI_MOSI',
	'SPI_MISO',
	'PORT_KEY',
	'LED_PORT',
	'HWTIMER_REG',
	'HWTIMER_TICK',
]

def parse_bitdefs(s):
	""" Parse bit position definitions
	    C-Syntax

		(1<<4)
		(1<<PB4)
		_BV(PB4)
		_BV(1<<3)
	"""
	s=s.replace('_BV', '').strip('(').strip(')')
	s=s[s.rfind('<')+1:]
	return s

def parse_include(basepath, fname):
	""" Parse a given include file """
	ret={}
	ret['has_keys']=True
	ret['has_leds']=True
	for ln in open(basepath + fname):
		if len(ln.strip()) > 0:
			if ln.strip()[0] == '#':
				toks = ln[1:].strip().split()
				if toks[0] == 'define':
					if toks[1] in signals_trx:
						ret.update({toks[1]:parse_bitdefs(toks[2])})

					for t in ['RADIO', 'BOARD', 'HIF']:
						if toks[1] == t+'_TYPE':
							s=toks[2].strip('(').strip(')').replace(t+'_','')
							ret.update({toks[1]:s})

					if toks[1] == 'NO_KEYS': ret['has_keys']=False
					if toks[1] == 'NO_LEDS': ret['has_leds']=False

					if toks[1] in ['LED_NUMBER', 'LED_SHIFT']:
						s=toks[2].strip('(').strip(')').replace(t+'_','')
						ret.update({toks[1]:s})

				elif toks[0] == 'include':
					fname = toks[1].strip('"')
					print "Including file %s" %fname

					p = basepath
					if fname[:7] != 'boards/':
						p = basepath + 'boards/'
					# recursive call
					ret.update(parse_include(p, fname))

	return ret

class Canvas(object):
	def __init__(self, *args, **kwargs):
		str.__init__(self, *args, **kwargs)
		self.width = 70
		self.height = 30
		self.data = [' ']*self.width*self.height

	def pos(self, x, y):
		""" Calc position in array from x,y """
		return y*self.width+x

	def setpixel(self, x, y, c):
		self.data[self.pos(x, y)]= c

	def string(self, x, y, s):
		for i, c in enumerate(s):
			self.setpixel(x+i,y, c)

	def box(self, x0, y0, x1, y1, name=''):
		""" Draw a box (x0,y0) upper left corner, (x1,y1) lower right corner """

		# Top and bottom row
		for x in range(x0+1,x1):
			for y in [y0, y1]:
				self.setpixel(x,y,'-')

		# Left and right wall
		for y in range(y0+1,y1):
			for x in [x0, x1]:
				self.setpixel(x,y,'|')

		# Corners
		for x in [x0,x1]:
			for y in [y0, y1]:
				self.setpixel(x,y,'+')

		for i, c in enumerate(name):
			self.setpixel(x0+2+i,y0+1, c)


	def connector(self, x0, x1, y, namel='', namer=''):
		# self.data[self.pos(x0,y)] = '+'
		# self.data[self.pos(x1,y)] = '+'
		for x in range(x0+1, x1):
			self.data[self.pos(x,y)] = '-'
		for i, c in enumerate(namel):
			self.data[self.pos(x0-1-len(namel)+i,y)] = c
		for i, c in enumerate(namer):
			self.data[self.pos(x1+2+i,y)] = c

	def tostr(self):
		""" Convert canvas data array to string """
		return string.join([string.join(self.data[y*self.width:(y+1)*self.width],'') for y in range(self.height)], '\n')

def generate_ditaa(bname, board, b2):
	canv = Canvas()

	canv.string(0,0,bname)

	top=1
	bottom=20

	distbox = 4

	# reserved space for antenna
	widthant = 5

	right=widthant

	name_timer = board['HWTIMER_REG'] + ' TICK=?? ms'

	# Single Chip
	if b2['cpu']=='atmega128rfa1':
		canv.box(right,1,right+24,bottom-2, name=b2['cpu'])
		canv.box(right,bottom-2,right+24,bottom, name=name_timer)
		right += 24

	# MCU + TRX	connected via SPI
	else:
		widthtrx = 13
		widthcpu = 20
		canv.box(right,1,right+widthtrx,bottom, name=board['RADIO_TYPE'])
		canv.box(right+widthtrx+distbox,1,right+widthtrx+distbox+widthcpu,bottom-2, name=b2['cpu'])
		canv.box(right+widthtrx+distbox,bottom-2,right+widthtrx+distbox+widthcpu,bottom, name=name_timer)

		right += widthtrx

		y=4

		map={'SELN':'SPI_SS', 'SCK':'SPI_SCK', 'MOSI':'SPI_MOSI', 'MISI':'SPI_MISO'}
		for i, s in enumerate(map.keys()):
			# some boards do an addittional include where these are defined, cannot
			# handle it, fix this
			if board.has_key('SPI_SS'):
				portname = board[map[s]]
			else:
				print "Skipping, SPI_SS not defined"
				portname = 'Px'
			canv.connector(right,right+distbox,y, namel=s, namer=portname)
			y += 1

		map={'RSTN': {'PORT':'PORT_TRX_RESET','MASK':'MASK_TRX_RESET'},
			 'SLP_TR': {'PORT':'PORT_TRX_SLPTR','MASK':'MASK_TRX_SLPTR'},
		}
		y += 1
		for s in ['RSTN', 'SLP_TR', 'IRQ']:
			if s in map.keys() and board.has_key(map[s]['MASK']):
				portname = board[map[s]['MASK']]
				if portname[0] != 'P':
					portname = board[map[s]['PORT']].replace('ORT','') + portname
			elif s == 'IRQ':
				portname=board['TRX_IRQ_vect'].replace('_vect', '')
			else:
				portname='??'
			canv.connector(right,right+distbox,y, namel=s, namer=portname)
			y += 1

		right += widthcpu+distbox

	# Antenna
	canv.setpixel(1,4,'^')
	canv.setpixel(1,5,'|')
	canv.connector(1,widthant,6, namer='RF')

	# IOs
	y = 2
	distbox = 4
	wiobox = 7 # width of IO box

	if board['has_leds']:

	# sometimes the LEDS are distributed and LED_SHIFT makes no sense
		# improve this
		nb=int(board['LED_NUMBER'])
		sh=int(board['LED_SHIFT'])

		# sometimes the LEDS are not connected directly to a port, e.g. STB (memory mapped)
		# handle here
		if board.has_key('PORT_KEY'):
			port=board['LED_PORT'].replace('ORT','')
		else:
			port='???'
		s='%s[%d..%d]'%(port, sh+nb-1, sh)
		canv.box(right+distbox, y, right+distbox+wiobox, y+4, name='LEDS')
		canv.connector(right,right+distbox,y+2, namel=s)
		y+=6

	if board['has_keys']:

		# sometimes the keys are not connected directly to a port, e.g. STB (memory mapped)
		# handle here
		if board.has_key('PORT_KEY'):
			port = '%s[7..4]'%(board['PORT_KEY'].replace('ORT',''))
		else:
			port = '???'
		canv.box(right+distbox, y, right+distbox+wiobox, y+4, name='KEYS')
		canv.connector(right,right+distbox,y+2, namel=port)
		y+=6

	if board['HIF_TYPE'] != 'NONE':
		canv.box(right+distbox, y, right+distbox+wiobox, y+4, name='HIF')
		canv.connector(right,right+distbox,y+2, namel=board['HIF_TYPE'])
		y+=6

	return canv.tostr()

def generate_graphviz(bname, board, b2):
	gviz = "digraph G {\ngraph [rankdir = LR];\nnode[shape=record];\n"

	name_timer = board['HWTIMER_REG'] + ' TICK=?? ms'
	# gviz += "title[label=\"%s\"];\n"%bname

	# MCU + TRX	connected via SPI
	if b2['cpu'].find('rfa')>=0 or b2['cpu'].find('rfr')>=0:
		soc = True
	else:
		soc = False
		trxpins=[]
		idx = 0
		map={
			'SELN':{'name':'SPI_SS',   'dir':0},
			'SCK' :{'name':'SPI_SCK',  'dir':0},
			'MOSI':{'name':'SPI_MOSI', 'dir':0},
			'MISO':{'name':'SPI_MISO', 'dir':1},
			}
		for s in map.keys():
			# some boards do an addittional include where these are defined, cannot
			# handle it, fix this
			if board.has_key('SPI_SS'):
				portname = board[map[s]['name']]
			else:
				print "Skipping, SPI_SS not defined"
				portname = 'Px'

			trxpins.append({'mcu':portname, 'trx':s, 'dir':map[s]['dir'], 'idx':idx})
			idx+=1

		map={'RSTN': {'PORT':'PORT_TRX_RESET','MASK':'MASK_TRX_RESET','dir':0},
			 'SLP_TR': {'PORT':'PORT_TRX_SLPTR','MASK':'MASK_TRX_SLPTR','dir':0},
		}
		for i, s in enumerate(['RSTN', 'SLP_TR', 'IRQ']):
			if s in map.keys() and board.has_key(map[s]['MASK']):
				portname = board[map[s]['MASK']]
				if portname[0] != 'P':
					portname = board[map[s]['PORT']].replace('ORT','') + portname
				dir=0
			elif s == 'IRQ':
				portname=board['TRX_IRQ_vect'].replace('_vect', '')
				dir=1
			else:
				portname='??'
				dir=0

			trxpins.append({'mcu':portname, 'trx':s, 'dir':dir, 'idx':idx})
			idx+=1

		gviz += "trx[label=\"{%s|{%s}}\"];\n"%(board['RADIO_TYPE'], '|'.join(['<p%(idx)u>%(trx)s'%p for p in trxpins]))

		for p in trxpins:
			if p['dir']:
				gviz += '\n'.join(["trx:<p%(idx)u> -> mcu:<p%(idx)u>;\n"%p])
			else:
				gviz += '\n'.join(["mcu:<p%(idx)u> -> trx:<p%(idx)u>;\n"%p])


	if soc: idx=1 # otherwise continue the one from above
	perpins = []
	if board['has_leds']:
	# sometimes the LEDS are distributed and LED_SHIFT makes no sense
		# improve this
		nb=int(board['LED_NUMBER'])
		sh=int(board['LED_SHIFT'])

		# sometimes the LEDS are not connected directly to a port, e.g. STB (memory mapped)
		# handle here
		if board.has_key('PORT_KEY'):
			port=board['LED_PORT'].replace('ORT','')
		else:
			port='???'
		s='%s[%d..%d]'%(port, sh+nb-1, sh)

		gviz += "leds[label=\"LEDS\"];\n"
		gviz += "mcu:<p%u> -> leds\n"%idx
		perpins.append({'name':s, 'idx':idx})
		idx += 1

	if board['has_keys']:
		# sometimes the keys are not connected directly to a port, e.g. STB (memory mapped)
		# handle here
		if board.has_key('PORT_KEY'):
			port = '%s[7..4]'%(board['PORT_KEY'].replace('ORT',''))
		else:
			port = '???'
		gviz += "keys[label=\"KEYS\"];\n"
		gviz += "mcu:<p%u> -> keys\n"%idx
		perpins.append({'name':port, 'idx':idx})
		idx += 1

	if board['HIF_TYPE'] != 'NONE':
		gviz += "hif[label=\"HIF\"];\n"
		gviz += "mcu:<p%u> -> hif\n"%idx
		perpins.append({'name':board['HIF_TYPE'], 'idx':idx})
		idx += 1
		
	gviz += "mcu[label=\"{"
	if not soc:
		gviz += "{%s}|"%('|'.join(['<p%(idx)u>%(mcu)s'%p for p in trxpins]))
	gviz += "%s"%b2['cpu']
	if len(perpins)>0:
		gviz += "|{%s}"%('|'.join(['<p%(idx)u>%(name)s'%p for p in perpins]))
	gviz += "}|{<p88> Timer}\"];\n"


	gviz += "}"
	return gviz


if __name__ == '__main__':
	basepath = '../Src/Lib/Inc/'

	cfgfile = '../Config/board.cfg'
	assert os.path.exists(cfgfile)
	boards = boardcfg.get_boards(cfgfile)

	outdir = './generated/'
	assert os.path.exists(outdir)

	for name, vals in boards.iteritems():
		print "Processing", name
		data=parse_include(basepath, vals['include'])
		# data=parse_include_pycp(basepath, vals['include'])

		print data

		# s=generate_ditaa(name, data, vals)
		# print '-'*79
		# print s
		# print '-'*79
		# fname = outdir+name+'.txt'
		# print >> open(fname,'w'), s
		# os.system('java -jar ../../ditaa0_9.jar %s %s'%(fname, fname+'.png'))

		s=generate_graphviz(name, data, vals)
		print '-'*79
		print s
		print '-'*79
		fname = outdir+name+'.dot'
		print >> open(fname,'w'), s
		assert os.system('dot -Tpng %s > %s'%(fname,fname+'.png'))==0

# EOF
