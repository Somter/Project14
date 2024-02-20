#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include "resource.h"

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void ProcessList(HWND hWndList);
void TerminateSelectedProcess(HWND hWndList);
DWORD GetProcessIdFromName(LPCTSTR szProcessName); 
HWND hWndList;

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)    
{
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        ProcessList(GetDlgItem(hWnd, IDC_LIST1));
        hWndList = GetDlgItem(hWnd, IDC_LIST1); 
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON2:
            TerminateSelectedProcess(GetDlgItem(hWndList, IDC_LIST1));
            return TRUE;
            break;      
        case IDC_BUTTON1:
            ProcessList(GetDlgItem(hWndList, IDC_LIST1));
            break;
        case IDC_BUTTON3:
        {
            int index = SendMessage(hWndList, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
                TCHAR szName[MAX_PATH];
                SendMessage(hWndList, LB_GETTEXT, index, (LPARAM)szName);
                DWORD Id = GetProcessIdFromName(szName);

                HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                PROCESSENTRY32 pe32;
                pe32.dwSize = sizeof(PROCESSENTRY32);
                if (Process32First(hSnapshot, &pe32)) {     
                    while (TRUE)
                    {
                        if (pe32.th32ProcessID == Id) {
                            TCHAR szMessage[512];
                            swprintf_s(szMessage, _T("Process ID: %u\nThreads: %u\nPriority: %u\nName: %s"), pe32.th32ProcessID, pe32.cntThreads, pe32.pcPriClassBase, pe32.szExeFile);
                            MessageBox(hWndList, szMessage, _T("Дополнительная информация о процессе"), MB_OK | MB_ICONINFORMATION);
                            break;
                        }
                        if (!Process32Next(hSnapshot, &pe32))
                            break;
                    }
                }
                CloseHandle(hSnapshot);
            }
            return TRUE;
        }
        break;
        case IDC_BUTTON4:
        {
            TCHAR addProcess[MAX_PATH];
            GetDlgItemText(hWnd, IDC_EDIT2, addProcess, MAX_PATH);
            PROCESSENTRY32 pe32;    

            pe32.dwSize = sizeof(PROCESSENTRY32);   
            _tcscpy_s(pe32.szExeFile, addProcess);
                
            SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);     
        }
        }   
        break; 
    case WM_CLOSE:
        EndDialog(hWnd, 0);
        return TRUE;
    }
    return FALSE;
}

void ProcessList(HWND hWndList)
{
    SendMessage(hWndList, LB_RESETCONTENT, 0, 0);

    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapShot, &pe32))
    {
        SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
        while (Process32Next(hSnapShot, &pe32))
        {
            SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
        }
    }
    CloseHandle(hSnapShot);
}

void TerminateSelectedProcess(HWND hWndList)
{
    int index = SendMessage(hWndList, LB_GETCURSEL, 0, 0);
    if (index != LB_ERR)
    {
        TCHAR szProcessName[MAX_PATH];
        SendMessage(hWndList, LB_GETTEXT, index, (LPARAM)szProcessName);
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, GetProcessIdFromName(szProcessName));
        if (hProcess != NULL)
        {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess); 
        }
    }
}
    
DWORD GetProcessIdFromName(LPCTSTR szProcessName)
{
    DWORD dwId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        while (true)
        {
            if (_tcscmp(szProcessName, pe32.szExeFile) == 0)
            {
                dwId = pe32.th32ProcessID;  
                break;
            }
            if (!Process32Next(hSnapshot, &pe32))
                break;
        }
    }

    CloseHandle(hSnapshot);
    return dwId;
}