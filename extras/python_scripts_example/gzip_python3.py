#!/usr/bin/env python3
#-*- coding: UTF-8 -*-

# Released as open source by NCC Group Plc - http://www.nccgroup.com/
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
# https://github.com/nccgroup/pip3line
# Released under AGPL see LICENSE for more information

import gzip

def pip3line_is_two_ways():
	return True

def pip3line_transform(inputData):
	ret = None
	if (Pip3line_INBOUND):
		ret = bytearray(gzip.compress(inputData))
	else:
		ret = bytearray(gzip.decompress(inputData))
	return ret

