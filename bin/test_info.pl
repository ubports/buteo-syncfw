#!/usr/bin/perl 
#/*
# * This file is part of buteo-syncfw package
# *
# * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# *
# * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public License
# * version 2.1 as published by the Free Software Foundation.
# *
# * This library is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library; if not, write to the Free Software
# * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# * 02110-1301 USA
# *
# */
# Author : Srikanth Kavoori 
# Not Tested fully yet. There could be some issues.
# If you see any issues Let me know and i shall fix it :)
open(INPUTFILE, "<$ARGV[0]");
#	Totals: 0 passed, 2 failed, 0 skipped
$tests_string = "Totals:";
$pass_count = 0;
$fail_count = 0;
$skip_count = 0;
$total_count = 0;

while(<INPUTFILE>) {
my($line) = $_;
if(/$tests_string/) {
 
  #print $line;
  #print "Found usr_include or header file on line $. \n" ;
    $pass_start_index = rindex($line,':') + 1;
    $passed_index = rindex($line,"passed") - 1;
    $length = $passed_index - $start_index;
    $pass_count += substr($line,$pass_start_index,$length);

    #print  "pass_count $pass_count ";
    $fail_start_index = rindex($line,'passed,') + 8;
    $failed_index = rindex($line,"failed") - 1;
    $length = $failed_index - $fail_start_index;
    $fail_count += substr($line,$fail_start_index,$length);
    #print  "fail_count $fail_count ";

    $skip_start_index = rindex($line,'failed,') + 8;
    $skipped_index = rindex($line,"skipped") - 1;
    $length = $failed_index - $skip_start_index;
    $skip_count += substr($line,$skip_start_index,$length);
    #print  "skip_count $skip_count \n";
} 
}

#if($filecount != 0 ) {
#    $coverage_percent=sprintf("%.2f",$coverage_percent/$filecount);
#}
$total_count += $pass_count +  $fail_count + $skip_count;
print "Total No. TESTS: $total_count \n";
print "Passed: $pass_count \n";
print "Failed: $fail_count \n";
print "Skipped: $skip_count \n";

open(SUMMARY_FILE,">$ARGV[1]");
print SUMMARY_FILE "" ;
print SUMMARY_FILE "Total TESTS: $total_count \n";
print SUMMARY_FILE "Passed: $pass_count \n";
print SUMMARY_FILE "Failed: $fail_count \n";
print SUMMARY_FILE "Skipped: $skip_count \n";
close(SUMMARY_FILE);
close(INPUTFILE);
