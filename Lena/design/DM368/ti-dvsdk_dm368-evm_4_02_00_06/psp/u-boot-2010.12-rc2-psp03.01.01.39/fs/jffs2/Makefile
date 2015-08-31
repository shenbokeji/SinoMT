#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libjffs2.o

AOBJS	=
COBJS-$(CONFIG_CMD_JFFS2)  += jffs2_1pass.o
COBJS-$(CONFIG_CMD_JFFS2)  += compr_rtime.o
COBJS-$(CONFIG_CMD_JFFS2)  += compr_rubin.o
COBJS-$(CONFIG_CMD_JFFS2)  += compr_zlib.o
COBJS-$(CONFIG_CMD_JFFS2)  += mini_inflate.o
COBJS-$(CONFIG_CMD_JFFS2)  += compr_lzo.o
COBJS-$(CONFIG_CMD_JFFS2)  += compr_lzari.o

COBJS-$(CONFIG_CMD_JFFS2)  += jffs2_linux_adapt.o

#begin:yangbin 2009-03-30  add jffs2
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_fs/inode.o
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_fs/read_write.o
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_fs/namei.o
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_jffs2/crc32.o 
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_jffs2/dir.o	
#COBJS-$(CONFIG_CMD_JFFS2_TEST) += linux2.4_jffs2/jffs2_test.o		
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_jffs2/file.o 
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_jffs2/nodelist.o 
COBJS-$(CONFIG_JFFS2_WRITE) += linux2.4_jffs2/malloc.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/read.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/nodemgmt.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/readinode.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/super.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/write.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/scan.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/gc.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/build.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/erase.o
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/background.o
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/scan.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += zlib/compress.o
COBJS-$(CONFIG_JFFS2_WRITE)  += zlib/deflate.o
COBJS-$(CONFIG_JFFS2_WRITE)  += zlib/trees.o
COBJS-$(CONFIG_JFFS2_WRITE)  += zlib/zutil.o
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/compr.o 
COBJS-$(CONFIG_JFFS2_WRITE)  += linux2.4_jffs2/compr_rtime.o 
#yangbin add debug 2010-8-14
COBJS-$(CONFIG_JFFS2_DEBUG_EXCEPTION)  += linux2.4_jffs2/debug.o 

COBJS-$(CONFIG_CMD_JFFS2)  += linux2.4_jffs2/compr_rubin.o
COBJS-$(CONFIG_CMD_JFFS2)  += linux2.4_jffs2/pushpull.o

COBJS-$(CONFIG_JFFS2_SUMMARY) += linux2.4_jffs2/summary.o
COBJS-$(CONFIG_JFFS2_SUMMARY) += linux2.4_jffs2/writev.o
COBJS-$(CONFIG_JFFS2_SUMMARY) += linux2.4_jffs2/crc.o


COBJS	:= $(COBJS-y)
SRCS	:= $(AOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

#CPPFLAGS +=

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
