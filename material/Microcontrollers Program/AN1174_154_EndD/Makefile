#########################################################################
#
# MODULE:   JN-AN-1046-IEEE802.15.4-App-Template
#
# DESCRIPTION: AN1046_154_EndD MakeFile
#
############################################################################
# 
# This software is owned by Jennic and/or its supplier and is protected
# under applicable copyright laws. All rights are reserved. We grant You,
# and any third parties, a license to use this software solely and
# exclusively on Jennic products. You, and any third parties must reproduce
# the copyright and warranty notice and any other legend of ownership on each
# copy or partial copy of the software.
# 
# THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
# ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
# BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
# INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
# 
# Copyright Jennic Ltd 2009. All rights reserved
#
#############################################################################
# Subversion variables
# $HeadURL:  $
# $Revision: 13257 $
# $LastChangedBy: pjtw $
# $LastChangedDate: 2009-05-26 16:11:24 +0100 (Tue, 26 May 2009) $
# $Id: Makefile 13257 2009-05-26 15:11:24Z mwild $ 
#
#############################################################################

DIRS	= Build

TARGETS: clean all

all: 
	-for d in $(DIRS); do (cd $$d; $(MAKE) $(MFLAGS) all ); done

clean: 
	-for d in $(DIRS); do (cd $$d; $(MAKE) $(MFLAGS) clean ); done

