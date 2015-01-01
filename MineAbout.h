/** 
    @file MineAbout.h

    @author Craig Burkhart

    @brief Header file for the about dialog window.
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
    MineAbout_Dialog
*//**
    Dialog procedure for the about dialog.

    @param[in] hDlg    - Handle to about dialog window.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return TRUE if message was processed, FALSE to have system process message.
*/
INT_PTR CALLBACK
MineAbout_Dialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

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
MineAbout_LicenseDialog(_In_ HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);