/**
    @file MineCustom.cpp

    @author Craig Burkhart

    @brief Code for the custom dialog window.
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
#include "MineCustom.h"
#include "MineDebug.h"

/**
    MineCustom_Dialog
*//**
    Dialog procedure for the custom dialog.

    @param[in] hDlg    - Handle to custom dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineCustom_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL       bReturn = TRUE;
    INT_PTR    returnValue = (INT_PTR) FALSE;
    UINT       value = 0;

    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        /** Set dialog edit boxes to previous values. */ 
        if (0 == SetDlgItemInt(hDlg, IDC_CUSTOM_HEIGHT, (UINT) menuData.customHeight, FALSE))
        {
            MineDebug_PrintWarning("Setting custom height int: %lu\n", GetLastError());
        }

        if (0 == SetDlgItemInt(hDlg, IDC_CUSTOM_WIDTH, (UINT) menuData.customWidth, FALSE))
        {
            MineDebug_PrintWarning("Setting custom width int: %lu\n", GetLastError());
        }

        if (0 == SetDlgItemInt(hDlg, IDC_CUSTOM_MINES, (UINT) menuData.customMines, FALSE))
        {
            MineDebug_PrintWarning("Setting custom mines int: %lu\n", GetLastError());
        }
        break;

    case WM_COMMAND:
        /** Upon button press, close about dialog. */
        if (IDOK == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;

            /** If OK was pressed, get values from dialog. */
            value = GetDlgItemInt(hDlg, IDC_CUSTOM_HEIGHT, &bReturn, FALSE);
            if (FALSE == bReturn)
            {
                MineDebug_PrintWarning("Getting custom height int: %lu\n", GetLastError());
                value = MINE_BEGINNER_HEIGHT;
            }
            else if (MINE_BEGINNER_HEIGHT > value)
            {
                MineDebug_PrintWarning("Custom height too small: %u\n", value);
                value = MINE_BEGINNER_HEIGHT;
            }
            else if (MINE_MAX_HEIGHT < value)
            {
                MineDebug_PrintWarning("Custom height too big: %u\n", value);
                value = MINE_MAX_HEIGHT;
            }

            menuData.customHeight = (DWORD) value;

            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"CustomHeight", menuData.customHeight))
            {
                MineDebug_PrintWarning("In function Mine_SetRegDword\n");
            }

            value = GetDlgItemInt(hDlg, IDC_CUSTOM_WIDTH, &bReturn, FALSE);
            if (FALSE == bReturn)
            {
                MineDebug_PrintWarning("Getting custom width int: %lu\n", GetLastError());
                value = MINE_BEGINNER_WIDTH;
            }
            else if (MINE_BEGINNER_WIDTH > value)
            {
                MineDebug_PrintWarning("Custom width too small: %u\n", value);
                value = MINE_BEGINNER_WIDTH;
            }
            else if (MINE_MAX_WIDTH < value)
            {
                MineDebug_PrintWarning("Custom width too big: %u\n", value);
                value = MINE_MAX_WIDTH;
            }

            menuData.customWidth = (DWORD) value;

            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"CustomWidth", menuData.customWidth))
            {
                MineDebug_PrintWarning("In function Mine_SetRegDword\n");
            }

            value = GetDlgItemInt(hDlg, IDC_CUSTOM_MINES, &bReturn, FALSE);
            if (FALSE == bReturn)
            {
                MineDebug_PrintWarning("Getting custom mines int: %lu\n", GetLastError());
                value = MINE_BEGINNER_MINES;
            }
            else if (MINE_BEGINNER_MINES > value)
            {
                MineDebug_PrintWarning("Custom mines too small: %u\n", value);
                value = MINE_BEGINNER_MINES;
            }
            else if ((menuData.customHeight - 1)*(menuData.customWidth - 1) < (DWORD) value)
            {
                MineDebug_PrintWarning("Custom mines too big: %u\n", value);
                value = (UINT) (menuData.customHeight - 1)*(menuData.customWidth - 1);
            }

            menuData.customMines = (DWORD) value;

            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"CustomMines", menuData.customMines))
            {
                MineDebug_PrintWarning("In function Mine_SetRegDword\n");
            }

            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }            
        }
        /** If cancel was pressed, exit without changing any settings. */
        else if (IDCANCEL == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;
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