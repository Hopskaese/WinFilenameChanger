#include <iostream>
#include <Windows.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <tchar.h> 
#include <stdio.h>
#include <string>

bool DoEverything();
bool GetAllFilesInDirAndChange(TCHAR* path);
bool BuildFileTime(FILETIME ft, LPTSTR lpszString);
TCHAR* FindExtension(TCHAR* path);

int main()
{
	char c;
	if (DoEverything())
	{
		std::cout << "" << std::endl;
		std::cout << "success. Press any key to exit" << std::endl;
	}
	std::cin >> c;
	return 0;
}

bool DoEverything()
{
	TCHAR szDir[MAX_PATH];
	LPITEMIDLIST pidlBrowse;
	BROWSEINFO BRinfo;
	BRinfo.hwndOwner = NULL;
	BRinfo.pidlRoot = NULL;
	BRinfo.pszDisplayName = NULL;
	BRinfo.lpszTitle = L"Select Folder";
	BRinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	BRinfo.lpfn = NULL;
	BRinfo.lParam = NULL;

	pidlBrowse = SHBrowseForFolder(&BRinfo);

	if (pidlBrowse == NULL)
	{
		std::cout << "pidlBrowse == null" << std::endl;
		return false;
	}
	
	bool outcome = SHGetPathFromIDList(pidlBrowse, szDir);
	std::wstring message = L"ARE U SUURE THAT THIS IS THE RIGHT FOLDER?";
	message += L"\n";
	message += szDir;
	if (MessageBox(NULL, message.c_str(), L"Verifying action", 1) == IDCANCEL)
	{
		if (DoEverything())
			return true;
		else
			return false;
	}
	
	if (outcome)
	{
		if (GetAllFilesInDirAndChange(szDir))
			return true;
		else
			return false;
	}
	else
	{
		std::cout << "Error on shgetpathfromidlist" << std::endl;
		return false;
	}
}

bool GetAllFilesInDirAndChange(TCHAR* path)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR szBuf[MAX_PATH];
	std::wstring orig_path(path);
	orig_path += L"\\";
	StringCchCat(path, MAX_PATH, TEXT("\\*"));
	TCHAR* extension = NULL;
	std::wstring old_name = L"";
	std::wstring new_name = L"";
	int cnt = 1;

	hFind = FindFirstFile(path, &ffd);
	
	if (INVALID_HANDLE_VALUE == hFind)
	{
		std::cout << "Error on FindFirstFile()" << std::endl;
		return false;
	}
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			printf("Found dir. Ignoring..\n");
		}
		else
		{
			if (BuildFileTime(ffd.ftLastWriteTime, szBuf))
			{
				extension = FindExtension(ffd.cFileName);
				old_name = orig_path + ffd.cFileName;
				if (extension != NULL)
					new_name = orig_path + szBuf + extension;
				else
					return false;

				if (!MoveFile(old_name.c_str(), new_name.c_str()))
				{
					HRESULT temp = GetLastError();
					if (temp == 183)
					{
						new_name = orig_path + szBuf;
						new_name += L"(";
						new_name += std::to_wstring(cnt);
						new_name += L")";
						new_name += extension;
						MoveFile(old_name.c_str(), new_name.c_str());
						cnt++;
						continue;
					}
					std::cout << "Movefile error" << std::endl;
					std::cout << "errorcode" << temp << std::endl;
					return false;
				}
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	return true;
}

bool BuildFileTime(FILETIME ft, LPTSTR lpszString)
{
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet = 0;
	DWORD dwSize = MAX_PATH;

	FileTimeToSystemTime(&ft, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	dwRet = StringCchPrintf(lpszString, dwSize,
		TEXT("%d_%02d_%02d %02d_%02d_%02d"),
		stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

	if (S_OK == dwRet)
		return true;
	
	std::cout << "Error building filetime " << std::endl;
	return false;
}

TCHAR* FindExtension(TCHAR* path)
{
	while (path++)
	{
		if (*path == L'.')
			return path;
	}
	return NULL;
}