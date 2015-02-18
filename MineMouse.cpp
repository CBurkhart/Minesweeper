/**
    @file MineMouse.cpp
    
    @author Craig Burkhart

    @brief Code to handle mouse movement and mouse button clicks.
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
#include "MineMouse.h"
#include "MineDebug.h"

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
MineMouse_FirstClick(LONG xGrid, LONG yGrid)
{
    BOOLEAN    bFalse = FALSE;
    BOOLEAN    needToPlace = FALSE;
    UINT       newXGrid = 0;
    UINT       newYGrid = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        /** If first click is a mine, move mine to a different random tile. */
        if (MINE_BOMB_VALUE == gameData.gameBoard[MINE_INDEX(xGrid, yGrid)])
        {
            gameData.gameBoard[MINE_INDEX(xGrid, yGrid)] = 0;
            needToPlace = TRUE;
        }
    
        while (needToPlace)
        {
            //Choose random tiles until one is not a mine 
            status = Mine_Random((UINT) gameData.width, &newXGrid);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_Random: %i\n", (int) status);
                break;
            }

            status = Mine_Random((UINT) gameData.height, &newYGrid);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_Random: %i\n", (int) status);
                break;
            }

            if ((MINE_BOMB_VALUE != gameData.gameBoard[MINE_INDEX(newXGrid, newYGrid)]) && 
                (MINE_INDEX(xGrid, yGrid) != MINE_INDEX(newXGrid, newYGrid)))
            {
                gameData.gameBoard[MINE_INDEX(newXGrid, newYGrid)] = MINE_BOMB_VALUE;
                needToPlace = FALSE;
            }
        }
        if (MINE_ERROR_SUCCESS != status)
        {
            break;
        }

        /** Determine the mine count for all non-mine tiles. */
        status = Mine_AssignNumbers(0, (LONG) (gameData.width - 1), 
                                    0, (LONG) (gameData.height - 1));
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_AssignNumbers: %i\n", (int) status);
            break;
        }

        gameData.gameStarted = TRUE;
        gameData.gameStartTime = GetTickCount64(); //Record game start time

        __assume(FALSE == bFalse);
    } while (bFalse);

    return status;
}

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
MineMouse_MoveDoubleClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    LONG       ix = 0;
    LONG       jx = 0;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = -1;
    LONG       xGridUpdate = 0;
    LONG       yGrid = -1;
    LONG       yGridUpdate = 0;

    do
    {
        //Convert from mouse coordinates to grid coordinates
        if (Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
            yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);
        }

        //If mouse hasn't moved to new tile, do nothing
        if ((gameData.prevGridX == xGrid) && (gameData.prevGridY == yGrid))
        {
            break;
        }

        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If mouse was previously on grid, unhighlight previously held tiles. */
        if ((-1 != gameData.prevGridX) && (-1 != gameData.prevGridY))
        {
            if (NULL == SelectObject(memoryDC, imageData.unclicked))
            {
                MineDebug_PrintError("Selecting unclicked into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            //All surrounding squares could have been highlighted in a double click
            for (ix = -1; ix <= 1; ix++)
            {
                xGridUpdate = gameData.prevGridX + ix;

                //Allow wrapping for x coordinates if in wrap horizontal mode
                if (xGridUpdate < 0)
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = (LONG) gameData.width - 1;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (xGridUpdate > (LONG) (gameData.width - 1))
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = 0;
                    }
                    else
                    {
                        continue;
                    }
                }
     
                for (jx = -1; jx <= 1; jx++)
                {
                    yGridUpdate = gameData.prevGridY + jx;

                    //Allow wrapping for y coordinates if in wrap vertical mode
                    if (yGridUpdate < 0)
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = (LONG) gameData.height - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (yGridUpdate > (LONG) (gameData.height - 1))
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    //Only change tiles in the HELD state
                    if (MINE_TILE_STATUS_HELD == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                    {
                        gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)] = MINE_TILE_STATUS_NORMAL;

                        if (0 == BitBlt(hDC, windowData.boardRegion.left+xGridUpdate*MINE_TILE_PIXELS,
                                        windowData.boardRegion.top+yGridUpdate*MINE_TILE_PIXELS,
                                        MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                        {
                            MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                            status = MINE_ERROR_PAINT;
                            break;
                        }
                    }
                }
                if (MINE_ERROR_SUCCESS != status)
                {
                    break;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }

            gameData.prevGridX = -1;
            gameData.prevGridY = -1;
        }

        /** If mouse is now on grid, highlight held tiles. */
        if (Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            if (NULL == SelectObject(memoryDC, imageData.held))
            {
                MineDebug_PrintError("Selecting held into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            for (ix = -1; ix <= 1; ix++)
            {
                xGridUpdate = xGrid + ix;

                /** Allow wrapping for x coordinates if in wrap horizontal mode. */
                if (xGridUpdate < 0)
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = (LONG) gameData.width - 1;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (xGridUpdate > (LONG) (gameData.width - 1))
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = 0;
                    }
                    else
                    {
                        continue;
                    }
                }

                for (jx = -1; jx <= 1; jx++)
                {
                    yGridUpdate = yGrid + jx;

                    /** Allow wrapping for y coordinates if in wrap vertical mode. */
                    if (yGridUpdate < 0)
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = (LONG) gameData.height - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (yGridUpdate > (LONG) (gameData.height - 1))
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    //Only change tiles in the NORMAL state
                    if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                    {
                        gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)] = MINE_TILE_STATUS_HELD;

                        if (0 == BitBlt(hDC, windowData.boardRegion.left+xGridUpdate*MINE_TILE_PIXELS,
                                        windowData.boardRegion.top+yGridUpdate*MINE_TILE_PIXELS, 
                                        MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                        {
                            MineDebug_PrintError("Copying held to screen: %lu\n", GetLastError());
                            status = MINE_ERROR_PAINT;
                            break;
                        }
                    }
                }
                if (MINE_ERROR_SUCCESS != status)
                {
                    break;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }

            gameData.prevGridX = xGrid;
            gameData.prevGridY = yGrid;
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release device context\n");
        }
        hDC = NULL;
    }

    return status;
}

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
MineMouse_MoveLeftClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = -1;
    LONG       yGrid = -1;

    do
    {
        //Convert from mouse coordinates to grid coordinates
        if (Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
            yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);
        }

        //If mouse hasn't moved to new tile, do nothing
        if ((gameData.prevGridX == xGrid) && (gameData.prevGridY == yGrid))
        {
            break;
        }

        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If mouse was previously on grid, unhighlight previously held tile. */
        if ((-1 != gameData.prevGridX) && (-1 != gameData.prevGridY))
        {
            //Only change a tile in the HELD state
            if (MINE_TILE_STATUS_HELD == gameData.tileStatus[MINE_INDEX(gameData.prevGridX, gameData.prevGridY)])
            {
                if (NULL == SelectObject(memoryDC, imageData.unclicked))
                {
                    MineDebug_PrintError("Selecting unclicked into memory DC\n");
                    status = MINE_ERROR_OBJECT;
                    break;
                }

                if (0 == BitBlt(hDC, windowData.boardRegion.left+gameData.prevGridX*MINE_TILE_PIXELS,
                                windowData.boardRegion.top+gameData.prevGridY*MINE_TILE_PIXELS, 
                                MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                {
                    MineDebug_PrintError("Copying unclicked to screen: %i\n", GetLastError());
                    status = MINE_ERROR_PAINT;
                    break;
                }

                gameData.tileStatus[MINE_INDEX(gameData.prevGridX, gameData.prevGridY)] = MINE_TILE_STATUS_NORMAL;
            }

            gameData.prevGridX = -1;
            gameData.prevGridY = -1;
        }

        /** If mouse is now on grid, highlight held tile. */
        if (Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            //Only change a tile in the NORMAL state
            if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
            {
                if (NULL == SelectObject(memoryDC, imageData.held))
                {
                    MineDebug_PrintError("Selecting held into memory DC\n");
                    status = MINE_ERROR_OBJECT;
                    break;
                }

                if (0 == BitBlt(hDC, windowData.boardRegion.left+xGrid*MINE_TILE_PIXELS,
                                windowData.boardRegion.top+yGrid*MINE_TILE_PIXELS, 
                                MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                {
                    MineDebug_PrintError("Copying held to screen: %i\n", GetLastError());
                    status = MINE_ERROR_PAINT;
                    break;
                }

                gameData.tileStatus[MINE_INDEX(xGrid, yGrid)] = MINE_TILE_STATUS_HELD;
            }

            gameData.prevGridX = xGrid;
            gameData.prevGridY = yGrid;
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release device context\n");
        }
        hDC = NULL;
    }

    return status;
}

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
MineMouse_ProcessDoubleClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    CHAR       boardNumber = 0;
    CHAR       flagCount = 0;
    HDC        hDC = NULL;
    LONG       ix = 0;
    LONG       jx = 0;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = 0;
    LONG       xGridUpdate = 0;
    LONG       yGrid = 0;
    LONG       yGridUpdate = 0;

    do
    {
        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If mouse was previously in board region, unhighlight held tiles. */
        if ((-1 != gameData.prevGridX) && (-1 != gameData.prevGridY))
        {
            if (NULL == SelectObject(memoryDC, imageData.unclicked))
            {
                MineDebug_PrintError("Selecting unclicked into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            for (ix = -1; ix <= 1; ix++)
            {
                //Allow wrapping for x coordinates if in wrap horizontal mode
                xGridUpdate = gameData.prevGridX + ix;

                if (xGridUpdate < 0)
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = (LONG) gameData.width - 1;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (xGridUpdate > (LONG) (gameData.width - 1))
                {
                    if(menuData.wrapHorz)
                    {
                        xGridUpdate = 0;
                    }
                    else
                    {
                        continue;
                    }
                }

                for (jx = -1; jx <= 1; jx++)
                {
                    //Allow wrapping for y coordinates if in wrap vertical mode
                    yGridUpdate = gameData.prevGridY + jx;

                    if (yGridUpdate < 0)
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = (LONG) gameData.height - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (yGridUpdate > (LONG) (gameData.height - 1))
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    //Unhighlight held tiles
                    if (MINE_TILE_STATUS_HELD == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                    {
                        gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)] = MINE_TILE_STATUS_NORMAL;

                        if (0 == BitBlt(hDC, windowData.boardRegion.left+xGridUpdate*MINE_TILE_PIXELS,
                                        windowData.boardRegion.top+yGridUpdate*MINE_TILE_PIXELS, 
                                        MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                        {
                            MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                            status = MINE_ERROR_PAINT;
                            break;
                        }
                    }
                }
                if (MINE_ERROR_SUCCESS != status)
                {
                    break;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }

            //Store that no tiles are currently being highlighted
            gameData.prevGridX = -1;
            gameData.prevGridY = -1;
        }

        xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
        yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);

        if (MINE_TILE_STATUS_REVEALED == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
        {
            boardNumber = gameData.gameBoard[MINE_INDEX(xGrid, yGrid)];

            for (ix = -1; ix <= 1; ix++)
            {
                /** Allow wrapping for x coordinates if in wrap horizontal mode. */
                xGridUpdate = xGrid + ix;

                if (xGridUpdate < 0)
                {
                    if (menuData.wrapHorz)
                    {
                        xGridUpdate = (LONG) gameData.width - 1;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (xGridUpdate > (LONG) (gameData.width - 1))
                {
                    if(menuData.wrapHorz)
                    {
                        xGridUpdate = 0;
                    }
                    else
                    {
                        continue;
                    }
                }

                for (jx = -1; jx <= 1; jx++)
                {
                    /** Allow wrapping for y coordinates if in wrap vertical mode. */
                    yGridUpdate = yGrid + jx;

                    if (yGridUpdate < 0)
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = (LONG) gameData.height - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (yGridUpdate > (LONG) (gameData.height - 1))
                    {
                        if (menuData.wrapVert)
                        {
                            yGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    /** Count all the flags surround the double clicked square. */
                    if (MINE_TILE_STATUS_FLAG == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                    {
                        flagCount++;
                    }
                }
            }

            /** If flag count equals number of mines, reveal remaining tiles. */
            if (flagCount == boardNumber)
            {
                //Allow wrapping for x coordinates if in wrap horizontal mode
                for (ix = -1; ix <= 1; ix++)
                {
                    xGridUpdate = xGrid + ix;

                    if (xGridUpdate < 0)
                    {
                        if (menuData.wrapHorz)
                        {
                            xGridUpdate = (LONG) gameData.width - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (xGridUpdate > (LONG) (gameData.width - 1))
                    {
                        if(menuData.wrapHorz)
                        {
                            xGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    for (jx = -1; jx <= 1; jx++)
                    {
                        //Allow wrapping for y coordinates if in wrap vertical mode
                        yGridUpdate = yGrid + jx;

                        if (yGridUpdate < 0)
                        {
                            if (menuData.wrapVert)
                            {
                                yGridUpdate = (LONG) gameData.height - 1;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else if (yGridUpdate > (LONG) (gameData.height - 1))
                        {
                            if (menuData.wrapVert)
                            {
                                yGridUpdate = 0;
                            }
                            else
                            {
                                continue;
                            }
                        }

                        //Uncover all unclicked tiles surrounding clicked tile
                        if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                        {
                            status = MineMouse_UncoverTile(xGridUpdate, yGridUpdate, hDC, memoryDC);
                            if (MINE_ERROR_SUCCESS != status)
                            {
                                MineDebug_PrintError("In function MineMouse_UncoverTile: %i\n", (int) status);
                                break;
                            }
                        }
                    }
                    if (MINE_ERROR_SUCCESS != status)
                    {
                        break;
                    }
                }
                if (MINE_ERROR_SUCCESS != status)
                {
                    break;
                }
            }
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release DC\n");
        }
        hDC = NULL;
    }
    
    return status;
}

/**
    MineMouse_ProcessLeftClick
*//**
    Process the release of a left click. Reveal if tile unclicked.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR 
MineMouse_ProcessLeftClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = 0;
    LONG       yGrid = 0;

    do
    {
        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If mouse was previously in board region, unhighlight previously held tile. */
        if ((-1 != gameData.prevGridX) && (-1 != gameData.prevGridY))
        {
            if (MINE_TILE_STATUS_HELD == gameData.tileStatus[MINE_INDEX(gameData.prevGridX, gameData.prevGridY)])
            {
                if (NULL == SelectObject(memoryDC, imageData.unclicked))
                {
                    MineDebug_PrintError("Selecting unclicked into memory DC\n");
                    status = MINE_ERROR_OBJECT;
                    break;
                }

                if (0 == BitBlt(hDC, windowData.boardRegion.left+gameData.prevGridX*MINE_TILE_PIXELS,
                                windowData.boardRegion.top+gameData.prevGridY*MINE_TILE_PIXELS, 
                                MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                {
                    MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                    status = MINE_ERROR_PAINT;
                    break;
                }

                gameData.tileStatus[MINE_INDEX(gameData.prevGridX, gameData.prevGridY)] = MINE_TILE_STATUS_NORMAL;
            }

            //Store that no tiles are currently being highlighted
            gameData.prevGridX = -1;
            gameData.prevGridY = -1;
        }

        //Convert from mouse coordinates to grid coordinates
        xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
        yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);

        //Check if this is the first left click of the game
        if (!gameData.gameStarted)
        {
            MineMouse_FirstClick(xGrid, yGrid);
        }

        /** Reveal the tile if the tile has not been clicked. */
        if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
        {
            status = MineMouse_UncoverTile(xGrid, yGrid, hDC, memoryDC);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function MineMouse_UncoverTile: %i\n", (int) status);
                break;
            }
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release DC\n");
        }
        hDC = NULL;
    }

    return status;
}

/**
    MineMouse_ProcessRightDown
*//**
    Process the initial press of a right click. Flags or un-flags a tile.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR 
MineMouse_ProcessRightDown(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = 0;
    LONG       yGrid = 0;

    do
    {
        //Convert from mouse coordinates to grid coordinates
        xGrid = (((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS;
        yGrid = (((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS;

        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If tile is unclicked, place a flag. */
        if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
        {
            gameData.tileStatus[MINE_INDEX(xGrid, yGrid)] = MINE_TILE_STATUS_FLAG;
            gameData.numFlagged += 1;

            if (NULL == SelectObject(memoryDC, imageData.flag))
            {
                MineDebug_PrintError("Selecting flag into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.boardRegion.left + xGrid * MINE_TILE_PIXELS,
                            windowData.boardRegion.top + yGrid * MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying flag to screen: %lu\n", GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            if (0 == InvalidateRect(hwnd, &(windowData.mineCountRegion), FALSE))
            {
                MineDebug_PrintWarning("Unable to invalidate rectangle\n");
            }
        }
        /** If tile already has a flag, remove the flag. */
        else if (MINE_TILE_STATUS_FLAG == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
        {
            gameData.tileStatus[MINE_INDEX(xGrid, yGrid)] = MINE_TILE_STATUS_NORMAL;
            gameData.numFlagged -= 1;

            if (NULL == SelectObject(memoryDC, imageData.unclicked))
            {
                MineDebug_PrintError("Selecting unclicked into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.boardRegion.left + xGrid * MINE_TILE_PIXELS,
                            windowData.boardRegion.top + yGrid * MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            if (0 == InvalidateRect(hwnd, &(windowData.mineCountRegion), FALSE))
            {
                MineDebug_PrintWarning("Unable to invalidate rectangle\n");
            }
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release DC\n");
        }
        hDC = NULL;
    }

    return status;
}

/**
    MineMouse_StartDoubleClick
*//**
    Start a new double click by showing which new tiles are held down.
    
    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR 
MineMouse_StartDoubleClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    LONG       ix = 0;
    LONG       jx = 0;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = 0;
    LONG       xGridUpdate = 0;
    LONG       yGrid = 0;
    LONG       yGridUpdate = 0;

    do
    {
        //Handle all graphics in this function
        hDC = GetDC(hwnd);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Obtaining device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating compatible memory DC\n");
            status = MINE_ERROR_DC;
            break;
        }

        //The only new image we need is the tile held image
        if (NULL == SelectObject(memoryDC, imageData.held))
        {
            MineDebug_PrintError("Selecting held into memory DC\n");
            status = MINE_ERROR_OBJECT;
            break;
        }

        //Convert from mouse coordinates to grid coordinates
        xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
        yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);

        /** Allow wrapping for x coordinates if in wrap horizontal mode. */
        for (ix = -1; ix <= 1; ix++)
        {
            xGridUpdate = xGrid + ix;

            if (xGridUpdate < 0)
            {
                if (menuData.wrapHorz)
                {
                    xGridUpdate = (LONG) gameData.width - 1;
                }
                else
                {
                    continue;
                }
            }
            else if (xGridUpdate > (LONG) (gameData.width - 1))
            {
                if (menuData.wrapHorz)
                {
                    xGridUpdate = 0;
                }
                else
                {
                    continue;
                }
            }

            /** Allow wrapping for y coordinates if in wrap vertical mode. */
            for (jx = -1; jx <= 1; jx++)
            {
                yGridUpdate = yGrid + jx;

                if (yGridUpdate < 0)
                {
                    if (menuData.wrapVert)
                    {
                        yGridUpdate = (LONG) gameData.height - 1;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (yGridUpdate > (LONG) (gameData.height - 1))
                {
                    if (menuData.wrapVert)
                    {
                        yGridUpdate = 0;
                    }
                    else
                    {
                        continue;
                    }
                }

                /** Highlight newly held tiles. */
                if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                {
                    if (0 == BitBlt(hDC, windowData.boardRegion.left+xGridUpdate*MINE_TILE_PIXELS,
                                    windowData.boardRegion.top+yGridUpdate*MINE_TILE_PIXELS, 
                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                    {
                        MineDebug_PrintError("Copying held to screen: %lu\n", GetLastError());
                        status = MINE_ERROR_PAINT;
                        break;
                    }

                    gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)] = MINE_TILE_STATUS_HELD;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }
        }
        if (MINE_ERROR_SUCCESS != status)
        {
            break;
        }

        //Store location of mouse so highlighted tiles can be unhighlighted later
        gameData.prevGridX = xGrid;
        gameData.prevGridY = yGrid;

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up

    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release DC\n");
        }
        hDC = NULL;
    }

    return status;
}

/**
    MineMouse_StartLeftClick
*//**
    Start a new left click by showing which new tile is held down.

    @param[in] xMouse - X coordinate of mouse in client area pixels.
    @param[in] yMouse - Y coordinate of mouse in client area pixels.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR 
MineMouse_StartLeftClick(short xMouse, short yMouse)
{
    BOOLEAN    bFalse = FALSE;
    HDC        hDC = NULL;
    HDC        memoryDC = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGrid = 0;
    LONG       yGrid = 0;

    do
    {
        //Convert from mouse coordinates to grid coordinates
        xGrid = ((((LONG) xMouse) - windowData.boardRegion.left) / MINE_TILE_PIXELS);
        yGrid = ((((LONG) yMouse) - windowData.boardRegion.top) / MINE_TILE_PIXELS);

        if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGrid, yGrid)])
        {
            //Handle all graphics in this function
            hDC = GetDC(hwnd);
            if (NULL == hDC)
            {
                MineDebug_PrintError("Obtaining device context\n");
                status = MINE_ERROR_DC;
                break;
            }

            memoryDC = CreateCompatibleDC(hDC);
            if (NULL == memoryDC)
            {
                MineDebug_PrintError("Creating compatible memory DC\n");
                status = MINE_ERROR_DC;
                break;
            }

            if (NULL == SelectObject(memoryDC, imageData.held))
            {
                MineDebug_PrintError("Selecting held into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            /** Highlight newly held tile. */
            if (0 == BitBlt(hDC, windowData.boardRegion.left+xGrid*MINE_TILE_PIXELS,
                            windowData.boardRegion.top+yGrid*MINE_TILE_PIXELS, 
                            MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying held to screen: %lu\n", GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            gameData.tileStatus[MINE_INDEX(xGrid, yGrid)] = MINE_TILE_STATUS_HELD;

            //Store location of mouse so highlighted tiles can be unhighlighted later
            gameData.prevGridX = xGrid;
            gameData.prevGridY = yGrid;
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    //Clean up
    if (NULL != memoryDC)
    {
        if (0 == DeleteDC(memoryDC))
        {
            MineDebug_PrintWarning("Unable to delete memory device context\n");
        }
        memoryDC = NULL;
    }

    if (NULL != hDC)
    {
        if (0 == ReleaseDC(hwnd, hDC))
        {
            MineDebug_PrintWarning("Unable to release DC\n");
        }
        hDC = NULL;
    }

    return status;
}

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
MineMouse_UncoverTile(LONG xGrid, LONG yGrid, _In_ HDC hDC, _In_ HDC hMemDC)
{
    BOOLEAN    bFalse = FALSE;
    CHAR       boardNumber = 0;
    LONG       ix = 0;
    LONG       jx = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGridUpdate = 0;
    LONG       yGridUpdate = 0;

    do
    {
        if (NULL == hDC)
        {
            MineDebug_PrintError("Partameter hDC is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        if (NULL == hMemDC)
        {
            MineDebug_PrintError("Parameter memDC is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        /** Determine the number to be displayed. */
        boardNumber = gameData.gameBoard[MINE_INDEX(xGrid, yGrid)];

        //Set tile status to revealed
        gameData.tileStatus[MINE_INDEX(xGrid, yGrid)] = MINE_TILE_STATUS_REVEALED;

        /** If a mine was revealed, the game was lost. */
        if (MINE_BOMB_VALUE == boardNumber)
        {
            gameData.gameOver = TRUE;
            gameData.gameWon = FALSE;
            if (0 == InvalidateRect(hwnd, NULL, FALSE))
            {
                MineDebug_PrintWarning("Unable to invalidate rectangle\n");
            }
        }
        else
        {
            //Increment a counter which holds the number of tile uncovered thus far
            gameData.numUncovered++;

            /* Display the number image for the newly revealed tile. */
            if (NULL == SelectObject(hMemDC, imageData.numbers[boardNumber]))
            {
                MineDebug_PrintError("Selecting numbers %i into memory DC\n", (int) boardNumber);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.boardRegion.left+xGrid*MINE_TILE_PIXELS,
                            windowData.boardRegion.top+yGrid*MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                            MINE_TILE_PIXELS, hMemDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying numbers %i to screen: %lu\n", (int) boardNumber, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            /** If the tile has zero surrounding mines, reveal all surrounding tiles as well. */
            if (0 == boardNumber)
            {
                /** Allow wrapping for x coordinates if in wrap horizontal mode. */
                for (ix = -1; ix <= 1; ix++)
                {
                    xGridUpdate = xGrid + ix;

                    if (xGridUpdate < 0)
                    {
                        if (menuData.wrapHorz)
                        {
                            xGridUpdate = (LONG) gameData.width - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (xGridUpdate > (LONG) (gameData.width - 1))
                    {
                        if (menuData.wrapHorz)
                        {
                            xGridUpdate = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    /** Allow wrapping for y coordinates if in wrap vertical mode. */
                    for (jx = -1; jx <= 1; jx++)
                    {
                        yGridUpdate = yGrid + jx;

                        if (yGridUpdate < 0)
                        {
                            if (menuData.wrapVert)
                            {
                                yGridUpdate = (LONG) gameData.height - 1;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else if (yGridUpdate > (LONG) (gameData.height - 1))
                        {
                            if (menuData.wrapVert)
                            {
                                yGridUpdate = 0;
                            }
                            else
                            {
                                continue;
                            }
                        }

                        if (MINE_TILE_STATUS_NORMAL == gameData.tileStatus[MINE_INDEX(xGridUpdate, yGridUpdate)])
                        {
                            //Recursive call to uncover surrounding tiles
                            status = MineMouse_UncoverTile(xGridUpdate, yGridUpdate, hDC, hMemDC);
                            if (MINE_ERROR_SUCCESS != status)
                            {
                                MineDebug_PrintError("In function MineMouse_UncoverTile: %i\n", (int) status);
                                break;
                            }
                        }
                    }
                    if (MINE_ERROR_SUCCESS != status)
                    {
                        break;
                    }
                }
                if (MINE_ERROR_SUCCESS != status)
                {
                    break;
                }
            }
        }

        /** If all non-mine tiles have been uncovered, the game has been won. */
        if (gameData.height*gameData.width-gameData.mines == gameData.numUncovered)
        {
            //This is a hack. When a dialog is created in Mine_GameWon a WM_PAINT message
            //gets sent to the window procedure without this function returning first. This
            //causes a second memory device context to be created while this memory device 
            //context still exists. Since a bitmap can only be loaded in one device context
            //at a time, we load a bitmap that the other device context will not need.
            if (NULL == SelectObject(hMemDC, imageData.mineHit))
            {
                MineDebug_PrintError("Selecting mineHit into memory DC\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            Mine_GameWon();
        }

        __assume(FALSE == bFalse);
    } while (bFalse);

    return status;
}