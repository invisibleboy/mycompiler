###############################################################################
##
##                    Illinois Open Source License
##                     University of Illinois/NCSA
##                         Open Source License
##
## Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
## All rights reserved.
##
## Developed by:
##
##              IMPACT Research Group
##
##              University of Illinois at Urbana-Champaign
##
##              http://www.crhc.uiuc.edu/IMPACT
##              http://www.gelato.org
##
## Permission is hereby granted, free of charge, to any person
## obtaining a copy of this software and associated documentation
## files (the "Software"), to deal with the Software without
## restriction, including without limitation the rights to use, copy,
## modify, merge, publish, distribute, sublicense, and/or sell copies
## of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimers.
##
## Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimers in
## the documentation and/or other materials provided with the
## distribution.
##
## Neither the names of the IMPACT Research Group, the University of
## Illinois, nor the names of its contributors may be used to endorse
## or promote products derived from this Software without specific
## prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
## WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
## LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
## PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
## OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
## OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
## OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
##
###############################################################################


# Robert Kidd 10/1/02
# This module maintains a list of files that can be written to disk
# in a special format.

import re;
import struct;

class FileListItem:
    tag=0;
    len=0;
    data="";

    # Initialize the object
    def __init__(self):
	self.tag=0;
	self.len=0;
	self.data="";
    # def __init__

    # Sets up the object to hold the name of a file.
    def set_file(self, file):
	self.tag=1;
	self.len=len(file);
	self.data=file;
    # def set_file

    # Sets up the object to hold the name of a directory.
    def set_dir(self, dir):
	self.tag=2;
	if re.search("/$", dir):
	    self.len=len(dir);
	    self.data=dir;
	else:
	    self.len=len(dir)+1;
	    self.data=dir + "/";
    # def set_dir

    # Sets up the object to hold a sublist.
    def set_list(self, list):
	self.tag=3;
	self.data=list;
    # def set_list

    # Writes the item to an open filehandle.
    def write(self, fileHandle):
	fileHandle.write(struct.pack("B", self.tag));

	if self.tag==1 or self.tag==2:
	    fileHandle.write(struct.pack("<I", self.len));
	    fileHandle.write(self.data);
	    fileHandle.write(struct.pack("B", 0));
	else: # self.tag==3
	    self.data.write(fileHandle);
    # def write
# class FileListItem

class FileList:
    numItems=0;
    items=[];

    # Initialize the object
    def __init__(self):
	self.numItems=0;
	self.items=[];
    # def __init__

    # Adds a file to the list.
    def add_file(self, file):
	tmp=FileListItem();
	tmp.set_file(file);
	self.items.append(tmp);
    # def add_file

    # Adds a directory to the list.
    def add_dir(self, dir):
	tmp=FileListItem();
	tmp.set_dir(dir);
	self.items.append(tmp);
    # def add_dir

    # Adds a sublist to the list.
    def add_list(self, list):
	tmp=FileListItem();
	tmp.set_list(list);
	self.items.append(tmp);
    # def add_list

    # Writes the list to the given open filehandle.
    def write(self, fileHandle):
	fileHandle.write(struct.pack("<I", len(self.items)));

	for item in self.items:
	    item.write(fileHandle);
	# for item
    # def write
# class FileList
