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
                                                                                

# Portability.  Perl is located in different places on different systems.
# Search the user's path in sh, and then invoke perl on the remainder of
# the file.
perl -x $0 "$@"
exit $?
#!/usr/bin/perl

$usage = "merge_pfsam.pl -type prof_file...";

$type = shift || die $usage;

while (<>) {
  ($val,$name) = split;
  $res{$name} += $val;
}


if ($type eq "-ripc") {
  $der{"IPC"} = $res{IA64_INST_RETIRED} / $res{CPU_CYCLES};
  $der{"Non-NOP IPC"} = ($res{IA64_INST_RETIRED} - $res{NOPS_RETIRED}) / $res{CPU_CYCLES};
  $der{"Executed IPC"} = ($res{IA64_INST_RETIRED} - $res{PREDICATE_SQUASHED_RETIRED} - $res{NOPS_RETIRED}) / 
    $res{CPU_CYCLES};
}

if ($type eq "-ca") {
  $der{CPU_CYCLES} = $res{UNSTALLED_BACKEND_CYCLE} +
                     $res{MEMORY_CYCLE} +
		     $res{DEPENDENCY_ALL_CYCLE} +
		     $res{PIPELINE_ALL_FLUSH_CYCLE};
  $der{EXE} = $res{INST_FETCH_CYCLE} / $der{CPU_CYCLES};
  $der{DEP} = $res{EXECUTION_CYCLE} / $der{CPU_CYCLES};
  $der{MEM} = $res{MEMORY_CYCLE} / $der{CPU_CYCLES};
  $der{FLU} = $res{PIPELINE_ALL_FLUSH_CYCLE} / $der{CPU_CYCLES};
}

for $i (sort keys %res) {
  print "$i:$res{$i}\n";
}

for $i (sort keys %der) {
  printf "$i:%0.5f\n", $der{$i};
}
