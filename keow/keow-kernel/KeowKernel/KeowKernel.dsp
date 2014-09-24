# Microsoft Developer Studio Project File - Name="KeowKernel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=KeowKernel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KeowKernel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KeowKernel.mak" CFG="KeowKernel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KeowKernel - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "KeowKernel - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KeowKernel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /Zi /O2 /I ".\linux-abi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"includes.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "NDEBUG"
# ADD RSC /l 0x1409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Release/KeowKernel.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying data files
PostBuild_Cmds=copy keow-gate\keow-gate.dso ..\..\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "KeowKernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".\linux-abi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"includes.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "_DEBUG"
# ADD RSC /l 0x1409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Debug/KeowKernel.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying data files
PostBuild_Cmds=copy keow-gate\keow-gate.dso ..\..\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "KeowKernel - Win32 Release"
# Name "KeowKernel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ConstantMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\DevConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\Device.cpp
# End Source File
# Begin Source File

SOURCE=.\Filesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesystemDev.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesystemGenericStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesystemKeow.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesystemProc.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHNtConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHNull.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHPipe.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHRandom.cpp
# End Source File
# Begin Source File

SOURCE=.\IOHStaticData.cpp
# End Source File
# Begin Source File

SOURCE=.\KernelStartup.cpp
# End Source File
# Begin Source File

SOURCE=.\KernelTable.cpp
# End Source File
# Begin Source File

SOURCE=.\LegacyWindows.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\MountPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\Path.cpp
# End Source File
# Begin Source File

SOURCE=.\Process.cpp
# End Source File
# Begin Source File

SOURCE=.\SocketCalls.cpp
# End Source File
# Begin Source File

SOURCE=.\String.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_io.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_mount.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_net.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_perms.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_process.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_rtsig.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_sys.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCallDll_Kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\SysCalls.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ConstantMapping.h
# End Source File
# Begin Source File

SOURCE=.\DevConsole.h
# End Source File
# Begin Source File

SOURCE=.\Device.h
# End Source File
# Begin Source File

SOURCE=.\Filesystem.h
# End Source File
# Begin Source File

SOURCE=.\FilesystemDev.h
# End Source File
# Begin Source File

SOURCE=.\FilesystemGenericStatic.h
# End Source File
# Begin Source File

SOURCE=.\FilesystemKeow.h
# End Source File
# Begin Source File

SOURCE=.\FilesystemProc.h
# End Source File
# Begin Source File

SOURCE=.\includes.h
# End Source File
# Begin Source File

SOURCE=.\IOHandler.h
# End Source File
# Begin Source File

SOURCE=.\IOHFile.h
# End Source File
# Begin Source File

SOURCE=.\IOHNtConsole.h
# End Source File
# Begin Source File

SOURCE=.\IOHNull.h
# End Source File
# Begin Source File

SOURCE=.\IOHPipe.h
# End Source File
# Begin Source File

SOURCE=.\IOHRandom.h
# End Source File
# Begin Source File

SOURCE=.\IOHStaticData.h
# End Source File
# Begin Source File

SOURCE=.\KernelStartup.h
# End Source File
# Begin Source File

SOURCE=.\KernelTable.h
# End Source File
# Begin Source File

SOURCE=.\LegacyWindows.h
# End Source File
# Begin Source File

SOURCE=.\linux_includes.h
# End Source File
# Begin Source File

SOURCE=.\List.h
# End Source File
# Begin Source File

SOURCE=.\MemoryHelper.h
# End Source File
# Begin Source File

SOURCE=.\MountPoint.h
# End Source File
# Begin Source File

SOURCE=.\Path.h
# End Source File
# Begin Source File

SOURCE=.\Process.h
# End Source File
# Begin Source File

SOURCE=.\SocketCalls.h
# End Source File
# Begin Source File

SOURCE=.\String.h
# End Source File
# Begin Source File

SOURCE=.\SysCallDll.h
# End Source File
# Begin Source File

SOURCE=.\SysCalls.h
# End Source File
# Begin Source File

SOURCE=.\Tree.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\WinFiles.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resources.rc
# End Source File
# Begin Source File

SOURCE=.\Tux.ico
# End Source File
# End Group
# End Target
# End Project
