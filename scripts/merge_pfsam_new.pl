#!/bin/sh
# Portability.  Perl is located in different places on different systems.
# Search the user's path in sh, and then invoke perl on the remainder of
# the file.
perl -x $0 "$@"
exit $?
#!/usr/bin/perl
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
$do_print_btb = 0;
$do_print_tpct = 0;
$do_gnu = 0;
$asmfilelist = undef;
$outext = "SUM";

%opttbl = ( "-merge"   => [\$do_merge, '1',
			   "merge results across all pfsam files"],
	    "-debug"   => [\$do_debug, '1',"turn on debugging"],
	    "-bundles" => [\$do_bundles, '1',"turn on bundle output"],
	    "-ipc"     => [\$do_print_ipc, '1',"turn on IPC mode"],
	    "-tpct"    => [\$do_print_tpct, '1',"turn on TPCT mode"],
	    "-crel"    => [\$do_print_crel, '1',"turn on CREL mode"],
	    "-fpc"     => [\$do_print_fpc, '1',"turn on FPC mode"],
	    "-mst"     => [\$do_print_mst, '1',"turn on MST mode"],
	    "-ist"     => [\$do_print_ist, '1',"turn on IST mode"],
	    "-ripc"    => [\$do_print_ripc, '1',"turn on RIPC mode"],
	    "-rawf"    => [\$do_print_rawf, '1',"turn on RAWF mode"],
	    "-out"     => [\$outext, 'shift || die $usage',
			   "use specified output extension"],
	    "-gnu"     => [\$do_gnu, '1', "expect asm in gnu as format"],
	    "-ias"     => [\$do_gnu, '0', "expect asm in ias format"],
	    "-asm"     => [\$asmfilelist, 'shift || die $usage',
			   "use specified assembly file list for cb labels"]);

$usage = "merge_pfsam.pl nm_file [-asm assembly_file_list]\n" . 
	 "	[-merge] [-debug] [-bundles] [-ipc] [-mst] [-ist] [-ripc]\n" . 
	 "	[-raw] [-out EXT] (pfsam_file)+\n";

$do_bundles = 1 if ($do_print_btb);

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

	# ias format asm uses two colons to mark global symbols.  GNU as format
	# uses a .global <symbol># line, then a single colon on the symbol.
	# cur_global will hold the last string found in a .global line.
	$cur_global = "";
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
		} elsif (($do_gnu == 0 && $symtm eq "::") ||
			 ($do_gnu == 1 && $sym eq $cur_global)) {
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
	    } elsif (/^\s*\.global ($identpat)#/) {
		$cur_global = $1;
	    }
		     
	    $bun_ctr++ if /\{/;    # } A closing brace to make emacs happy.
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
	undef %rate;
	undef %RESU;
    }

    open (FPFSAM,"<$proffilename") || die "Failed to open $proffilename\n";

    print "> Reading $proffilename\n";

    # Read header line

    while ($_ = <FPFSAM>) {
	if (/^(IIP:)?\w+ \d+ \d+/) {
	    $ldr = $1;
	    @dclass = split ':', $_;
            shift @dclass if ($ldr eq "IIP:");
	    for $dc (@dclass) {
		@dinfo = split ' ', $dc;
		push @name, $dinfo[0];
		$rate{@name[$#name]} = $dinfo[1];
	    }
	    last;
	}
    }

    my ($totwt, $funcwt, $fcbwt, $funccbs, %symwt, $bundwt, $bn_array);

    #
    # Read an entire section into pdata array and identify the next
    # section, if present.
    #
    undef @pdata;
	
    while ($_ = <FPFSAM>) {
	chomp;
	@line = split ":",$_;
	$addr = shift @line;
	push @pdata, [str2hexqw($addr), @line];
    }
    
    # Match profile data to symbols

    for $i (0..$#name) {
	my $nmidx = 0;

	$ename = $name[$i];
	($totwt, $funcwt, $funccbs, $bn_array, $fcbwt) = @{$RESU{$ename}};

	for $prec (@pdata) {
	    my @erec;
	    @erec = @$prec;
	    $hl = shift @erec;
	    $hh = shift @erec;
	    $wt = @erec[$i];

	    next if (!$wt);
	    
	    # Advance symbol pointer
	    
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

		$bundwt->{$symnm}->[$bund] += $wt if ($bund >= 0);
	    }
	}
	
	if ($do_bundles) {
	    for $symrec (@sym) {
		$symnm = $symrec->[2];
		$swt = $symwt->{$symnm};

		next if (!$swt || !$totwt);
		
		for ($p = 0; $p <= $#{$bundwt->{$symnm}}; $p++) {
		    if ($temp3 = $bundwt->{$symnm}->[$p]) {
			$bn_pct1 = 100*$temp3 / $swt;
			$bn_pct2 = 100*$temp3 / $totwt;
			push @{$bn_array->{$symnm}}, 
			     [$p, $temp3, $bn_pct1, $bn_pct2];
		    }
		}
	    }
	}

	# Store results
	    
	$RESU{$ename} = [$totwt, $funcwt, $funccbs, $bn_array, $fcbwt];
    }

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
    
    if ($do_print_crel) {
        foreach $key (keys %{$restbl}) {
	  my (@clocks, @retire);
	  next if ($key eq "CPU_CYCLES");

	  @clocks = @{$restbl->{"CPU_CYCLES"}};
	  @retire = @{$restbl->{$key}};

	  die "Insufficient data to print IPC statistics\n"
	      if (! @clocks || ! @retire);

	  print_rat ("$key-PER-CYCLE", $clocks[0], $retire[0], $clocks[1], $retire[1], 
	  	     $clocks[2], $retire[2]);
        }
    } elsif ($do_print_ipc) {
	my (@clocks, @retire);

	@clocks = @{$restbl->{"CPU_CYCLES"}};
	@retire = @{$restbl->{"IA64_INST_RETIRED"}};

	die "Insufficient data to print IPC statistics\n"
	    if (! @clocks || ! @retire);

	print_rat ("IPC", $clocks[0], $retire[0], $clocks[1], $retire[1], 
		   $clocks[2], $retire[2]);
    } elsif ($do_print_tpct) {
	my (@clocks, @retire);

	print "Printing TPCT\n";

	@clocks = @{$restbl->{"CPU_CYCLES"}};

	for $rt (sort keys %$restbl) {
	    print "$rt\n";
	    next if ($rt eq "CPU_CYCLES");

	    @retire = @{$restbl->{"$rt"}};

	    die "Insufficient data to print IPC statistics\n"
		if (! @clocks || !@retire);
	    $name = $rt . " / CY";
	    print_tpct ($name, $clocks[0], $retire[0], 
			$clocks[1], $retire[1], 
			$clocks[2], $retire[2]);
        }
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
	@nop = @{$restbl->{"NOPS_RETIRED"}};
	@squash = @{$restbl->{"PREDICATE_SQUASHED_RETIRED"}};
	if (! @clocks || ! @retire) {
	    die "Insufficient data to print IPC statistics\n";
	}

	print_rat ("IPC", $clocks[0], $retire[0], $clocks[1], $retire[1], 
		   $clocks[2], $retire[2]);

	print_rat ("SqPC", $clocks[0], $squash[0], $clocks[1], $squash[1], 
		   $clocks[2], $squash[2]);
    } else {
	for $k (sort keys %$restbl) {
	    print_results ($k, $rate{$k}, @{$restbl->{$k}});
	}
    }
}

sub engstring {
    my ($val) = @_;
    my $str, $red;

    if (abs($val) > 1e12) {
	$str = sprintf ("%0.3fG", $val / 1e12);
    } elsif (abs($val) > 1e9) {
	$str = sprintf ("%0.3fB", $val / 1e9);
    } elsif (abs($val) > 1e6) {
	$str = sprintf ("%0.3fM", $val / 1e6);
    } elsif (abs($val) > 1e3) {
	$str = sprintf ("%0.3fk", $val / 1e3);
    } else {
	$str = sprintf ("%0.3f", $val);
    }

    return $str;
}

sub print_btb {
  my ($evname, $rate, $totwt, $pfwt, $pfcb, $pbna) = @_;
  my %funcwt = %{$pfwt};
  my %funccbs = %{$pfcb};
  my %bn_array = %{$pbna};

  print FSUM "-" x 79 . "\n";
  print FSUM "FUNCTION SUMMARY ($evname $totwt samples @ rate $rate)\n";
  print FSUM "-" x 79 . "\n";

  for $f (sort {$funcwt{$b} <=> $funcwt{$a}} keys %funcwt) {
      $pct = 100.0 * $funcwt{$f} / $totwt;
      $eng = engstring($funcwt{$f} * $rate);
      printf FSUM "%s:%s:%ss:%0.05f%%\n",$f,$fnfile{$f},$eng,$pct;

      if (@cbs = @{$funccbs{$f}}) {
	  for $c (sort { $b->[1] <=> $a->[1] } @cbs ) {
	      printf FSUM "\tcb %5d %10.05f%% %10.05f%%\n", 
	      $c->[0], $c->[2], $c->[3];

	      if ($do_bundles) {
		  @bunds = @{$bn_array{"." . $f . "_" . $c->[0]}};
		  for $q (sort { $b->[1] <=> $a->[1] } @bunds) {
		      printf FSUM "\t\tbundle %5d %10.05f%% %10.05f%%\n", 
		      $q->[0], $q->[2], $q->[3];
		  }
	      }
	  }
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
      printf FSUM "%s:%s:%0.05f%%:%d\n",$f,$fnfile{$f},$pct, $funcwt {$f};

      if (@cbs = @{$funccbs{$f}}) {
	  for $c (sort { $b->[1] <=> $a->[1] } @cbs ) {
	      printf FSUM "\tcb %5d %10.05f%% %10.05f%% %12d\n", 
	      $c->[0], $c->[2], $c->[3], $c->[2] * $funcwt{$f};

	      if ($do_bundles) {
		  @bunds = @{$bn_array{"." . $f . "_" . $c->[0]}};
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
	$fclocks = $fn_clk{$f};
	$pct = 100.0 * $fclocks / $clk;

	$ipc = $fclocks ? ($fn_ret{$f} / $fclocks) : 0.0;

	$eng = engstring($fclocks);
	printf FSUM "%s:%s:%ss:%0.3f:%0.05f%%\n",$f,$fnfile{$f},$eng,$ipc,$pct;

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
		printf FSUM "\tcb %5d %6.03f %10.05f%% %10.05f%% %ss\n", 
		            $c->[0], $ipc, $c->[2], $c->[3], 
		            engstring($c->[1]);
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
	
	$eng = engstring($fn_ret{$f});
	printf FSUM "%s:%s:%ss:%0.3f:%0.05f%%\n",$f,$fnfile{$f},$eng,$ipc,$pct;

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


sub print_tpct {
    # clocks, inst. retired, fn clocks, fn rets, cb clocks, cb rets
    my ($name, $clk, $ret, $clkf, $retf, $clkc, $retc) = @_;

    my %fn_clk = %{$clkf};
    my %fn_ret = %{$retf};
    my %cb_clk = %{$clkc};
    my %cb_ret = %{$retc};

    my $bm_ipc = $ret / $clk;

    print  FSUM "-" x 79 . "\n";
    printf FSUM "FUNCTION $name SUMMARY BY CLOCK\n" . 
	"($ret / $clk samples) RAT=%0.3f\n", 
	$bm_ipc;
    print  FSUM "-" x 79 . "\n";

    for $f (sort {$fn_clk{$b} <=> $fn_clk{$a} or $a cmp $b} keys %fn_clk) {
	$fclocks = $fn_clk{$f};
	$pct = 100.0 * $fclocks / $clk;

	$ipc = $fclocks ? ($fn_ret{$f} / $fclocks) : 0.0;

	$eng = engstring($fclocks);
	printf FSUM "%s:%s:%ss:%0.3f:%0.05f%%\n",$f,$fnfile{$f},$eng,$ipc,$pct;

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
		printf FSUM "\tcb %5d %6.03f %10.05f%% %10.05f%% %ss\n", 
		            $c->[0], $ipc, $c->[2], $c->[3], 
		            engstring($c->[1]);
	    }
	}
    }

    print  FSUM "-" x 79 . "\n";
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




