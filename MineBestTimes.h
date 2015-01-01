/**
    @file MineBestTimes.h

    @author Craig Burkhart

    @brief Header file for the "Fastest Mine Sweepers" dialog window.
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
#pragma once

#include "Mine.h"

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
MineBestTimes_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/**
    MineBestTimes_ResetScores
*//**
    Reset all of the high scores and names back to default values.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineBestTimes_ResetScores(VOID);

/**
    MineBestTimes_UpdateLabels
*//**
    Update the text on the best times dialog to show names and times.

    @param[in] hDlg - Handle to the best times dialog window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineBestTimes_UpdateLabels(_In_ HWND hDlg);