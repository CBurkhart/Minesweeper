/**
    @file MineMovement.cpp

    @author Craig Burkhart

    @brief Code for the movement dialog window.
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
    MineMovement_Dialog
*//**
    Dialog procedure for the movement dialog.

    @param[in] hDlg    - Handle to movement dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed. FALSE to have system process message.
*/
INT_PTR CALLBACK 
MineMovement_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND aggressiveHwnd = NULL;
    BOOL        bReturn = TRUE;
    static HWND checkHwnd = NULL;
    static HWND freqHwnd = NULL;
    INT_PTR     returnValue = (INT_PTR) FALSE;
    MINE_ERROR  status = MINE_ERROR_SUCCESS;
    UINT        value = 0;

    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        /** Upon initialization, instruct system to set
            input focus to first applicable child window. */
        returnValue = (INT_PTR) TRUE;

        //Get handles to individual control windows
        checkHwnd = GetDlgItem(hDlg, IDC_MOVEMENT_CHECK);
        if (NULL == checkHwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting check control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        aggressiveHwnd = GetDlgItem(hDlg, IDC_MOVEMENT_AGGRESSIVE);
        if (NULL == aggressiveHwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting aggressiveness edit control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        freqHwnd = GetDlgItem(hDlg, IDC_MOVEMENT_FREQ);
        if (NULL == checkHwnd)
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Getting frequency edit control handle: %lu\n", GetLastError());
            if (0 == EndDialog(hDlg, MINE_DIALOG_ERROR_OFFSET + (INT_PTR) status))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
            break;
        }

        /** Upon initialization, set edit controls to show previous value. */
        if (0 == SetDlgItemInt(hDlg, IDC_MOVEMENT_FREQ, (UINT) menuData.movementFreq, FALSE))
        {
            MineDebug_PrintWarning("Setting movement frequency int: %lu\n", GetLastError);
        }

        if (0 == SetDlgItemInt(hDlg, IDC_MOVEMENT_AGGRESSIVE, (UINT) menuData.movementAggressive, FALSE))
        {
            MineDebug_PrintWarning("Setting movement aggressiveness int: %lu\n", GetLastError);
        }

        if (menuData.useMovement)
        {
            (void) Button_SetCheck(checkHwnd, BST_CHECKED);
        }
        else
        {
            /** If movement is not enabled, display edit controls as grayed out. */
            (void) Edit_Enable(aggressiveHwnd, FALSE);

            (void) Edit_Enable(freqHwnd, FALSE);
        }

        break;

    case WM_COMMAND:
        /** Upon button press, close about dialog. */
        /** Pressing OK button makes specified changes. */
        if (IDOK == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;

            //Movement is enabled
            if (BST_CHECKED == Button_GetCheck(checkHwnd))
            {
                //Retrieve and set movement frequency value
                value = GetDlgItemInt(hDlg, IDC_MOVEMENT_FREQ, &bReturn, FALSE);
                if (FALSE == bReturn)
                {
                    MineDebug_PrintWarning("Getting movement frequency int: %lu\n", GetLastError());
                    //Upon error getting value from edit control, use default value as backup
                    value = MINE_SECOND;
                }
                //Check bounds on movement frequency value
                else if (MINE_QUARTER_SECOND > value)
                {
                    MineDebug_PrintWarning("Movement frequency too small: %u\n", value);
                    value = MINE_QUARTER_SECOND;
                }
                else if (MINE_MAX_TIME*MINE_SECOND < value)
                {
                    MineDebug_PrintWarning("Movement frequency too big: %u\n", value);
                    value = MINE_MAX_TIME*MINE_SECOND;
                }

                menuData.movementFreq = (DWORD) value;

                //Store movement frequency value in registry
                if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"MovementFrequency", menuData.movementFreq))
                {
                    MineDebug_PrintWarning("In function Mine_SetRegDword\n");
                }

                //Retrieve and set movement aggressiveness value
                value = GetDlgItemInt(hDlg, IDC_MOVEMENT_AGGRESSIVE, &bReturn, FALSE);
                if (FALSE == bReturn)
                {
                    MineDebug_PrintWarning("Getting movement aggressiveness int: %lu\n", GetLastError());
                    //Upon error getting value from edit control, use default value as backup
                    value = MINE_MOVEMENT_DEFAULT_AGGRESSIVENESS;
                }
                //Check bounds on movement aggressiveness value
                else if (1 > value)
                {
                    MineDebug_PrintWarning("Movement aggressiveness too small: %u\n", value);
                    value = 1;
                }
                else if (10 < value)
                {
                    MineDebug_PrintWarning("Movement aggressiveness too big: %u\n", value);
                    value = 10;
                }

                menuData.movementAggressive = (DWORD) value;

                //Store movement aggressiveness value in registry
                if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"MovementAggressiveness", menuData.movementAggressive))
                {
                    MineDebug_PrintWarning("In function Mine_SetRegDword\n");
                }

                //Set the flag that indicates movement is to occur
                menuData.useMovement = (DWORD) TRUE;

                if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"Movement", (DWORD) TRUE))
                {
                    MineDebug_PrintWarning("In function Mine_SetRegDword\n");
                }

                //Set a timer to perform the actual movement
                if (0 == SetTimer(hwnd, MINE_TIMER_MOVE, (UINT) menuData.movementFreq, NULL))
                {
                    MineDebug_PrintWarning("Creating movement timer: %lu\n", GetLastError());
                }
                else
                {
                    movementTimerCreated = TRUE;
                }
            }
            //Movement is disabled
            else
            {
                menuData.useMovement = (DWORD) FALSE;

                if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"Movement", (DWORD) FALSE))
                {
                    MineDebug_PrintWarning("In function Mine_SetRegDword\n");
                }

                //Stop the movement timer
                if (movementTimerCreated)
                {
                    if (0 == KillTimer(hwnd, MINE_TIMER_MOVE))
                    {
                        MineDebug_PrintWarning("Unable to kill movement timer: %lu\n", GetLastError());
                    }
                    movementTimerCreated = FALSE;
                }
            }

            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
        }
        /** Pressing cancel button makes no changes. */
        else if (IDCANCEL == LOWORD(wParam))
        {
            returnValue = (INT_PTR) TRUE;
            if (0 == EndDialog(hDlg, LOWORD(wParam)))
            {
                MineDebug_PrintWarning("Unable to end dialog window: %lu\n", GetLastError());
            }
        }
        /** Clicking the check mark either enables or disables movement. */
        else if ((IDC_MOVEMENT_CHECK == LOWORD(wParam)) && (BN_CLICKED == HIWORD(wParam)))
        {
            returnValue = (INT_PTR) TRUE;

            if (BST_CHECKED == Button_GetCheck(checkHwnd))
            {
                (void) Edit_Enable(aggressiveHwnd, TRUE);

                (void) Edit_Enable(freqHwnd, TRUE);
            }
            else
            {
                (void) Edit_Enable(aggressiveHwnd, FALSE);

                (void) Edit_Enable(freqHwnd, FALSE);
            }  
        }
        break;

    default:
        break;
    }

    return returnValue;
}

/**
    MineMovement_ProcessMovement
*//**
    Attempt, with specified aggressiveness, to move a mine from one tile to another.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineMovement_ProcessMovement(VOID)
{
    BOOLEAN    bFalse = FALSE;
    LONG       currentX = -1;
    LONG       currentY = -1;
    UINT       directionsToCheck = 0;
    BOOLEAN    finished = FALSE;
    HANDLE     hHeap = NULL;
    UINT       ix = 0;
    UINT       jx = 0;
    UINT       kx = 0;
    UINT       minesChecked = 0;
    UINT       minesToCheck = 0;
    BOOLEAN    mineWillMove = FALSE;
    LONG       newX = -1;
    LONG       newY = -1;
    UINT *     pMoveOrder = NULL;
    UINT *     pXOrder = NULL;
    UINT *     pYOrder = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    RECT       windowUpdate = {0};

    do
    {
        hHeap = GetProcessHeap();
        if (NULL == hHeap)
        {
            MineDebug_PrintError("Getting process heap: %lu\n", GetLastError());
            status = MINE_ERROR_HEAP;
            break;
        }

        /** The more aggressive, the more directions of movement will be checked. */
        switch (menuData.movementAggressive)
        {
        case 1:
        case 2:
            directionsToCheck = 2;
            break;
        case 3:
        case 4:
        case 5:
            directionsToCheck = 4;
            break;
        case 6:
        case 7:
        case 8:
            directionsToCheck = 6;
            break;
        case 9:
        case 10:
            directionsToCheck = 8;
            break;
        default:
            __assume(0);
            break;
        }

        /** The more aggressive, the more mines will try to move. */
        minesToCheck = ((UINT) gameData.mines) * ((UINT) menuData.movementAggressive) / 10;

        pXOrder = (UINT *) HeapAlloc(hHeap, 0, gameData.width*sizeof(UINT));
        if (NULL == pXOrder)
        {
            MineDebug_PrintError("Unable to allocate memory\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        pYOrder = (UINT *) HeapAlloc(hHeap, 0, gameData.height*sizeof(UINT));
        if (NULL == pYOrder)
        {
            MineDebug_PrintError("Unable to allocate memory\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        pMoveOrder = (UINT *) HeapAlloc(hHeap, 0, 8*sizeof(UINT));
        if (NULL == pMoveOrder)
        {
            MineDebug_PrintError("Unable to allocate memory\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        for (ix = 0; ix < gameData.width; ix++)
        {
            pXOrder[ix] = ix;
        }

        for (ix = 0; ix < gameData.height; ix++)
        {
            pYOrder[ix] = ix;
        }

        for (ix = 0; ix < 8; ix++)
        {
            pMoveOrder[ix] = ix;
        }

        /** Try to moves mines based on a random order of the X coordinate. */
        status = Mine_RandomPerm(pXOrder, (UINT) gameData.width, (UINT) gameData.width);
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_RandomPerm: %i\n", (int) status);
            break;
        }

        /** Try to move mines based on a random order of the Y coordinate. */
        status = Mine_RandomPerm(pYOrder, (UINT) gameData.height, (UINT) gameData.height);
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_RandomPerm: %i\n", (int) status);
            break;
        }

        status = Mine_RandomPerm(pMoveOrder, 8, directionsToCheck);
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_RandomPerm: %i\n", (int) status);
            break;
        }

        /** Check mines a random column at a time, randomly amongst each column. */
        //This might make it slightly more likely that a specific mine that is in a column
        //by itself will move versus a mine in a column with other mines. This slight
        //imbalance is not large enough to justify a more complicated randomness scheme.
        for (ix = 0; ix < gameData.width; ix++)
        {
            for (jx = 0; jx < gameData.height; jx++)
            {
                //Only check tiles that contain mines
                if (MINE_BOMB_VALUE != gameData.gameBoard[MINE_INDEX(pXOrder[ix], pYOrder[jx])])
                {
                    continue;
                }

                minesChecked++;

                //Only check tiles that are not flagged or currently held down
                if (MINE_TILE_STATUS_NORMAL != gameData.tileStatus[MINE_INDEX(pXOrder[ix], pYOrder[jx])])
                {
                    continue;
                }

                currentX = (LONG) pXOrder[ix];
                currentY = (LONG) pYOrder[jx];
                //Check if the current mine can move in one of the random directions.
                for (kx = 0; kx < directionsToCheck; kx++)
                {
                    switch (pMoveOrder[kx])
                    {
                    case 0:
                        newX = currentX - 1;
                        newY = currentY - 1;
                        break;
                    case 1:
                        newX = currentX;
                        newY = currentY - 1;
                        break;
                    case 2:
                        newX = currentX + 1;
                        newY = currentY - 1;
                        break;
                    case 3:
                        newX = currentX - 1;
                        newY = currentY;
                        break;
                    case 4:
                        newX = currentX + 1;
                        newY = currentY;
                        break;
                    case 5:
                        newX = currentX - 1;
                        newY = currentY + 1;
                        break;
                    case 6:
                        newX = currentX;
                        newY = currentY + 1;
                        break;
                    case 7:
                        newX = currentX + 1;
                        newY = currentY + 1;
                        break;
                    default:
                        __assume(0);
                        break;
                    }

                    //If wrapHorz flag is set, allow mine to move across horizontal boundary
                    if (newX < 0)
                    {
                        if (menuData.wrapHorz)
                        {
                            newX = (LONG) gameData.width - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (newX >= (LONG) gameData.width)
                    {
                        if (menuData.wrapHorz)
                        {
                            newX = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    //If wrap vert flag is set, allow mine to move across vertical boundary
                    if (newY < 0)
                    {
                        if (menuData.wrapVert)
                        {
                            newY = (LONG) gameData.height - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (newY >= (LONG) gameData.height)
                    {
                        if (menuData.wrapVert)
                        {
                            newY = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    /** Check if the mine can move to the tile in the randomly chosen direction. */
                    if ((MINE_BOMB_VALUE != gameData.gameBoard[MINE_INDEX(newX, newY)]) &&
                        (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(newX, newY)]))
                    {
                        mineWillMove = TRUE;
                        finished = TRUE;
                        break;
                    }

                    //Stop checking after limit has been reached
                    if (minesChecked >= minesToCheck)
                    {
                        finished = TRUE;
                        break;
                    }
                }
                if (finished)
                {
                    break;
                }
            }
            if (finished)
            {
                break;
            }
        }

        if (mineWillMove)
        {
            gameData.gameBoard[MINE_INDEX(currentX, currentY)] = 0;
            gameData.gameBoard[MINE_INDEX(newX, newY)] = MINE_BOMB_VALUE;

            /** If mine moved, update screen to reflect new tile numbers. */
            if ((menuData.wrapHorz && ((min(currentX - 1, newX - 1) < 0) || 
                                       (max(currentX + 1, newX + 1) > (LONG) gameData.width - 1 ))) ||
                (menuData.wrapVert && ((min(currentY - 1, newY - 1) < 0) || 
                                       (max(currentY + 1, newY + 1) > (LONG) gameData.height - 1))))
            {
                //In the case of wrapping and mines moving near borders, simplify by updating entire board
                status = Mine_AssignNumbers(0, (LONG) gameData.width - 1, 0, (LONG) gameData.height - 1);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function Mine_AssignNumbers: %i\n", (int) status);
                    break;
                }

                if (0 == InvalidateRect(hwnd, &(windowData.boardRegion), FALSE))
                {
                    MineDebug_PrintWarning("Unable to invalidate rectangle\n");
                }
            }
            else
            {
                //Otherwise, only update affected region
                status = Mine_AssignNumbers(max(min(currentX - 1, newX - 1), 0),
                                            min(max(currentX + 1, newX + 1), (LONG) gameData.width - 1),
                                            max(min(currentY - 1, newY - 1), 0),
                                            min(max(currentY + 1, newY + 1), (LONG) gameData.height - 1));
                
                windowUpdate.left = max(min(currentX - 1, newX - 1), 0) * MINE_TILE_PIXELS + 
                                    windowData.boardRegion.left;
                windowUpdate.right = min(max(currentX + 2, newX + 2), (LONG) gameData.width) * MINE_TILE_PIXELS +
                                     windowData.boardRegion.left;
                windowUpdate.top = max(min(currentY - 1, newY - 1), 0) * MINE_TILE_PIXELS +
                                   windowData.boardRegion.top;
                windowUpdate.bottom = min(max(currentY + 2, newY + 2), (LONG) gameData.height) * MINE_TILE_PIXELS +
                                      windowData.boardRegion.top;

                if (0 == InvalidateRect(hwnd, &windowUpdate, FALSE))
                {
                    MineDebug_PrintWarning("Unable to invalidate rectangle\n");
                }
            }
        }
    } while (bFalse);

    //Clean up
    if (NULL != hHeap)
    {
        if (NULL != pXOrder)
        {
            if (0 == HeapFree(hHeap, 0, pXOrder))
            {
                MineDebug_PrintWarning("Unable to free pXOrder: %lu\n", GetLastError());
            }
            pXOrder = NULL;
        }

        if (NULL != pYOrder)
        {
            if (0 != HeapFree(hHeap, 0, pYOrder))
            {
                MineDebug_PrintWarning("Unable to free pYOrder: %lu\n", GetLastError());
            }
            pYOrder = NULL;
        }

        if (NULL != pMoveOrder)
        {
            if (0 != HeapFree(hHeap, 0, pMoveOrder))
            {
                MineDebug_PrintWarning("Unable to free pMoveOrder: %lu\n", GetLastError());
            }
            pMoveOrder = NULL;
        }
    }

    return status;
}