#!/bin/sh

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


perl -x $0 "$@"
exit $?

#!/usr/bin/perl

use strict;
use English;

my ($line, $varName, $varType, $varValue, $file, $link, $arg, $abs_srcdir);

# sourceToLink maps source files to their linked location in bin/lib/include
my %sourceToLink;

# linkToSource maps links in bin/lib/include to their source files.
my %linkToSource;

# Arrays to hold the _PRG, _LIB, _INC, and _CLASS variables.
my (%var_PRG, %var_LIB, %var_INC, %var_CLASS);


if (!open (MAKEFILE, "< Makefile"))
{
    print "Error: Could not open Makefile\n";
    exit 1;
}

LINE: while ($line = <MAKEFILE>)
{
    chomp $line;

    # Skip blank lines and comments
    next LINE if ($line =~ /^\s*$/ or $line =~ /^\s*#/);
    
    # Read in additional lines as long as line ends with a backslash.
    while ($line =~ /\\$/)
    {
	chop $line;
	$line .= <MAKEFILE>;
	chomp $line;
    }

    # Extract definitions of the _PRG, _LIB, _INC, and _CLASS variables.
    # 8/6/04 REK Adding _DBIN for distributed binaries (EDG)
    if ($line =~ /^(noinst_|nodist_)?(\S+)(_PRG|_LIB|_INC|_CLASS|_DBIN)\s*=\s*(.*)/)
    {
	$varName = $2;
	$varType = $3;
	$varValue = $4;

	# Because we're removing noinst_ or nodist_ from the variable name,
	# there may be multiple definitions of the variable, so append
	# each value with a space afterwards.
	if ($varType eq "_PRG" || $varType eq "_DBIN")
	{
	    $var_PRG{$varName} .= $varValue . " ";
	}
	elsif ($varType eq "_LIB")
	{
	    $var_LIB{$varName} .= $varValue . " ";
	}
	elsif ($varType eq "_INC")
	{
	    $var_INC{$varName} .= $varValue . " ";
	}
	else # $varType eq "_CLASS"
	{
	    $var_CLASS{$varName} = $varValue;
	}
    }
    elsif ($line =~ /^abs_srcdir\s*=\s*(.*)/)
    {
	$abs_srcdir = $1;
    }
}

close (MAKEFILE);

foreach $varName (keys %var_PRG)
{
  FILE: foreach $file (split (/\s+/, $var_PRG{$varName}))
    {
	next FILE if ($file =~ /^\s*$/ or $file =~ /_PRG\)?$/);

	$link = "bin/" . `basename $file`;
	chomp $link;

	$sourceToLink{$file} = $link;
	$linkToSource{$link} = $file;
    }
}

foreach $varName (keys %var_LIB)
{
  FILE: foreach $file (split (/\s+/, $var_LIB{$varName}))
    {
	next FILE if ($file =~ /^\s*$/ or $file =~ /_LIB\)?$/);

	$link = "lib/" . `basename $file`;
	chomp $link;

	$sourceToLink{$file} = $link;
	$linkToSource{$link} = $file;
    }
}

foreach $varName (keys %var_INC)
{
  FILE: foreach $file (split (/\s+/, $var_INC{$varName}))
    {
	next FILE if ($file =~ /^\s*$/ or $file =~ /_INC\)?$/);

	$link = "include/$var_CLASS{$varName}/" . `basename $file`;
	$file = "$abs_srcdir/$file";
	chomp $link;

	$sourceToLink{$file} = $link;
	$linkToSource{$link} = $file;
    }
}

# Create all links specified on the command line.
# If the script has no arguments, create all links.
if (scalar (@ARGV) == 0)
{
    foreach $link (keys %linkToSource)
    {
	make_link ($link);
    }
}
else
{
    while ($arg = shift (@ARGV))
    {
	if ($arg =~ /^bin/)
	{
	    if ($arg eq "bin" or $arg eq "bin/")
	    {
		foreach $link (keys %linkToSource)
		{
		    if ($link =~ /^bin/)
		    {
			make_link ($link);
		    }
		}
	    }
	    elsif ($linkToSource{$arg} ne "")
	    {
		make_link ($arg);
	    }
	}
	elsif ($arg =~ /^lib/)
	{
	    if ($arg eq "lib" or $arg eq "lib/")
	    {
		foreach $link (keys %linkToSource)
		{
		    if ($link =~ /^lib/)
		    {
			make_link ($link);
		    }
		}
	    }
	    elsif ($linkToSource{$arg} ne "")
	    {
		make_link ($arg);
	    }
	}
	elsif ($arg =~ /^include/)
	{
	    if ($arg eq "include" or $arg eq "include/")
	    {
		foreach $link (keys %linkToSource)
		{
		    if ($link =~ /^include/)
		    {
			make_link ($link);
		    }
		}
	    }
	    elsif ($arg =~ /^(include\/[^\/]+)\/?/)
	    {
		my $prefix = $1;

		foreach $link (keys %linkToSource)
		{
		    if ($link =~ /^\Q$prefix\E/)
		    {
			make_link ($link);
		    }
		}
	    }
	    elsif ($linkToSource{$arg} ne "")
	    {
		make_link ($arg);
	    }
	}
	elsif ($sourceToLink{$arg} ne "")
	{
	    make_link ($sourceToLink{$arg});
	}
    }
}

# $1: A link in bin, lib, or include to create.
#
# Checks if the target file of $1 exists.  If it doesn't exist, runs
# 'make $target' to try to build it.  Creates a link to $1 in bin,
# lib, or include.
sub make_link ($)
{
    use English;

    my ($link) = @ARG;
    my $target = $linkToSource{$link};
    my $dir = `dirname $link`;

    chomp $dir;
    if (! -d $dir)
    {
	system ("mkdir -p $dir");
    }

    if (! -f $target)
    {
	print STDERR "$target\n";
	system ("make $target");
    }

    if ($link =~ /^(bin|lib)/)
    {
	system ("ln -sf ../$target $link");
    }
    else
    {
        system ("ln -sf $target $link");
    }

    return;
}
