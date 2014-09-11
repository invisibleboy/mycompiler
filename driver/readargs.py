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


# Robert Kidd 6/19/02
# This module handles processing command line arguments in a more flexible
# manner than the getopt module.

import re;

# readargs parses the command line and returns two items, in the same
# form as getopt.getopt.  The first item is a list of [arg, value] pairs.
# The second item is a list of command line arguments left over.
# readargs takes sys.argv[1:] as the first argument and a list of
# arguments to accept as the second.
# If an option takes an argument, the argument can be specified immediately
# after to option (-oarg), separated by an = (-o=arg), or separated by
# whitespace (-o arg).  Because the first form is supported, no option
# should be a prefix of another option.  For example, if you specify
# both -op= and -option as valid arguments, the parser won't know whether
# -option on the command line means -op with argument 'tion' or -option.
# Since single character option are much more likely to form a prefix,
# they are processed separately, after all long options have been.  There is
# still some chance of confusion, for example specifying -o=ption as -option,
# but it is unlikely.
# 07/26/02 REK Adding support for options that take optional arguments.  These
#              are specified as option=?.
# 07/29/02 REK This function now stops processing arguments when it sees '--'.
def readargs(argv, shortOptions, longOptions, ignoreShort=[], ignoreLong=[]):
    """Parses command line arguments

    argv:         The command line arguments to the program (usually
                  sys.argv[1:])
    shortOptions: A list of single character options that the parser accepts.
                  Add = to the end of the option if it should take a value.
		  Add =? to the end of the option if it should take an optional
		  value.
    longOptions:  A list of options the parser should accept.  Add = to the end
                  of the option if it should take a value.
    ignoreShort:  A list of single character options to ignore.  These will
                  be fully parsed, but not reported in argumentList or
		  extraList.
    ignoreLong:   A list of long options to ignore."""

    # This function returns argumentList, extraList.  
    argumentList=[];
    extraList=[];

    # First, build a dictionary where each argument keys a regular expression
    # to match it.
    optDict={};

    # A regular expression to find options with '=' at the end
    equalsRegEx=re.compile("(.*)=$");
    
    # A regular expression to find options with '=?' at the end
    equalsQMarkRegEx=re.compile("(.*)=\?$");

    # A regular expression to find options with '=' or '=?' at the end
    optionRegEx=re.compile("(.*)=\??$");

    # A regular expression to match a dash at the beginning of an option
    dashRegEx=re.compile("^-");

    # 07/29/02 REK A regular expression to match two dashes without a name.
    #              This is used to stop option processing.
    doubleDashRegEx=re.compile("^--$");

    for opt in shortOptions + ignoreShort:
        match=equalsRegEx.match(opt);
        if match:
            optDict[opt]=re.compile("(^-" + re.escape(match.group(1)) + \
                                    ")(=?)(.*)");
        else:
	    match=equalsQMarkRegEx.match(opt);
	    if match:
		optDict[opt]=re.compile("(^-" + re.escape(match.group(1)) + \
					")(=)?(.*)");
	    else:
		optDict[opt]=re.compile("(^-" + re.escape(opt) + "$)");

    for opt in longOptions + ignoreLong:
        match=equalsRegEx.match(opt);
        if match:
            optDict[opt]=re.compile("(^--?" + re.escape(match.group(1)) + \
                                    ")(=?)(.*)");
        else:
	    match=equalsQMarkRegEx.match(opt);
	    if match:
		optDict[opt]=re.compile("(^--?" + re.escape(match.group(1)) + \
					")(=)?(.*)");
	    else:
		optDict[opt]=re.compile("(^--?" + re.escape(opt) + "$)");
    # for opt
        
    # curOpt is the option we're currently processing.  It is only set if
    # an option can't be competely processed by looking at one element of
    # argv, for example, if it is specified as '-o arg'.
    curOpt="";

    # argOptional is a boolean.  It is only defined when curOpt is.  If
    # argOptional is 1, curOpt may not take a value.  If the next element
    # in argv begins with a dash, it is another command line argument.  If
    # not, it is the value for curOpt.
    argOptional=0;

    # 07/29/02 REK processingDone is a boolean.  We set it once we see two
    #              dashes with no name on the command line.  Once it is set,
    #              all subsequent options are copied to extraList.
    processingDone=0;

    # 03/14/05 REK ignoreNext is set when the next word is known to be the
    #              argument to an option that will be ignored.
    ignoreNext = False

    # Walk through the arguments
    for arg in argv:
	# If processingDone is set, copy the argument to extraList.
	if processingDone:
	    extraList.append(arg);
	    continue;

	# If we see the double dash, stop processing arguments.
	if doubleDashRegEx.match(arg):
	    processingDone=1;
	    continue;

        # If curOpt is set, then we must be looking at the argument to
        # the previous option.
        if curOpt!="" and not (argOptional and dashRegEx.match(arg)):
	    if ignoreNext:
		# This word is the argument to an option that should be
		# ignored
		ignoreNext = False
	    else:
		argumentList.append([curOpt, arg]);
		curOpt="";
		argOptional=0;
        else:
	    # Handle the case where an option takes an optional argument that
	    # isn't specified
	    if curOpt!="" and argOptional:
		argumentList.append([curOpt, ""]);
		curOpt="";
		argOptional=0;
		
            # optFound tells the inner for loop when we have found the option
            # so it can continue to the next element in the outer loop
            optFound=0;
            
            # Look through all options, first the long ones, then the short.
            for opt in longOptions + shortOptions:
                match=optDict[opt].match(arg);
                if match:
                    # Determine if this option takes an argument
                    optFound=1;
                    if optionRegEx.match(opt):
                        # This option does take an argument.  The argument will
                        # either be in the third group or the next element in
                        # argv.  If this option has an optional argument, we'll
			# have to check the next element to make sure it
			# doesn't start with a dash before accepting it as
			# the option value.
                        if match.group(3) == "":
			    # the arg is argv's next element.
			    curOpt=match.group(1);
			    if equalsQMarkRegEx.match(opt):
				argOptional=1;
                        else: # The arg is specified in the option
                            argumentList.append([match.group(1), \
						 match.group(3)]);
                    else:
                        # This option takes no argument, so it is added to the
                        # list with an empty second element.
                        argumentList.append([match.group(1), ""]);
                # If we found an option, break out of this loop
                if optFound==1:
                    break;
            # for opt

	    # Look through the list of options to ignore.
	    for opt in ignoreLong + ignoreShort:
		match = optDict[opt].match (arg)
		if match:
		    # Determine if this option takes an argument.
		    optFound = 1
		    if optionRegEx.match (opt):
			if match.group (3) == "":
			    curOpt = match.group (1)
			    ignoreNext = True
			    if equalsQMarkRegEx.match (opt):
				argOptional = 1
			else: # The arg is specified in the option.
			    pass
		    else:
			# This option takes no argument
			pass
		if optFound == 1:
		    break
	    # for opt

            # At this point, optFound should be 1 if we found an option.
            # Since option arguments are handled in the if part of the
            # current if...else block, optFound==0 means that we just looked
            # at something that belongs on the extraList for the calling
            # program to deal with.
            if optFound==0:
                extraList.append(arg);
    # for arg

    # If curOpt is still set, we haven't found the argument for the last
    # option.  If the option was optional, add this argument to the list.
    # Otherwise, print an error message.
    if curOpt!="":
	if argOptional:
	    argumentList.append([curOpt, ""]);
	else:
	    print("Error: readargs: Mandatory value not specified for %s" % \
		  curOpt);

    return argumentList, extraList;
# def readargs


# Make_option_dictionary converts the list of pairs that readargs returns
# into a dictionary where the value for an option is keyed by the option.
def make_option_dictionary(optList):
    """Converts the output of getopt to a dictionary

    optList: A list of (option, value) pairs as is returned by
             getopt.getopt.
    This function converts the given list to a dictionary where the option's
    value is keyed by the option.  Command line switches have a value of
    1, while options that require an argument have that argument as a value."""
    result = {}

    # A regular expression to pull the dashes off the front of the option
    regex = re.compile ("^--?")

    for option in optList:
        # If the value part of the pair isn't empty, it should be stored in
        # the dictionary.  Otherwise, this option is a switch, so we should
        # just store 1 in the dictionary.
        if option[1]:
            result[regex.sub ("", option[0])]=option[1]
        else:
            result[regex.sub ("", option[0])]=1
    # for option

    return result
# def make_option_dictionary


