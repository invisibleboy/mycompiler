#!/bin/sh
###############################################################################
##
##		      Illinois Open Source License
##                     University of Illinois/NCSA
##                         Open Source License
##
## Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
## All rights reserved.
##
## Developed by:             
##
##		IMPACT Research Group
##
##		University of Illinois at Urbana-Champaign
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
# Portability.  Perl is located in different places on different systems.
# Search the user's path in sh, and then invoke perl on the remainder of
# the file.
perl -x $0 "$@"
exit $?
#!/usr/bin/perl

# ASSUMPTIONS:
#   pfsum files are in the format benchmark.extension.input.<TYPE>.pfsum.
#   data.{ext}, if present, is the first file in the Lcode file list. 

$statistic_multiplier = 100;
$temp_file = "lcode.tmp";
$usage = "annot_pfsum.pl L_code_file_list summary_file(s)\n";
%funcs;

use File::Copy;

$file_list = shift || die $usage;

while ($temp1 = shift) {
  for ($i=0;$i<=$#summaries;$i++) {
    if ($temp1 eq @summaries[$i]) {
      print "File ", $temp1, " included multiple times for back-annotation.\n"; 
      next;
    }
  }
  push(@summaries, $temp1);
}

die $usage if ($#summaries == -1); 

for ($i=0; $i<=$#summaries; $i++) {
  $func = "";
  $cb = "";
  $sum_file = @summaries[$i];
  @line = split /\./, $sum_file;
  pop @line;
  $type = pop @line;  # find type of pf summary
  push @types, $type;

  open (SUM,"<$sum_file") || die "Failed to open $sum_file\n";
  $_=<SUM>;  # Ignore first line
  $_=<SUM>;  # Ignore second line
  $_=<SUM>;  # Ignore third line 
  while ($_=<SUM>) {
    chomp;
    @line = split /\s+|:/,$_;
    if ($#line == 1)  # function percentage
      {
        $func = @line[0];
        chop @line[1];
        $funcs{$func}[0]{$type} = int (@line[1]*$statistic_multiplier);
      } 
    elsif ($#line == 4)  # cb percentage
      {
        chop @line[4];
        $funcs{$func}[1]{@line[2]}{$type} = int (@line[4]*$statistic_multiplier);
      }
  }  # end of while loop

}  # end of for loop  

#
# Back annotate the assembly files with the data
#
open (LCODE_FILE_LIST,"<$file_list") || die "Failed to open $file_list\n";
while ($_=<LCODE_FILE_LIST>) {
  push @Lcode_files, $_;
}
close FILE_LIST;

# Remove data.{ext} from list if present: it is assumed to be the first one
$temp1 = @Lcode_files[0];
split /\./, $temp1;
if ($_[0] eq "data") {
  splice(@Lcode_files, 0, 1);
}

# Go through files 
FILE: for ($i=0; $i<=$#Lcode_files; $i++) {
  $writeback = 0;
  $insert_func_comments = 0;
  $insert_cb_comments = 0;
  chomp(@Lcode_files[$i]);
  $Lcode_file = @Lcode_files[$i];
  open (LCODE,"<$Lcode_file") || die "Failed to open $Lcode_file\n";
  open (TEMP_FILE,">$temp_file");
  LINE: while ($_=<LCODE>) {
    chomp; 
    $entire_line = $_;
    @line = split " ",$entire_line;

    # If comment, continue/next to next line
    if (@line[0] eq "#") {
      print TEMP_FILE $entire_line."\n";
      next LINE;
    }
    # Look for function name, then do a lookup in the hash table. If not present, exit file

    elsif ($insert_func_comments == 1) {
      $temp1 = index $entire_line, ">";
      if ($temp1 == -1) {
	print TEMP_FILE $entire_line."\n";
	next LINE;
      }
      else {
        # write back line
	@line = split />/, $entire_line;
	$temp1 = @line[0];  # $temp1 contains the string to append to
	@key_list = sort (keys %{$funcs{$func}[0]});
	for ($j=0;$j<=$#key_list;$j++) {
	  $temp2 = "\n\t(".@key_list[$j]." (".$funcs{$func}[0]{@key_list[$j]}."))";
	  $temp1 = join '', $temp1, $temp2;
	}
	$entire_line = join '>', $temp1, @line[1];
	print TEMP_FILE $entire_line."\n";
	$insert_func_comments = 0;
      }
    }
    elsif ($insert_cb_comments == 1) {
      $temp1 = index $entire_line, ">";
      if ($temp1 == -1) {
	print TEMP_FILE $entire_line."\n";
	next LINE;
      }
      else {
	@line = split />/, $entire_line;
	$temp1 = @line[0];
	@key_list = sort (keys %{$funcs{$func}[1]{$cb}});
	for ($j=0; $j<=$#key_list; $j++) {
	  $temp2 = "\n\t (".@key_list[$j]." (".$funcs{$func}[0]{@key_list[$j]}."))";
	  $temp1 = join '', $temp1, $temp2;
	}
	$entire_line = join '>', $temp1, @line[1];
	print TEMP_FILE $entire_line."\n";
	$insert_cb_comments = 0;
      }
    }

    elsif (@line[0] eq "\(function")
    {
      $func = @line[1];
      $func = reverse $func;
      chop $func;
      $func = reverse $func;

      # if function not in hash table, go to next file
      @key_list = keys %funcs;
      for ($j=0; $j<=$#key_list; $j++) {
        if (@key_list[$j] eq $func) {
          $writeback = 1;
          last;
        }
      }
      if ($writeback == 0) {
	close LCODE;
	close TEMP_FILE;
	last LINE;
      } 
      else {
        $temp1 = index $entire_line, ">";
        $temp1 = index $entire_line, ">", ($temp1+1);
        if ($temp1 == -1) {
          $insert_func_comments = 1;
	  print TEMP_FILE $entire_line."\n";
	  next LINE;
        }
        else {
	  # write back line
	  @line = split />/, $entire_line;
	  $temp1 = @line[1];  # $temp1 contains the string to append to
	  @key_list = sort (keys %{$funcs{$func}[0]});
	  for ($j=0;$j<=$#key_list;$j++) {
	    $temp2 = "\n\t(".@key_list[$j]." (".$funcs{$func}[0]{@key_list[$j]}."))" ;
	    $temp1 = join '', $temp1, $temp2;
	  }
	  $entire_line = join '>', @line[0], $temp1, @line[2];
	  print TEMP_FILE $entire_line."\n";
        }
      }
    }

    elsif (@line[0] eq "\(cb")
    {
      $cb = @line[1];
      # if cb not in hash table, continue
      @key_list = keys %{$funcs{$func}[1]};
      $temp1 = 0; 
      for ($j=0; $j<=$#key_list; $j++) {
        if (@key_list[$j] == $cb) {
          $temp1 = 1;
          last;
        }
      }
      if ($temp1 == 0) {
	print TEMP_FILE $entire_line."\n";
	next LINE;
      }
      else {
        $temp1 = index $entire_line, ">";
        $temp1 = index $entire_line, ">", ($temp1+1);
        if ($temp1 == -1) {
          $insert_cb_comments = 1;
	  print TEMP_FILE $entire_line."\n";
          next LINE;
        }
        else {
	  @line = split />/, $entire_line;
	  $temp1 = @line[1];
	  @key_list = sort (keys %{$funcs{$func}[1]{$cb}});
	  for ($j=0; $j<=$#key_list; $j++) {
	    $temp2 = " (".@key_list[$j]." (".$funcs{$func}[0]{@key_list[$j]}."))";
	    $temp1 = join '', $temp1, $temp2;
	  }
	  $entire_line = join '>', @line[0], $temp1, @line[2];
	  print TEMP_FILE $entire_line."\n";
	  $insert_cb_comments = 0;
        }
      }
    }
    else {
      print TEMP_FILE $entire_line."\n";
    }
  }  # end of LINE loop

  close LCODE;
  close TEMP_FILE;
  if ($writeback == 1)
    {
      copy($temp_file, $Lcode_file);
    }
}  # end of FILE loop

print "> Done back-annotating Lcode files.\n"

# ----------------------------------------------------------------------
# SUBROUTINES
# ----------------------------------------------------------------------

