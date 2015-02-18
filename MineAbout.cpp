/**
    @file MineAbout.cpp

    @author Craig Burkhart

    @brief Code for the about dialog window.
*//*
    Copyright (C) 2014 - Craig Burkhart

    This file is part of Minesweeper Deluxe.

    Minesweeper Deluxe is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Minesweeper Deluxe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Minesweeper Deluxe.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "Mine.h"
#include "MineAbout.h"
#include "MineDebug.h"

/**
    MineAbout_Dialog
*//**
    Dialog procedure for the about dialog.

    @param[in] hDlg    - Handle to about dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineAbout_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR     dialogReturn = 0;
    static HWND link1Hwnd = NULL;
    static HWND link2Hwnd = NULL;
    INT_PTR     returnValue = (INT_PTR) FALSE;
    HINSTANCE   shellExecuteReturn = 0;
    MINE_ERROR  status = MINE_ERROR_SUCCESS;

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        link1Hwnd = GetDlgItem(hDlg, IDC_ABOUT_LINK1);
        if (NULL == link1Hwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting link 1 control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        link2Hwnd = GetDlgItem(hDlg, IDC_ABOUT_LINK2);
        if (NULL == link2Hwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting link 2 control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }      
        break;

    case WM_COMMAND:
        /** Upon button press, close about dialog. */
        if ((IDOK == LOWORD(wParam)) || (IDCANCEL == LOWORD(wParam)))
        {
            returnValue = (INT_PTR) TRUE;
            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
        }
        else if(IDC_ABOUT_LICENSE == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;

            //Open dialog with full license text
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_LICENSE), hDlg, MineAbout_LicenseDialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show license dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show license dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }
        }
        break;

    case WM_NOTIFY:
        if ((NM_CLICK == ((LPNMHDR)lParam)->code) || (NM_RETURN == ((LPNMHDR)lParam)->code))
        {
            if (link1Hwnd == ((LPNMHDR)lParam)->hwndFrom)
            {
                returnValue = (INT_PTR) TRUE;
                shellExecuteReturn = ShellExecuteW(hwnd, L"open", L"http://www.example.com", 
                                                   NULL, NULL, 0);
                if ((int) shellExecuteReturn <= 32)
                {
                    MineDebug_PrintWarning("Unable to open www.example.com: %i\n",
                                           (int) shellExecuteReturn);
                } 
            }
            else if (link2Hwnd == ((LPNMHDR)lParam)->hwndFrom)
            {
                returnValue = (INT_PTR) TRUE;
                shellExecuteReturn = ShellExecuteW(hwnd, L"open", L"http://www.gnu.org/licenses/", 
                                                   NULL, NULL, 0);
                if ((int) shellExecuteReturn <= 32)
                {
                    MineDebug_PrintWarning("Unable to open www.gnu.org/licenses: %i\n",
                                           (int) shellExecuteReturn);
                }
            }
        }
        break;
        
    default:
        break;
    }

    return returnValue;
}

/**
    MineAbout_LicenseDialog
*//**
    Dialog procedure for the full license dialog.

    @param[in] hDlg    - Handle to license dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineAbout_LicenseDialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND    editHwnd = NULL;
    static HGLOBAL licenseHandle = NULL;
    HRSRC          licenseResource = NULL;
    LPCSTR         licenseText = NULL;
    INT_PTR        returnValue = (INT_PTR) FALSE;
    MINE_ERROR     status = MINE_ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        editHwnd = GetDlgItem(hDlg, IDC_LICENSE_EDIT);
        if (NULL == editHwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting edit control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        /** Load and then display full GNU License text */
        licenseResource = FindResourceW(hInst, MAKEINTRESOURCEW(IDT_LICENSE), L"TEXT");
        if (NULL == licenseResource)
        {
            status = MINE_ERROR_RESOURCE;
            MineDebug_PrintError("Getting license text resource: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        licenseHandle = LoadResource(hInst, licenseResource);
        if (NULL == licenseHandle)
        {
            status = MINE_ERROR_RESOURCE;
            MineDebug_PrintError("Getting handle to license text resource: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }
        
        licenseText = (LPCSTR) LockResource(licenseHandle);
        if (NULL == licenseText)
        {
            status = MINE_ERROR_RESOURCE;
            MineDebug_PrintError("Getting pointer to full license text: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        if (0 == SetWindowTextA(editHwnd, licenseText))
        {
            MineDebug_PrintWarning("Setting edit control to hold license text: %lu\n", GetLastError());
        }

        Edit_SetSel(editHwnd, (WPARAM) -1, -1); 

        break;

    case WM_COMMAND:
        /** Upon button press, close license dialog. */
        if ((IDOK == LOWORD(wParam)) || (IDCANCEL == LOWORD(wParam)))
        {
            returnValue = (INT_PTR) TRUE;

            //Clean up
            if (NULL != licenseHandle)
            {
                if (0 != FreeResource(licenseHandle))
                {
                    MineDebug_PrintWarning("Freeing license resource\n");
                }
            }

            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
        }
        break;

    default:
        break;
    }

    return returnValue;
}