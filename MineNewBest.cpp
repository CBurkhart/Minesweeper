/**
    @file MineNewBest.cpp

    @author Craig Burkhart

    @brief Code for the new best time dialog window.
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
#include "MineNewBest.h"
#include "MineDebug.h"

/**
    MineNewBest_Dialog
*//**
    Dialog procedure for the new best time dialog.

    @param[in] hDlg    - Handle to new best time dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineNewBest_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND   editHwnd = NULL;
    static LPWSTR pLevelString = NULL;
    INT_PTR       returnValue = (INT_PTR) FALSE;
    MINE_ERROR    status = MINE_ERROR_SUCCESS;
    MINEDEBUG_INITIALIZE_ERROR_VALUE;

    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        editHwnd = GetDlgItem(hDlg, IDC_NEWBEST_EDIT);
        if (NULL == editHwnd)
        {
            MINEDEBUG_GET_ERROR_VALUE;
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting edit control handle: %lu\n", errorValue);
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MINEDEBUG_GET_ERROR_VALUE;
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", errorValue);
            }
            break;
        }

        //Store pointer to correct string in menu data global variable
        pLevelString = ((MINE_LEVEL_BEGINNER == menuData.gameLevel) ?  menuData.beginnerName : 
                        ((MINE_LEVEL_INTERMEDIATE == menuData.gameLevel) ? 
                         menuData.intermediateName : menuData.expertName));

        //Show which level was won in the dialog
        if (0 == SetDlgItemTextW(hDlg, IDC_NEWBEST_LEVELID, 
                                 ((MINE_LEVEL_BEGINNER == menuData.gameLevel) ? L"for beginner level." :
                                  ((MINE_LEVEL_INTERMEDIATE == menuData.gameLevel) ?
                                    L"for intermediate level." : L"for expert level."))))
        {
            MINEDEBUG_GET_ERROR_VALUE;
            MineDebug_PrintWarning("Setting level identification string: %lu\n", errorValue);
        }

        //Limit edit box to MINE_NAME_BUFFER_CHARS characters
        Edit_LimitText(editHwnd, MINE_NAME_BUFFER_CHARS);

        /** Display name of previous record holder for level in edit box. */
        if (0 == SetWindowTextW(editHwnd, pLevelString))
        {
            MINEDEBUG_GET_ERROR_VALUE;
            MineDebug_PrintWarning("Setting edit text to previous value: %lu", errorValue);
        }

        Edit_SetSel(editHwnd, 0, -1);

        break;

    case WM_COMMAND:
        /** Upon button press, close about dialog. */
        if ((IDOK == LOWORD(wParam)) || (IDCANCEL == LOWORD(wParam)))
        {
            returnValue = (INT_PTR) TRUE;

            if (0 == GetWindowTextW(editHwnd, pLevelString, MINE_NAME_BUFFER_CHARS + 1))
            {
                MINEDEBUG_GET_ERROR_VALUE;
                MineDebug_PrintWarning("Problem getting edit text, or no text present: %lu", errorValue);
                pLevelString[0] = '\0';
            }

            /** Store new record holder name and time in registry. */
            if (MINE_ERROR_SUCCESS != Mine_SetRegString(((MINE_LEVEL_BEGINNER == menuData.gameLevel) ? 
                                                         L"BeginnerName" :
                                                         ((MINE_LEVEL_INTERMEDIATE == menuData.gameLevel) ?
                                                         L"IntermediateName" : L"ExpertName")), pLevelString))
            {
                MineDebug_PrintWarning("In function Mine_SetRegString\n");
            }

            if (MINE_LEVEL_BEGINNER == menuData.gameLevel)
            {
                menuData.beginnerTime = (DWORD) gameData.time;
            }
            else if (MINE_LEVEL_INTERMEDIATE == menuData.gameLevel)
            {
                menuData.intermediateTime = (DWORD) gameData.time;
            }
            else 
            {
                menuData.expertTime = (DWORD) gameData.time;
            }

            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(((MINE_LEVEL_BEGINNER == menuData.gameLevel) ?
                                                        L"BeginnerTime" :
                                                        ((MINE_LEVEL_INTERMEDIATE == menuData.gameLevel) ? 
                                                        L"IntermediateTime" : L"ExpertTime")), gameData.time))
            {
                MineDebug_PrintWarning("In function Mine_SetRegDword\n");
            }

            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MINEDEBUG_GET_ERROR_VALUE;
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", errorValue);
            }
        }
        break;

    default:
        break;
    }

    return returnValue;
}