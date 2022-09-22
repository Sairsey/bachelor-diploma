import newGameLib
from newGameLib import *
import Blender
import subprocess

BINDPOSE = 0

class Node:
	def __init__(self):
		self.name = None
		self.children = []
		self.osgChildren = []
		self.offset = None
		self.start = None
		self.end = None
		self.header = ""
		self.data = ""

class Yson:
	def __init__(self):
		self.input = None
		self.filename = None
		self.root = Node()

	def parse(self):
		global offset, string, txt
		if self.filename is not None:
			file = open("./BinFiles/file.osgjs", "rb")
			self.input = file.read().replace("\x20", "").replace("\x0A", "")
			line = self.input
			if line is not None and len(line) > 0:
				offset = 0
				n = 0
				string = []
				self.tree(self.root, n)
			file.close()

	def getTree(self, parent, list, key):
		for child in parent.children:
			if key in child.header:
				list.append(child)
			self.getTree(child, list, key)

	def values(self, data, type):
		list = {}
		dataValues = data.split(",")
		if type == ":":
			for value in dataValues:
				if ":" in value:
					complete = False
					alist = []
					string = ""
					for char in value:
						if char == '"' and not complete:
							if len(string) > 0:
								alist.append(string)
							string = "" + char
							complete = True
						elif char == '"' and complete:
							string += char
							if len(string) > 0:
								alist.append(string)
							string = ""
							complete = False
						elif char == ":":
							pass
						else:
							string += char
					if len(string) > 0:
						alist.append(string)
					if len(alist) == 2:
						list[alist[0]] = alist[1]
		if type == "f":
			list = map(float, dataValues)
		if type == "i":
			list = map(int, dataValues)
		if type == "s":
			list = dataValues
		return list

	def getValue(self, values, name, type=None):
		if name in values:
			if type == '"f"':
				return float(values[name].split('"')[1])
			elif type == '"i"':
				return int(values[name].split('"')[1])
			elif type == "i":
				return int(values[name].replace('"', ""))
			elif type == '""':
				return values[name].split('"')[1]
			else:
				return values[name]
		else:
			return None

	def get(self, node, key):
		list = []
		self.getTree(node, list, key)
		if len(list) > 0:
			return list
		else:
			return None

	def tree(self, parentNode, n):
		global offset, string
		n += 4
		offset += 1
		while True:
			if offset >= len(self.input):
				break
			value = self.input[offset]
			if value == "}":
				if len(string) > 0:
					parentNode.data = self.input[string[0] : offset]
				string = []
				offset += 1
				break

			elif value == "{":
				node = Node()
				parentNode.children.append(node)
				node.offset = offset
				if len(string) > 0:
					node.header = self.input[string[0] : offset]
				string = []
				self.tree(node, n)

			elif value == "]":
				if len(string) > 0:
					parentNode.data = self.input[string[0] : offset]
				offset += 1
				string = []
				break

			elif value == "[":
				node = Node()
				parentNode.children.append(node)
				node.offset = offset
				node.name = string
				if len(string) > 0:
					node.header = self.input[string[0] : offset]
				else:
					node.header = ""
				string = []
				self.tree(node, n)

			else:
				if len(string) == 0:
					string.append(offset)
				offset += 1


def decodeVarint(fileReader, offset, size, type):
	fileReader.seek(offset)
	decoded = [0] * size
	numDec = 0
	while numDec != size:
		shift = 0
		result = 0
		while True:
			byte = fileReader.B(1)[0]
			result |= (byte & 127) << shift
			shift += 7
			if not (byte & 0x80):
				break
		decoded[numDec] = result
		numDec += 1
	if type[0] != "U":
		completeNum = 0
		while completeNum < size:
			tmp = decoded[completeNum]
			decoded[completeNum] = tmp >> 1 ^ -(1 & tmp)
			completeNum += 1
	return decoded


def decodeDelta(t, e):
	i = e | 0
	n = len(t)
	if i >= len(t):
		r = None
	else:
		r = t[i]
	a = i + 1
	while a < n:
		s = t[a]
		r = t[a] = r + (s >> 1 ^ -(1 & s))
		a += 1
	return t


def decodeImplicit(input, n):
	e = [0] * input[0]
	a = input[2]
	s = input[1]
	o = input[3 : s + 3]
	r = 2
	u = 32 * s - len(e)
	l = 1 << 31
	h = 0
	while h < s:
		c = o[h]
		d = 32
		p = h * d
		if h == s - 1:
			f = u
		else:
			f = 0
		g1 = f
		while g1 < d:
			if c & l >> g1:
				e[p] = input[n]
				n += 1
			else:
				if r:
					e[p] = a
				else:
					e[p] = a
					a += 1
			g1 += 1
			p += 1
		h += 1
	return e


def decodeWatermark(bytes, i):
	num = i[0]
	bytesSize = len(bytes)
	bytesCompleted = 0
	while bytesCompleted < bytesSize:
		s = num - bytes[bytesCompleted]
		bytes[bytesCompleted] = s
		if num <= s:
			num = s + 1
		bytesCompleted += 1
	return bytes, num


def getIndices(itemsize, size, offset, type, fileReader, mode, magic):
	if type != "Uint8Array":
		bytes = decodeVarint(fileReader, offset, size * itemsize, type)
	else:
		fileReader.seek(offset)
		bytes = list(fileReader.B(size * itemsize))
	indexFix = True
	if not indexFix:
		if mode == '"TRIANGLE_STRIP"':
			k = 3 + bytes[1]
			bytes = decodeDelta(bytes, k)
			bytes = decodeImplicit(bytes, k)
			i = [magic]
			bytes, magic = decodeWatermark(bytes, i)
		elif mode == '"TRIANGLES"':
			k = 0
			bytes = decodeDelta(bytes, k)
			i = [magic]
			bytes, magic = decodeWatermark(bytes, i)
	return magic, bytes


def decodePredict(indices, input, itemsize):
	t = input
	if len(indices) > 0:
		t = input
		e = itemsize
		i = indices
		n = len(t) / e
		r = [0] * n
		a = len(i) - 1
		r[i[0]] = 1
		r[i[1]] = 1
		r[i[2]] = 1
		s = 2
		while s < a:
			o = s - 2
			u = i[o]
			l = i[o + 1]
			h = i[o + 2]
			c = i[o + 3]
			if 1 != r[c]:
				r[c] = 1
				u *= e
				l *= e
				h *= e
				c *= e
				d = 0
				while d < e:
					t[c + d] = t[c + d] + t[l + d] + t[h + d] - t[u + d]
					d += 1
			s += 1
	return t


def etap1(input, ItemSize):
	n = len(input) / ItemSize
	r = 0
	output = [0] * len(input)
	while r < n:
		a = r * ItemSize
		s = 0
		while s < ItemSize:
			output[a + s] = input[r + n * s]
			s += 1
		r += 1
	return output


def etap2(input, ItemSize, atributes):
	i = [atributes['"bx"'], atributes['"by"'], atributes['"bz"']]
	n = [atributes['"hx"'], atributes['"hy"'], atributes['"hz"']]

	a = len(input) / ItemSize
	s = 0
	output = [0] * len(input)
	while s < a:
		o = s * ItemSize
		u = 0
		while u < ItemSize:
			output[o + u] = i[u] + input[o + u] * n[u]
			u += 1
		s += 1

	return output


def etap3(input, ItemSize):
	i = ItemSize | 1
	n = 1
	r = len(input) / i
	while n < r:
		a = (n - 1) * i
		s = n * i
		o = 0
		while o < i:
			input[s + o] += input[a + o]
			o += 1
		n += 1
	return input


def etap4(input):
	e = 1
	i = len(input) / 4
	while e < i:
		n = 4 * (e - 1)
		r = 4 * e
		a = input[n]
		s = input[n + 1]
		o = input[n + 2]
		u = input[n + 3]
		l = input[r]
		h = input[r + 1]
		c = input[r + 2]
		d = input[r + 3]
		input[r] = a * d + s * c - o * h + u * l
		input[r + 1] = -a * c + s * d + o * l + u * h
		input[r + 2] = a * h - s * l + o * d + u * c
		input[r + 3] = -a * l - s * h - o * c + u * d
		e += 1
	return input


def int3float4(input, atributes, ItemSize):
	c = 4
	d = atributes['"epsilon"']
	p = int(atributes['"nphi"'])
	e = [0] * len(input) * 4
	i = 1.57079632679
	n = 6.28318530718
	r = 3.14159265359
	a = 0.01745329251
	s = 0.25
	o = 720
	u = 832
	l = 47938362584151635e-21
	h = {}
	f = True

	d = d or s
	p = p or o
	g = math.cos(d * a)
	m = 0
	v = 0
	_ = []

	v = (p + 1) * (u + 1) * 3
	_ = [None] * v

	b = r / float(p - 1)
	x = i / float(p - 1)

	if f:
		y = 3
	else:
		y = 2

	m = 0
	v = len(input) / y
	while m < v:
		A = m * c
		S = m * y
		C = input[S]
		w = input[S + 1]
		if c == 0:
			if f == 0:
				if (C & -1025) != 4:
					e[A + 3] = -1
				else:
					e[A + 3] = 1
		M = None
		T = None
		E = None
		I = 3 * (C + p * w)
		M = _[I]
		if M == None:
			N = C * b
			k = cos(N)
			F = sin(N)
			N += x
			D = (g - k * cos(N)) / float(max(1e-5, F * sin(N)))
			if D > 1:
				D = 1
			else:
				if D < -1:
					D = -1
			P = w * n / float(math.ceil(r / float(max(1e-5, math.acos(D)))))
			M = _[I] = F * math.cos(P)
			T = _[I + 1] = F * math.sin(P)
			E = _[I + 2] = k
		else:
			T = _[I + 1]
			E = _[I + 2]
		if f:
			R = input[S + 2] * l
			O = math.sin(R)
			e[A] = O * M
			e[A + 1] = O * T
			e[A + 2] = O * E
			e[A + 3] = math.cos(R)
		else:
			e[A] = M
			e[A + 1] = T
			e[A + 2] = E
		m += 1

	##write(log,_,0)
	return e


def getSplitName(name, what, which):
	a = None
	if what in name:
		a = ""
		splits = name.split(what)
		if which < 0:
			num = len(splits) + which - 1
		else:
			num = which
		if num < 0:
			a = name
		else:
			if which < len(splits):
				for m in range(num):
					a += splits[m] + what
				a += splits[num]
			else:
				a = name
	return a


def getAnimation(ys, A, n):
	action = Action()
	action.ARMATURESPACE = True
	action.BONESORT = True
	action.skeleton = skeleton.name
	n += 4
	Channels = ys.get(A, '"Channels"')
	boneList = {}
	if Channels:
		values = ys.values(Channels[0].header, ":")
		Name = ys.getValue(values, '"Name"')
		action.name = Name
		# write(log,[Name],n)

		for a in Channels[0].children:
			# write(log,['Bone'],n)
			Vec3LerpChannel = ys.get(a, '"osgAnimation.Vec3LerpChannel"')
			bone = None
			if Vec3LerpChannel:
				KeyFrames = ys.get(a, '"KeyFrames"')
				if KeyFrames:
					values = ys.values(KeyFrames[0].header, ":")
					Name = ys.getValue(values, '"Name"')
					TargetName = ys.getValue(values, '"TargetName"', '""')
					# write(log,['Vec3LerpChannel:',Name,'TargetName:',TargetName],n+4)
					name = getSplitName(TargetName, "_", -1)
					if Name == '"translate"':
						if name in boneIndeksList:
							name = boneIndeksList[name]
							if name not in boneList.keys():
								bone = ActionBone()
								action.boneList.append(bone)
								bone.name = name
								boneList[name] = bone
							bone = boneList[name]

						Key = ys.get(a, '"Key"')
						if Key:
							values = ys.values(Key[0].data, ":")
							ItemSize = ys.getValue(values, '"ItemSize"', "i")
							Float32Array = ys.get(Key[0], '"Float32Array"')
							if Float32Array:
								values = ys.values(Float32Array[0].data, ":")
								File = ys.getValue(values, '"File"')
								Size = ys.getValue(values, '"Size"')
								Offset = ys.getValue(values, '"Offset"')
								# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
								path = File
								if os.path.exists(path):
									file = open(path, "rb")
									g = BinaryReader(file)
									g.seek(int(Offset))
									for m in range(int(Size)):
										value = g.f(ItemSize)
										# write(log,value,n+8)
										if bone:
											boneMatrix = (
												skeleton.object.getData()
												.bones[bone.name]
												.matrix["ARMATURESPACE"]
											)
											bone.posKeyList.append(
												boneMatrix * VectorMatrix(value)
											)
									file.close()

						Time = ys.get(a, '"Time"')
						if Time:
							values = ys.values(Time[0].data, ":")
							ItemSize = ys.getValue(values, '"ItemSize"', "i")
							Float32Array = ys.get(Time[0], '"Float32Array"')
							if Float32Array:
								values = ys.values(Float32Array[0].data, ":")
								File = ys.getValue(values, '"File"')
								Size = ys.getValue(values, '"Size"')
								Offset = ys.getValue(values, '"Offset"')
								# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
								path = File
								if os.path.exists(path):
									file = open(path, "rb")
									g = BinaryReader(file)
									g.seek(int(Offset))
									for m in range(int(Size)):
										value = g.f(ItemSize)
										if ItemSize == 1:
											value = value[0]
										##write(log,[value],n+8)
										if bone:
											bone.posFrameList.append(int(value * 33))
									file.close()

			Vec3LerpChannelCompressedPacked = ys.get(
				a, '"osgAnimation.Vec3LerpChannelCompressedPacked"'
			)
			if Vec3LerpChannelCompressedPacked:

				atributes = {}
				UserDataContainer = ys.get(
					Vec3LerpChannelCompressedPacked[0], '"UserDataContainer"'
				)
				if UserDataContainer:
					Values = ys.get(UserDataContainer[0], '"Values"')
					if Values:
						for child in Values[0].children:
							values = ys.values(child.data, ":")
							Name = ys.getValue(values, '"Name"')
							Value = ys.getValue(values, '"Value"', '"f"')
							##write(log,[Name,Value],n+4)
							atributes[Name] = Value

				KeyFrames = ys.get(a, '"KeyFrames"')
				if KeyFrames:
					values = ys.values(KeyFrames[0].header, ":")
					Name = ys.getValue(values, '"Name"')
					TargetName = ys.getValue(values, '"TargetName"', '""')
					# write(log,['Vec3LerpChannelCompressedPacked:',Name,'TargetName:',TargetName],n+4)
					name = getSplitName(TargetName, "_", -1)
					if Name == '"translate"':
						if name in boneIndeksList:
							name = boneIndeksList[name]
							if name not in boneList.keys():
								bone = ActionBone()
								action.boneList.append(bone)
								bone.name = name
								boneList[name] = bone
							bone = boneList[name]

						Key = ys.get(a, '"Key"')
						if Key:
							values = ys.values(Key[0].data, ":")
							ItemSize = int(ys.getValue(values, '"ItemSize"'))
							Uint16Array = ys.get(Key[0], '"Uint16Array"')
							type = "Uint16Array"
							if Uint16Array:
								values = ys.values(Uint16Array[0].data, ":")
								File = ys.getValue(values, '"File"')
								Size = int(ys.getValue(values, '"Size"'))
								Offset = int(ys.getValue(values, '"Offset"'))
								Encoding = ys.getValue(values, '"Encoding"')
								# write(log,[File,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding,'ItemSize:',ItemSize],n+4)
								path = File
								if os.path.exists(path):
									file = open(path, "rb")
									g = BinaryReader(file)

									list = decodeVarint(
										g, Offset, Size * ItemSize, type
									)
									list1 = etap1(list, ItemSize)
									out = etap2(list1, ItemSize, atributes)
									list2 = [
										atributes['"ox"'],
										atributes['"oy"'],
										atributes['"oz"'],
									]
									list2.extend(out)
									list3 = etap3(list2, ItemSize)
									for m in range(Size + 1):
										value = list3[m * 3 : m * 3 + 3]
										# write(log,value,n+8)
										if bone:
											boneMatrix = (
												skeleton.object.getData()
												.bones[bone.name]
												.matrix["ARMATURESPACE"]
											)
											bone.posKeyList.append(
												boneMatrix * VectorMatrix(value)
											)
									file.close()

						Time = ys.get(a, '"Time"')
						if Time:
							values = ys.values(Time[0].data, ":")
							ItemSize = ys.getValue(values, '"ItemSize"', "i")
							Float32Array = ys.get(Time[0], '"Float32Array"')
							if Float32Array:
								values = ys.values(Float32Array[0].data, ":")
								File = ys.getValue(values, '"File"')
								Size = ys.getValue(values, '"Size"', "i")
								Offset = ys.getValue(values, '"Offset"', "i")
								# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
								path = File
								if os.path.exists(path):
									file = open(path, "rb")
									g = BinaryReader(file)
									g.seek(int(Offset))
									list = g.f(Size * ItemSize)
									list1 = etap1(list, ItemSize)
									# out=etap2(list1,ItemSize,atributes)
									list2 = [atributes['"ot"']]
									list2.extend(list1)
									list3 = etap3(list2, ItemSize)
									##write(log,list3,0)
									for m in range(Size + 1):
										value = list3[m]
										if bone:
											bone.posFrameList.append(int(value * 33))
									file.close()

			QuatSlerpChannel = ys.get(a, '"osgAnimation.QuatSlerpChannel"')
			if QuatSlerpChannel:
				KeyFrames = ys.get(a, '"KeyFrames"')
				if KeyFrames:
					values = ys.values(KeyFrames[0].header, ":")
					Name = ys.getValue(values, '"Name"')
					TargetName = ys.getValue(values, '"TargetName"', '""')
					# write(log,['QuatSlerpChannel:',Name,'TargetName:',TargetName],n+4)
					name = getSplitName(TargetName, "_", -1)
					if name in boneIndeksList:
						name = boneIndeksList[name]
						if name not in boneList.keys():
							bone = ActionBone()
							action.boneList.append(bone)
							bone.name = name
							boneList[name] = bone
						bone = boneList[name]

					Key = ys.get(a, '"Key"')
					if Key:
						values = ys.values(Key[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"')
						Float32Array = ys.get(Key[0], '"Float32Array"')
						if Float32Array:
							values = ys.values(Float32Array[0].data, ":")
							File = ys.getValue(values, '"File"')
							Size = ys.getValue(values, '"Size"')
							Offset = ys.getValue(values, '"Offset"')
							# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
							path = File
							if os.path.exists(path):
								file = open(path, "rb")
								g = BinaryReader(file)
								g.seek(int(Offset))
								for m in range(int(Size)):
									value = g.f(4)
									value = Quaternion(value)
									if bone:
										boneMatrix = (
											skeleton.object.getData()
											.bones[bone.name]
											.matrix["ARMATURESPACE"]
										)
										bone.rotKeyList.append(
											boneMatrix * QuatMatrix(value).resize4x4()
										)
								file.close()

					Time = ys.get(a, '"Time"')
					if Time:
						values = ys.values(Time[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"', "i")
						Float32Array = ys.get(Time[0], '"Float32Array"')
						if Float32Array:
							values = ys.values(Float32Array[0].data, ":")
							File = ys.getValue(values, '"File"')
							Size = ys.getValue(values, '"Size"')
							Offset = ys.getValue(values, '"Offset"')
							# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
							path = File
							if os.path.exists(path):
								file = open(path, "rb")
								g = BinaryReader(file)
								g.seek(int(Offset))
								for m in range(int(Size)):
									value = g.f(ItemSize)
									if ItemSize == 1:
										value = value[0]
									if bone:
										bone.rotFrameList.append(int(value * 33))
								file.close()

			QuatSlerpChannelCompressedPacked = ys.get(
				a, '"osgAnimation.QuatSlerpChannelCompressedPacked"'
			)
			if QuatSlerpChannelCompressedPacked:

				atributes = {}
				UserDataContainer = ys.get(
					QuatSlerpChannelCompressedPacked[0], '"UserDataContainer"'
				)
				if UserDataContainer:
					Values = ys.get(UserDataContainer[0], '"Values"')
					if Values:
						for child in Values[0].children:
							values = ys.values(child.data, ":")
							Name = ys.getValue(values, '"Name"')
							Value = ys.getValue(values, '"Value"', '"f"')
							##write(log,[Name,Value],n+4)
							atributes[Name] = Value

				KeyFrames = ys.get(a, '"KeyFrames"')
				if KeyFrames:
					values = ys.values(KeyFrames[0].header, ":")
					Name = ys.getValue(values, '"Name"')
					TargetName = ys.getValue(values, '"TargetName"', '""')
					# write(log,['QuatSlerpChannelCompressedPacked:',Name,'TargetName:',TargetName],n+4)
					name = getSplitName(TargetName, "_", -1)
					if name in boneIndeksList:
						name = boneIndeksList[name]
						if name not in boneList.keys():
							bone = ActionBone()
							action.boneList.append(bone)
							bone.name = name
							boneList[name] = bone
						bone = boneList[name]

					Key = ys.get(a, '"Key"')
					if Key:
						values = ys.values(Key[0].data, ":")
						ItemSize = int(ys.getValue(values, '"ItemSize"'))
						Uint16Array = ys.get(Key[0], '"Uint16Array"')
						type = "Uint16Array"
						if Uint16Array:
							values = ys.values(Uint16Array[0].data, ":")
							File = ys.getValue(values, '"File"')
							Size = int(ys.getValue(values, '"Size"'))
							Offset = int(ys.getValue(values, '"Offset"'))
							Encoding = ys.getValue(values, '"Encoding"')
							# write(log,[File,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding,'ItemSize:',ItemSize],n+4)
							path = File
							if os.path.exists(path):
								file = open(path, "rb")
								g = BinaryReader(file)

								list = decodeVarint(g, Offset, Size * ItemSize, type)
								##write(log,list,0)
								list1 = etap1(list, ItemSize)
								##write(log,list1,0)

								list2 = int3float4(list1, atributes, ItemSize)
								##write(log,list2,0)
								list3 = [
									atributes['"ox"'],
									atributes['"oy"'],
									atributes['"oz"'],
									atributes['"ow"'],
								]
								list3.extend(list2)
								list4 = etap4(list3)
								##write(log,list4,0)

								for m in range(Size + 1):
									value = list4[m * 4 : m * 4 + 4]
									value = Quaternion(value)
									##write(log,value,n+8)
									if bone:
										boneMatrix = (
											skeleton.object.getData()
											.bones[bone.name]
											.matrix["ARMATURESPACE"]
										)
										##bone.rotKeyList.append((boneMatrix.rotationPart()*QuatMatrix(value)).resize4x4())
										bone.rotKeyList.append(
											boneMatrix * QuatMatrix(value).resize4x4()
										)
								file.close()

					Time = ys.get(a, '"Time"')
					if Time:
						values = ys.values(Time[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"', "i")
						Float32Array = ys.get(Time[0], '"Float32Array"')
						if Float32Array:
							values = ys.values(Float32Array[0].data, ":")
							File = ys.getValue(values, '"File"')
							Size = ys.getValue(values, '"Size"', "i")
							Offset = ys.getValue(values, '"Offset"', "i")
							# write(log,[File,'Size:',Size,'Offset:',Offset,'ItemSize:',ItemSize],n+4)
							path = File
							if os.path.exists(path):
								file = open(path, "rb")
								g = BinaryReader(file)
								g.seek(int(Offset))
								list = g.f(Size * ItemSize)
								list1 = etap1(list, ItemSize)
								# out=etap2(list1,ItemSize,atributes)
								list2 = [atributes['"ot"']]
								list2.extend(list1)
								list3 = etap3(list2, ItemSize)
								##write(log,list3,0)
								for m in range(Size + 1):
									value = list2[m]
									if bone:
										bone.rotFrameList.append(int(value * 33))
								file.close()

			if bone:
				print name, bone.name

	action.draw()
	action.setContext()


def getPrimitiveSetList(ys, PrimitiveSetList, n):
	global magic
	mode = None
	magic = 0
	indiceArray = []
	for child in PrimitiveSetList[0].children:
		for b in child.children:
			if '"DrawElementsUInt"' in b.header:
				values = ys.values(b.data, ":")
				mode = values['"Mode"']
				Size = None
				Offset = None
				Encoding = None
				ItemSize = None
				type = None
				if mode != '"LINES"':
					Indices = ys.get(b, '"Indices"')
					if Indices:
						values = ys.values(Indices[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"', "i")
						Uint32Array = ys.get(Indices[0], '"Uint32Array"')
						type = "Uint32Array"
						print "DrawElementsUInt", type
						if Uint32Array:
							values = ys.values(Uint32Array[0].data, ":")
							Size = ys.getValue(values, '"Size"', "i")
							Offset = ys.getValue(values, '"Offset"', "i")
							Encoding = ys.getValue(values, '"Encoding"', '""')
							# write(log,['Indice:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding,'magic:',magic],n)
							if Encoding == "varint":
								path = "./BinFiles/model_file.bin.gz"
								if os.path.exists(path) == False:
									path = "./BinFiles/model_file.bin"
								if os.path.exists(path) == True:
									file = open(path, "rb")
									g = BinaryReader(file)
									magic, indiceList = getIndices(
										ItemSize, Size, Offset, type, g, mode, magic
									)
									indiceArray.append([indiceList, mode])
									file.close()
				else:
					print "LINES"

			if '"DrawElementsUShort"' in b.header:
				values = ys.values(b.data, ":")
				mode = values['"Mode"']
				Size = None
				Offset = None
				Encoding = None
				ItemSize = None
				type = None
				if mode != '"LINES"':
					Indices = ys.get(b, '"Indices"')
					if Indices:
						values = ys.values(Indices[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"', "i")
						Uint16Array = ys.get(Indices[0], '"Uint16Array"')
						type = "Uint16Array"
						print "DrawElementsUShort", type
						if Uint16Array:
							values = ys.values(Uint16Array[0].data, ":")
							Size = ys.getValue(values, '"Size"', "i")
							Offset = ys.getValue(values, '"Offset"', "i")
							Encoding = ys.getValue(values, '"Encoding"', '""')
							# write(log,['Indice:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding,'magic:',magic],n)
							print Encoding
							if Encoding == "varint":
								path = "./BinFiles/model_file.bin.gz"
								if os.path.exists(path) == False:
									path = "./BinFiles/model_file.bin"

								if os.path.exists(path) == True:
									file = open(path, "rb")
									g = BinaryReader(file)
									magic, indiceList = getIndices(
										ItemSize, Size, Offset, type, g, mode, magic
									)
									indiceArray.append([indiceList, mode])
									file.close()
							else:
								path = "./BinFiles/model_file.bin.gz"
								if os.path.exists(path) == False:
									path = "./BinFiles/model_file.bin"

								if os.path.exists(path) == True:
									file = open(path, "rb")
									g = BinaryReader(file)
									g.seek(Offset)
									indiceList = g.H(ItemSize * Size)
									indiceArray.append([indiceList, mode])
									file.close()
				else:
					print "LINES"

			if '"DrawElementsUByte"' in b.header:
				values = ys.values(b.data, ":")
				mode = values['"Mode"']
				Size = None
				Offset = None
				Encoding = None
				ItemSize = None
				type = None
				if mode != '"LINES"':
					Indices = ys.get(b, '"Indices"')
					if Indices:
						values = ys.values(Indices[0].data, ":")
						ItemSize = ys.getValue(values, '"ItemSize"', "i")
						Uint8Array = ys.get(Indices[0], '"Uint8Array"')
						type = "Uint8Array"
						print "DrawElementsUByte", type
						if Uint8Array:
							values = ys.values(Uint8Array[0].data, ":")
							Size = ys.getValue(values, '"Size"', "i")
							Offset = ys.getValue(values, '"Offset"', "i")
							Encoding = ys.getValue(values, '"Encoding"', '""')
							# write(log,['Indice:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding,'magic:',magic],n)
							path = "./BinFiles/model_file.bin.gz"
							if os.path.exists(path) == False:
								path = "./BinFiles/model_file.bin"

							if os.path.exists(path) == True:
								file = open(path, "rb")
								g = BinaryReader(file)
								magic, indiceList = getIndices(
									ItemSize, Size, Offset, type, g, mode, magic
								)
								indiceArray.append([indiceList, mode])
								file.close()
				else:
					print "LINES"

	return indiceArray


def getPath(File):
	path = "./BinFiles/model_file.bin"
	if os.path.exists(path) == False:
		path = os.path.dirname(input.filename) + os.sep + File
	if os.path.exists(path) == True:
		return path
	else:
		return None

def readFloats(br,offset,size,itemSize):
	if br.inputFile.mode=='rb':
		br.seek(offset)
		i = 0
		numbers = []
		while i < size * itemSize:
			numbers.append(float(struct.unpack('<f',br.inputFile.read(4))[0]))
			i += 1
		return numbers

def getVertexAttributeList(ys, VertexAttributeList, n):
	vertexArray = []
	texArray0 = []
	texArray1 = []
	texArray2 = []
	texArray3 = []
	texArray4 = []
	texArray5 = []
	texArray6 = []
	texArray7 = []
	texArray8 = []
	texArray9 = []
	texArray10 = []
	tangentArray = []
	normalArray = []

	try:
		Normal = ys.get(VertexAttributeList[0], '"Normal"')
		mode = "Normal"
		for b in Normal:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Uint32Array = ys.get(b, '"Uint32Array"')
				if Uint32Array:
					type = "Uint32Array"
					print mode, type
					values = ys.values(Uint32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['Normal:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							normalArray.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['Vertex:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							global modelID
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = readFloats(g, Offset, Size, ItemSize)
							with open("./BinFiles/vertexNormalData/" + str(modelID) + ".dat", "a") as modelData:
								modelID += 1
								for line in bytes:
									modelData.write(str(line) + "\n")
							normalArray.append([bytes, Encoding, ItemSize])
							file.close()
	except:
		pass

	try:
		Tangent = ys.get(VertexAttributeList[0], '"Tangent"')
		mode = "Tangent"
		for b in Tangent:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Uint32Array = ys.get(b, '"Uint32Array"')
				if Uint32Array:
					type = "Uint32Array"
					print mode, type
					values = ys.values(Uint32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['Tangent:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							tangentArray.append([bytes, Encoding, ItemSize])
							file.close()

				'''Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['Vertex:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								list.append(bytes[m * ItemSize : m * ItemSize + ItemSize])
							vertexArray.append([list, Encoding])
							file.close()'''
	except:
		pass

	Vertex = ys.get(VertexAttributeList[0], '"Vertex"')
	mode = "Vertex"
	for b in Vertex:
		Size = None
		Offset = None
		Encoding = None
		ItemSize = None
		type = None
		values = ys.values(b.data, ":")
		if '"ItemSize"' in values:
			ItemSize = int(values['"ItemSize"'])
			Int32Array = ys.get(b, '"Int32Array"')
			if Int32Array:
				type = "Int32Array"
				print mode, type
				values = ys.values(Int32Array[0].data, ":")
				Size = ys.getValue(values, '"Size"', "i")
				Offset = ys.getValue(values, '"Offset"', "i")
				File = ys.getValue(values, '"File"', '""')
				Encoding = ys.getValue(values, '"Encoding"')
				print ['Vertex:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
				if Encoding == '"varint"':
					path = getPath(File)
					if path:
						file = open(path, "rb")
						g = BinaryReader(file)
						bytes = decodeVarint(g, Offset, Size * ItemSize, type)
						vertexArray.append([bytes, Encoding, ItemSize])
						file.close()

			Float32Array = ys.get(b, '"Float32Array"')
			if Float32Array:
				type = "Float32Array"
				print mode, type
				values = ys.values(Float32Array[0].data, ":")
				Size = ys.getValue(values, '"Size"', "i")
				Offset = ys.getValue(values, '"Offset"', "i")
				File = ys.getValue(values, '"File"', '""')
				Encoding = ys.getValue(values, '"Encoding"')
				print ['Vertex:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
				if Encoding != '"varint"':
					path = getPath(File)
					if path:
						file = open(path, "rb")
						g = BinaryReader(file)
						g.seek(Offset)
						bytes = g.f(Size * ItemSize)
						list = []
						for m in range(Size):
							list.append(bytes[m * ItemSize : m * ItemSize + ItemSize])
						vertexArray.append([list, Encoding])
						file.close()

	TexCoord0 = ys.get(VertexAttributeList[0], '"TexCoord0"')
	if TexCoord0:
		mode = "TexCoord0"
		for b in TexCoord0:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord0:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray0.append([bytes, Encoding, ItemSize])
							texArray0.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord0:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray0.append([list, Encoding])
							file.close()

	TexCoord1 = ys.get(VertexAttributeList[0], '"TexCoord1"')
	if TexCoord1:
		mode = "TexCoord1"
		for b in TexCoord1:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord1:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray1.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord1:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray1.append([list, Encoding])
							file.close()

	TexCoord2 = ys.get(VertexAttributeList[0], '"TexCoord2"')
	if TexCoord2:
		mode = "TexCoord2"
		for b in TexCoord2:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord2:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray2.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord2:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray2.append([list, Encoding])
							file.close()

	TexCoord3 = ys.get(VertexAttributeList[0], '"TexCoord3"')
	if TexCoord3:
		mode = "TexCoord3"
		for b in TexCoord3:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord3:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray3.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord3:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray3.append([list, Encoding])
							file.close()

	TexCoord4 = ys.get(VertexAttributeList[0], '"TexCoord4"')
	if TexCoord4:
		mode = "TexCoord4"
		for b in TexCoord4:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord4:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray4.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord4:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray4.append([list, Encoding])
							file.close()

	TexCoord5 = ys.get(VertexAttributeList[0], '"TexCoord5"')
	if TexCoord5:
		mode = "TexCoord5"
		for b in TexCoord5:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord5:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray5.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord5:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray5.append([list, Encoding])
							file.close()

	TexCoord6 = ys.get(VertexAttributeList[0], '"TexCoord6"')
	if TexCoord6:
		mode = "TexCoord6"
		for b in TexCoord6:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord6:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray6.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord6:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray6.append([list, Encoding])
							file.close()

	TexCoord7 = ys.get(VertexAttributeList[0], '"TexCoord7"')
	if TexCoord7:
		mode = "TexCoord7"
		for b in TexCoord7:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord7:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray7.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord7:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray7.append([list, Encoding])
							file.close()

	TexCoord8 = ys.get(VertexAttributeList[0], '"TexCoord8"')
	if TexCoord8:
		mode = "TexCoord8"
		for b in TexCoord8:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord8:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray8.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord8:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray8.append([list, Encoding])
							file.close()

	TexCoord9 = ys.get(VertexAttributeList[0], '"TexCoord9"')
	if TexCoord9:
		mode = "TexCoord9"
		for b in TexCoord9:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord9:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray9.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord9:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray9.append([list, Encoding])
							file.close()

	TexCoord10 = ys.get(VertexAttributeList[0], '"TexCoord10"')
	if TexCoord10:
		mode = "TexCoord10"
		for b in TexCoord10:
			Size = None
			Offset = None
			Encoding = None
			ItemSize = None
			type = None
			values = ys.values(b.data, ":")
			if '"ItemSize"' in values:
				ItemSize = int(values['"ItemSize"'])
				Int32Array = ys.get(b, '"Int32Array"')
				if Int32Array:
					type = "Int32Array"
					print mode, type
					values = ys.values(Int32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					print ['TexCoord10:','Mode:',mode,'Type:',type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding]
					if Encoding == '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							bytes = decodeVarint(g, Offset, Size * ItemSize, type)
							texArray10.append([bytes, Encoding, ItemSize])
							file.close()

				Float32Array = ys.get(b, '"Float32Array"')
				if Float32Array:
					type = "Float32Array"
					print mode, type
					values = ys.values(Float32Array[0].data, ":")
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					File = ys.getValue(values, '"File"', '""')
					Encoding = ys.getValue(values, '"Encoding"')
					# write(log,['TexCoord10:','mode:',mode,type,'Size:',Size,'Offset:',Offset,'Encoding:',Encoding],n)
					if Encoding != '"varint"':
						path = getPath(File)
						if path:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							bytes = g.f(Size * ItemSize)
							list = []
							for m in range(Size):
								u, v = bytes[m * ItemSize : m * ItemSize + ItemSize]
								list.append([u, 1 - v])
							texArray10.append([list, Encoding])
							file.close()

	return vertexArray, texArray0, texArray1, texArray2, texArray3, texArray4, texArray5, texArray6, texArray7, texArray8, texArray9, texArray10, normalArray

def getRigGeometry(ys, parent, n):
	print "#" * 50, "RigGeometry"
	n += 4
	BoneMap = [0] * 1000
	bones = []
	weights = []
	mode = None
	indiceArray = []
	vertexArray = []
	texArray0 = []
	texArray1 = []
	texArray2 = []
	texArray3 = []
	texArray4 = []
	texArray5 = []
	texArray6 = []
	texArray7 = []
	texArray8 = []
	texArray9 = []
	texArray10 = []
	normArray = []
	atributes = {}
	
	Name1 = ''
	for child in parent.children:
		if '"SourceGeometry"' in child.header:
			for C2 in child.children:
				if '"osg.Geometry"' in C2.header:
					for C3 in C2.children:
						if '"Name"' in C3.header:
							values = ys.values(C3.header, ":")
							Name1 = ys.getValue(values, '"Name"').replace('"','')
							#print '.....................'
							#print Name1
	
	#print 'vVvVvVvVvVvVvVvVvVvVvVvVvVvVvVvVvVvVvVvV'
	#print Name1

	Name2 = ''
	for child in parent.children:
		if '"SourceGeometry"' in child.header:
			for C2 in child.children:
				if '"osg.Geometry"' in C2.header:
					for C3 in C2.children:
						if '"StateSet"' in C3.header:
							C4 = C3.children[0]
							if '"Name"' in C4.header:
								values1=ys.values(C4.header,':')
								Name2 = ys.getValue(values1,'"Name"','""')
							C5 = C4.children[0]
							if '"Name"' in C5.header:
								values1=ys.values(C5.header,':')
								Name2 = ys.getValue(values1,'"Name"','""')
							C6 = C5.children[0]
							if '"Name"' in C6.header:
								values1=ys.values(C6.header,':')
								Name2 = ys.getValue(values1,'"Name"','""')
							C7 = C6.children[0]
							if '"Name"' in C7.header:
								values1=ys.values(C7.header,':')
								Name2 = ys.getValue(values1,'"Name"','""')
							C00 = C7.children[0]
							if '"Name"' in C00.header:
								values1=ys.values(C00.header,':')
								Name2 = ys.getValue(values1,'"Name"','""')
							#values2 = ys.values(C00.header, ":")
							#Name2 = ys.getValue(values2, '"Name"').replace('"','')
							
							#print '+++++++++++ ' + Name2
							#with open("sample.txt", "a") as file_object:
							#	file_object.write(C00.header)
							#	file_object.close()	

	for child in parent.children:
		if "BoneMap" in child.header:
			# write(log,['BoneMap'],n)
			values = ys.values(child.data, ":")
			# print values
			for value in values:
				id = ys.getValue(values, value, "i")
				name = value.replace('"', "")
				BoneMap[id] = getSplitName(name, "_", -1)
		if "SourceGeometry" in child.header:
			values = ys.values(child.data, ":")
			PrimitiveSetList = ys.get(child, '"PrimitiveSetList"')
			if PrimitiveSetList:
				indiceArray = getPrimitiveSetList(ys, PrimitiveSetList, n)

			UserDataContainer = ys.get(child, '"UserDataContainer"')
			if UserDataContainer:
				for UserData in UserDataContainer:
					Values = ys.get(UserData, '"Values"')
					if Values:
						for a in Values[0].children:
							values = ys.values(a.data, ":")
							Name = ys.getValue(values, '"Name"', '""')
							Value = ys.getValue(values, '"Value"', '""')
							if Name:
								atributes[Name] = Value

			VertexAttributeList = ys.get(child, '"VertexAttributeList"')
			if VertexAttributeList:
				vertexArray, texArray0, texArray1, texArray2, texArray3, texArray4, texArray5, texArray6, texArray7, texArray8, texArray9, texArray10, normArray = getVertexAttributeList(
					ys, VertexAttributeList, n
				)

		if "UserDataContainer" in child.header:
			# write(log,['UserDataContainer'],n)
			Values = ys.get(child, '"Values"')
			if Values:
				for a in Values[0].children:
					values = ys.values(a.data, ":")
					for value in values:
						id = ys.getValue(values, value)
						# write(log,[value,':',id],n+4)
		if "VertexAttributeList" in child.header:
			# write(log,['VertexAttributeList'],n)
			Bones = ys.get(child, '"Bones"')
			if Bones:
				# write(log,['Bones'],n+4)
				values = ys.values(Bones[0].data, ":")
				ItemSize = ys.getValue(values, '"ItemSize"', "i")
				# write(log,['"ItemSize"',':',ItemSize],n+8)
				Uint16Array = ys.get(Bones[0], '"Uint16Array"')
				if Uint16Array:
					type = "Uint16Array"
					values = ys.values(Uint16Array[0].data, ":")
					File = ys.getValue(values, '"File"', '""')
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					Encoding = ys.getValue(values, '"Encoding"', '""')
					# write(log,['"File"',':',File],n+8)
					# write(log,['"Size"',':',Size],n+8)
					# write(log,['"Offset"',':',Offset],n+8)
					# write(log,['"Encoding"',':',Encoding],n+8)

					if Encoding == "varint":
						path = (
							os.path.dirname(ys.filename)
							+ os.sep
							+ "./BinFiles/model_file.bin.gz.txt"
						)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ "./BinFiles/model_file.bin"
							)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ values['"File"'].split('"')[1]
							)  # +'.txt'
						if os.path.exists(path) == True:
							file = open(path, "rb")
							g = BinaryReader(file)
							list = decodeVarint(g, Offset, Size * ItemSize, type)
							##write(log,list,0)
							for m in range(Size):
								bones.append(
									list[m * ItemSize : m * ItemSize + ItemSize]
								)
							file.close()

			Weights = ys.get(child, '"Weights"')
			if Weights:
				# write(log,['Weights'],n+4)
				values = ys.values(Weights[0].data, ":")
				ItemSize = ys.getValue(values, '"ItemSize"', "i")
				# write(log,['"ItemSize"',':',ItemSize],n+8)
				Float32Array = ys.get(Weights[0], '"Float32Array"')
				if Float32Array:
					values = ys.values(Float32Array[0].data, ":")
					File = ys.getValue(values, '"File"', '""')
					Size = ys.getValue(values, '"Size"', "i")
					Offset = ys.getValue(values, '"Offset"', "i")
					Encoding = ys.getValue(values, '"Encoding"', '""')
					# write(log,['"File"',':',File],n+8)
					# write(log,['"Size"',':',Size],n+8)
					# write(log,['"Offset"',':',Offset],n+8)
					# write(log,['"Encoding"',':',Encoding],n+8)

					if Encoding == "varint":
						path = (
							os.path.dirname(ys.filename)
							+ os.sep
							+ "./BinFiles/model_file.bin.gz.txt"
						)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ "./BinFiles/model_file.bin"
							)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ values['"File"'].split('"')[1]
							)  # +'.txt'
						if os.path.exists(path) == True:
							file = open(path, "rb")
							g = BinaryReader(file)
							list = decodeVarint(g, Offset, Size * ItemSize, type)
							##write(log,list,0)
							file.close()
					else:
						path = (
							os.path.dirname(ys.filename)
							+ os.sep
							+ "./BinFiles/model_file.bin.gz.txt"
						)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ "./BinFiles/model_file.bin"
							)
						if os.path.exists(path) == False:
							path = (
								os.path.dirname(ys.filename)
								+ os.sep
								+ values['"File"'].split('"')[1]
							)  # +'.txt'
						if os.path.exists(path) == True:
							file = open(path, "rb")
							g = BinaryReader(file)
							g.seek(Offset)
							list = g.f(Size * ItemSize)
							##write(log,list,0)
							for m in range(Size):
								weights.append(
									list[m * ItemSize : m * ItemSize + ItemSize]
								)
							file.close()

	# print atributes
	print "* * * * * * * * * * * * * * * * * * * * * *"
	print Name1
	print Name2
	mesh = Mesh(Name2,Name1)
	if len(bones) > 0 and len(Weights) > 0:
		mesh.BoneMap = BoneMap
		skin = Skin()
		mesh.skinList.append(skin)
		mesh.skinIndiceList = bones
		mesh.skinWeightList = weights
	if len(indiceArray) > 0:
		for [indices, mode] in indiceArray:
			print mode, len(indices)
			mat = Mat()
			mesh.matList.append(mat)
			mat.IDStart = len(mesh.indiceList)
			mat.IDCount = len(indices)
			mesh.indiceList.extend(indices)
			if mode == '"TRIANGLE_STRIP"':
				mat.TRISTRIP = True
			if mode == '"TRIANGLES"':
				mat.TRIANGLE = True

		indices = indiceArray[0][0]
		mode = indiceArray[0][1]
		
		if len(vertexArray) == 1:
			if vertexArray[0][1] == '"varint"':
				bytes = vertexArray[0][0]
				ItemSize = vertexArray[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["vtx_bbl_x"])
				s2 = float(atributes["vtx_bbl_y"])
				s3 = float(atributes["vtx_bbl_z"])
				s = [s1, s2, s3]
				a1 = float(atributes["vtx_h_x"])
				a2 = float(atributes["vtx_h_y"])
				a3 = float(atributes["vtx_h_z"])
				a = [a1, a2, a3]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				mesh.vertPosList = [
					floats[m : m + ItemSize] for m in range(0, len(floats), 3)
				]
			else:
				list = vertexArray[0][0]
				mesh.vertPosList = list
		
		uvLists = []
		if len(texArray0) >= 1:
			if texArray0[0][1] == '"varint"':
				bytes = texArray0[0][0]
				ItemSize = texArray0[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_0_bbl_x"])
				s2 = float(atributes["uv_0_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_0_h_x"])
				a2 = float(atributes["uv_0_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray0[0][0])
		else:
			uvLists.append([])
		if len(texArray1) >= 1:
			if texArray1[0][1] == '"varint"':
				bytes = texArray1[0][0]
				ItemSize = texArray1[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_1_bbl_x"])
				s2 = float(atributes["uv_1_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_1_h_x"])
				a2 = float(atributes["uv_1_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray1[0][0])
		else:
			uvLists.append([])
		if len(texArray2) >= 1:
			if texArray2[0][1] == '"varint"':
				bytes = texArray2[0][0]
				ItemSize = texArray2[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_2_bbl_x"])
				s2 = float(atributes["uv_2_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_2_h_x"])
				a2 = float(atributes["uv_2_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray2[0][0])
		else:
			uvLists.append([])
		if len(texArray3) >= 1:
			if texArray3[0][1] == '"varint"':
				bytes = texArray3[0][0]
				ItemSize = texArray3[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_3_bbl_x"])
				s2 = float(atributes["uv_3_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_3_h_x"])
				a2 = float(atributes["uv_3_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray3[0][0])
		else:
			uvLists.append([])
		if len(texArray4) >= 1:
			if texArray4[0][1] == '"varint"':
				bytes = texArray4[0][0]
				ItemSize = texArray4[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_4_bbl_x"])
				s2 = float(atributes["uv_4_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_4_h_x"])
				a2 = float(atributes["uv_4_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray4[0][0])
		else:
			uvLists.append([])
		if len(texArray5) >= 1:
			if texArray5[0][1] == '"varint"':
				bytes = texArray5[0][0]
				ItemSize = texArray5[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_5_bbl_x"])
				s2 = float(atributes["uv_5_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_5_h_x"])
				a2 = float(atributes["uv_5_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray5[0][0])
		else:
			uvLists.append([])
		if len(texArray6) >= 1:
			if texArray6[0][1] == '"varint"':
				bytes = texArray6[0][0]
				ItemSize = texArray6[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_6_bbl_x"])
				s2 = float(atributes["uv_6_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_6_h_x"])
				a2 = float(atributes["uv_6_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray6[0][0])
		else:
			uvLists.append([])
		if len(texArray7) >= 1:
			if texArray7[0][1] == '"varint"':
				bytes = texArray7[0][0]
				ItemSize = texArray7[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_7_bbl_x"])
				s2 = float(atributes["uv_7_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_7_h_x"])
				a2 = float(atributes["uv_7_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray7[0][0])
		else:
			uvLists.append([])
		if len(texArray8) >= 1:
			if texArray8[0][1] == '"varint"':
				bytes = texArray8[0][0]
				ItemSize = texArray8[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_8_bbl_x"])
				s2 = float(atributes["uv_8_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_8_h_x"])
				a2 = float(atributes["uv_8_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray8[0][0])
		else:
			uvLists.append([])
		if len(texArray9) >= 1:
			if texArray9[0][1] == '"varint"':
				bytes = texArray9[0][0]
				ItemSize = texArray9[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_9_bbl_x"])
				s2 = float(atributes["uv_9_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_9_h_x"])
				a2 = float(atributes["uv_9_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray9[0][0])
		else:
			uvLists.append([])
		if len(texArray10) >= 1:
			if texArray10[0][1] == '"varint"':
				bytes = texArray10[0][0]
				ItemSize = texArray10[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_10_bbl_x"])
				s2 = float(atributes["uv_10_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_10_h_x"])
				a2 = float(atributes["uv_10_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray10[0][0])
		else:
			uvLists.append([])

		if len(uvLists) >= 1:
			for list in uvLists:
				mesh.vertUVLists.append(list)
	return mesh


def decodeQuantize(input, s, a, itemsize):
	x = [0] * len(input)
	id = 0
	for r in range(len(input) / itemsize):
		for l in range(itemsize):
			x[id] = s[l] + input[id] * a[l]
			id += 1
	return x


def getGeometry(ys, parent, n):
	Name1 = ''
	for child in parent.children:
		values = ys.values(child.header, ":")
		Name = ys.getValue(values, '"Name"', '""')
		if Name:
			Name1 = Name.replace('"','')
	#print 'vVvVvVvV'
	#print Name1

	Name2 = ''
	for child in parent.children:
		if '"StateSet"' in child.header:
			C4 = child.children[0]
			C5 = C4.children[0]
			C6 = C5.children[0]
			C7 = C6.children[0]
			C00 = C7.children[0]
			values2 = ys.values(C00.header, ":")
			Name = ys.getValue(values2, '"Name"')
			if Name:
				Name2 = Name.replace('"','')
	
	print "#" * 50, "Geometry"
	n += 4
	mode = None
	indiceArray = []
	vertexArray = []
	texArray0 = []
	texArray1 = []
	texArray2 = []
	texArray3 = []
	texArray4 = []
	texArray5 = []
	texArray6 = []
	texArray7 = []
	texArray8 = []
	texArray9 = []
	texArray10 = []
	normArray = []
	atributes = {}

	# write(log,['Geometry'],n)
	PrimitiveSetList = ys.get(parent, '"PrimitiveSetList"')
	if PrimitiveSetList:
		indiceArray = getPrimitiveSetList(ys, PrimitiveSetList, n)

	UserDataContainer = ys.get(parent, '"UserDataContainer"')
	if UserDataContainer:
		for UserData in UserDataContainer:
			Values = ys.get(UserData, '"Values"')
			if Values:
				for a in Values[0].children:
					values = ys.values(a.data, ":")
					Name = ys.getValue(values, '"Name"', '""')
					Value = ys.getValue(values, '"Value"', '""')
					# if Name:#write(log,[Name,Value],n+4)
					if Name:
						atributes[Name] = Value

	VertexAttributeList = ys.get(parent, '"VertexAttributeList"')
	if VertexAttributeList:
		vertexArray, texArray0, texArray1, texArray2, texArray3, texArray4, texArray5, texArray6, texArray7, texArray8, texArray9, texArray10, normArray = getVertexAttributeList(ys, VertexAttributeList, n)

	# print atributes
	#print '+++++++++++++++'
	#print Name1
	#print Name2
	#print '+++++++++++++++'
	print "* * * * * * * * * * * * * * * * * * * * * *"
	print Name1
	print Name2
	mesh = Mesh(Name2,Name1)
	if len(indiceArray) > 0:
		for [indices, mode] in indiceArray:
			print mode, len(indices)
			mat = Mat()
			mesh.matList.append(mat)
			mat.IDStart = len(mesh.indiceList)
			mat.IDCount = len(indices)
			mesh.indiceList.extend(indices)
			if mode == '"TRIANGLE_STRIP"':
				mat.TRISTRIP = True
			if mode == '"TRIANGLES"':
				mat.TRIANGLE = True

		indices = indiceArray[0][0]
		mode = indiceArray[0][1]
		
		if len(vertexArray) == 1:
			if vertexArray[0][1] == '"varint"':
				bytes = vertexArray[0][0]
				ItemSize = vertexArray[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["vtx_bbl_x"])
				s2 = float(atributes["vtx_bbl_y"])
				s3 = float(atributes["vtx_bbl_z"])
				s = [s1, s2, s3]
				a1 = float(atributes["vtx_h_x"])
				a2 = float(atributes["vtx_h_y"])
				a3 = float(atributes["vtx_h_z"])
				a = [a1, a2, a3]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				mesh.vertPosList = [
					floats[m : m + ItemSize] for m in range(0, len(floats), 3)
				]
			else:
				list = vertexArray[0][0]
				mesh.vertPosList = list
		
		uvLists = []
		if len(texArray0) >= 1:
			if texArray0[0][1] == '"varint"':
				bytes = texArray0[0][0]
				ItemSize = texArray0[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_0_bbl_x"])
				s2 = float(atributes["uv_0_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_0_h_x"])
				a2 = float(atributes["uv_0_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray0[0][0])
		else:
			uvLists.append([])
		if len(texArray1) >= 1:
			if texArray1[0][1] == '"varint"':
				bytes = texArray1[0][0]
				ItemSize = texArray1[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_1_bbl_x"])
				s2 = float(atributes["uv_1_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_1_h_x"])
				a2 = float(atributes["uv_1_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray1[0][0])
		else:
			uvLists.append([])
		if len(texArray2) >= 1:
			if texArray2[0][1] == '"varint"':
				bytes = texArray2[0][0]
				ItemSize = texArray2[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_2_bbl_x"])
				s2 = float(atributes["uv_2_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_2_h_x"])
				a2 = float(atributes["uv_2_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray2[0][0])
		else:
			uvLists.append([])
		if len(texArray3) >= 1:
			if texArray3[0][1] == '"varint"':
				bytes = texArray3[0][0]
				ItemSize = texArray3[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_3_bbl_x"])
				s2 = float(atributes["uv_3_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_3_h_x"])
				a2 = float(atributes["uv_3_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray3[0][0])
		else:
			uvLists.append([])
		if len(texArray4) >= 1:
			if texArray4[0][1] == '"varint"':
				bytes = texArray4[0][0]
				ItemSize = texArray4[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_4_bbl_x"])
				s2 = float(atributes["uv_4_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_4_h_x"])
				a2 = float(atributes["uv_4_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray4[0][0])
		else:
			uvLists.append([])
		if len(texArray5) >= 1:
			if texArray5[0][1] == '"varint"':
				bytes = texArray5[0][0]
				ItemSize = texArray5[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_5_bbl_x"])
				s2 = float(atributes["uv_5_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_5_h_x"])
				a2 = float(atributes["uv_5_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray5[0][0])
		else:
			uvLists.append([])
		if len(texArray6) >= 1:
			if texArray6[0][1] == '"varint"':
				bytes = texArray6[0][0]
				ItemSize = texArray6[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_6_bbl_x"])
				s2 = float(atributes["uv_6_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_6_h_x"])
				a2 = float(atributes["uv_6_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray6[0][0])
		else:
			uvLists.append([])
		if len(texArray7) >= 1:
			if texArray7[0][1] == '"varint"':
				bytes = texArray7[0][0]
				ItemSize = texArray7[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_7_bbl_x"])
				s2 = float(atributes["uv_7_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_7_h_x"])
				a2 = float(atributes["uv_7_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray7[0][0])
		else:
			uvLists.append([])
		if len(texArray8) >= 1:
			if texArray8[0][1] == '"varint"':
				bytes = texArray8[0][0]
				ItemSize = texArray8[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_8_bbl_x"])
				s2 = float(atributes["uv_8_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_8_h_x"])
				a2 = float(atributes["uv_8_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray8[0][0])
		else:
			uvLists.append([])
		if len(texArray9) >= 1:
			if texArray9[0][1] == '"varint"':
				bytes = texArray9[0][0]
				ItemSize = texArray9[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_9_bbl_x"])
				s2 = float(atributes["uv_9_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_9_h_x"])
				a2 = float(atributes["uv_9_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray9[0][0])
		else:
			uvLists.append([])
		if len(texArray10) >= 1:
			if texArray10[0][1] == '"varint"':
				bytes = texArray10[0][0]
				ItemSize = texArray10[0][2]
				if mode == '"TRIANGLE_STRIP"':
					bytes = decodePredict(indices, bytes, ItemSize)
				s1 = float(atributes["uv_10_bbl_x"])
				s2 = float(atributes["uv_10_bbl_y"])
				s = [s1, s2]
				a1 = float(atributes["uv_10_h_x"])
				a2 = float(atributes["uv_10_h_y"])
				a = [a1, a2]
				floats = decodeQuantize(bytes, s, a, ItemSize)
				uvList = []
				for m in range(0, len(floats), ItemSize):
					u, v = floats[m : m + ItemSize]
					uvList.append([u, 1 - v])
				uvLists.append(uvList)
			else:
				uvLists.append(texArray10[0][0])
		else:
			uvLists.append([])

		if len(uvLists) >= 1:
			for list in uvLists:
				mesh.vertUVLists.append(list)
	return mesh


def getMatrixTransform(ys, parent, n, boneParent):
	# write(log,['MatrixTransform'],n)
	n += 4
	bone = Bone()
	bone.name = str(len(skeleton.boneList))
	skeleton.boneList.append(bone)
	bone.parentName = boneParent.name

	Name = None
	for child in parent.children:
		values = ys.values(child.header, ":")
		Name = ys.getValue(values, '"Name"', '""')
		if Name:
			Name = getSplitName(Name, "_", -1)
			# write(log,[Name],n)
			# if len(Name)<25:bone.name=Name
			boneIndeksList[Name] = bone.name

	for child in parent.children:
		if '"Matrix"' in child.header:
			floats = ys.values(child.data, "f")
			# write(log,floats,n)
			bone.matrix = Matrix4x4(floats)
			bone.matrix *= boneParent.matrix
	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, bone)


def getSkeletonNode(ys, parent, n, boneParent):
	global firstmatrix
	# write(log,['Skeleton'],n)
	n += 4
	bone = Bone()
	bone.name = str(len(skeleton.boneList))
	skeleton.boneList.append(bone)
	bone.parentName = boneParent.name

	firstmatrix = boneParent.matrix

	Name = None
	for child in parent.children:
		values = ys.values(child.header, ":")
		Name = ys.getValue(values, '"Name"', '""')
		if Name:
			Name = getSplitName(Name, "_", -1)
			# print Name
			# write(log,[Name],n)
			# if len(Name)<25:bone.name=Name
			boneIndeksList[Name] = bone.name

	for child in parent.children:
		if '"Matrix"' in child.header:
			floats = ys.values(child.data, "f")
			# write(log,floats,n)
			bone.matrix = Matrix4x4(floats)
			bone.matrix *= boneParent.matrix
	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, bone)


def getRigGeometryNode(ys, parent, n, boneParent):
	# write(log,['RigGeometry'],n)
	mesh = getRigGeometry(ys, parent, n)
	if len(mesh.vertPosList) > 0:
		model.meshList.append(mesh)
		mesh.matrix = boneParent.matrix
		mesh.draw()
		if mesh.object:
			if len(mesh.skinList) > 0:
				if BINDPOSE == 1:
					if bindskeleton.object and skeleton.object:
						mesh.object.getData(mesh=1).transform(mesh.matrix)
						mesh.object.getData(mesh=1).update()
						##mesh.object.setMatrix(mesh.matrix)
						bindPose(bindskeleton.object, skeleton.object, mesh.object)
						##mesh.object.setMatrix(mesh.matrix.invert()*mesh.object.matrixWorld)
						scene = bpy.data.scenes.active
						scene.objects.unlink(bindskeleton.object)
				else:
					if bindskeleton.object and skeleton.object:
						mesh.object.getData(mesh=1).transform(mesh.matrix)
						mesh.object.getData(mesh=1).update()

			else:
				mesh.object.setMatrix(mesh.matrix)
		print "Triangles: " + str(len(mesh.triangleList))

	n += 4
	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, boneParent)


def getGeometryNode(ys, parent, n, boneParent):
	# write(log,['Geometry'],n)
	mesh = getGeometry(ys, parent, n)
	if len(mesh.vertPosList) > 0:
		model.meshList.append(mesh)
		mesh.matrix = boneParent.matrix
		mesh.draw()
		if mesh.object:
			if len(mesh.skinList) > 0:
				if BINDPOSE == 1:
					if bindskeleton.object and skeleton.object:
						mesh.object.getData(mesh=1).transform(mesh.matrix)
						mesh.object.getData(mesh=1).update()
						##mesh.object.setMatrix(mesh.matrix)
						bindPose(bindskeleton.object, skeleton.object, mesh.object)
						##mesh.object.setMatrix(mesh.matrix.invert()*mesh.object.matrixWorld)
						scene = bpy.data.scenes.active
						scene.objects.unlink(bindskeleton.object)
				else:
					if bindskeleton.object and skeleton.object:
						mesh.object.getData(mesh=1).transform(mesh.matrix)
						mesh.object.getData(mesh=1).update()

			else:
				mesh.object.setMatrix(mesh.matrix)
		print "Triangles: " + str(len(mesh.triangleList))

	n += 4
	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, boneParent)


def getBoneNode(ys, parent, n, boneParent):
	# write(log,['Bone'],n)
	bone = Bone()
	bone.parentName = boneParent.name
	bone.name = str(len(skeleton.boneList))
	skeleton.boneList.append(bone)

	n += 4
	Name = None
	for child in parent.children:
		values = ys.values(child.header, ":")
		# print child.header
		Name = ys.getValue(values, '"Name"', '""')
		if Name:
			Name = getSplitName(Name, "_", -1)
			# write(log,[Name],n)
			# print Name
			# if len(Name)<25:bone.name=Name
			boneIndeksList[Name] = bone.name

	for child in parent.children:
		if '"Matrix"' in child.header:
			values = ys.values(child.header, ":")
			floats = ys.values(child.data, "f")
			bone.matrix = Matrix4x4(floats)
			bone.matrix *= boneParent.matrix

		if '"InvBindMatrixInSkeletonSpace"' in child.header:
			bindbone = Bone()
			# if Name:bindbone.name=Name
			bindbone.name = bone.name
			bindskeleton.boneList.append(bindbone)
			floats = ys.values(child.data, "f")
			# write(log,[floats],n+4)
			matrix = Matrix4x4(floats).invert()
			bindbone.matrix = matrix * firstmatrix

	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, bone)


def getChildren(ys, parent, n, boneParent):
	# write(log,['Children'],n)
	n += 4
	for child in parent.children:
		for a in child.children:
			if '"osg.MatrixTransform"' in a.header:
				getMatrixTransform(ys, a, n, boneParent)
			if '"osg.Node"' in a.header:
				getNode(ys, a, n, boneParent)
			if '"osgAnimation.Skeleton"' in a.header:
				getSkeletonNode(ys, a, n, boneParent)
			if '"osgAnimation.RigGeometry"' in a.header:
				getRigGeometryNode(ys, a, n, boneParent)
			if '"osg.Geometry"' in a.header:
				getGeometryNode(ys, a, n, boneParent)
			if '"osgAnimation.Bone"' in a.header:
				getBoneNode(ys, a, n, boneParent)


def getNode(ys, parent, n, boneParent):
	# write(log,['Node'],n)
	n += 4

	bone = Bone()
	bone.name = str(len(skeleton.boneList))
	skeleton.boneList.append(bone)
	bone.parentName = boneParent.name
	bone.matrix = boneParent.matrix

	Name = None
	for child in parent.children:
		values = ys.values(child.header, ":")
		Name = ys.getValue(values, '"Name"', '""')
		if Name:
			# Name=getSplitName(Name,'_',-1)
			# write(log,[Name],n)
			# if len(Name)<25:bone.name=Name
			boneIndeksList[Name] = bone.name

	for child in parent.children:
		if '"Children"' in child.header:
			getChildren(ys, child, n, bone)


def bindPose(bindSkeleton, poseSkeleton, meshObject):
	# print 'BINDPOSE'
	mesh = meshObject.getData(mesh=1)
	poseBones = poseSkeleton.getData().bones
	bindBones = bindSkeleton.getData().bones
	# mesh.transform(meshObject.matrixWorld)
	mesh.update()
	for vert in mesh.verts:
		index = vert.index
		skinList = mesh.getVertexInfluences(index)
		vco = vert.co.copy() * meshObject.matrixWorld
		vector = Vector()
		sum = 0
		for skin in skinList:
			bone = skin[0]
			weight = skin[1]
			if bone in bindBones.keys() and bone in poseBones.keys():
				matA = (
					bindBones[bone].matrix["ARMATURESPACE"] * bindSkeleton.matrixWorld
				)
				matB = (
					poseBones[bone].matrix["ARMATURESPACE"] * poseSkeleton.matrixWorld
				)
				vector += vco * matA.invert() * matB * weight
				sum += weight
			else:
				vector = vco
				break
		vert.co = vector
	mesh.update()
	Blender.Window.RedrawAll()


def osgParser(filename):
	global skeleton, bindskeleton, model, boneIndeksList, firstmatrix
	boneIndeksList = {}
	model = Model(filename)
	skeleton = Skeleton()
	skeleton.ARMATURESPACE = True
	bindskeleton = Skeleton()
	bindskeleton.NICE = True
	bindskeleton.ARMATURESPACE = True
	ys = Yson()
	ys.log = True
	ys.filename = filename
	ys.parse()

	firstmatrix = Matrix4x4([1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1])

	n = 0
	bone = Bone()
	bone.matrix = Matrix().resize4x4()
	bone.name = str(len(skeleton.boneList))
	bone.name = "scene"
	skeleton.boneList.append(bone)
	Node = ys.get(ys.root, '"osg.Node"')
	if Node:
		getNode(ys, Node[0], n, bone)
	if len(bindskeleton.boneList) > 0:
		bindskeleton.draw()

	for mesh in model.meshList:
		if len(mesh.skinList) > 0:
			for map in mesh.BoneMap:
				if map == 0:
					break
				mesh.boneNameList.append(boneIndeksList[map])

	for mesh in model.meshList:
		if len(mesh.skinList) > 0:
			skeleton.NICE = True
			skeleton.draw()
			break

	n = 0
	Animations = ys.get(ys.root, '"osgAnimation.Animation"')
	if Animations:
		for animation in Animations:
			getAnimation(ys, animation, n)


def Parser():
	global log
	# log=open('log.txt','w')
	filename = input.filename
	print
	print filename
	print
	os.system("cls")
	ext = filename.split(".")[-1].lower()
	osgParser(filename)
	log.close()


def openFile(flagList):
	global input, output
	input = Input(flagList)
	output = Output(flagList)
	parser = Parser()

modelID = 0
osgParser("./BinFiles/file.osgjs")
