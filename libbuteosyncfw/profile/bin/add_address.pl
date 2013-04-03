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

print "Second Argument Passed to add_address script is $ARGV[2] ";

if ( $ARGV[2] eq "root" ) {
print "ADDING THE ADDRESS ONLY TO /etc/profiles. Please run this as USER if you want to update /home/user \n"; 

$temp="/etc/buteo/profiles/sync/temp";

@files = </etc/buteo/profiles/sync/*.xml>;
foreach $file (@files) {
	  print $file . "\n";
open(INPUTFILE, '<', $file)
   or die "Cannot open $file";
open(OUTPUTFILE, '>', $temp)
   or die "Cannot open new ";

$string="bt_address";
$string2="bt_name";

while(<INPUTFILE>) {
if(/$string/) {
  print OUTPUTFILE "<key name=\"bt_address\" value=\"$ARGV[0]\"/> \n";
  next;
}

if(/$string2/) {
  print OUTPUTFILE "<key name=\"bt_name\" value=\"$ARGV[1]\"/> \n";
  next;
}

print OUTPUTFILE;
}
close(INPUTFILE);
close(OUTPUTFILE);
rename($temp,$file)
    or die "Cannot rename temp to $file";
}

} else {

print "ADDING THE ADDRESS ONLY TO /home/user.  Please run this as ROOT if you want to update /etc/buteo \n"; 

$temp="/home/user/.cache/msyncd/profiles/sync/temp";

@files = </home/user/.cache/msyncd/profiles/sync/*.xml>;
foreach $file (@files) {
	  print $file . "\n";
open(INPUTFILE, '<', $file)
   or die "Cannot open $file";
open(OUTPUTFILE, '>', $temp)
   or die "Cannot open new ";

$string="bt_address";
$string2="bt_name";

while(<INPUTFILE>) {
if(/$string/) {
  print OUTPUTFILE "<key name=\"bt_address\" value=\"$ARGV[0]\"/> \n";
  next;
}

if(/$string2/) {
  print OUTPUTFILE "<key name=\"bt_name\" value=\"$ARGV[1]\"/> \n";
  next;
}

print OUTPUTFILE;
}
close(INPUTFILE);
close(OUTPUTFILE);
rename($temp,$file)
    or die "Cannot rename temp to $file";
}
}
