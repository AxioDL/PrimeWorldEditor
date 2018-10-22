# Debug helpers for Prime World Editor classes
from dumper import *

def floatStr(value):
	s = "%.6f" % value.floatingPoint()
	s = s.rstrip('0')
	if s.endswith('.'): s += '0'
	return s
	
# Common
def qdump__CAssetID(d, value):
	length = value["mLength"].integer()
	id = value["mID"].integer()
	
	if length is 4:
		if id == 0xFFFFFFFF:
			d.putValue("Invalid 32-bit ID")
		else:
			d.putValue("[%08X]" % id)
	elif length is 8:
		if id == 0xFFFFFFFFFFFFFFFF:
			d.putValue("Invalid 64-bit ID")
		else:
			d.putValue("[%016X]" % id)
	else:
		d.putValue("[Invalid]")

def qdump__CColor(d, value):
	R = floatStr(value["R"])
	G = floatStr(value["G"])
	B = floatStr(value["B"])
	A = floatStr(value["A"])
	d.putValue("[%s, %s, %s, %s]" % (R, G, B, A))
	d.putNumChild(4)
	
	if d.isExpanded():
		with Children(d):
			d.putSubItem("R", value["R"])
			d.putSubItem("G", value["G"])
			d.putSubItem("B", value["B"])
			d.putSubItem("A", value["A"])
			
def qdump__CFourCC(d, value):
	fourCC = value["mFourCC"].integer()
	charA = chr((fourCC >> 24) & 0xFF)
	charB = chr((fourCC >> 16) & 0xFF)
	charC = chr((fourCC >>  8) & 0xFF)
	charD = chr((fourCC >>  0) & 0xFF)
	d.putValue("'%c%c%c%c'" % (charA, charB, charC, charD))
	
def qdump__TString(d, value):
	d.putItem( value["mInternalString"] )
	d.putType("TString")

def qdump__TWideString(d, value):
	d.putItem( value["mInternalString"] )
	d.putType("TWideString")
	
# Math
def qdump__CQuaternion(d, value):
	qdump__CVector4f(d, value)

def qdump__CVector2f(d, value):
	X = floatStr(value["X"])
	Y = floatStr(value["Y"])
	d.putValue("[%s, %s]" % (X, Y))
	d.putNumChild(2)
	
	if d.isExpanded():
		with Children(d):
			d.putSubItem("X", value["X"])
			d.putSubItem("Y", value["Y"])

def qdump__CVector2i(d, value):
	d.putValue("[%i, %i]" % (value["X"], value["Y"]))
	d.putNumChild(2)
	
	if d.isExpanded():
		with Children(d):
			d.putSubItem("X", value["X"])
			d.putSubItem("Y", value["Y"])
		
def qdump__CVector3f(d, value):
	X = floatStr(value["X"])
	Y = floatStr(value["Y"])
	Z = floatStr(value["Z"])
	d.putValue("[%s, %s, %s]" % (X, Y, Z))
	d.putNumChild(3)
	
	if d.isExpanded():
		with Children(d):
			d.putSubItem("X", value["X"])
			d.putSubItem("Y", value["Y"])
			d.putSubItem("Z", value["Z"])
			
def qdump__CVector4f(d, value):
	X = floatStr(value["X"])
	Y = floatStr(value["Y"])
	Z = floatStr(value["Z"])
	W = floatStr(value["W"])
	d.putValue("[%s, %s, %s, %s]" % (X, Y, Z, W))
	d.putNumChild(4)
	
	if d.isExpanded():
		with Children(d):
			d.putSubItem("X", value["X"])
			d.putSubItem("Y", value["Y"])
			d.putSubItem("Z", value["Z"])
			d.putSubItem("W", value["W"])

# Core
