#   Copyright (c) 2011 Axel Wachtler
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of the authors nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.

# $Id$

# === main parameters of the project =========================================
# next lines needs to be overwritten
URACOLIDIR =  ../../../../uracoli/install
BOARD = UNDEFINED
PART = UNDEFINED
F_CPU = 8000000UL

OBJDIR = ./obj
BINDIR = ./bin

help:
	@echo
	@echo "remove generated files:"
	@echo "  make clean"
	@echo "create a specific target:"
	@echo "  make [URACOLIDIR=<basedir>] BOARD=<board> MCU=<mcu> F_CPU=<F_CPU> <target>"
	@echo
	@echo "<basedir> : base installdir for uracoli libs."
	@echo "            <basedir>/inc, <basedir>/lib should exist"
	@echo "            default: $(URACOLIDIR)"
	@echo "<target>  : one of the following: $(PROJECTS)"
	@echo "<target>  : one of the following: $(PROJECTS)"
	@echo
	@echo "examples:"
	@echo "  make URACOLIDIR=../install BOARD=stb230 MCU=atmega1281 F_CPU=8000000UL wuart"
	@echo "  make BOARD=stb230 MCU=atmega128rfa1 F_CPU=8000000UL sensorapp"
	@echo

clean:
	rm -rf $(OBJDIR)/*.o $(OBJDIR)/*.lst $(BINDIR)/*.elf $(BINDIR)/*.hex

# === internal rules ===================================================

# temporary output directory
$(OBJDIR):
	-@mkdir --verbose -p $@

$(BINDIR):
	-@mkdir --verbose -p $@

# DBGFMT=stabs for Linux
# DBGFMT=dwarf-2 for Windows
DBGFMT=
# automatically derived parameters
INCDIRS = . $(URACOLIDIR)/inc
LIBDIRS = $(URACOLIDIR)/lib

SOURCES=$(wildcard *.c)
PROJECTS = $(basename $(SOURCES))

# TARGETFILES=$(addsuffix _$(BOARD).hex,$(PROJECTS))
# TARGETS = $(addprefix $(BINDIR)/,$(TARGETFILES))

# === tool parameters ======================================================

CC = avr-gcc
CCFLAGS = -Wall -Wundef -Os -g$(DBGFMT) -mmcu=$(MCU)
CCFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%_$(BOARD).lst)
CCFLAGS += -D$(BOARD) -DF_CPU=$(F_CPU)
CCFLAGS += -I$(URACOLIDIR)/inc -I.
LDFLAGS= $(patsubst %,-L%,$(LIBDIRS)) -lradio_$(BOARD) -lio_$(BOARD)

OC=avr-objcopy
OCFLAGS=-O ihex

# === build rules ============================================================
.PHONY:
%:  $(OBJDIR) $(BINDIR) $(BINDIR)/%_$(BOARD).hex
	@echo "created target $@ ($(BINDIR)/$*_$(BOARD).hex)"

$(BINDIR)/%_$(BOARD).hex: $(BINDIR)/%_$(BOARD).elf
	$(OC) $(OCFLAGS) $< $@

.SECONDARY:

$(BINDIR)/%_$(BOARD).elf: $(OBJDIR)/%_$(BOARD).o
	$(CC) -o $@ $(CCFLAGS) $^ $(LDFLAGS)

$(OBJDIR)/%_$(BOARD).o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.SUFFIXES:
