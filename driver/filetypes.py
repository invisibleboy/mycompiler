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


# Robert Kidd 7/2/02
# This module provides several functions to identify file types used by
# IMPACT.

import os
import re
import string
import sys

# Constants that examine_files may return.  These represent file types.
# Two miscellaneous types
UNKNOWN                         = -1
MIXED                           =  0

# The order is significant here.  These types are listed in the same
# order as the stages of compilation.  The numerical values must be kept
# in order.  These steps are part of a straight compilation, from source
# to binary.  There are other types that are used during compilation that
# aren't listed here.  Except for C_SOURCE and BINARY, all types here are
# generated from the type before and are used to generate the type after.
# If a file type is a dead end, it should not be in this sequence.  The
# binary type is used mainly as a marker.  Any code that examines files
# based on this sequence should use BINARY and C_SOURCE as the bounds so
# that it continues to work if more types are added.
C_SOURCE                        =  1
CXX_SOURCE                      =  2
PREPROCESSED_C                  =  3
PREPROCESSED_CXX                =  4
PCODE                           =  5
PCODE_WITH_SYM_TAB              =  6
PCODE_LINKED                    =  7
PCODE_FLAT                      =  8
PCODE_FLAT_PROFILED             =  9
PCODE_SPLIT                     = 10
PCODE_INLINED                   = 11
PCODE_SYNC                      = 12
GP                              = 13
LCODE                           = 14
LCODE_OPTIMIZED                 = 15
LCODE_PROFILED                  = 16
LCODE_PROFILED_GEN_VALUE        = 17
LCODE_PROFILED_MEM_REUSE        = 18
LCODE_PROFILED_REUSE            = 19
HYPERBLOCK                      = 20
LINDUCT_OUTPUT                  = 21
HYPERBLOCK_SUPERSCALAR          = 22
MCO                             = 23
HYPERBLOCK_SUPERSCALAR_ASSEMBLY = 24
ASSEMBLY                        = 25
OBJECT                          = 26
BINARY                          = 27 # The final output

# These are types that this module can detect, but that aren't part of a
# straight compilation.
PROBED_C_CODE                   = 1000
PROBED_OBJECT                   = 1001
ENCODED_LCODE                   = 1002
PROBED_LCODE                    = 1003
PROBED_HYPERBLOCK_SUPERSCALAR   = 1004
AR_ARCHIVE                      = 1005

# A dictionary that maps file type constants (above) to a the filename
# extension.
fileExtensions = {C_SOURCE:                        ".c", \
		  CXX_SOURCE:                      ".cc", \
		  PREPROCESSED_C:                  ".i", \
		  PREPROCESSED_CXX:                ".ii", \
		  PCODE:                           ".pc", \
		  PCODE_WITH_SYM_TAB:              ".pst", \
		  PCODE_LINKED:                    ".pstl", \
		  PCODE_FLAT:                      ".pcf", \
		  PCODE_FLAT_PROFILED:             ".pcf_p", \
		  PCODE_SPLIT:                     ".pcs", \
		  PCODE_INLINED:                   ".pci", \
		  PCODE_SYNC:                      ".pcj", \
		  GP:                              ".gp", \
		  LCODE:                           ".lc", \
		  LCODE_OPTIMIZED:                 ".O", \
		  LCODE_PROFILED:                  ".O_p", \
		  LCODE_PROFILED_GEN_VALUE:        ".O_v", \
		  LCODE_PROFILED_MEM_REUSE:        ".O_m", \
		  LCODE_PROFILED_REUSE:            ".O_r", \
		  HYPERBLOCK:                      ".H", \
		  LINDUCT_OUTPUT:                  ".Hi", \
		  HYPERBLOCK_SUPERSCALAR:          ".HS", \
		  MCO:                             ".HS_mco", \
		  HYPERBLOCK_SUPERSCALAR_ASSEMBLY: ".HS_s", \
		  ASSEMBLY:                        ".s", \
		  OBJECT:                          ".o", \
		  PROBED_C_CODE:                   "_rev.c", \
		  PROBED_OBJECT:                   "_rev.o", \
		  ENCODED_LCODE:                   ".encoded", \
		  PROBED_LCODE:                    ".O.c", \
		  PROBED_HYPERBLOCK_SUPERSCALAR:   ".HS.c", \
		  AR_ARCHIVE:                      ".a"}

# A dictionary that maps file type constants to a regular expression matching
# the extension.  This is mainly intended to provide expressions for types
# that have more than one possible extension.  If a type doesn't have an
# entry in this dictionary, get_ext_re() will use the extension
# in the fileExtensions dictionary.
fileExtRe = {CXX_SOURCE:        re.compile ("(\.C|\.cpp|\.CPP|\.cxx|\.CXX|\.cc)$"), \
	     PREPROCESSED_CXX:  re.compile ("(\.ii|\.I)$")}

def extension_without_dot (type):
    """Returns a file extension without the leading dot.

    type: The filetype to query."""

    result = fileExtensions[type]

    if result[0] == ".":
	result = result[1:]

    return result
# def extension_without_dot


def get_ext_re (type):
    """Returns a regular expression to match files of a type.

    type: The filetype to match."""

    if fileExtRe.has_key (type):
	result = fileExtRe[type]
    else:
	result = re.compile (re.escape (fileExtensions[type]) + "$")

    return result
#def get_ext_re


# filetypes_between returns a list of integer constants representing filetypes
# from the straight compilation sequence.
def filetypes_between (begin, end):
    """Returns a list of filetypes from the straight compilation sequence.

    begin: The starting filetype.
    end:   The ending filetype.
    This function returns a list of filetypes in the range begin to end-1.  It
    supports ascending and descending order through the filetypes."""

    if begin > end:
	return range (begin - 1, end - 1, -1)
    elif begin < end:
	return range (begin, end)
    else:
	return [begin]
# def filetypes_between

# examine_file returns a constant that describes the type of the file
# given as the argument.
def examine_file (file):
    """Determines the type of the given file based on its name.

    file: The file to examine
    This function returns a numeric constant that indicates the type of
    the file."""
    fileType = ""

    for ext in fileExtensions.keys ():
	if get_ext_re (ext).search (file):
	    fileType = ext
	    break
    else:
	return UNKNOWN

    return fileType
# def examine_file

# examine_source_files returns a constant that describes the type of the
# files named in the list given as an argument
def examine_source_files (list):
    """Determines the type of the files based on their name.

    list: A list of filenames to examine
    This function returns a numeric constant that indicates the type of
    files in the list."""

    # fileType is the type of the first file.  This is the type
    # that is returned if all files are the same type.  If there are
    # multiple file types, MIXED is returned.
    fileType = ""

    for file in list:
	curFileType = examine_file (file)
	if fileType == "" or fileType == curFileType:
	    fileType = curFileType
	else:
	    return MIXED
    # for file

    return fileType
# def examine_source_files

# group_files takes a list of filenames and returns a dictionary where the
# filetype constants key lists of names.
def group_files (list):
    """Groups input files into source code and libraries.

    list: A list of filenames.
    This function returns a dictionary with lists of files keyed by their
    file type."""
    fileDict = {}

    for file in list:
	add_file (file, fileDict)
    # for file

    return fileDict
# def group_files

# add_file adds a file to a preexisting dictionary created by group_files
def add_file (file, dict):
    """Adds a file to a dictionary created by group_files.

    file: The file to add.
    dict: The dictionary to add the file to.
    This function adds the file to the appropriate list in the dictionary
    based on the file's type."""
    fileType = examine_file (file)
	
    # If this is the first file of its type, initialize an array keyed
    # by the type.
    if not dict.has_key (fileType):
	dict[fileType] = []

    dict[fileType].append (file)
# def add_file

# remove_file removes a file from a preexisting dictionary created by
# group_files
def remove_file (file, fType, dict):
    """Removes a file from a dictionary created by group_files.

    file:  The file to remove.
    fType: The type of the file to remove.
    dict:  The dictionary to remove the file from."""

    dict[fType].remove (file)
    if len (dict[fType]) == 0:
	del dict[fType]
# def remove_file

# update_file changes a file's type in a preexisting dictionary created by
# group_files
def update_file (file, oldType, newType, dict):
    """Changes a file's type in a dictionary created by group_files.

    file:    The file to change.
    oldType: The old type of the file.
    newType: The new type of the file.
    dict:    The dictionary containing the file.
    This function moves the file from one list to another in the dictionary
    and returns the file's new name.  It returns the new name of the file."""

    newName = rename_file (newType, file, 1)
    remove_file (file, oldType, dict)
    if not dict.has_key (newType):
	dict[newType] = []
    dict[newType].append (newName)
    return newName
# def update_file

# update_files_by_type changes all files of a given type to a new type.
def update_files_by_type (oldType, newType, dict):
    """Changes the types of all files of a given type.

    oldType: The original type of the files.
    newType: The type to change to.
    dict:    The dictionary containing the files.
    This function moves all files of oldType to newType and returns a list
    of the new filenames."""
    result = []

    # The [:] at the end copies the array so that we iterate over a copy.
    # This is necessary because we modify the array inside the loop.
    for file in dict[oldType][:]:
	result.append (update_file (file, oldType, newType, dict))
    # for file

    return result
# def update_files_by_type

# type_in_main_sequence takes a type as the argument and returns 1 if this
# type falls in the straight compilation sequence.
def type_in_main_sequence (fileType):
    """Returns 1 if the given type is in the straight compilation sequence.

    fileType: The type to check."""

    return (fileType >= C_SOURCE and fileType <= BINARY)
# def type_in_main_sequence

# main_sequence_types takes a dict created by group_files and returns a list
# of the filetypes from the straight compilation sequence that are in the
# dictionary.
def main_sequence_types (dict):
    """Returns a list of types in the straight compilation sequence.

    dict: A dictionary of filenames created by group_files"""

    return filter (type_in_main_sequence, dict.keys ())
# def main_sequence_types

# least_advanced takes a dict created by group_files and returns the least
# advanced filetype of any file in the straight compilation sequence.
def least_advanced (dict):
    """Returns the least advanced type from the straight compilation sequence.

    dict: A dictionary of filenames created by group_files"""

    # types contains only those types from dict that are in the straight
    # compilation sequence.
    types = main_sequence_types (dict)

    # If types is empty, return UNKNOWN.  Otherwise, return the lowest type.
    if len (types) == 0:
	return UNKNOWN
    else:
	# Sort the types in ascending order.
	types.sort ()
	return types[0]
# def least_advanced

# most_advanced takes a dict created by group_files and returns the most
# advanced filetype of any file in the straight compilation sequence.
def most_advanced (dict):
    """ Returns the most advanced type from the straight compilation sequence.

    dict: A dictionary of filenames created by group_files"""

    # types contains only those types from dict that are in the straight
    # compilation sequence.
    types = main_sequence_types (dict)

    # If types is empty, return UNKNOWN.  Otherwise, return the greatest type.
    if len (types) == 0:
	return UNKNOWN
    else:
	# Sort the types in ascending order.
	types.sort ()
	return types[len (types) - 1]
# def most_advanced

# unique_files_of_type takes one or more file types as arguments and returns
# a list of filenames.
def unique_files_of_type (dict, types):
    """Returns a list of files of the given type(s).

    dict:  The dictionary containing the files.
    types: A list of one or more file types.

    This function returns a list of unique files of the given type(s).  Each
    file base name (before the extension) will only appear once in the output.
    The filetypes are listed in decreasing precedence, so all files of the
    first type will appear in the output.  Files of subsequent types will
    appear only if no file of an earlier type with the same base name is
    already in the output."""

    # Each time we add a file to the output, add its basename to this
    # dictionary so that we can quickly test to see if a basename is in the
    # output.
    baseNameDict = {}
    result = []

    for type in types:
	if dict.has_key (type):
	    for file in dict[type]:
		fileBaseName = os.path.splitext (file)[0]
		if not baseNameDict.has_key (fileBaseName):
		    baseNameDict[fileBaseName] = 1
		    result.append (file)
	    # for file
    # for type

    return result
# def unique_file_of_type

# strip_file_extensions takes a list of filenames and returns a list of
# filenames without extensions
def strip_file_extensions (list):
    """Removes the file extensions from files in the given list.

    list: A list of filenames
    This function removes the extension of the filenames and returns
    a new list with the base names."""

    result = []

    for file in list:
	result.append (os.path.splitext (file)[0])
    # for file

    return result
# def strip_file_extensions

# rename_files takes a file type and a list of file base names and returns
# a list of filenames with an extension appropriate for the given type.
def rename_files (fileType, list, hasExtension):
    """Adds an appropriate extension to the filenames in list

    fileType:     One of the numeric constants describing the type of filename
                  to generate.
    list:         A list of filenames to alter.
    hasExtension: A boolean indicating whether or not the incoming filenames
                  have an extension that needs to be stripped before renaming.
		  If this is 1 the file extension is stripped.
    This function adds a file extension to each file and returns a new list."""

    result = []

    if fileType != MIXED:
        for file in list:
            result.append (rename_file (fileType, file, hasExtension))
        # for file

    return result
# def rename_files

# rename_file takes a file type and a file name and returns the name with
# an appropriate extension for the type.
def rename_file (fileType, file, hasExtension):
    """Gives the file an appropriate extension for its type.

    fileType:     One of the numeric constants describign the type of
                  filename to generate.
    file:         A filename with or without extension.
    hasExtension: A boolean indicating whether or not the input filename
                  has a file extension.  If this is 1, the extension is
		  stripped before renaming."""
    if hasExtension:
	if get_ext_re (PROBED_C_CODE).search (file):
	    return get_ext_re (PROBED_C_CODE).sub (fileExtensions[fileType],
						   file)
	else:
	    return os.path.splitext (file)[0] + fileExtensions[fileType]
    else:
	return file + fileExtensions[fileType]
# def rename_file

# remove_files_of_type deletes all files of the given type from the current
# directory.
def remove_files_of_type (fileType, dict):
    """Removes all files of the given type from the current directory.

    fileType: The constant representing the type of of file to remove.
    dict:     The dictionary containing the files."""

    if dict.has_key (fileType):
	for file in dict[fileType][:]:
	    os.remove (file)
	    remove_file (file, fileType, dict)
	# for file
# def remove_files_of_type

# file_is_known returns 1 if the file exists in the dictionary.
def file_is_known (dict, fileType, file):
    """Returns 1 if the given file exists in the dictionary.

    dict:     The dictionary containing the files.
    fileType: The type of the file.
    file:     The filename to search for in the dictionary."""

    if dict.has_key (fileType):
	for curFile in dict[fileType]:
	    if curFile == file:
		return 1
	# for curFile

    return 0
# def file_is_known

# dump_dict prints all files stored in the filetypes dictionary.
def dump_dict (dict):
    """Prints all files stored in the dictionary.

    dict: The dictionary to dump."""

    for fileType in dict.keys ():
	for file in dict[fileType]:
	    sys.stderr.write ("%d => %s\n" % (fileType, file))
	# for file
    # for fileType
# def dump_dict
