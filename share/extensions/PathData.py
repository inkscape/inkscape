'''
Copyright (C) 2011 Felipe Correa da Silva Sanches <juca@members.fsf.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''

class PathData():
	def __init__ (self, d):
		self.data = self.parse(d)

	def transform_coordinate_values(self, callback, units_per_em, baseline):

		for d in range(len(self.data)):
			cmd = self.data[d][0]
			relative = (cmd.lower()==cmd)

			if cmd.lower() in "mtlsqc":
				for params in range(len(self.data[d][1])):
					for values in range(len(self.data[d][1][params])):

						if d==0 and params==0 and values==0:
							self.data[d][1][params][values] = callback(self.data[d][1][params][values], units_per_em, baseline, False)
						else:
							self.data[d][1][params][values] = callback(self.data[d][1][params][values], units_per_em, baseline, relative)

		return self.encode()
		
	def get_coord_pairs(self, num_pairs, tokens, i, multiple=True):
		pairs = []
		for _ in range(num_pairs):
			x,y = tokens[i].split(",")
			pairs.append((float(x), float(y)))
			i+=1
		coords = [pairs]

		while multiple:
			try:
				pairs = []
				for _ in range(num_pairs):
					x,y = tokens[i].split(",")
					pairs.append((float(x), float(y)))
					i+=1
				coords.append(pairs)
			except:
				return coords, i
		return coords, i

	def get_coords(self, num_values, tokens, i, multiple=True):
		values = []
		for _ in range(num_values):
			values.append(float(tokens[i]))
			i+=1
		coords = [values]

		while multiple:
			try:
				values = []
				for _ in range(num_values):
					values.append(float(tokens[i]))
					i+=1
				coords.append(values)
			except:
				return coords, i
		return coords, i

	def get_arc_coords(tokens, i):
		return [] #TODO

	def get_tokens(self, d):
		tokens = []
		for t in d.split(" "):
			if len(t):
				if len(t)>1 and t[0].lower() in "hvmtlsqcaz":
					tokens.append(t[0])
					tokens.append(t[1:])
				else:
					tokens.append(t)
		return tokens

	def parse(self, d):
		data = []
		tokens = self.get_tokens(d)
		i=0
		while i < len(tokens):
			command = tokens[i]
			i+=1
			if command in ["z","Z"]:
				data.append((command, None))
				i+=1
			elif command in ["h","H", "v", "V"]:
				coords, i = self.get_coords(1, tokens, i)
				data.append((command, coords))
			elif command in ["a","A"]:
				coords, i = self.get_arc_coords(tokens, i)
				data.append((command, coords))
			elif command in ["m","M", "t", "T", "l", "L"]:
				coords, i = self.get_coord_pairs(1, tokens, i)
				data.append((command, coords))
			elif command in ["s","S", "q", "Q"]:
				coords, i = self.get_coord_pairs(2, tokens, i)
				data.append((command, coords))
			elif command in ["c","C"]:
				coords, i = self.get_coord_pairs(3, tokens, i)
				data.append((command, coords))
		return data

	def encode(self, data=None):
		if data is None:
			data = self.data

		d = ""
		for cmd, coords in data:
			d += cmd + " "
			if cmd.lower() in "mtlsqc":
				for c in coords:
					for x,y in c:
						d += str(x) + "," + str(y) + " "
			elif cmd.lower() == "z":
				d += cmd
			elif cmd.lower() in "hv":
				for values in coords:
					for v in values:
						d += str(v) + " "
			elif cmd.lower() == "a":
				pass #TODO
		return d

