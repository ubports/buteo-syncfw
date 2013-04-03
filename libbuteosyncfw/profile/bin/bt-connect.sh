#!/bin/sh
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

#this script makes it easy to connect to a BT device from command line on the device

# check for hcitool availability
CWD=$PWD
WHOAMI=$USER
echo "Current working directory is $CWD and you are running this script as $USER"
if [ $# == 0 ];
then 
echo "Usage: bt-connect [DEVICe_NAME]"
exit 0
fi;

if [ -f /usr/bin/bluez-simple-agent ];
then 
echo "Found bluez simple agent "
else 
echo "Bluez-test package is not installed. this script needs that package"
echo "This script needs that package to run. install that package and re-run this script"
exit 0
fi

if [ -f /usr/bin/hcitool ];
then 
echo "Scanning and Getting the information for the device  $1"
hcitool scan > temp_file.txt
else 
echo "hcitool is not installed. Please Install Bluez-test package and then rerun the script"
exit 0
fi

/usr/bin/perl /opt/tests/buteo-syncfw/get_address.pl $CWD/temp_file.txt $CWD/temp_file2.txt $1

if [ $? -eq 0 ]; then
  echo "Perl Script exit normal address found"
else 
 echo "ADDRESS for the device not found"
 rm $CWD/temp_file.txt $CWD/temp_file2.txt
 exit 0
fi

#echo "Making pairing with $RET_VAL"
#get address for the string matching the argument
address=`cat temp_file2.txt`
echo "$address is the device's address"

/usr/bin/bluez-simple-agent hcio $address

if [ $? -eq 0 ]; then
   echo "Bluetooth pairing done"
else 
 echo "Bluetooth pairing failed exiting "
 rm $CWD/temp_file.txt $CWD/temp_file2.txt
 exit 0
fi
name=$1
rm $CWD/temp_file.txt $CWD/temp_file2.txt
if [ $USER == "root" ];
then 
      echo "Home Directory is  $HOME"
      echo "You are running the script as $USER .So Adding the bluetooth address to default profiles in /etc/buteo/profiles/ "
      /usr/bin/perl /opt/tests/buteo-syncfw/add_address.pl $address $name root
else 
      echo "Home Directory is $HOME "
      echo "Adding the bluetooth address to  user profiles in /home/user/.sync/profiles/"
      /usr/bin/perl /opt/tests/buteo-syncfw/add_address.pl $address $name user
      chmod -R 777 /home/user/.cache/msyncd/profiles
fi

if [ $? -eq 0 ]; then
	echo "Added BT information in all the sync profile files where bt_address field was found"
else 
 echo "Error while adding bt_address to the profiles file. Please add it manually "
exit 0
fi

