/**
    @file MineBestTimes.cpp

    @author Craig Burkhart

    @brief Code for the "Fastest Mine Sweepers" dialog window.
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
#include "MineBestTimes.h"
#include "MineDebug.h"

/**
    MineBestTimes_Dialog
*//**
    Dialog procedure for the best times dialog.

    @param[in] hDlg    - Handle to best times dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineBestTimes_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR    returnValue = (INT_PTR) FALSE;

    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        if (MINE_ERROR_SUCCESS != MineBestTimes_UpdateLabels(hDlg))
        {
            MineDebug_PrintWarning("In function MineBestTimes_UpdateLabels\n");
        }
        break;

    case WM_COMMAND:
        /** Upon press of OK button, close about dialog. */
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            returnValue = (INT_PTR) TRUE;
            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
        }
        /** Reset socres to default time and name if "Reset Scores" button is pressed. */
        else if (IDC_BESTTIME_RESET == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;

            if (MINE_ERROR_SUCCESS != MineBestTimes_ResetScores())
            {
                MineDebug_PrintWarning("In function MineBestTimes_ResetScores\n");
            }

            if (MINE_ERROR_SUCCESS != MineBestTimes_UpdateLabels(hDlg))
            {
                MineDebug_PrintWarning("In function MineBestTimes_UpdateLabels\n");
            }
        }
        break;

    default:
        break;
    }

    return returnValue;
}

/**
    MineBestTimes_ResetScores
*//**
    Reset all of the high scores and names back to default values.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineBestTimes_ResetScores(VOID)
{
    BOOLEAN    bFalse = FALSE;
    HRESULT    result = S_OK;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        /** Reset the best time and name for beginner level. */
        if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"BeginnerTime", MINE_MAX_TIME))
        {
            MineDebug_PrintWarning("In function Mine_SetRegDword\n");
        }

        menuData.beginnerTime = MINE_MAX_TIME;

        result = StringCchCopyW(menuData.beginnerName, MINE_NAME_BUFFER_CHARS+1, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintError("Copying anonymous string to beginnerName: %li\n", result);
            status = MINE_ERROR_STRING;
            break;
        }

        if (MINE_ERROR_SUCCESS != Mine_SetRegString(L"BeginnerName", menuData.beginnerName))
        {
            MineDebug_PrintWarning("In function Mine_SetRegString\n");
        }

        /** Reset the best time and name for intermediate level. */
        if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"IntermediateTime", MINE_MAX_TIME))
        {
            MineDebug_PrintWarning("In function Mine_SetRegDword\n");
        }

        menuData.intermediateTime = MINE_MAX_TIME;

        result = StringCchCopyW(menuData.intermediateName, MINE_NAME_BUFFER_CHARS+1, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintError("Copying anonymous string to intermediaterName: %li\n", result);
            status = MINE_ERROR_STRING;
            break;
        }

        if (MINE_ERROR_SUCCESS != Mine_SetRegString(L"IntermediateName", menuData.intermediateName))
        {
            MineDebug_PrintWarning("In function Mine_SetRegString\n");
        }

        /**Reset the best time and name for expert level. */
        if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"ExpertTime", MINE_MAX_TIME))
        {
            MineDebug_PrintWarning("In function Mine_SetRegDword\n");
        }

        menuData.expertTime = MINE_MAX_TIME;

        result = StringCchCopyW(menuData.expertName, MINE_NAME_BUFFER_CHARS+1, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintError("Copying anonymous string to expertName: %li\n", result);
            status = MINE_ERROR_STRING;
            break;
        }

        if (MINE_ERROR_SUCCESS != Mine_SetRegString(L"ExpertName", menuData.expertName))
        {
            MineDebug_PrintWarning("In function Mine_SetRegString\n");
        }

    } while (bFalse);

    return status;
}

/**
    MineBestTimes_UpdateLabels
*//**
    Update the text on the best times dialog to show names and times.

    @param[in] hDlg - Handle to the best times dialog window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineBestTimes_UpdateLabels(_In_ HWND hDlg)
{
    BOOLEAN    bFalse = FALSE;
    WCHAR      buffer[MINE_BESTTIME_BUFFER_CHARS] = {0};
    HRESULT    hresult = S_OK;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        /** Display the beginner best time and name on the dialog window. */
        hresult = StringCchPrintfW(buffer, MINE_BESTTIME_BUFFER_CHARS, 
                                   L"%3lu seconds", menuData.beginnerTime);
        if (FAILED(hresult))
        {
            MineDebug_PrintError("Formatting beginner time string: %li\n", hresult);
            status = MINE_ERROR_STRING;
            break;
        }

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_BEGTIME, buffer))
        {
            MineDebug_PrintWarning("Setting beginner time string: %lu\n", GetLastError());
        }

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_BEGNAME, menuData.beginnerName))
        {
            MineDebug_PrintWarning("Setting beginner name string: %lu\n", GetLastError());
        }

        /** Display the intermediate best time and name on the dialog window. */
        hresult = StringCchPrintfW(buffer, MINE_BESTTIME_BUFFER_CHARS, 
                                   L"%3lu seconds", menuData.intermediateTime);
        if (FAILED(hresult))
        {
            MineDebug_PrintError("Formatting intermediate time string: %li\n", hresult);
            status = MINE_ERROR_STRING;
            break;
        }

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_INTTIME, buffer))
        {
            MineDebug_PrintWarning("Setting intermediate time string: %lu\n", GetLastError());
        }     

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_INTNAME, menuData.intermediateName))
        {
            MineDebug_PrintWarning("Setting intermediate name string: %lu\n", GetLastError());
        }

        /** Display the expert best time and name on the dialog window. */
        hresult = StringCchPrintfW(buffer, MINE_BESTTIME_BUFFER_CHARS, 
                                   L"%3lu seconds", menuData.expertTime);
        if (FAILED(hresult))
        {
            MineDebug_PrintError("Formatting expert time string: %li\n", hresult);
            status = MINE_ERROR_STRING;
            break;
        }

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_EXPTIME, buffer))
        {
            MineDebug_PrintWarning("Setting expert time string: %lu\n", GetLastError());
        }

        if (0 == SetDlgItemTextW(hDlg, IDC_BESTTIME_EXPNAME, menuData.expertName))
        {
            MineDebug_PrintWarning("Setting expert name string: %lu\n", GetLastError());
        }
    } while (bFalse);

    return status;
}