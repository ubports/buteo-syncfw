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
open(EXTRACTED_INFO,">$ARGV[1]");

$search_string = $ARGV[2];
$address= "";
$string_found=0;

while(<INPUTFILE>) {
if(/$search_string/) {
  print "Found address for the device $search_string \n" ;
  print $_;
  $address = substr($_,0,18);
#  print "address of the device is $address \n" ;
  $string_found=1;
}
#write the address to EXTRACTED_INFO file 
}
open(EXTRACTED_INFO,">$ARGV[1]");
print EXTRACTED_INFO "$address" ;
close(EXTRACTED_INFO);
if($string_found == 0) {
print "address not found";
exit 1;
} else {
exit 0
}


