# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=SerialLIb - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to SerialLIb - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "SerialLIb - Win32 Release" && "$(CFG)" !=\
 "SerialLIb - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "SerialLIb.mak" CFG="SerialLIb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SerialLIb - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SerialLIb - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "SerialLIb - Win32 Debug"
CPP=cl.exe
MTL=mktyplib.exe
RSC=rc.exe

!IF  "$(CFG)" == "SerialLIb - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\CaracaLib.dll"

CLEAN : 
	-@erase ".\Release\CaracaLib.dll"
	-@erase ".\Release\seriallib.obj"
	-@erase ".\Release\rs232int.obj"
	-@erase ".\Release\CaracaLib.lib"
	-@erase ".\Release\CaracaLib.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/SerialLIb.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x410 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/SerialLIb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Release/CaracaLib.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/CaracaLib.pdb"\
 /machine:I386 /def:".\seriallib.def" /out:"$(OUTDIR)/CaracaLib.dll"\
 /implib:"$(OUTDIR)/CaracaLib.lib" 
DEF_FILE= \
	".\seriallib.def"
LINK32_OBJS= \
	"$(INTDIR)/seriallib.obj" \
	"$(INTDIR)/rs232int.obj"

"$(OUTDIR)\CaracaLib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SerialLIb - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\CaracaLib.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\CaracaLib.dll"
	-@erase ".\Debug\seriallib.obj"
	-@erase ".\Debug\rs232int.obj"
	-@erase ".\Debug\CaracaLib.ilk"
	-@erase ".\Debug\CaracaLib.lib"
	-@erase ".\Debug\CaracaLib.exp"
	-@erase ".\Debug\CaracaLib.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/SerialLIb.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/SerialLIb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/CaracaLib.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/CaracaLib.pdb" /debug\
 /machine:I386 /def:".\seriallib.def" /out:"$(OUTDIR)/CaracaLib.dll"\
 /implib:"$(OUTDIR)/CaracaLib.lib" 
DEF_FILE= \
	".\seriallib.def"
LINK32_OBJS= \
	"$(INTDIR)/seriallib.obj" \
	"$(INTDIR)/rs232int.obj"

"$(OUTDIR)\CaracaLib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "SerialLIb - Win32 Release"
# Name "SerialLIb - Win32 Debug"

!IF  "$(CFG)" == "SerialLIb - Win32 Release"

!ELSEIF  "$(CFG)" == "SerialLIb - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\seriallib.cpp
DEP_CPP_SERIA=\
	".\seriallib.h"\
	".\errcode.h"\
	".\rs232int.h"\
	".\types.h"\
	

"$(INTDIR)\seriallib.obj" : $(SOURCE) $(DEP_CPP_SERIA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\rs232int.cpp

!IF  "$(CFG)" == "SerialLIb - Win32 Release"

DEP_CPP_RS232=\
	".\rs232int.h"\
	".\errcode.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	".\types.h"\
	
NODEP_CPP_RS232=\
	".\e2profil.h"\
	

"$(INTDIR)\rs232int.obj" : $(SOURCE) $(DEP_CPP_RS232) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SerialLIb - Win32 Debug"

DEP_CPP_RS232=\
	".\rs232int.h"\
	".\errcode.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	".\types.h"\
	

"$(INTDIR)\rs232int.obj" : $(SOURCE) $(DEP_CPP_RS232) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\seriallib.def

!IF  "$(CFG)" == "SerialLIb - Win32 Release"

!ELSEIF  "$(CFG)" == "SerialLIb - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
