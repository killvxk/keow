/*
 * Copyright 2005 Paul Walker
 *
 * GNU General Public License
 * 
 * This file is part of: Kernel Emulation on Windows (keow)
 *
 * Keow is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Keow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keow; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// FilesystemKeow.cpp: implementation of the FilesystemKeow class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "FilesystemKeow.h"

#include <Shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>

//////////////////////////////////////////////////////////////////////

FilesystemKeow::FilesystemKeow()
{

}

FilesystemKeow::~FilesystemKeow()
{

}



//
// If lpszLinkFile is a shortcut then return the target it points to
// otherwise return the original path
//
string FilesystemKeow::GetShortcutTarget(string& path) 
{ 
    HRESULT hres; 
    IShellLinkA* psl = 0; 
    IPersistFile* ppf = 0;
	string Dest;
    wchar_t * pWsz = new wchar_t[MAX_PATH+1];
    char * pSz = new char[MAX_PATH+1];

    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                            IID_IShellLinkA, (LPVOID*)&psl); 
    if(FAILED(hres))  {
		ktrace("cocreate failed hr 0x%08lx\n", hres);
		goto cleanup;
	}

 
    // Get a pointer to the IPersistFile interface. 
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf); 
    if(FAILED(hres))  {
		ktrace("query interface failed hr 0x%08lx\n", hres);
		goto cleanup;
	}
	
    // Need a wide version of the filename
	memset(pWsz,0,sizeof(pWsz[0])*MAX_PATH);
    if(MultiByteToWideChar(CP_ACP, 0, path.c_str(), path.length(), pWsz, MAX_PATH) == 0) {
		hres=E_FAIL;
		ktrace("MultiByteToWideChar failed hr 0x%08lx\n", hres);
		goto cleanup;
	}


    // Load the shortcut. 
    hres = ppf->Load(pWsz, STGM_READ); 
    if(FAILED(hres))  {
		ktrace("load failed hr 0x%08lx\n", hres);
		goto cleanup;
	}


	// Resolve the link
	//lots of debug code when getting this to run on W2k
    hres = psl->Resolve(NULL, SLR_NO_UI|SLR_NOLINKINFO|SLR_NOUPDATE|SLR_NOSEARCH|SLR_NOTRACK);
    if(FAILED(hres)) {
		ktrace("resolve failed hr 0x%08lx, retrying with different flags\n", hres);
		//retry without some flags
	    hres = psl->Resolve(NULL, SLR_NO_UI|SLR_ANY_MATCH);
		if(FAILED(hres)) {
			ktrace("resolve failed hr 0x%08lx\n", hres);
			//ignore error, use whatever path was already there
		}
	}

    // Get the path to the link target.
	pSz[0] = 0;
#undef KEOW_USE_LNK_PATH
#if KEOW_USE_LNK_PATH
    WIN32_FIND_DATA wfd; 
    hres = psl->GetPath(pSz, MAX_PATH,
                        (WIN32_FIND_DATA*)&wfd, 
                        SLGP_RAWPATH);
	if(hres==NOERROR && path[0]!=0)
    { 
        //success! lpszPath populated
    }
	else
	{
		//cygwin sym-link use desc as well as target, so use desc if target not available
		ktrace("read link path failed 0x%08lx, falling back to description\n");
#else
	{
#endif
		psl->GetDescription(pSz, MAX_PATH);
		if(FAILED(hres))  {
			ktrace("get desc failed hr 0x%08lx\n", hres);
			goto cleanup;
		}
	}
	Dest = pSz;


cleanup:
	if(ppf)
		ppf->Release();
	if(psl)
		psl->Release();
    delete pWsz;
    delete pSz;

//    if(TempPath[0] == 0) {
//		ktrace("GetShortCutTarget failed to get a path\n");
//		hres=E_FAIL;
//	}

	if(FAILED(hres))
		ktrace("GetShortCutTarget failed, hr=0x%08lx\n", hres);

    return Dest; 
}



// CreateLink - uses the Shell's IShellLink and IPersistFile interfaces 
//              to create and store a shortcut to the specified object. 
//
// Returns the result of calling the member functions of the interfaces. 
//

HRESULT FilesystemKeow::CreateShortcut(const string& LinkTo, const string& TheShortcut, const string& Description)
{ 
    HRESULT hres; 
    IShellLink* psl; 
 
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                            IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hres)) 
    { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target and add the description. 
        psl->SetPath(LinkTo); 
        psl->SetDescription(Description); 
 
        // Query IShellLink for the IPersistFile interface for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
 
        if (SUCCEEDED(hres)) 
        { 
            string wsz;
 
            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, TheShortcut.c_str(), -1, (wchar_t*)wsz.GetBuffer(MAX_PATH), MAX_PATH); 

            // TODO: Check return value from MultiByteWideChar to ensure success.
 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save((wchar_t*)wsz.c_str(), TRUE); 

			wsz.ReleaseBuffer();			

            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return hres; 
}


IOHandler * FilesystemKeow::CreateIOHandler(Path& path)
{
	//everything in this filesystem should be a file?
	return new IOHFile(path);
}

string FilesystemKeow::GetPathSeperator()
{
	return "\\";
}

bool FilesystemKeow::IsSymbolicLink(string& strPath)
{
	string Dest = GetShortcutTarget(strPath);
	return !Dest.empty();
}

string FilesystemKeow::GetLinkDestination(string& strPath)
{
	if(GetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES)
		return "";

	//possibly a link, check it fully
	return GetShortcutTarget(strPath);
}

bool FilesystemKeow::IsRelativePath(string& strPath)
{
	return strPath[0]!='/' && strPath[1]!='\\'; //PathIsRelative(strPath)!=FALSE;
}

const char * FilesystemKeow::Name()
{
	return "keow";
}


bool FilesystemKeow::CreateSymbolicLink(string& OldPath, string& NewPath)
{
	Path shortcut;
	shortcut.SetUnixPath(NewPath+".lnk");
	HRESULT hr = CreateShortcut(OldPath, shortcut.GetWin32Path(), OldPath);
	return SUCCEEDED(hr);
}
