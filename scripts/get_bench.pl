#!/usr/bin/env perl
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
#
# 8/13/02 Robert Kidd
# This script copies the benchmark files to the current directory and builds
# a Makefile to compile them with oicc.

use strict;
use English;
use Getopt::Long;

my $defaultCompilerInvocation="oicc --verbose";

my ($releasePath, $hostPlatform, $hostCompiler, $benchPath, $benchmark);
my ($mckinley, $itanium, $help, $baselineParmsFile, $projectName);
my ($projectDir, $projectParmsFile, $benchDest, $userBenchPath);
my ($compilerInvocation, $input, $file, $line, $sourceFileString);
my (@sourceFiles);
my (%compileInfo);

$releasePath=$ENV{'IMPACT_REL_PATH'};
if ($releasePath eq "")
{
    print "Error: Environment variable IMPACT_REL_PATH is not set.\n";
    exit;
} # if

$hostPlatform=$ENV{'IMPACT_HOST_PLATFORM'};
if ($hostPlatform eq "")
{
    print "Error: Environment variable IMPACT_HOST_PLATFORM is not set.\n";
    exit;
} # if

$hostCompiler=$ENV{'IMPACT_HOST_COMPILER'};
if ($hostCompiler eq "")
{
    print "Error: Environment variable IMPACT_HOST_COMPILER is not set.\n";
    exit;
} # if

$userBenchPath=$ENV{'USER_BENCH_PATH'};
if ($userBenchPath eq "")
{
    $userBenchPath="$releasePath/benchmarks";
} # if

# Initialize the variables used to hold command line options.
$itanium=0;
$mckinley=1;
$baselineParmsFile="";
$projectName="";

# Handle the command line arguments.
GetOptions ("help"      => \$help,
	    "itanium"   => \$itanium,
	    "mckinley"  => \$mckinley,
	    "o=s"       => \$benchDest,
	    "p=s"       => \$baselineParmsFile,
	    "project=s" => \$projectName);

# Handle the command line options.
if ($baselineParmsFile eq "")
{
    if ($mckinley)
    {
	$baselineParmsFile="${releasePath}/parms/STD_PARMS.IPF-MCKINLEY";
    } # if
    elsif ($itanium)
    {
	$baselineParmsFile="${releasePath}/parms/STD_PARMS.IPF-ITANIUM";
    } # elsif
} # if

if ($projectName eq "")
{
    $projectName=$ENV{'DEFAULT_PROJECT'};
    if ($projectName eq "")
    {
	print "Error: Environment variable DEFAULT_PROJECT is not set.\n";
	print "       --project was not specified on command line.\n";
	exit;
    } # if
} # if

$projectDir=`find_project_dir ${projectName}`;
chomp ($projectDir);

# Show the help if --help is specified or if no benchmark is specified.
if (scalar (@ARGV) == 0 || $help) { show_help (); }

$benchmark=$ARGV[0];
$benchPath="$userBenchPath/$benchmark";
# If benchDest isn't specified, set it to the name of the benchmark
# This is the name of the directory we create in the current directory for
# the benchmark.
if ($benchDest eq "")
{
    $benchDest=$benchmark;
} # if

$projectParmsFile=`read_project_info ${benchmark} -parm_override_file`;
chomp ($projectParmsFile);

# Make sure the user specified a real benchmark.
if ( ! -d $benchPath ) { show_help (); }

### Figure out how we're going to build this benchmark ###
# If the environment variable CC is set to oicc, use it.  Otherwise, use the
# default.
$compilerInvocation=$ENV{'CC'};

if ($compilerInvocation !~ /^\s*oicc/ )
{
    $compilerInvocation=$defaultCompilerInvocation;
} # if

$compilerInvocation.=" --keep-intermediate-files";

# Read the compile_info file into a hash.
%compileInfo=read_compile_info ("$benchPath/compile_info");

# Add the parmfile option to specify the benchmark parm file if it exists.
if ( -f "$benchPath/compile_parms" )
{
    $compilerInvocation.=" --parmfile $benchPath/compile_parms";
} # if

# Add the krc option if necessary.
if ($compileInfo{'ANSI_C_SOURCE'} == 0)
{
# REK
#    $compilerInvocation.=" --krc";
} # if

# Add the appropriate option for the processor type.
if ($mckinley)
{
    $compilerInvocation.=" --mmckinley";
} # if
elsif ($itanium)
{
    $compilerInvocation.=" --mitanium";
} # elsif

# Add the preprocessing options if they are specified.
$compilerInvocation.=" $compileInfo{'PREPROCESSING_OPTIONS'}";

if ($compileInfo{'TYPE'} ne "tarball") 
{
    ### Generate the Makefiles for this benchmark ###
    # Create a directory for the benchmark.
    mkdir ("./$benchDest");
    
    if ($compileInfo{"COPY_SOURCES"} eq "")
    {
	# Copy the entire src directory to the current directory.
	system ("cp -R $benchPath/src ./$benchDest");
    }
    else
    {
	# Copy the specified files to the source directory.
	system ("mkdir -p ./$benchDest/src");
	foreach $file (split (/\s+/, $compileInfo{"COPY_SOURCES"}))
	{
	    system ("cp $benchPath/src/$file ./$benchDest/src");
	}
    }

    # Get a list of source (.c) files in the src directory
    if (!open (SOURCELIST, "(cd ./$benchDest/src ; /bin/ls -1 *.c) |"))
    {
        print "Error: Could not list src directory.\n";
        exit 1;
    } # if

    # 11/25/03 REK Adding support for explicitly specifying sources to compile
    if ($compileInfo{"SOURCES"} eq "")
    {
	# Get a list of source (.c) files in the src directory
	if (!open (SOURCELIST, "(cd ./$benchDest/src ; /bin/ls -1 *.c) |"))
	{
	    print "Error: Could not list src directory.\n";
	    exit 1;
	} # if
	
	while ($line=<SOURCELIST>)
	{
	    chomp ($line);
	    push (@sourceFiles, $line);
	} # while $line
	
	close (SOURCELIST);
    }
    else
    {
	@sourceFiles=split (/\s+/, $compileInfo{"SOURCES"});
    }

    # Convert the list of source files into a string separated by spaces.
    $sourceFileString=join (' ', @sourceFiles);

    # Write the top level Makefile for the benchmark
    if (!open (MAKEFILE, "> ./$benchDest/Makefile"))
    {
        print "Error: Could not open ./$benchDest/Makefile for writing.\n";
        exit 1;
    } # if

    print MAKEFILE "all: init\n";
    print MAKEFILE "\t(cd src && make $benchmark)\n\n";
    print MAKEFILE "no-profile: init\n";
    print MAKEFILE "\t(cd src && make no-profile)\n\n";
    print MAKEFILE "check: init\n";
    print MAKEFILE "\t(cd src && make check)\n\n";
    print MAKEFILE "init:\n";
    print MAKEFILE "\tBENCH_DIR=${benchPath}; export BENCH_DIR\n";
    print MAKEFILE "\tBENCH_NAME=${benchmark}; export BENCH_NAME\n";
    print MAKEFILE "\tBASE_DIR=${benchPath}; export BASE_DIR\n";
    print MAKEFILE "\tHOST_PLATFORM=${hostPlatform}; export HOST_PLATFORM\n";
    print MAKEFILE "\tHOST_COMPILER=${hostCompiler}; export HOST_COMPILER\n";
    print MAKEFILE "\tPLATFORM_DIR=${releasePath}/platform/${hostPlatform}_${hostCompiler}; export PLATFORM_DIR\n";
    print MAKEFILE "\tBASELINE_PARMS_FILE=${baselineParmsFile}; export BASELINE_PARMS_FILE\n";
    print MAKEFILE "\tPROJECT_DIR=${projectDir}; export PROJECT_DIR\n";
    print MAKEFILE "\tSTD_PARMS_FILE=${projectParmsFile}; export STD_PARMS_FILE\n";

    close (MAKEFILE);

    # Write the Makefile for the src directory
    if (!open (MAKEFILE, "> ./$benchDest/src/Makefile"))
    {
        print "Error: Could not open ./$benchDest/src/Makefile for writing.\n";
        exit 1;
    } # if

    print MAKEFILE <<EOF;
CC=$compilerInvocation

SOURCES=$sourceFileString

no-profile: \$(SOURCES)
\t\$(CC) -o $benchmark $compileInfo{'LINKING_PREOPTIONS'} \$(SOURCES) $compileInfo{'LINKING_POSTOPTIONS'}

EOF

    print MAKEFILE "$benchmark: \$(SOURCES)\n";
    print MAKEFILE "\t\$(CC) --pprof-gen -o $benchmark $compileInfo{'LINKING_PREOPTIONS'} \$(SOURCES) $compileInfo{'LINKING_POSTOPTIONS'}\n";

    # Add a profiling run for every training input
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
					  $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if
	
	# Run the benchmark on the input, then clean the directory
	print MAKEFILE "\t$execInfo{'PREFIX'} ./$benchmark.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";

	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input

    print MAKEFILE "\t\$(CC) --pprof-use --lprof-gen -o $benchmark $compileInfo{'LINKING_PREOPTIONS'} \$(SOURCES) $compileInfo{'LINKING_POSTOPTIONS'}\n";
  
    # Add a profiling run for every training input
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
        my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
					   $input);
        if ($execInfo{'SETUP'} !~ /^\s*$/)
        {
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
        } # if

        # Run the benchmark on the input, then clean the directory
        print MAKEFILE "\t$execInfo{'PREFIX'} ./$benchmark.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";
    
        if ($execInfo{'CLEANUP'} !~ /^\s*$/)
        {
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
        } # if
    } # foreach $input

    print MAKEFILE "\t\$(CC) --lprof-use -o $benchmark $compileInfo{'LINKING_PREOPTIONS'} \$(SOURCES) $compileInfo{'LINKING_POSTOPTIONS'}\n\n";

    print MAKEFILE "check: $benchmark\n";

    # Add a run for every evaluation input
    foreach $input (split (/ /, $compileInfo{'DEFAULT_EVAL'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
					  $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if

	# Run the benchmark on the input, check it, then cleanup
	print MAKEFILE "\t$execInfo{'PREFIX'} ./$benchmark $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";

	if ($execInfo{'CHECK'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CHECK'}\n";
	} # if

	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input

    close (MAKEFILE);
}
else 
{ 
    # Create a directory for the benchmark.
    mkdir ("./$benchDest");

    # Copy the src directory to the current directory.
    system ("cp -R $benchPath/$compileInfo{'SOURCE'} ./$benchDest");

    if (!open (MAKEFILE, "> ./$benchDest/Makefile"))
    {
	print "Error: Could not open ./$benchDest/Makefile for writing.\n";
	exit 1;
    } # if
    if ($compileInfo{'PROFILING'} ne "no")
    {
	compile_oicc();
	compile_gcc();
	compile_ecc();
    }
    else
    {
	print MAKEFILE "all no-profiled: init\n";
	compile_no_profiled();
	print MAKEFILE "\t$compileInfo{'COMPILE_POST'}\n\n";
	print MAKEFILE "gcc no-profiled: init\n";
	compile_no_profiled();
	print MAKEFILE "\t$compileInfo{'GCC_COMPILE_POST'}\n\n";       
	print MAKEFILE "ecc no-profiled: init\n";
	compile_no_profiled();
	print MAKEFILE "\t$compileInfo{'ECC_COMPILE_POST'}\n\n";

    }

    print MAKEFILE "init:\n";
    print MAKEFILE "\tBENCH_DIR=${benchPath}; export BENCH_DIR\n";
    print MAKEFILE "\tBENCH_NAME=${benchmark}; export BENCH_NAME\n";
    print MAKEFILE "\tBASE_DIR=${benchPath}; export BASE_DIR\n";
    print MAKEFILE "\tHOST_PLATFORM=${hostPlatform}; export HOST_PLATFORM\n";
    print MAKEFILE "\tHOST_COMPILER=${hostCompiler}; export HOST_COMPILER\n";
    print MAKEFILE "\tPLATFORM_DIR=${releasePath}/platform/${hostPlatform}_${hostCompiler}; export PLATFORM_DIR\n";
    print MAKEFILE "\tBASELINE_PARMS_FILE=${baselineParmsFile}; export BASELINE_PARMS_FILE\n";
    print MAKEFILE "\tPROJECT_DIR=${projectDir}; export PROJECT_DIR\n";
    print MAKEFILE "\tSTD_PARMS_FILE=${projectParmsFile}; export STD_PARMS_FILE\n";
    print MAKEFILE "check:\n";
    foreach $input (split (/ /, $compileInfo{'DEFAULT_EVAL'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
                                            $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if
                                                                                                                                      
	# Run the benchmark on the input, check it, then cleanup
	print MAKEFILE "\t$execInfo{'PREFIX'} $compileInfo{'EXECUTABLE'} $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";
                                                                                                                                      
	if ($execInfo{'CHECK'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CHECK'}\n";
	} # if
                                                                                                                                      
	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input
                                                                                                                                  
    close (MAKEFILE)
} 

sub compile_no_profiled ()
{
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n";
    print MAKEFILE "\t$compileInfo{'CONFIGURE'}\n";
    print MAKEFILE "\t$compileInfo{'COMPILE_NOPROF'}\n";
}


sub compile_oicc ()
{
    print MAKEFILE "oicc-profiled all: init\n";
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n";
    print MAKEFILE "\t$compileInfo{'CONFIGURE'}\n";
    print MAKEFILE "\t$compileInfo{'COMPILE_PROF1'}\n";
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
					   $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if

	# Run the benchmark on the input, then clean the directory
	print MAKEFILE "\t$execInfo{'PREFIX'} ./$compileInfo{'EXECUTABLE'}.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";

	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input

    print MAKEFILE "\t$compileInfo{'COMPILE_PROF2'}\n";
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
					   $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if

	# Run the benchmark on the input, then clean the directory
	print MAKEFILE "\t$execInfo{'PREFIX'} $compileInfo{'EXECUTABLE'}.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";
	
	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    }   # foreach $input

    print MAKEFILE "\t$compileInfo{'COMPILE_PROF3'}\n\n";
    print MAKEFILE "oicc: init\n";
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n";
    print MAKEFILE "\t$compileInfo{'CONFIGURE'}\n";
    print MAKEFILE "\t$compileInfo{'COMPILE_NOPROF'}\n";
    print MAKEFILE "\t$compileInfo{'COMPILE_POST'}\n\n";
       
}

sub compile_gcc ()
{
    print MAKEFILE "gcc-profiled: init\n";
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n";
    print MAKEFILE "\t$compileInfo{'GCC_CONFIGURE'}\n";
    print MAKEFILE "\t$compileInfo{'GCC_COMPILE_PROF1'}\n";
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
                                            $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if
                                                                                
	# Run the benchmark on the input, then clean the directory
	print MAKEFILE "\t$execInfo{'PREFIX'} ./$compileInfo{'EXECUTABLE'}.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";
                                                                                
	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input
    print MAKEFILE "\t$compileInfo{'GCC_COMPILE_PROF2'}\n";
    print MAKEFILE "gcc:\n";
    print MAKEFILE "\t$compileInfo{'GCC_CONFIGURE'}\n\n";
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n\n";       
    print MAKEFILE "\t$compileInfo{'GCC_CONFIGURE_NOPROF'}\n\n";
    print MAKEFILE "\t$compileInfo{'GCC_COMPILE_POST'}\n\n";

       
}
sub compile_ecc ()
{
    print MAKEFILE "ecc-profiled: init\n";
    print MAKEFILE "\t$compileInfo{'UNPACK'}\n";
    print MAKEFILE "\t$compileInfo{'ECC_CONFIGURE'}\n";
    print MAKEFILE "\t$compileInfo{'ECC_COMPILE_PROF1'}\n";
    foreach $input (split (/ /, $compileInfo{'DEFAULT_TRAIN'}))
    {
	my %execInfo=read_exec_info_input ("$benchPath/exec_info_$input",
                                            $input);
	if ($execInfo{'SETUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'SETUP'}\n";
	} # if
                                                                                
	# Run the benchmark on the input, then clean the directory
	print MAKEFILE "\t$execInfo{'PREFIX'} ./$compileInfo{'EXECUTABLE'}.prof $execInfo{'ARGS'} > $input.result 2> $input.stderr\n";
                                                                                
	if ($execInfo{'CLEANUP'} !~ /^\s*$/)
	{
	    print MAKEFILE "\t$execInfo{'CLEANUP'}\n";
	} # if
    } # foreach $input
    print MAKEFILE "\t$compileInfo{'ECC_COMPILE_PROF2'}\n";
    print MAKEFILE "ecc:\n";                                                        print MAKEFILE "\t$compileInfo{'UNPACK'}\n\n";
    print MAKEFILE "\t$compileInfo{'ECC_CONFIGURE'}\n\n";
    print MAKEFILE "\t$compileInfo{'ECC_CONFIGURE_NOPROF'}\n\n";
    print MAKEFILE "\t$compileInfo{'ECC_COMPILE_POST'}\n\n"; 
      
}
					     
    
exit;


# sub show_help ()
# This function simply prints the help message, then exits.
sub show_help ()
{
    use English;

    my ($benchList);

    print "Usage: $PROGRAM_NAME [options] benchmark\n";
    print " options:\n";
    print "  --help        Print this message.\n";
    print "  --itanium     Set up a makefile to produce code for a first generation\n";
    print "                Itanium processor. (default)\n";
    print "  --mckinley    Set up a makefile to produce code for a second generation\n";
    print "                Itanium processor.\n";
    print "  -o <file>     Specify the name of the directory to create for benchmark\n";
    print "                compilation.  Defaults to the benchmark name.\n";
    print "  -p <file>     Specify a parms file to use.\n";
    print " This script retrieves the named benchmark and generates a Makefile to build\n";
    print " it using oicc.\n\n";

    $benchList=`ls $userBenchPath 2>/dev/null`;
    
    if ($benchList ne "")
    {
	print " Benchmarks are:\n";
	print `ls $releasePath/benchmarks`;
    } # if
    else
    {
	print " No benchmarks found in benchmark directory\n";
	print " $userBenchPath\n\n";
	
	if ($ENV{'USER_BENCH_PATH'} eq "")
	{
	    print " USER_BENCH_PATH is not set.  This should be set to the location of the\n";
	    print " 'benchmarks' directory.\n\n";
	} # if
    } # else
    exit 0;
} # sub show_help


# sub read_compile_info ($)
# Takes the path to a compile_info file as the argument.
# The compile_info file is designed to be sourced by sh to set variables.
# This function sets up some environment variables that the file expects,
# then tells sh to source it and print all variables.  It returns a hash
# of the variables with the variable's value keyed by variable name.
sub read_compile_info ($) # A closing ) to make emacs' parser happy.
{
    use English;

    my ($file)=@ARG;

    my ($command);
    my (%table);
    
    # Set a few environment variables that the file can expect.
    $ENV{'BENCH_DIR'}=$benchPath;
    $ENV{'BENCH_NAME'}=$benchmark;
    $ENV{'BASE_DIR'}=$benchPath;
    $ENV{'HOST_PLATFORM'}=$hostPlatform;
    $ENV{'HOST_COMPILER'}=$hostCompiler;
    $ENV{'PLATFORM_DIR'}="$releasePath/platform/${hostPlatform}_$hostCompiler";
    $ENV{'BASELINE_PARMS_FILE'}=$baselineParmsFile;
    $ENV{'PROJECT_DIR'}=$projectDir;
    $ENV{'STD_PARMS_FILE'}=$projectParmsFile;

    # Set up a command to get sh to source the file and print its environment.
    $command="sh -c '. $file ; set' |";

    # Call read_arg_file to parse the VAR=val output into a hash.
    %table = read_arg_file($command);
    if (undef $table{'ECC_CONFIGURE'})
    {
	$table{'ECC_CONFILGURE'} = $table{'CONFIGURE'};
    }
    if ( undef $table{'ECC_COMPILE_NOPROF'})
    { 
	$table{'ECC_COMPILE_NOPROF'} = $table{'COMPILE_NOPROF'};
    } 
    if ( undef $table{'ECC_COMPILE_PROF1'})
    {
	$table{'ECC_COMPILE_PROF1'} = $table{'COMPILE_PROF1'};
    }
    if (undef $table{'ECC_COMPILE_PROF2'})
    {
	$table{'ECC_COMPILE_PROF2'} = $table{'COMPILE_PROF2'};
    }
    if (undef $table{'ECC_COMPILE_POST'})
    {
	$table{'ECC_COMPILE_POST'} = $table{'COMPILE_POST'};
    }
    if ( undef $table{'ECC_PREPROCESSING_OPTIONS'})
    {
	$table{'ECC_PREPROCESSING_OPTIONS'} = $table{'PREPROCESSING_OPTIONS'};
    }
    if (undef $table{'ECC_LINKING_PREOPTIONS'})
    {
	$table{'ECC_LINKING_PREOPTIONS'} = $table{'LINKING_PREOPTIONS'};
    }
    if (undef $table{'ECC_LINKING_POSTOPTIONS'})
    {
	$table{'ECC_LINKING_POSTOPTIONS'} = $table{'LINKING_POSTOPTIONS'};
    }
    if (undef $table{'ECC_LIB_REQUIREMENTS'})
    {
	$table{'ECC_LIB_REQUIREMENTS'} = $table{'LIB_REQUIREMENTS'};
    }
    if (undef $table{'GCC_CONFIGURE'})
    {
	$table{'GCC_CONFILGURE'} = $table{'CONFIGURE'};
    }
    if (undef $table{'GCC_COMPILE_NOPROF'})
    {
	$table{'GCC_COMPILE_NOPROF'} = $table{'COMPILE_NOPROF'};
    }
    if (undef $table{'GCC_COMPILE_PROF1'})
    {
	$table{'GCC_COMPILE_PROF1'} = $table{'COMPILE_PROF1'};
    }
    if (undef $table{'GCC_COMPILE_PROF2'})
    {
	$table{'GCC_COMPILE_PROF2'} = $table{'COMPILE_PROF2'};
    }
    if (undef $table{'GCC_COMPILE_POST'})
    {
	$table{'GCC_COMPILE_POST'} = $table{'COMPILE_POST'};
    }
    if (undef $table{'GCC_PREPROCESSING_OPTIONS'})
    {
	$table{'GCC_PREPROCESSING_OPTIONS'} = $table{'PREPROCESSING_OPTIONS'};
    }
    if (undef $table{'GCC_LINKING_PREOPTIONS'})
    {
	$table{'GCC_LINKING_PREOPTIONS'} = $table{'LINKING_PREOPTIONS'};
    }
    if (undef $table{'GCC_LINKING_POSTOPTIONS'} )
    {
	$table{'GCC_LINKING_POSTOPTIONS'} = $table{'LINKING_POSTOPTIONS'};
    }
    if (undef $table{'GCC_LIB_REQUIREMENTS'} )
    {
	$table{'GCC_LIB_REQUIREMENTS'} = $table{'LIB_REQUIREMENTS'};
    }


    return (%table);
} # sub read_compile_info


# sub read_exec_info_input ($$)
# Takes the path to an exec_info_input* file as the first argument.
# Takes the name of the input (input1, input2, etc) as the second argument.
# The exec_info_input* files are designed to be sourced by sh to set
# variables.  This function sets up some environment variables that the
# file expects, then tells sh to source it and print all variables.  It
# returns a hash of the variables with the variable's value keyed by
# variable name.
sub read_exec_info_input ($$)
{
    use English;

    my ($file, $input)=@ARG;

    my ($command);

    # Set a few environment variables that the file can expect.
    $ENV{'BENCH_DIR'}=$benchPath;
    $ENV{'BENCH_NAME'}=$benchmark;
    $ENV{'BASE_DIR'}=$benchPath;
    $ENV{'HOST_PLATFORM'}=$hostPlatform;
    $ENV{'HOST_COMPILER'}=$hostCompiler;
    $ENV{'PLATFORM_DIR'}="$releasePath/platform/${hostPlatform}_$hostCompiler";
    $ENV{'BASELINE_PARMS_FILE'}=$baselineParmsFile;
    $ENV{'PROJECT_DIR'}=$projectDir;
    $ENV{'STD_PARMS_FILE'}=$projectParmsFile;
    $ENV{'RESULT_FILE'}="$input.result";
    $ENV{'STDERR_FILE'}="$input.stderr";

    # Set up a command to parse the VAR=val output into a hash.
    $command="sh -c '. $file ; set' |";

    return (read_arg_file ($command));
} # sub read_exec_info_input


# sub read_arg_file ($) # A closing ) to make emacs' parser happy.
# Takes a filename (or a pipe or anything else that will present lines
# in the form VAR=val) and returns a hash where the variable values are
# keyed by variable name.
# This function expects lines of the form variable=value\n.  Comments
# (using '#') and blank lines are allowed.
sub read_arg_file($)
{
    use English;

    my ($file)=@ARG;
    
    my ($line);
    my (%result);
    
    # If we can't open the compile_info file, return an empty hash.
    %result=();

    if (open (INPUTFILE, "$file"))
    {
      LINE: while ($line=<INPUTFILE>)
      {
	  # Skip comments and blank lines.
	  next LINE if ($line =~ /^\s*(\#.*)?\s*$/);
	  
	  # Remove the newline at the end.
	  chomp ($line);
	  
	  # Split the line at the = to get the variable name and value.
	  # Specify that we only split into two sections so that val gets its
	  # full string if it contains '='.
	  my ($var, $val)=split (/=/, $line, 2);

	  # If they exist, remove single quotes from the ends of the value.
	  $val =~ s/^\$?\'//;
	  $val =~ s/\'$//;

	  # Save the variable and value in the hash.
	  $result{$var}=$val;
      } # LINE: while
    } # if

    close (INPUTFILE);

    return (%result);
} # sub read_arg_file
