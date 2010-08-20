#!/bin/bash
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
#
#Author - Srikanth Kavoori
# This file should run unittests for sync-fw
# and create the result file with unittest rate
# and coverage to this folder with name
# sync-fw-results
#
# The release number should be in the file
# this script generates the results automatically 
# for the latest weekXxX directory under sync-fw
#this script updation should be rarely needed 

#Script Specific Variables 

TARGET=sync-fw
TARGET_WEEK=week$(date +%V)$(date +%G)
RESULTS_DIR=$PWD

# adding environment varaible to get coverage rate
export SBOX_USE_CCACHE=no
ccache -c

if [ $# == 0 ];
then
WD=$PWD
ROOTDIR=$WD/..
else
WD=$1
TARGET_WEEK=$(ls -c $WD/../$TARGET | head -1)
echo "TARGET WEEK is $TARGET_WEEK"
ROOTDIR=$WD/../$TARGET/$TARGET_WEEK
echo "ROOTDIR is $ROOTDIR"
echo $1
RESULTS_DIR=$2
echo "Results Dir is $RESULTS_DIR"
fi


TARGETS=(libsyncpluginmgr libsyncprofile msyncd)
RESULTS_FILE=$RESULTS_DIR/$TARGET-results_$TARGET_WEEK
TEMPFILE1=$WD/.temp_results

if [ -f $TEMPFILE1 ]
then
	rm -f $TEMPFILE1
fi

TEMPFILE2=$WD/.gcov_info.txt
if [ -f $TEMPFILE2 ]
then
       rm -f $TEMPFILE2
fi

TEMPFILE3=$WD/.percent
if [ -f $TEMPFILE3 ]
then
      rm -f $TEMPFILE3
fi

export LD_LIBRARY_PATH=$ROOT_DIR/libsyncprofile:$ROOT_DIR/libsyncpluginmgr:$LD_LIBRARY_PATH

echo "Running the unit tests for $TARGET..."
echo "Results will be stored in: $RESULTS_FILE ...."
cd $ROOTDIR
echo ${TARGETS[@]}
echo "Building sync-fw so as to run unit tests on the libraries"
qmake
make clean
make coverage

if [ $? -ne 0 ];
then 
    echo "sync-fw building failed...exiting"
exit 1
else 
    echo "sync-fw build successful"
    echo "Building unit tests "
    cd unittests
        echo "running unit tests "
        cd tests
        export LD_LIBRARY_PATH=../../libsyncprofile:../../libsyncpluginmgr
	timed -d
        meego-run ./sync-fw-tests >> $TEMPFILE1 2>&1
        rm ../../msyncd/*Test.*
        rm ../../msyncd/tests.*
        cd ..
fi

if [ $? -ne 0 ];
then 
    echo "Running unit tests failed...exiting "
    exit 1
else 

    echo "Getting Code Coverage Results "
    cd $ROOTDIR
    
    for target in ${TARGETS[@]}
       do
       if [ -d $target ];
       then	   
              echo "Getting Coverage Results from $PWD"
              cd $target
    	      rm -f moc_*
    	      file_list=$(ls *.gcno)
                  #echo "Printing File List :$file_list "
                  for file in $file_list
                  do
                    echo "Running gcov on $file"
                    gcov $file >> $TEMPFILE2 
                 done
                  cd ..	      
                  
         fi
       done
fi
cd $ROOTDIR
# get coverage information for the files using perl
echo "executing perl $ROOTDIR/bin/gcov_info.pl $TEMPFILE2 $TEMPFILE3"
GCOV_REPORT=$WD/gcov_report.txt
perl $WD/gcov_info.pl $TEMPFILE2 $TEMPFILE3 $GCOV_REPORT

if [ ! $? -eq 0 ]; then
 echo "Perl Script Failed to execute ... Exiting ...  "
 exit 0
fi

SUMMARY_FILE=$WD/.summary_file
if [ -f $SUMMARY_FILE ]
then
      rm -f $SUMMARY_FILE
fi
perl $WD/test_info.pl $TEMPFILE1 $SUMMARY_FILE

echo "Writing the file $RESULTS_FILE"
echo "#Results for $TARGET_WEEK  " > $RESULTS_FILE

echo "Results Summary STARTED " >> $RESULTS_FILE
                                                                                               
echo "#Current gcov reported coverage (line rate) is" >> $RESULTS_FILE
cat $TEMPFILE3 >> $RESULTS_FILE

echo "Unit test Results Summary " >> $RESULTS_FILE
cat $SUMMARY_FILE >> $RESULTS_FILE 

echo "Results Summary ENDED " >> $RESULTS_FILE

echo "****************UNIT_TEST Results START**************"  >> $RESULTS_FILE
cat $TEMPFILE1 >> $RESULTS_FILE
echo "****************UNIT_TEST Results END**************"  >> $RESULTS_FILE
rm -f $TEMPFILE1 $TEMPFILE2 $TEMPFILE3
cd $ROOTDIR
echo $PWD
qmake
make distclean
cd $WD
echo "$RESULTS_FILE created"
