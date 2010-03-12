# Microsoft Developer Studio Project File - Name="smapiDll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=smapiDll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "smapiDll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "smapiDll.mak" CFG="smapiDll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "smapiDll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smapiDll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smapiDll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\nd_r\bin"
# PROP Intermediate_Dir "..\nd_r\obj\smapi"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMAPI_DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_WINDOWS" /D "_DLL" /D "__NT__" /D "_CONSOLE" /D "_SMAPI_EXT" /D "_MAKE_DLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /D "NDEBUG" /win32
# SUBTRACT MTL /nologo /mktyplib203
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"..\nd_r\lib/smapiDll.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msvcrt.lib kernel32.lib /nologo /dll /machine:I386 /nodefaultlib /out:"..\nd_r\bin/smapimvc.dll" /implib:"..\nd_r\lib/smapimvc.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "smapiDll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\nd_d\bin"
# PROP Intermediate_Dir "..\nd_d\obj\smapi"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMAPI_DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_WINDOWS" /D "_DLL" /D "__NT__" /D "_CONSOLE" /D "_SMAPI_EXT" /D "_MAKE_DLL" /FR /FD /GZ /c
# SUBTRACT CPP /u /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /D "_DEBUG" /win32
# SUBTRACT MTL /nologo /mktyplib203
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"..\nd_d\lib/smapiDll.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrtd.lib kernel32.lib /nologo /dll /profile /debug /machine:I386 /nodefaultlib /out:"..\nd_d\bin/smapimvc.dll" /implib:"..\nd_d\lib/smapimvc.lib"

!ENDIF 

# Begin Target

# Name "smapiDll - Win32 Release"
# Name "smapiDll - Win32 Debug"
# Begin Group "headerz"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\alc.h
# End Source File
# Begin Source File

#SOURCE=.\api_brow.h
# End Source File
# Begin Source File

SOURCE=.\api_jam.h
# End Source File
# Begin Source File

SOURCE=.\api_jamp.h
# End Source File
# Begin Source File

SOURCE=.\api_sdm.h
# End Source File
# Begin Source File

SOURCE=.\api_sdmp.h
# End Source File
# Begin Source File

SOURCE=.\api_sq.h
# End Source File
# Begin Source File

SOURCE=.\api_sqp.h
# End Source File
# Begin Source File

SOURCE=.\apidebug.h
# End Source File
# Begin Source File

SOURCE=.\compiler.h
# End Source File
# Begin Source File

SOURCE=.\cvsdate.h
# End Source File
# Begin Source File

SOURCE=.\dr.h
# End Source File
# Begin Source File

SOURCE=.\ffind.h
# End Source File
# Begin Source File

SOURCE=.\msgapi.h
# End Source File
# Begin Source File

SOURCE=.\old_msg.h
# End Source File
# Begin Source File

SOURCE=.\patmat.h
# End Source File
# Begin Source File

SOURCE=.\prog.h
# End Source File
# Begin Source File

SOURCE=.\progprot.h
# End Source File
# Begin Source File

SOURCE=.\stamp.h
# End Source File
# Begin Source File

SOURCE=.\strextra.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\unused.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\1stchar.c
# End Source File
# Begin Source File

SOURCE=.\api_jam.c
# End Source File
# Begin Source File

SOURCE=.\api_sdm.c
# End Source File
# Begin Source File

SOURCE=.\cvtdate.c
# End Source File
# Begin Source File

SOURCE=.\date2bin.c
# End Source File
# Begin Source File

SOURCE=.\dosdate.c
# End Source File
# Begin Source File

SOURCE=.\fexist.c
# End Source File
# Begin Source File

SOURCE=.\ffind.c
# End Source File
# Begin Source File

SOURCE=.\flush.c
# End Source File
# Begin Source File

SOURCE=.\genmsgid.c
# End Source File
# Begin Source File

SOURCE=.\locking.c
# End Source File
# Begin Source File

SOURCE=.\months.c
# End Source File
# Begin Source File

SOURCE=.\msgapi.c
# End Source File
# Begin Source File

SOURCE=.\parsenn.c
# End Source File
# Begin Source File

SOURCE=.\patmat.c
# End Source File
# Begin Source File

SOURCE=.\qksort.c
# End Source File
# Begin Source File

SOURCE=.\setfsize.c
# End Source File
# Begin Source File

SOURCE=.\sq_area.c
# End Source File
# Begin Source File

SOURCE=.\sq_hash.c
# End Source File
# Begin Source File

SOURCE=.\sq_help.c
# End Source File
# Begin Source File

SOURCE=.\sq_idx.c
# End Source File
# Begin Source File

SOURCE=.\sq_kill.c
# End Source File
# Begin Source File

SOURCE=.\sq_lock.c
# End Source File
# Begin Source File

SOURCE=.\sq_misc.c
# End Source File
# Begin Source File

SOURCE=.\sq_msg.c
# End Source File
# Begin Source File

SOURCE=.\sq_read.c
# End Source File
# Begin Source File

SOURCE=.\sq_uid.c
# End Source File
# Begin Source File

SOURCE=.\sq_write.c
# End Source File
# Begin Source File

SOURCE=.\strextra.c
# End Source File
# Begin Source File

SOURCE=.\strftim.c
# End Source File
# Begin Source File

SOURCE=.\strocpy.c
# End Source File
# Begin Source File

SOURCE=.\structrw.c
# End Source File
# Begin Source File

SOURCE=.\tdelay.c
# End Source File
# Begin Source File

SOURCE=.\trail.c
# End Source File
# Begin Source File

SOURCE=.\weekday.c
# End Source File
# End Target
# End Project
