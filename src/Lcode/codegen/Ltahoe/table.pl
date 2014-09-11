#!/usr/bin/env perl
#############################################################################
#
#		      Illinois Open Source License
#                     University of Illinois/NCSA
#                         Open Source License
#
# Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
# All rights reserved.
#
# Developed by:             
#
#		IMPACT Research Group
#
#		University of Illinois at Urbana-Champaign
#
#              http://www.crhc.uiuc.edu/IMPACT
#              http://www.gelato.org
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal with the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimers.
#
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimers in
# the documentation and/or other materials provided with the
# distribution.
#
# Neither the names of the IMPACT Research Group, the University of
# Illinois, nor the names of its contributors may be used to endorse
# or promote products derived from this Software without specific
# prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
# PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
# OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#
##############################################################################
# 8/30/02 Robert Kidd
#
# This script builds the table that is used to process Ltahoe opcodes.  The
# table (in C) is an array of structs.  Each struct has several fields that
# hold information about the opcode or a pointer to a function to do some
# processing on the opcode.  The array is indexed by the numerical value of
# the TAHOEop constant defined in m_tahoe.h.  So that humans don't have to
# know about the numerical value, the table is defined in a pair of perl data
# structures that only know strings.  This script translates those perl
# structures into the C table.

use strict;
use English;
use Getopt::Long;

# This is the number of constants (defined in m_tahoe.h) that begin with
# TAHOEop but do not correspond to an instruction that should appear
# in the table.  There are currently only two, TAHOEop_VERSION and
# TAHOEop_START.  This number is used as a simple check to see whether all
# instruction appear in the table that should.
my $NUM_EXTRA_TAHOE_OPS = 3;

my ($key, $field, $header, $input, $output, $numOpcodes, $maxVal, $val);
my ($tahoeOpOffset, $defineString, $include);
my (@fieldNames);
my (%tahoeOps);

# These two data structures are defined in the input file.  
# struct is an array of hash references that defines the struct that is
# used as the base structure for the array.
our ($structTypeName, $tableName);
our (@struct, @includes);
our (%table, %structHash);

# Header is the path to the m_tahoe.h file.
$header="";

# Input is the source file defining the two perl data structures.
$input="";

# Output is the name of the C header file to write.
$output="";
GetOptions ("h:s" => \$header,
	    "i=s" => \$input,
	    "o=s" => \$output);

if (($header eq "") or ($input eq "") or ($output eq ""))
{
    print "usage: $PROGRAM_NAME -h <header> -i <input> -o <output>\n";
    print " header: The path to the m_tahoe.h C header.\n";
    print " input: The source file defining the perl data structures.\n";
    print " output: The name of the C header/source file to write.\n";
    exit ();
} # if

# Remove trailing newlines from the options.
chomp ($header, $input, $output);

# Remove a .c or .h extension from the output name
$output =~ s/\.[ch]//;

# Run the input file to define the data structures.
do $input;

# Read the header file for TAHOEop constant definitions.
($tahoeOpOffset, %tahoeOps)=read_tahoe_ops ($header);

# Figure out the highest value of the tahoeOp constant so that we know
# how many array slots we need.
$maxVal=0;
foreach $val (keys (%tahoeOps))
{
    if ($val > $maxVal) 
    { 
	$maxVal=$val; 
    } # if
} # foreach $val

$numOpcodes=$maxVal-$tahoeOpOffset;

# Do an initial check to make sure that every tahoe op has an entry in the
# table.  If this test fails, there is some op that is not defined in the
# table.  If this test is successful, that does not necessarily mean that
# the table is correct.
if ((scalar (keys (%tahoeOps)) - $NUM_EXTRA_TAHOE_OPS) !=
    scalar (keys (%table)))
{
    my %tmpOpHash;
    my $tmpVal;

    print "Warning: Table in $input is missing some opcodes.\n";

    # Figure out which opcodes are missing.
    foreach $tmpVal (values (%tahoeOps))
    {
	$tmpOpHash{$tmpVal}=1;
	if ($table{$tmpVal} eq "")
	{
	    print "$tmpVal defined in $header, but not $input.\n";
	} # if
    } # foreach $tmpVal

    foreach $tmpVal (keys (%table))
    {
	if ($tmpOpHash{$tmpVal} eq "")
	{
	    print "$tmpVal defined in $input, but not in $header.\n";
	} # if
    } # foreach $tmpVal
} # if

# Open the output file for writing.
open (HEADER, "> ${output}.h") or \
    die ("Could not open ${output}.h for writing.");

print HEADER "/* ${output}.h generated by $PROGRAM_NAME on ", scalar (localtime), "\n";
print HEADER " * Do not edit this file manually.  Edit $input, then run\n";
print HEADER " * $PROGRAM_NAME -h $header -i $input -o $output\n";
print HEADER " * to generate this file. */\n\n";

print HEADER <<EOF;
/*****************************************************************************\\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2002, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
 *
 *              http://www.crhc.uiuc.edu/IMPACT
 *              http://www.gelato.org
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal with the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimers.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of the IMPACT Research Group, the University of
 * Illinois, nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without specific
 * prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
\\*****************************************************************************/

EOF

# Convert the output filename to upper case and replace non-text characters
# with underscores.  This string will be used as the define at the top of 
# the header to make sure that we only include the output file once.
$defineString=$output;

# Extract the filename part (only the part after the last slash).
$defineString=`basename $defineString`;

# Convert defineString to upper case
$defineString =~ tr/[a-z]/[A-Z]/;

# Replace non-letter characters with underscores
$defineString =~ s/\W/_/g;

# Remove the trailing underscore if the previous step added it.
if ($defineString =~ /_$/)
{
    chop ($defineString);
} # if

# Add a leading underscore to the name
$defineString="_$defineString";

print HEADER "#ifndef $defineString\n";
print HEADER "#define $defineString\n\n";

# Include each file in the @includes array
foreach $include (@includes)
{
    # If the include string is inside angle brackets, don't add double
    # quotes.
    if ($include =~ /^\<.*\>$/)
    {
	print HEADER "#include $include\n";
    } # if
    else
    {
	print HEADER "#include \"$include\"\n";
    } # else
} # foreach $include

print HEADER "\n";
print HEADER "#define table_num_opcodes $numOpcodes\n";
print HEADER "#define table_offset ",${tahoeOpOffset}+1,"\n\n";

print HEADER "#define LTAHOE_TABLE_ENTRY(t, o)   (t)[(o)-table_offset]\n\n";

print HEADER "typedef struct _$structTypeName\n{\n";

foreach $field (@struct)
{
    push (@fieldNames, $field->{'name'});
    $structHash{$field->{'name'}}{'dlim'}=$field->{'dlim'};
    $structHash{$field->{'name'}}{'decl'}=$field->{'decl'};
    $structHash{$field->{'name'}}{'dflt'}=$field->{'dflt'};

    if ($field->{'dflt'} eq "")
    {
	print HEADER "\t$field->{'decl'}\t/* Default: \"\" */\n";
    } # if
    else
    {
	print HEADER "\t$field->{'decl'}\t/* Default: $field->{'dflt'} */\n";
    } # else
} # foreach field

print HEADER "} $structTypeName;\n\n";

print HEADER "extern $structTypeName $tableName\[table_num_opcodes\];\n\n";

print HEADER "#endif\n";

close (HEADER);

open (SOURCE, "> ${output}.c") or \
    die ("Could not open ${output}.c for writing.");

print SOURCE "/* ${output}.c generated by $PROGRAM_NAME on ", scalar (localtime), "\n";
print SOURCE " * Do not edit this file manually.  Edit $input, then run\n";
print SOURCE " * $PROGRAM_NAME -h $header -i $input -o $output\n";
print SOURCE " * to generate this file. */\n\n";

print SOURCE <<EOF;
/*****************************************************************************\\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2002, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
 *
 *              http://www.crhc.uiuc.edu/IMPACT
 *              http://www.gelato.org
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal with the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimers.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of the IMPACT Research Group, the University of
 * Illinois, nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without specific
 * prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
\\*****************************************************************************/

EOF

print SOURCE "#include \"${output}.h\"\n\n";

print SOURCE "$structTypeName $tableName\[table_num_opcodes\]=\n{\n";

my $op=$tahoeOpOffset+1;
while ($op <= $maxVal)
{
    my $firstField=1;
    print SOURCE "\t{";
    foreach $field (@fieldNames)
    {
	if ($firstField == 1)
	{
	    $firstField=0;
	} # if
	else
	{
	    print SOURCE ",";
	} # else

	print SOURCE $structHash{$field}{'dlim'};

	if ($table{$tahoeOps{$op}}{$field} eq "")
	{
	    print SOURCE "$structHash{$field}{'dflt'}";
	} # if
	else
	{
	    print SOURCE "$table{$tahoeOps{$op}}{$field}";
	} # else

	print SOURCE $structHash{$field}{'dlim'};
    }

    if (($op+1) > $maxVal)
    {
	print SOURCE "}\n";
    } # if
    else
    {
	print SOURCE "},\n";
    } # else

    $op++;
} # while

print SOURCE "};\n\n";

close (SOURCE);

exit ();



# sub read_tahoe_ops ($)
# Takes the path to the m_tahoe.h file as the argument.
# This function reads the file looking for lines of the form:
# #define TAHOEop_XXXXXXX  <integer>.  It builds a hash where the
# symbolic constant name is keyed by the integer value.  It returns
# a two element list.  The first element is the offset of the lowest
# numbered opcode.  The second element is the hash.
sub read_tahoe_ops ($)
{
    use English;

    my ($header)=@ARG;

    my ($line, $offset);
    my (%result);

    open (HEADER, "< $header") or die ("Could not open $header for reading.");

    LINE: while ($line = <HEADER>)
    {
	# Skip all lines until we find one that starts with 
	# '#define TAHOEop'
	next LINE unless ($line =~ /^\#define (TAHOEop\S+)\s+(\d+)/);

	if ($1 eq "TAHOEop_START")
	{
	    $offset=$2-1;
	} # if

	$result{$2}=$1;
    } # LINE: while

    return ($offset, %result);
} # sub read_tahoe_ops
