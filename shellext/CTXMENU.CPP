/***************************************************************************
 Copyright 2004 Sebastian Ewert

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

//  MODULE:   ctxmenu.cpp
//
//  PURPOSE:   Implements the IContextMenu member functions necessary to support
//             the context menu portions of this shell extension.  Context menu
//             shell extensions are called when the user right clicks on a file
//             (of the type registered for the shell extension--see SHELLEXT.REG
//             for details on the registry entries.
//

#include "priv.h"
#include "shellext.h"
#include "resource.h"
#include "SysInfo.h"
#include "IconBitmapUtils.h"

extern HINSTANCE g_hmodThisDll;

HBITMAP LoadIconAsBitmap(HINSTANCE hResourceDll, UINT nIDResource)
{
	HICON hIcon = (HICON)LoadImage(hResourceDll, MAKEINTRESOURCE(nIDResource),
						IMAGE_ICON, 12, 12, LR_DEFAULTCOLOR);

	if (hIcon)
	{
		ICONINFO iconInfo;
		GetIconInfo(hIcon, & iconInfo);
		DeleteObject(iconInfo.hbmMask); 
		DestroyIcon(hIcon);
		return iconInfo.hbmColor;
	}

	return NULL;
}

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::QueryContextMenu()\r\n"));
	#endif
	
	HBITMAP hBmp;
	HMENU hSubMenu;
	MENUITEMINFO mii = {0};
    if(!(CMF_DEFAULTONLY & uFlags))
    {
		hSubMenu = CreateMenu();
        unsigned int mask = 0;

        HKEY hKey;
        if(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\RapidCRC"),
					    0, KEY_QUERY_VALUE 
#ifdef _WIN64
					    | KEY_WOW64_32KEY 
#endif
					    , &hKey) == ERROR_SUCCESS)
        {
	        DWORD dwRegLength = 4;
            DWORD dwRegContent, dwRegType;

            if(RegQueryValueEx(hKey, TEXT("ShellExtMenuItemsMask"), NULL, & dwRegType, (LPBYTE)&dwRegContent, & dwRegLength) == ERROR_SUCCESS ) {
                if(dwRegType == REG_DWORD)
                    mask = dwRegContent;
            }
	        RegCloseKey(hKey);
        }

#define InsertOptional(f, id, t)\
    if(!(1 << id & mask)) InsertMenu(hSubMenu, count++, f, idCmdFirst + id, t);

        unsigned int count = 0;
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_RAPIDCRC, TEXT("Open"));
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_REPARENT, TEXT("Open with reparent"));
        InsertOptional(MF_STRING | MF_BYPOSITION, IDM_OPENHASHFILES, TEXT("Open all hash files"));
		InsertOptional(MF_SEPARATOR | MF_BYPOSITION, 3, NULL);
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_SFV, TEXT("Create SFV"));
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_MD5, TEXT("Create MD5"));
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_SHA1, TEXT("Create SHA1"));
        InsertOptional(MF_STRING | MF_BYPOSITION, IDM_SHA256, TEXT("Create SHA256"));
        InsertOptional(MF_STRING | MF_BYPOSITION, IDM_SHA512, TEXT("Create SHA512"));
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_NAME, TEXT("Put CRC in Filename"));
		InsertOptional(MF_STRING | MF_BYPOSITION, IDM_NTFS, TEXT("Put CRC in NTFS Stream"));

		mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
		mii.cbSize = sizeof(mii);
		mii.wID = idCmdFirst + IDM_SUBMENU;
		mii.hSubMenu = hSubMenu;
		mii.dwTypeData = TEXT("RapidCRC");

		InsertMenuItem(hMenu,indexMenu,TRUE,&mii);

        InsertMenu(hMenu, 
                   indexMenu + 1, 
				   MF_SEPARATOR | MF_BYPOSITION, 
                   NULL,
				   NULL);
		if(SysInfo::Instance().IsVistaOrLater()) {
			ZeroMemory(&mii,sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_BITMAP;
			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_RAPIDCRC);
			SetMenuItemInfo(hMenu, indexMenu, TRUE, &mii);

			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_OPEN_FOLDER);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_RAPIDCRC, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_REPARENT, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_OPENHASHFILES, FALSE, &mii);

			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_HASH_FILE);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_SFV, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_MD5, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_SHA1, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_SHA256, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_SHA512, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_NAME, FALSE, &mii);
            SetMenuItemInfo(hSubMenu, idCmdFirst + IDM_NTFS, FALSE, &mii);
		} else {
			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_RAPIDCRC);
			SetMenuItemBitmaps(hMenu,indexMenu,MF_BYPOSITION,hBmp,hBmp);

			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_OPEN_WITH);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_RAPIDCRC, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_REPARENT, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_OPENHASHFILES, MF_BYCOMMAND, hBmp, hBmp);

			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_CREATE_SFV);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_SFV, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_MD5, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_SHA1, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_SHA256, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_SHA512, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_NAME, MF_BYCOMMAND, hBmp, hBmp);
            SetMenuItemBitmaps(hSubMenu, idCmdFirst + IDM_NTFS, MF_BYCOMMAND, hBmp, hBmp);
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_SUBMENU + 1));
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
}


////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::GetCommandString(...)
//
//  PURPOSE: Retrieve various text strings associated with the context menu
//
//	Param			Type			Use
//	-----			----			---
//	idCmd			UINT			ID of the command
//	uFlags			UINT			which type of info are we requesting
//	reserved		UINT *			must be NULL
//	pszName			LPSTR			output buffer
//	cchMax			UINT			max chars to copy to pszName
//
////////////////////////////////////////////////////////////////////////

STDMETHODIMP CShellExt::GetCommandString(THIS_
										 UINT_PTR idCommand,
										 UINT uFlags,
										 LPUINT lpReserved,
										 LPSTR pszName,
										 UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;
	

    if(idCommand == IDM_RAPIDCRC)
    {
	    switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and calculates checksum");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and calculates checksum");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCOpen");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCOpen");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_REPARENT)
    {
	    switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and calculates checksum, offering reparent action for checksum files");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and calculates checksum, offering reparent action for checksum files");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCOpenReparent");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCOpenReparent");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
    else if(idCommand == IDM_OPENHASHFILES)
    {
	    switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens all hash files in RapidCRC and calculates checksums");
				break; 

	        case GCS_HELPTEXTW: 
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens all hash files in RapidCRC and calculates checksums");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCOpenHashFiles");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCOpenHashFiles");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_SFV)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an SFV File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an SFV File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateSFV");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateSFV");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_MD5)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an MD5 File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an MD5 File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateMD5");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateMD5");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_SHA1)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an SHA1 File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an SHA1 File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateSHA1");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateSHA1");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
    else if(idCommand == IDM_SHA256)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an SHA256 File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an SHA256 File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateSHA256");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateSHA256");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
    else if(idCommand == IDM_SHA512)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an SHA512 File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an SHA512 File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateSHA512");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateSHA512");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_NAME)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and puts the CRC into the Filename");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and puts the CRC into the Filename");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCPutName");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCPutName");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_NTFS)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and puts the CRC into an NTFS Stream");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and puts the CRC into an NTFS Stream");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCPutNTFS");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCPutNTFS");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else
	{
		    return hr;
    }
	return hr;
}


//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::InvokeCommand()\r\n"));
	#endif

	BOOL fEx = FALSE;
	BOOL fUnicode = FALSE;

    if(lpcmi->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        fEx = TRUE;
        if((lpcmi->fMask & CMIC_MASK_UNICODE))
        {
            fUnicode = TRUE;
        }
    }


    if( !fUnicode && HIWORD(lpcmi->lpVerb))
    {
        if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCOpen"))
        {
            doRapidCrcOpen(NULL);
        }
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCOpenReparent"))
        {
            doRapidCrcOpen(TEXT(" -Reparent"));
        }
        else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCOpenHashFiles"))
        {
            doRapidCrcOpen(TEXT(" -HashFilesOnly"));
        }
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateSFV"))
		{
			doRapidCrcOpen(TEXT(" -CreateSFV"));
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateMD5"))
		{
			doRapidCrcOpen(TEXT(" -CreateMD5"));
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateSHA1"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA1"));
		}
        else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateSHA256"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA256"));
		}
        else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateSHA512"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA512"));
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCPutName"))
		{
			doRapidCrcOpen(TEXT(" -PutNAME"));
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCPutNTFS"))
		{
			doRapidCrcOpen(TEXT(" -PutNTFS"));
		}
		else
		{
			return E_FAIL;
		}

    }

    else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) lpcmi)->lpVerbW))
    {
        if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCOpen"))
        {
            doRapidCrcOpen(NULL);
        }
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCOpenReparent"))
        {
            doRapidCrcOpen(TEXT(" -Reparent"));
        }
        else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCHashFiles"))
        {
            doRapidCrcOpen(TEXT(" -HashFilesOnly"));
        }
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateSFV"))
		{
			doRapidCrcOpen(TEXT(" -CreateSFV"));
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateMD5"))
		{
			doRapidCrcOpen(TEXT(" -CreateMD5"));
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateSHA1"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA1"));
		}
        else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateSHA256"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA256"));
		}
        else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateSHA512"))
		{
			doRapidCrcOpen(TEXT(" -CreateSHA512"));
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCPutName"))
		{
			doRapidCrcOpen(TEXT(" -PutNAME"));
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCPutNTFS"))
		{
			doRapidCrcOpen(TEXT(" -PutNTFS"));
		}
		else
		{
			return E_FAIL;
		}
    }

    else if(LOWORD(lpcmi->lpVerb) == IDM_RAPIDCRC)
    {
        doRapidCrcOpen(NULL);
    }
	else if(LOWORD(lpcmi->lpVerb) == IDM_REPARENT)
    {
        doRapidCrcOpen(TEXT(" -Reparent"));
    }
    else if(LOWORD(lpcmi->lpVerb) == IDM_OPENHASHFILES)
    {
        doRapidCrcOpen(TEXT(" -HashFilesOnly"));
    }
	else if(LOWORD(lpcmi->lpVerb) == IDM_SFV)
	{
		doRapidCrcOpen(TEXT(" -CreateSFV"));
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_MD5)
	{
		doRapidCrcOpen(TEXT(" -CreateMD5"));
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_SHA1)
	{
		doRapidCrcOpen(TEXT(" -CreateSHA1"));
	}
    else if(LOWORD(lpcmi->lpVerb) == IDM_SHA256)
	{
		doRapidCrcOpen(TEXT(" -CreateSHA256"));
	}
    else if(LOWORD(lpcmi->lpVerb) == IDM_SHA512)
	{
		doRapidCrcOpen(TEXT(" -CreateSHA512"));
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_NAME)
	{
		doRapidCrcOpen(TEXT(" -PutNAME"));
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_NTFS)
	{
		doRapidCrcOpen(TEXT(" -PutNTFS"));
	}
    else // and here is the real action
    {
		return E_FAIL;
	}

    return S_OK;
}

void CShellExt::doRapidCrcOpen(TCHAR *command) {
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, command);
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}
