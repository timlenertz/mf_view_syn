#!/usr/bin/python

import sys
import os
import fnmatch

root_dirs = ('./src', './test', './prog')
extensions = ('.cc', '.h', '.tcc', '.icc')


def readfile(filename):
	with open(filename, 'r') as f:	
		return f.read()

if(len(sys.argv) < 3):
	sys.exit(1)

mode = sys.argv[1]
copyright_content = readfile(sys.argv[2])

files = []
for root_dir in root_dirs:
	for root, dirnames, filenames in os.walk(root_dir):
		for filename in filenames:
			if filename.endswith(extensions):
				files.append(root + '/' + filename);

def add_copyright(filename):
	content = readfile(filename)
	if not content.startswith(copyright_content):
		content = copyright_content + content
		with open(filename, 'w') as f:
			f.write(content)

def remove_copyright(filename):
	content = readfile(filename)
	if content.startswith(copyright_content):
		length = len(copyright_content)
		content = content[length:]
		with open(filename, 'w') as f:
			f.write(content)		

	
for filename in files:
	print filename
	if mode == 'add':
		add_copyright(filename)
	elif mode == 'remove':
		remove_copyright(filename)
