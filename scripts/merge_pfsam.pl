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

###############################################################################
# merge_pfsam.pl                                                              #
# --------------------------------------------------------------------------- #
# Increase readability of pfsam output by merging with NM and CB offset       #
# information.                                                                #
# --------------------------------------------------------------------------- #
# (C) John W. Sias, Wen-mei W. Hwu and the Board of Trustees of the           #
#     University of Illinois,  2002                                           #
###############################################################################

use POSIX;

# ----------------------------------------------------------------------
# PROTOTYPES
# ----------------------------------------------------------------------

sub str2hexqw ($);
sub hexqw2str (@);
sub hexqwadd (@);
sub print_result_table ($);
sub cracklocal($);

# ----------------------------------------------------------------------
# OPTIONS
# ----------------------------------------------------------------------

$identpat="[A-Za-z_\@\$\?\.][A-Za-z_0-9\@\$\?\.]*"; # Identifier regexp

$do_merge = 0;
$do_debug = 0;
$do_bundles = 0;
$do_print_ipc = 0;
$do_print_fpc = 0;
$do_print_mst = 0;
$do_print_ist = 0;
$do_print_raw = 0;
$do_print_ripc = 0;
$asmfilelist = undef;
$outext = "SUM";

%opttbl = ( "-merge"   => [\$do_merge, '1',
			   "merge results across all pfsam files"],
	    "-debug"   => [\$do_debug, '1',"turn on debugging"],
	    "-bundles" => [\$do_bundles, '1',"turn on bundle output"],
	    "-ipc"     => [\$do_print_ipc, '1',"turn on IPC mode"],
	    "-fpc"     => [\$do_print_fpc, '1',"turn on FPC mode"],
	    "-mst"     => [\$do_print_mst, '1',"turn on MST mode"],
	    "-ist"     => [\$do_print_ist, '1',"turn on IST mode"],
	    "-ripc"    => [\$do_print_ripc, '1',"turn on RIPC mode"],
	    "-rawf"    => [\$do_print_rawf, '1',"turn on RAWF mode"],
	    "-out"     => [\$outext, 'shift || die $usage',
			   "use specified output extension"],
	    "-asm"     => [\$asmfilelist, 'shift || die $usage',
			   "use specified assembly file list for cb labels"]);

$usage = "merge_pfsam.pl nm_file [-asm assembly_file_list]\n" . 
	 "	[-merge] [-debug] [-bundles] [-ipc] [-mst] [-ist] [-ripc]\n" . 
	 "	[-raw] [-out EXT] (pfsam_file)+\n";

#
# Read arguments
#

($nm = shift) || die $usage;

while ($opt = shift) {
    if (substr($opt,0,1) eq "-") {
	# option token
	if (($ov, $of, $os) = @{$opttbl{$opt}}) {
	    $$ov = eval $of;
	    printf ("> Option: $os\n");
	} else {
	    die "Unknown option \"$opt\"\n$usage";
	}
    } elsif (-f $opt) {
	# pfsam file
	push @proflist, $opt;
    } else {
	# oops
	die "Missing file \"$opt\"\n$usage";
    }
}

@proflist || die $usage;

#
# Process NM file
#

print "> Processing symbol file $nm\n";

open (NM,"<$nm") || die "Failed to open $nm\n";
while ($_=<NM>) {
  chomp;
  ($addrstr, $type, $sym) = split " ",$_;

  if ($type eq "T" || $type eq "t") {
      my @addr;
      @addr = str2hexqw($addrstr);
      if ($fnaddr{$sym}) {
         $mngl = $sym;
         $k = 1;
         for ($k = 1; defined $fnaddr{$mngl}; $k++) {
           $mngl = "$sym" . "\@0x$addrstr";
         }
	 $sym = $mngl;
      }
      $fnaddr{$sym} = [$addr[0],$addr[1]];
      push @inorderflist, $sym;
    }
}
close NM;

#
# Process assembly file list, if provided

#
if ($asmfilelist) {

    open (ASSEM_LIST,"<$asmfilelist") || die "Failed to open $asmfilelist\n";
    while ($_=<ASSEM_LIST>) {
	chomp $_;
	push @files, $_;
    }
    close ASSEM_LIST;

    #
    # Read assembly files and compute offsets for each local label
    #

    print "> Processing assembly files from $asmfilelist: ";

    while ($assem = shift (@files)) {
	open (ASSEM,"<$assem") || die "Failed to open $assem\n";
	print ".";
	$infunc = 0;
	$bun_ctr = 0;
	while ($_=<ASSEM>) {
	    if (/^\s*($identpat)(:+)/) {
		$sym = $1;
		$symtm = $2;
		if ($infunc && $symtm eq ":") {
		    # local (cb) label
		    @caddr = hexqwadd(@addr,16*$bun_ctr);
		    push @{$cblab{$cur_func}}, [@caddr,$sym];
		    # $cstr = hexqw2str (@caddr);
		    # printf "L $sym $cstr\n";
		} elsif ($symtm eq "::") {
		    if (@addr = @{$fnaddr{$sym}}) {
			# global text label
			# $cstr = hexqw2str (@addr);
			# print "G $sym $cstr\n";
			$cur_func = $sym;
			$bun_ctr = 0;
			$infunc = 1;
			$fnfile{$sym} = $assem;
		    } else {
			# other global label
			$infunc = 0;
		    }
		}
	    }
	    $bun_ctr++ if /\{/;
	}
	close ASSEM;
    }
    print "(done)\n";
}

# Generate the table of all labels.  Functions are in order (assuming
# nm -n) and cb's are in order on a per-function basis.

# $sym[i] = [addr_low32, addr_high32, symbol_name]

foreach $fn (@inorderflist) {
    my (@addr, @cbs);

    @addr = @{$fnaddr{$fn}};

    printf "%s G $fn\n", &hexqw2str(@addr) if $do_debug;

    push @sym, [@addr, $fn];

    # add any cb labels belonging to $fn

    @cbs = @{$cblab{$fn}};

    foreach $cb (@cbs) {
	push @sym, [@$cb];
	printf "%s L $$cb[2]\n", &hexqw2str(@$cb) if $do_debug;
    }
}

@cblab = undef;

#
# Process the PFSAM files
#

foreach $proffilename (@proflist) {
    my ($sumfilename, $event_name, $next_name);

    if (!$do_merge) {
	%rate = undef;
	%RESU = undef;
    }

    open (FPFSAM,"<$proffilename") || die "Failed to open $proffilename\n";

    print "> Reading $proffilename\n";

    # Identify a section header

    while ($_ = <FPFSAM>) {
	if (/^>>> (\w+) (\d+)/) {
	    $next_name = $1;
	    $rate{$next_name} = $2;
	    last;
	} elsif (/^>>> (\w+)/) {
	    $next_name = $1;
	    $rate{$next_name} = "?";
	    last;
	}
    }
    
    do {
	my ($totwt, $funcwt, $fcbwt, $funccbs, %symwt, $bundwt, $bn_array);

	$event_name = $next_name;

	#
	# Read an entire section into pdata array and identify the next
	# section, if present.
	#
	undef @pdata;
	
	while ($_ = <FPFSAM>) {
	    if (/>>> (\w+) (\d+)/) {
		$next_name = $1;
		$rate{$next_name} = $2;
		last;
	    } elsif (/>>> (\w+)/) {
		$next_name = $1;
		$rate{$next_name} = "?";
		last;
	    } else {
		chomp;
		@line = split ":",$_;
		push @pdata, [str2hexqw($line[0]), $line[1]];
	    }
	}

	undef $next_name if (! $_ ); # end of file encountered

	($totwt, $funcwt, $funccbs, $bn_array, $fcbwt) = @{$RESU{$event_name}};

	# Sort profile records by address

	# @pdata = sort {$a->[1] <=> $b->[1] or $a->[0] <=>$b->[0]} @pdata;

	# Match profile data to symbols

	my $nmidx = 0;
	    
	for $prec (@pdata) {
	    ($hl, $hh, $wt) = @$prec;
	    while (($nmidx < $#sym) && ( ($sym[$nmidx+1]->[1] < $hh) || 
					 (($sym[$nmidx+1]->[1] == $hh) && 
					  ($sym[$nmidx+1]->[0] <= $hl)))) {
		$nmidx++;
	    }
	    $symnm = $sym[$nmidx]->[2];
	    $symwt->{$symnm} += $wt;

	    ($func, $cb) = cracklocal ($symnm);

	    $fcbwt->{$func}->{$cb} += $wt if $cb;
	    $funcwt->{$func}+= $wt;
	    $totwt += $wt;

	    if ($do_bundles && $cb) {
		$prof_addr = $hl;
		$sym_addr = $sym[$nmidx]->[0];
		
		$bund = floor (($prof_addr - $sym_addr) / 16);
		$bund++;
		    
		$bundwt->{$symnm}->[$bund] += $wt if ($bund > 0);
	    }
	}

        if ($do_bundles) {
	    for $symrec (@sym) {
		$symnam = $symrec->[2];
		$swt = $symwt->{$symnam};
		
		next if (!$swt);
	    
		for ($p = 0; $p < $#{$bundwt->{$symnam}}; $p++) {
		    if ($temp3 = $bundwt->{$symnam}->[$p]) {
			$bn_pct1 = 100*$temp3 / $swt;
			$bn_pct2 = 100*$temp3 / $totwt;
			push @{$bn_array->{$symnam}}, [$p, $temp3, $bn_pct1, 
						       $bn_pct2];
		    }
		}
	    }
	}

        # Store results
	
        $RESU{$event_name} = [$totwt, $funcwt, $funccbs, $bn_array, $fcbwt];

    } while ($next_name);

    close FPFSAM;

    #
    # Print results if not merging
    #

    if (!$do_merge) {
	$sumfilename = $proffilename . "." . $outext;
	create_cb_records(\%RESU);
	open (FSUM,">$sumfilename") || die "Failed to open $sumfilename\n";
	print "> Writing $sumfilename\n";
	print_result_table(\%RESU);
	%RESU = undef;
	close FSUM;
    }
}

#
# Print merged results if appropriate
#

if ($do_merge) {
    $sumfilename = "MERGE" . "." . $outext;
    create_cb_records(\%RESU);
    open (FSUM,">$sumfilename") || die "Failed to open $sumfilename\n";
    print "> Writing $sumfilename\n";
    print_result_table(\%RESU);
    %RESU = undef;
    close FSUM;
}

exit 0;

# ----------------------------------------------------------------------
# SUBROUTINES
# ----------------------------------------------------------------------

sub cracklocal($) {
    my ($func, $cb, $str);

    $str = shift;

    if (substr($str,0,1) eq ".") {
	# cb label
	$str =~ /\.($identpat)_(\d+)/;
	$func = $1;
	$cb = $2;    
    } else {
	$func = $str;
    }
    
    return ($func, $cb);
}

sub create_cb_records($) {
    my ($event_name);
    $restbl = shift;

    foreach $event_name (keys %$restbl) {
	my ($totwt, $funcwt, $funccbs, $bn_array, $fcbwt, $fn, $cb);
	($totwt, $funcwt, $funccbs, $bn_array, $fcbwt) = 
	    @{$restbl->{$event_name}};

	foreach $fn (keys %$fcbwt) {
	    foreach $cb (sort keys %{$fcbwt->{$fn}}) {
		$wt = ${$fcbwt->{$fn}}{$cb};

	        # print "$fn (cb $cb) $wt / $funcwt->{$fn}\n";

	        $pct = 100 * $wt / $totwt;
		$fpct = 100 * $wt / $funcwt->{$fn};

		push @{$funccbs->{$fn}}, [$cb, $wt, $fpct, $pct];
	    }
	}

	$restbl->{$event_name} = [$totwt, $funcwt, $funccbs, $bn_array];
    }
}

sub str2hexqw ($) {
    my ($s, $h, $l);
    $s = shift;
    $h = hex(substr(($s),0,8));
    $l = hex(substr(($s),8,8));
    return ($l, $h);
}

sub hexqw2str (@) {
    my ($s, $h, $l);
    $l = shift;
    $h = shift;
    $s = sprintf ("%08X%08X", $h, $l);
    return $s;
}

sub hexqwadd (@) {
    use integer;
    my ($s, $h, $l, $i);
    $l = shift;
    $h = shift;
    $i = shift;
    $s = $l + $i;
    if (($l >= 0)^($s >= 0)) {
	$death = sprintf "Overflow occurred in " . 
	    "hexqwadd(0x%08X%08X + 0x%08X)\n", $h, $l, $i;
	die $death;
    }
    return ($s,$h);
}

sub print_result_table($) {

    my $restbl;
    $restbl = shift;
    
    if ($do_print_ipc) {
	my (@clocks, @retire);

	@clocks = @{$restbl->{"CPU_CYCLES"}};
	@retire = @{$restbl->{"IA64_INST_RETIRED"}};

	die "Insufficient data to print IPC statistics\n"
	    if (! @clocks || ! @retire);

	print_rat ("IPC", $clocks[0], $retire[0], $clocks[1], $retire[1], 
		   $clocks[2], $retire[2]);
    } elsif ($do_print_fpc) {
        my (@clocks, @retire);

        @clocks = @{$restbl->{"CPU_CYCLES"}};
        @allflush = @{$restbl->{"PIPELINE_ALL_FLUSH_CYCLE"}};
        @backflush = @{$restbl->{"PIPELINE_BACKEND_FLUSH_CYCLE"}};

        die "Insufficient data to print FPC statistics\n"
            if (! @clocks || ! @allflush || !backflush);

        print_rat ("FPC", $clocks[0], $allflush[0], $clocks[1], $allflush[1],
                   $clocks[2], $allflush[2]);
        print_rat ("FPC", $clocks[0], $backflush[0], $clocks[1], $backflush[1],
                   $clocks[2], $backflush[2]);
    } elsif ($do_print_rawf) {
	my (@clocks, @retire);

	@clocks = @{$restbl->{"CPU_CYCLES"}};
	@retire = @{$restbl->{"IA64_INST_RETIRED"}};

	die "Insufficient data to print RAW statistics\n"
	    if (! @clocks || ! @retire);

	print_raw ("IPC", $clocks[0], $retire[0], $clocks[1], $retire[1], 
		   $clocks[2], $retire[2]);
    } elsif ($do_print_mst) {
	my (@clk, @mclk);
	
	@clk = @{$restbl->{"CPU_CYCLES"}};
	@mclk = @{$restbl->{"DATA_ACCESS_CYCLE"}};
	
	die "Insufficient data to print MST statistics\n"
	    if (! @clk || ! @mclk);

	print_rat ("MEM STALL RATIO", $clk[0], $mclk[0], $clk[1], $mclk[1], 
		   $clk[2], $mclk[2]);
    } elsif ($do_print_ist) {
	my (@clk, @mclk);

	@clk = @{$restbl->{"CPU_CYCLES"}};
	@mclk = @{$restbl->{"INST_ACCESS_CYCLE"}};

	die "Insufficient data to print IST statistics\n"
	    if (! @clk || ! @mclk);

	print_rat ("INST STALL RATIO", $clk[0], $mclk[0], $clk[1], $mclk[1], 
		   $clk[2], $mclk[2]);
    } elsif ($do_print_ripc) {
	my (@clocks, @retire);
	@clocks = @{$restbl->{"CPU_CYCLES"}};
	@retire = @{$restbl->{"IA64_INST_RETIRED"}};
	# @nop = @{$restbl->{"NOPS_RETIRED"}};
	# @squash = @{$restbl->{"PRED_SQUASHED_RETIRED"}};
	if (! @clocks || ! @retire) {
	    die "Insufficient data to print IPC statistics\n";
	}
	print_ipc ($clocks[0], $retire[0], $clocks[1], $retire[1], 
		   $clocks[2], $retire[2]);
    } else {
	for $k (sort keys %$restbl) {
	    print_results ($k, $rate{$k}, @{$restbl->{$k}});
	}
    }
}

sub print_results {
  my ($evname, $rate, $totwt, $pfwt, $pfcb, $pbna) = @_;
  my %funcwt = %{$pfwt};
  my %funccbs = %{$pfcb};
  my %bn_array = %{$pbna};

  print FSUM "-" x 79 . "\n";
  print FSUM "FUNCTION SUMMARY ($evname $totwt samples @ rate $rate)\n";
  print FSUM "-" x 79 . "\n";

  for $f (sort {$funcwt{$b} <=> $funcwt{$a}} keys %funcwt) {
      $pct = 100.0 * $funcwt{$f} / $totwt;
      printf FSUM "%s:%s:%0.05f%%\n",$f,$fnfile{$f},$pct;

      if (@cbs = @{$funccbs{$f}}) {
	  for $c (sort { $b->[1] <=> $a->[1] } @cbs ) {
	      printf FSUM "\tcb %5d %10.05f%% %10.05f%%\n", 
	      $c->[0], $c->[2], $c->[3];

	      if ($do_bundles) {
		  @bunds = @{$bn_array{$f . "?" . $c->[0]}};
		  for $q (sort { $b->[1] <=> $a->[1] } @bunds) {
		      printf FSUM "\t\tbundle %5d %10.05f%% %10.05f%%\n", 
		      $q->[0], $q->[2], $q->[3];
		  }
	      }
	  }
      }
  }
}

sub print_rat {
    # clocks, inst. retired, fn clocks, fn rets, cb clocks, cb rets
    my ($name, $clk, $ret, $clkf, $retf, $clkc, $retc) = @_;

    my %fn_clk = %{$clkf};
    my %fn_ret = %{$retf};
    my %cb_clk = %{$clkc};
    my %cb_ret = %{$retc};

    my $bm_ipc = $ret / $clk;

    print  FSUM "-" x 79 . "\n";
    printf FSUM "FUNCTION $name SUMMARY BY CLOCK" . 
	" ($clk samples) $name=%0.3f\n", 
	$bm_ipc;
    print  FSUM "-" x 79 . "\n";

    for $f (sort {$fn_clk{$b} <=> $fn_clk{$a} or $a cmp $b} keys %fn_clk) {
	$pct = 100.0 * $fn_clk{$f} / $clk;

	$ipc = $fn_clk{$f} ? ($fn_ret{$f} / $fn_clk{$f}) : 0.0;

	printf FSUM "%s:%s:%0.3f:%0.05f%%\n",$f,$fnfile{$f},$ipc,$pct;

	if (@rck = @{$cb_ret{$f}}) {
	    @cck = @{$cb_clk{$f}};
	    for $c (sort { $b->[1] <=> $a->[1] } @cck ) {
		my $ipc = 0;
		for $r ( @rck ) {
		    if ($r->[0] == $c->[0]) {
			$ipc = $r->[1] / $c->[1];
			last;
		    }
		}
		printf FSUM "\tcb %5d %6.03f %10.05f%% %10.05f%%\n", $c->[0], 
		$ipc, $c->[2], $c->[3];
	    }
	}
    }

    print  FSUM "-" x 79 . "\n";
    printf FSUM "FUNCTION $name SUMMARY BY RET" .
	" ($ret samples) $name=%0.3f\n", $bm_ipc;
    print  FSUM "-" x 79 . "\n";

    for $f (sort {$fn_ret{$b} <=> $fn_ret{$a} or $a cmp $b} keys %fn_ret) {
	$pct = 100.0 * $fn_ret{$f} / $ret;
	
	$ipc = $fn_clk{$f} ? ($fn_ret{$f} / $fn_clk{$f}) : 0.0;
	
	printf FSUM "%s:%s:%0.3f:%0.05f%%\n",$f,$fnfile{$f},$ipc,$pct;

	if (@cck = @{$cb_clk{$f}}) {
	    @rck = @{$cb_ret{$f}};
	    for $r (sort { $b->[1] <=> $a->[1] } @rck ) {
		my $ipc = 0;
		for $c ( @cck ) {
		    if ($r->[0] == $c->[0]) {
			$ipc = $r->[1] / $c->[1];
			last;
		    }
		}
		printf FSUM "\tcb %5d %6.03f %10.05f%% %10.05f%%\n", 
		    $r->[0], $ipc, $r->[2], $r->[3];
	    }
	}
    }
}


sub print_raw {
    # clocks, inst. retired, fn clocks, fn rets, cb clocks, cb rets
    my ($name, $clk, $ret, $clkf, $retf, $clkc, $retc) = @_;

    my %fn_clk = %{$clkf};
    my %fn_ret = %{$retf};
    my %cb_clk = %{$clkc};
    my %cb_ret = %{$retc};

    my $bm_ipc = $ret / $clk;

    print  FSUM "-" x 79 . "\n";
    printf FSUM "FUNCTION $name RAW SUMMARY " . 
	"($clk samples) clk ret $name=%0.3f\n", $bm_ipc;
    print  FSUM "-" x 79 . "\n";

    for $f (sort {$fn_clk{$b} <=> $fn_clk{$a} or $a cmp $b} keys %fn_clk) {
	$pct = 100.0 * $fn_clk{$f} / $clk;

	$ipc = $fn_clk{$f} ? ($fn_ret{$f} / $fn_clk{$f}) : 0.0;

	printf("%s:%s:$fn_clk{$f}:$fn_ret{$f}:%0.3f\n", $f, $fnfile{$f}, $ipc);
    }
}




