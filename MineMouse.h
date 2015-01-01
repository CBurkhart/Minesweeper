/**
    @brief MineMouse.h

    @author Craig Burkhart

    @brief Header file for mouse input functionality.
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
    MineMouse_FirstClick
*//**
    Start the game after the first left click. Moves a clicked on mine,
    assigns the numbers, and records game start time.

    @param[in] xGrid - X grid coordinate for first click.
    @param[in] yGrid - Y grid coordinate for first click.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_FirstClick(LONG xGrid, LONG yGrid);

/**
    MineMouse_MoveDoubleClick
*//**
    Move the tiles that are shown as being held when the mouse moves 
    during a double click.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_MoveDoubleClick(short xMouse, short yMouse);

/**
    MineMouse_MoveLeftClick
*//**
    Move the tile that is shown as being held when the mouse moves 
    during a left click.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_MoveLeftClick(short xMouse, short yMouse);

/**
    MineMouse_ProcessDoubleClick
*//**
    Process the release of a double click. If number of flags around clicked
    tile equals number of mines, reveal the surrounding tiles.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineMouse_ProcessDoubleClick(short xMouse, short yMouse);

/**
    MineMouse_ProcessLeftClick
*//**
    Process the release of a left click. Reveal if tile unclicked.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineMouse_ProcessLeftClick(short xMouse, short yMouse);

/**
    MineMouse_ProcessRightDown
*//**
    Process the initial press of a right click. Flags or un-flags a tile.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
MineMouse_ProcessRightDown(short xMouse, short yMouse);

/**
    MineMouse_StartDoubleClick
*//**
    Start a new double click by showing which new tiles are held down.
    
    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_StartDoubleClick(short xMouse, short yMouse);

/**
    MineMouse_StartLeftClick
*//**
    Start a new left click by showing which new tile is held down.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_StartLeftClick(short xMouse, short yMouse);

/**
    MineMouse_UncoverTile
*//**
    Reveal a tile and determine if the game has been either won or lost. 

    @param[in] xGrid  - X coordinate of tile in grid coordinates.
    @param[in] yGrid  - Y coordinate of tile in grid coordinates.
    @param[in] hDC    - Handle to device context for main window.
    @param[in] hMemDC - Handle to memory device context to hold bitmap
                        before it is transfered to screen.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
MineMouse_UncoverTile(LONG xGrid, LONG yGrid, _In_ HDC hDC, _In_ HDC memDC);