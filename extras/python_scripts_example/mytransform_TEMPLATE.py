#!/usr/bin/env python
#-*- coding: UTF-8 -*-

# Released as open source by NCC Group Plc - http://www.nccgroup.com/
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
# https://github.com/nccgroup/pip3line
# Released under AGPL see LICENSE for more information

# You need to implement a function with this exact name

def pip3line_transform(inputData):
	""" Take a bytearray as input and needs to return a bytearray"""
	# if you need to convert to a python 'string'
	string = inputData.decode('utf-8')
	string = string[::-1]	

	# just remember to convert it back to a bytearray
	return bytearray(string, 'utf-8')
      
# Optionally you can precise that this transformation is bidirectional and check for the 
# Pip3line_INBOUND module attribute later on (see gzip and bz2 for examples)

def pip3line_is_two_ways():
	return True

