#!/usr/bin/env python2
#-*- coding: UTF-8 -*-

# Released as open source by NCC Group Plc - http://www.nccgroup.com/
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
# https://github.com/nccgroup/pip3line
# Released under AGPL see LICENSE for more information


import bz2

def pip3line_is_two_ways():
	return True

def pip3line_transform(inputData):
	ret = None
	if (Pip3line_INBOUND):
		ret = bytearray(bz2.compress(inputData))
	else:
		ret = bytearray(bz2.decompress(inputData))
	return ret

