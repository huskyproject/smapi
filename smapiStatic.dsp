# Microsoft Developer Studio Project File - Name="smapiStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=smapiStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "smapiStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "smapiStatic.mak" CFG="smapiStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "smapiStatic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "smapiStatic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smapiStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\nd_r\lib"
# PROP Intermediate_Dir "..\nd_r\obj\smapiStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "__NT__" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\nd_r\lib\smapimvc.lib"

!ELSEIF  "$(CFG)" == "smapiStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\nd_d\lib"
# PROP Intermediate_Dir "..\nd_d\obj\smapiStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "__NT__" /D "_CONSOLE" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\nd_d\lib\smapimvc.lib"

!ENDIF 

# Begin Target

# Name "smapiStatic - Win32 Release"
# Name "smapiStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\alc.h
# End Source File
# Begin Source File

SOURCE=.\api_brow.h
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

SOURCE=.\dr.h
# End Source File
# Begin Source File

SOURCE=.\ffind.h
# End Source File
# Begin Source File

SOURCE=.\makefile.hco
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
# End Target
# End Project
