/**
    @file Mine.cpp

    @author Craig Burkhart

    @brief Main code for Minesweeper Deluxe project.
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
#include "MineBestTimes.h"
#include "MineDebug.h"
#include "MineMouse.h"
#include "MineNewBest.h"
#include "MineCustom.h"
#include "MineMovement.h"

//Needed to link against proper version of comctl32.lib
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global Variables:
BOOLEAN              clockTimerCreated = FALSE;
MINE_GAME_SETTINGS   gameData = {0};
HCRYPTPROV           hCrypto = NULL;
HINSTANCE            hInst = NULL;
HWND                 hwnd = NULL;
MINE_IMAGE_STORAGE   imageData = {0};
MINE_GLOBAL_SETTINGS menuData = {0};
BOOLEAN              movementTimerCreated = FALSE;
WCHAR                szTitle[MINE_LOADSTRING_MAX_CHARS] = {0};
WCHAR                szWindowClass[MINE_LOADSTRING_MAX_CHARS] = {0};
MINE_WINDOW_SETTINGS windowData = {0};

/**
    wWinMain
*//**
    Main function. Creates window and runs message loop.

    @param[in] hInstance     - Handle to process.
    @param[in] hPrevInstance - Not used.
    @param[in] lpCmdLine     - Command line arguments (not checked).
    @param[in] nCmdShow      - How window should be shown.

    @return Mine error code cast to an int (MINE_ERROR_SUCCESS on success).
*/
int APIENTRY
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
         _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    BOOLEAN              bFalse = FALSE;
    HACCEL               hAccelTable;
    INITCOMMONCONTROLSEX icc;
    BOOL                 messageReturn = 1;
    MSG                  msg;
    MINE_ERROR           status = MINE_ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    do
    {
        hInst = hInstance;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_LINK_CLASS;

        if (FALSE == InitCommonControlsEx(&icc))
        {
            status = MINE_ERROR_CONTROL;
            MineDebug_PrintError("Unable to load common controls\n");
            break;
        }

        //Setup initial game state in global variables
        status = Mine_SetupGlobal();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_SetupGlobal: %i\n", (int) status);
            break;
        }

        status = Mine_SetupImageData();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_SetupImageData: %i\n", (int) status);
            break;
        }

        status = Mine_SetupGame();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_SetupGame: %i\n", (int) status);
            break;
        }

        Mine_SetupWindow();

        //Load strings defined in the resource file
        if (0 == LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MINE_LOADSTRING_MAX_CHARS))
        {
            MineDebug_PrintError("Loading title string: %lu\n", GetLastError());
            status = MINE_ERROR_STRING;
            break;
        }

        if (0 == LoadStringW(hInstance, IDC_MINESWEEPER, szWindowClass, MINE_LOADSTRING_MAX_CHARS))
        {
            MineDebug_PrintError("Loading window class string: %lu\n", GetLastError());
            status = MINE_ERROR_STRING;
            break;
        }

        /** Register the class for the main window. */
        status = Mine_RegisterClass();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_RegisterClass: %i\n", (int) status);
            break;
        }

        /** Create the main window. */
        status = Mine_InitInstance(nCmdShow);
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_InitInstance: %i\n", (int) status);
            break;
        }  

        /** Create the timer that updates the clock. */
        if (0 == SetTimer(hwnd, MINE_TIMER_CLOCK, MINE_CLOCK_UPDATE_TIME, NULL))
        {
            MineDebug_PrintWarning("Creating clock timer: %lu\n", GetLastError());
        }
        else
        {
            clockTimerCreated = TRUE;
        }

        if (menuData.useMovement)
        {
            /** Create the timer that signals mine movement if needed. */
            if (0 == SetTimer(hwnd, MINE_TIMER_MOVE, (UINT) menuData.movementFreq, NULL))
            {
                MineDebug_PrintWarning("Creating movement timer: %lu\n", GetLastError());
            }
            else
            {
                movementTimerCreated = TRUE;
            }
        }

        hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDA_ACCELERATORS));
        if (NULL == hAccelTable)
        {
            MineDebug_PrintError("Loading accelerator table: %lu\n", GetLastError());
            status = MINE_ERROR_ACCELERATORS;
            DestroyWindow(hwnd);
            break;
        }

        //Message loop
        /** Pass messages to the main window. */
        while (0 != (messageReturn = GetMessageW(&msg, NULL, 0, 0)))
        {
            if (-1 == messageReturn)
            {
                MineDebug_PrintError("Getting message: %lu\n", GetLastError());
                status = MINE_ERROR_MESSAGE;
                DestroyWindow(hwnd);
                break;
            }
            else
            {
                if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
                {
                    (void) TranslateMessage(&msg);
                    (void) DispatchMessageW(&msg);
                }
            }
        }
        if (MINE_ERROR_SUCCESS != status)
        {
            break;
        }

        //Quit message is set to status that was kept in window procedure 
        status = (MINE_ERROR) msg.wParam;
    } while (bFalse);

    //Clean up
    Mine_Cleanup();

    return ((int) status);
}

/** 
    Mine_AssignNumbers
*//**
    Count the number of mines surrounding the tiles in a region of the board.

    @param[in] xGridMin - Minimum x coordinate for board region.
    @param[in] xGridMax - Maximum x coordinate for board region.
    @param[in] yGridMin - Minimum y coordinate for board region.
    @param[in] yGridMax - Maximum y coordinate for board region.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
Mine_AssignNumbers(LONG xGridMin, LONG xGridMax, LONG yGridMin, LONG yGridMax)
{
    BOOLEAN    bFalse = FALSE;
    INT        ix = 0;
    INT        jx = 0;
    CHAR       mines = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    LONG       xGridBase = 0;
    LONG       xGridPos = 0;
    LONG       yGridBase = 0;
    LONG       yGridPos = 0;

    do
    {
        xGridMin = max(0, min(xGridMin, (LONG) gameData.width - 1));
        xGridMax = max(0, min(xGridMax, (LONG) gameData.width - 1));
        yGridMin = max(0, min(yGridMin, (LONG) gameData.height - 1));
        yGridMax = max(0, min(yGridMax, (LONG) gameData.height - 1));

        if ((xGridMin > xGridMax) || (yGridMin > yGridMax))
        {
            status = MINE_ERROR_PARAMETER;
            MineDebug_PrintError("Grid input min is greater than max\n");
            break;
        }

        for (xGridBase = xGridMin; xGridBase <= xGridMax; xGridBase++)
        {
            for (yGridBase = yGridMin; yGridBase <= yGridMax; yGridBase++)
            {
                //Tiles that are bombs do not get a number
                if (MINE_BOMB_VALUE == gameData.gameBoard[MINE_INDEX(xGridBase, yGridBase)])
                {
                    continue;
                }

                mines = 0;

                /** For each non-mine tile, count number of mines in surrounding tiles. */

                /** Allow wrapping for x coordinates if in wrap horizontal mode. */
                for (ix = -1; ix <= 1; ix++)
                {
                    xGridPos = xGridBase + ix;

                    if (xGridPos < 0)
                    {
                        if (menuData.wrapHorz)
                        {
                            xGridPos = (LONG) gameData.width - 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if  (xGridPos > ((LONG) (gameData.width - 1)))
                    {
                        if (menuData.wrapHorz)
                        {
                            xGridPos = 0;
                        }
                        else
                        {
                            continue;
                        }
                    }
                
                    /** Allow wrapping for y coordinate if in wrap vertical mode. */
                    for (jx = -1; jx <= 1; jx++)
                    {
                        //Skip the check for the current tile itself
                        if ((0 == ix) && (0 == jx))
                        {
                            continue;
                        }

                        yGridPos = yGridBase + jx;

                        if (yGridPos < 0)
                        {
                            if (menuData.wrapVert)
                            {
                                yGridPos = (LONG) gameData.height - 1;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else if (yGridPos > (LONG) (gameData.height - 1))
                        {
                            if (menuData.wrapVert)
                            {
                                yGridPos = 0;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        
                        //Check for a mine
                        if (MINE_BOMB_VALUE == gameData.gameBoard[MINE_INDEX(xGridPos, yGridPos)])
                        {
                            mines += 1;
                        }
                    }
                }

                gameData.gameBoard[MINE_INDEX(xGridBase, yGridBase)] = mines;
            }
        }
    } while (bFalse);

    return status;
}

/**
    Mine_Cleanup
*//**
    Free all memory and delete all objects remaining at program close.
*/
VOID
Mine_Cleanup(VOID)
{
    HANDLE hHeap = NULL;
    INT    ix = 0;

    hHeap = GetProcessHeap();
    if (NULL == hHeap)
    {
        MineDebug_PrintWarning("Getting process heap: %lu\n", GetLastError());
    }
    else
    {
        /** Free allocated memory. */
        if(NULL != menuData.beginnerName)
        {
            if (0 == HeapFree(hHeap, 0, menuData.beginnerName))
            {
                MineDebug_PrintWarning("Unable to free beginner name: %lu\n", GetLastError());
            }
            menuData.beginnerName = NULL;
        }

        if(NULL != menuData.intermediateName)
        {
            if (0 == HeapFree(hHeap, 0, menuData.intermediateName))
            {
                MineDebug_PrintWarning("Unable to free intermediate name: %lu\n", GetLastError());
            }
            menuData.intermediateName = NULL;
        }

        if(NULL != menuData.expertName)
        {
            if (0 == HeapFree(hHeap, 0, menuData.expertName))
            {
                MineDebug_PrintWarning("Unable to free expert name: %lu\n", GetLastError());
            }
            menuData.expertName = NULL;
        }

        if (NULL != gameData.gameBoard)
        {
            if (0 == HeapFree(hHeap, 0, gameData.gameBoard))
            {
                MineDebug_PrintWarning("Unable to free game board: %lu\n", GetLastError());
            }
            gameData.gameBoard = NULL;
        }

        if (NULL != gameData.tileStatus)
        {
            if (0 == HeapFree(hHeap, 0, gameData.tileStatus))
            {
                MineDebug_PrintWarning("Unable to free tile status: %lu\n", GetLastError());
            }
            gameData.tileStatus = NULL;
        }
    }

    /** Delete stored image objects. */
    for (ix = 0; ix < 10; ix++)
    {
        if (NULL != imageData.timer[ix])
        {
            if (0 == DeleteObject((HGDIOBJ) imageData.timer[ix]))
            {
                MineDebug_PrintWarning("Unable to delete timer %i object\n", ix);
            }
            imageData.timer[ix] = NULL;
        }
    }

    if (NULL != imageData.timerDash)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.timerDash))
        {
            MineDebug_PrintWarning("Unable to delete timer dash object\n");
        }
        imageData.timerDash = NULL;
    }

    if (NULL != imageData.faceNormal)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.faceNormal))
        {
            MineDebug_PrintWarning("Unable to delete face normal object\n");
        }
        imageData.faceNormal = NULL;
    }

    if (NULL != imageData.faceClicked)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.faceClicked))
        {
            MineDebug_PrintWarning("Unable to delete face clicked object\n");
        }
        imageData.faceClicked = NULL;
    }

    if (NULL != imageData.faceWon)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.faceWon))
        {
            MineDebug_PrintWarning("Unable to delete face won object\n");
        }
        imageData.faceWon = NULL;
    }

    if (NULL != imageData.faceLost)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.faceLost))
        {
            MineDebug_PrintWarning("Unable to delete face lost object\n");
        }
        imageData.faceLost = NULL;
    }

    for (ix = 0; ix < 9; ix++)
    {
        if (NULL != imageData.numbers[ix])
        {
            if (0 == DeleteObject((HGDIOBJ) imageData.numbers[ix]))
            {
                MineDebug_PrintWarning("Unable to delete number %i object\n", ix);
            }
            imageData.numbers[ix] = NULL;
        }
    }

    if (NULL != imageData.unclicked)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.unclicked))
        {
            MineDebug_PrintWarning("Unable to delete unclicked object\n");
        }
        imageData.unclicked = NULL;
    }

    if (NULL != imageData.held)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.held))
        {
            MineDebug_PrintWarning("Unable to delete held object\n");
        }
        imageData.held = NULL;
    }

    if (NULL != imageData.flag)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.flag))
        {
            MineDebug_PrintWarning("Unable to delete flag object\n");
        }
        imageData.flag = NULL;
    }

    if (NULL != imageData.falseFlag)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.falseFlag))
        {
            MineDebug_PrintWarning("Unable to delete false flag object\n");
        }
        imageData.falseFlag = NULL;
    }

    if (NULL != imageData.mine)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.mine))
        {
            MineDebug_PrintWarning("Unable to delete mine object\n");
        }
        imageData.mine = NULL;
    }

    if (NULL != imageData.mineHit)
    {
        if (0 == DeleteObject((HGDIOBJ) imageData.mineHit))
        {
            MineDebug_PrintWarning("Unable to delete mine hit object\n");
        }
        imageData.mineHit = NULL;
    }

    if (NULL != hCrypto)
    {
        if (0 == CryptReleaseContext(hCrypto, 0))
        {
            MineDebug_PrintWarning("Unable to release crypt context: %lu\n", GetLastError());
        }
        hCrypto = NULL;
    }

    return;
}

/**
    Mine_DoRectOverlap
*//**
    Determine if two rectangles have any overlap. Mainly used to see
    if an invalid rectangle includes parts of a region of the window.

    @param[in] pRect1 - Pointer to a RECT.
    @param[in] pRect2 - Pointer to a RECT.

    @return TRUE if rectangles overlap. FALSE otherwise.
*/
BOOLEAN 
Mine_DoRectOverlap(_In_ PRECT pRect1, _In_ PRECT pRect2)
{
    BOOLEAN returnValue = FALSE;
    RECT    temp = {0}; //Windows API requires temp RECT to hold actual intersection

    if ((NULL == pRect1) || (NULL == pRect2))
    {
        MineDebug_PrintWarning("Input parameter to Mine_DoRectOverlap is NULL\n");
    }
    else
    {
        returnValue = (IntersectRect(&temp, pRect1, pRect2) ? (BOOLEAN) TRUE : (BOOLEAN) FALSE);
    }

    return returnValue;
}

/**
    Mine_GameWon
*//**
    Process a win of the game.
*/
VOID
Mine_GameWon(VOID)
{
    BOOLEAN    bFalse = FALSE;
    INT_PTR    dialogReturn = 0;

    do
    {
        /** Timer is updated for final game time. */
        Mine_ProcessTimer();
        //All mines will be set to flags, update remaining mine counter to match
        gameData.numFlagged = gameData.mines;
        gameData.gameOver = TRUE;
        gameData.gameWon = TRUE;
        if (0 == InvalidateRect(hwnd, NULL, FALSE))
        {
            MineDebug_PrintWarning("Unable to invalidate rectangle\n");
        }

        /** Check if the time is a new record for standard board sizes. */
        if (((MINE_LEVEL_BEGINNER     == menuData.gameLevel) && (gameData.time < menuData.beginnerTime))     ||
            ((MINE_LEVEL_INTERMEDIATE == menuData.gameLevel) && (gameData.time < menuData.intermediateTime)) ||
            ((MINE_LEVEL_EXPERT       == menuData.gameLevel) && (gameData.time < menuData.expertTime)))
        {
            //Display dialog to get name of record setter
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_NEWBEST), hwnd, MineNewBest_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show new best time dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show new best time dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }

            //Display dialog of record holders
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_BESTTIMES), hwnd, MineBestTimes_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show best times dialog: %lu\n", GetLastError());
                break;
            }
        }
    } while (bFalse);

    return;
}

/**
    Mine_InitInstance
*//**
    Create and display an instance of the main window.

    @param[in] nCmdShow - How window should be shown.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR 
Mine_InitInstance(int nCmdShow)
{
   BOOLEAN    bFalse = FALSE;
   MINE_ERROR status = MINE_ERROR_SUCCESS;
   RECT       windowSize = {0};

   do
   {
       windowSize.left = 0;
       windowSize.right = (LONG) windowData.clientWidth;
       windowSize.top = 0;
       windowSize.bottom = (LONG) windowData.clientHeight;

       //Convert from client area size to window area size
       if (0 == AdjustWindowRect(&windowSize, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, TRUE))
       {
           MineDebug_PrintError("Calculating window size from client size: %lu\n", GetLastError());
           status = MINE_ERROR_CREATE_WINDOW;
           break;
       }

       /** Create the main window. */
        hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                             CW_USEDEFAULT, 0, windowSize.right-windowSize.left,
                             windowSize.bottom-windowSize.top, NULL, NULL, hInst, NULL);
        if (NULL == hwnd)
        {
            MineDebug_PrintError("Creating window: %lu\n", GetLastError());
            status = MINE_ERROR_CREATE_WINDOW;
            break;
        }

        /** Make the window visible. */
        (void) ShowWindow(hwnd, nCmdShow);        
        if (0 == UpdateWindow(hwnd))
        {
            MineDebug_PrintError("Updating window\n");
            status = MINE_ERROR_PAINT;
            if (0 == DestroyWindow(hwnd))
            {
                MineDebug_PrintWarning("Unable to destroy window: %lu\n", GetLastError());
            }
            break;
        }
   } while (bFalse);

   return status;
}

/**
    Mine_IsRectSubset
*//**
    Check if a rectangle is a subset of the boundary rectangle.

    @param[in] pBoundary - Boundary rectangle.
    @param[in] pRect     - Rectangle to check.

    @return TRUE if rectangle is a subset. FALSE otherwise.
*/
BOOLEAN 
Mine_IsRectSubset(_In_ PRECT pBoundary, _In_ PRECT pRect)
{
    BOOLEAN returnValue = FALSE;

    if ((NULL == pBoundary) || (NULL == pRect))
    {
        MineDebug_PrintWarning("Input parameter to Mine_IsRectSubset is NULL\n");
    }
    else
    {
        returnValue = (((pBoundary->left <= pRect->left) && (pBoundary->top <= pRect->top) &&
                        (pBoundary->bottom >= pRect->bottom) && (pBoundary->right >= pRect->right)) ? 
                         (BOOLEAN) TRUE : (BOOLEAN) FALSE);
    }

    return returnValue;
}

/**
    Mine_NewRandomBoard
*//**
    Assign the correct number of mines randomly to a new game board.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR 
Mine_NewRandomBoard(VOID)
{
    BOOLEAN    bFalse = FALSE;
    INT        ix = 0;
    DWORD      minesRemaining = gameData.mines;
    UINT       rand = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    DWORD      tilesRemaining = gameData.height * gameData.width;

    do
    {
        /** For each tile, assign a mine with probability (num mines left)/(num tiles left).
            This will produce a uniform distribution over all possible boards. */
        for(ix = 0; (minesRemaining > 0) && (tilesRemaining > 0); ix++)
        {
            status = Mine_Random(tilesRemaining, &rand);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_Random: %i\n", (int) status);
                break;
            }

            if (rand < minesRemaining)
            {
                gameData.gameBoard[ix] = MINE_BOMB_VALUE;
                minesRemaining--;
            }

            tilesRemaining--;
        }
        if (MINE_ERROR_SUCCESS != status)
        {
            break;
        }

        //All mines should be assigned. Tiles left starts greater than mines left. If mines left ever 
        //equals tiles left then each remaining tile would be assigned a mine with probability one.
    } while (bFalse);

    return status;
}

/**
    Mine_PaintScreen
*//**
    Draw everything in the main window.

    @param[in] hwnd - Handle to the main window.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR 
Mine_PaintScreen(_In_ HWND hwnd)
{
    BOOLEAN     bFalse = FALSE;
    HPEN        greyPen = NULL;
    HDC         hDC = NULL;
    INT         index = 0;
    INT         ix = 0;
    INT         jx = 0;
    HDC         memoryDC = NULL;
    DWORD       minesLeft = 0;
    HGDIOBJ     prevObject = NULL;
    PAINTSTRUCT ps = {0};
    MINE_ERROR  status = MINE_ERROR_SUCCESS;
    HPEN        whitePen = NULL;

    do
    {
        if(NULL == hwnd)
        {
            MineDebug_PrintError("Parameter hwnd is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        hDC = BeginPaint(hwnd, &ps);
        if (NULL == hDC)
        {
            MineDebug_PrintError("Starting paint\n");
            status = MINE_ERROR_PAINT;
            break;
        }

        /** If the invalid region is not restricted to either the game board
            or the top banner, draw the decorative lines. */
        if(!Mine_IsRectSubset(&windowData.topBannerRegion, &ps.rcPaint) &&
           !Mine_IsRectSubset(&windowData.boardRegion, &ps.rcPaint))
        {
            whitePen = CreatePen(PS_SOLID, 0, RGB(255,255,255));
            if (NULL == whitePen)
            {
                MineDebug_PrintError("Getting white pen\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            prevObject = SelectObject(hDC, (HGDIOBJ) whitePen);
            if (NULL == prevObject)
            {
                MineDebug_PrintError("Selecting white pen\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            //Draw each of the white lines
            for (ix = 0; ix < MINE_NUM_WHITE_LINES; ix++)
            {
                if (0 == MoveToEx(hDC, (int) windowData.whiteLines[ix].left, 
                                  (int) windowData.whiteLines[ix].top, NULL))
                {
                    MineDebug_PrintError("Moving to new point x: %ld y: %ld\n", 
                                         windowData.whiteLines[ix].left, 
                                         windowData.whiteLines[ix].top);
                    status = MINE_ERROR_PAINT;
                    break;
                }

                if (0 == LineTo(hDC, (int) windowData.whiteLines[ix].right,
                                (int) windowData.whiteLines[ix].bottom))
                {
                    MineDebug_PrintError("Drawing line to point x: %ld y: %ld\n",
                                         windowData.whiteLines[ix].right, 
                                         windowData.whiteLines[ix].bottom);
                    status = MINE_ERROR_PAINT;
                    break;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }

            greyPen = CreatePen(PS_SOLID, 0, RGB(128,128,128));
            if (NULL == greyPen)
            {
                MineDebug_PrintError("Getting grey pen\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (NULL == SelectObject(hDC, (HGDIOBJ) greyPen))
            {
                MineDebug_PrintError("Selecting grey pen\n");
                status = MINE_ERROR_OBJECT;
                break;
            }

            //Draw each of the grey lines
            for (ix = 0; ix < MINE_NUM_GREY_LINES; ix++)
            {
                if (0 == MoveToEx(hDC, (int) windowData.greyLines[ix].left, 
                                  (int) windowData.greyLines[ix].top, NULL))
                {
                    MineDebug_PrintError("Moving to new point x: %ld y: %ld\n", 
                                         windowData.greyLines[ix].left,
                                         windowData.greyLines[ix].top);
                    status = MINE_ERROR_PAINT;
                    break;
                }

                if (0 == LineTo(hDC, (int) windowData.greyLines[ix].right,
                                (int) windowData.greyLines[ix].bottom))
                {
                    MineDebug_PrintError("Drawing line to point x: %ld y: %ld\n",
                                         windowData.greyLines[ix].right,
                                         windowData.greyLines[ix].bottom);
                    status = MINE_ERROR_PAINT;
                    break;
                }
            }
            if (MINE_ERROR_SUCCESS != status)
            {
                break;
            }

            //Return previous pen to device context
            if (NULL == SelectObject(hDC, prevObject))
            {
                MineDebug_PrintError("Selecting previous object\n");
                status = MINE_ERROR_OBJECT;
                break;
            }
        }

        //Create memory device context to hold bitmap before tranfer to screen
        memoryDC = CreateCompatibleDC(hDC);
        if (NULL == memoryDC)
        {
            MineDebug_PrintError("Creating a compatible memory device context\n");
            status = MINE_ERROR_DC;
            break;
        }

        /** If invalid region overlaps timer, draw the timer. */
        if (Mine_DoRectOverlap(&windowData.timerRegion, &ps.rcPaint))
        {
            //Timer has max value of 999, it will always be displayed with three digits
            index = (((INT) gameData.time) / 100) % 10; //Hundreds digit
            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.timerRegion.left, windowData.timerRegion.top,
                            MINE_TIMER_WIDTH, MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            index = (((INT) gameData.time) / 10) % 10; //Tens digit
            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.timerRegion.left+MINE_TIMER_WIDTH, windowData.timerRegion.top,
                            MINE_TIMER_WIDTH, MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            index = ((INT) gameData.time) % 10; //Units digit
            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.timerRegion.left+(2*MINE_TIMER_WIDTH), windowData.timerRegion.top,
                            MINE_TIMER_WIDTH, MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }
        }

        /** If invalid region overlaps the mine counter region, draw the mine counter. */ 
        if (Mine_DoRectOverlap(&windowData.mineCountRegion, &ps.rcPaint))
        {
            /** Use a negative sign if more tiles flagged than there are mines. */
            //Max number of mines in max size custom board is restricted to four digits
            //Counter will wrap around to -000 if it reaches -1000 (consistent with Windows)
            if (gameData.numFlagged > gameData.mines)
            {
                minesLeft = gameData.numFlagged - gameData.mines;

                if (NULL == SelectObject(memoryDC, imageData.timerDash))
                {
                    MineDebug_PrintError("Selecting timer bitmap dash into memory DC\n");
                    status = MINE_ERROR_OBJECT;
                    break;
                }

                if (0 == BitBlt(hDC, windowData.mineCountRegion.left,
                                windowData.mineCountRegion.top, MINE_TIMER_WIDTH, 
                                MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                {
                    MineDebug_PrintError("Copying timer bitmap dash to screen: %lu\n", GetLastError());
                    status = MINE_ERROR_PAINT;
                    break;
                }
            }
            else 
            {
                minesLeft = gameData.mines - gameData.numFlagged;
                index = (((INT) minesLeft) / 1000) % 10; //Thousands digit

                if (NULL == SelectObject(memoryDC, imageData.timer[index]))
                {
                    MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                    status = MINE_ERROR_OBJECT;
                    break;
                }

                if (0 == BitBlt(hDC, windowData.mineCountRegion.left,
                                windowData.mineCountRegion.top, MINE_TIMER_WIDTH, 
                                MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                {
                    MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                    status = MINE_ERROR_PAINT;
                    break;
                }
            }

            index = (((INT) minesLeft) / 100) % 10; //Hundreds digit

            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.mineCountRegion.left + MINE_TIMER_WIDTH,
                            windowData.mineCountRegion.top, MINE_TIMER_WIDTH, 
                            MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            index = (((INT) minesLeft) / 10) % 10; //Tens digit

            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.mineCountRegion.left + 2*MINE_TIMER_WIDTH,
                            windowData.mineCountRegion.top, MINE_TIMER_WIDTH, 
                            MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError());
                status = MINE_ERROR_PAINT;
                break;
            }

            index = ((INT) minesLeft) % 10; //Units digit

            if (NULL == SelectObject(memoryDC, imageData.timer[index]))
            {
                MineDebug_PrintError("Selecting timer bitmap %i into memory DC\n", index);
                status = MINE_ERROR_OBJECT;
                break;
            }

            if (0 == BitBlt(hDC, windowData.mineCountRegion.left + 3*MINE_TIMER_WIDTH,
                            windowData.mineCountRegion.top, MINE_TIMER_WIDTH, 
                            MINE_TIMER_HEIGHT, memoryDC, 0, 0, SRCCOPY))
            {
                MineDebug_PrintError("Copying timer bitmap %i to screen: %lu\n", index, GetLastError);
                status = MINE_ERROR_PAINT;
                break;
            }
        }

        /** If invalid regions overlaps face, draw the smiley face. */
        if (Mine_DoRectOverlap(&windowData.faceRegion, &ps.rcPaint))
        {
            if (gameData.gameOver)
            {
                if (gameData.gameWon)
                {
                    if (NULL == SelectObject(memoryDC, imageData.faceWon))
                    {
                        MineDebug_PrintError("Selecting face won into memory DC\n");
                        status = MINE_ERROR_OBJECT;
                        break;
                    }

                    if (0 == BitBlt(hDC, windowData.faceRegion.left,
                                    windowData.faceRegion.top, MINE_FACE_WIDTH, 
                                    MINE_FACE_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                    {
                        MineDebug_PrintError("Copying face won to screen: %lu\n", GetLastError());
                        status = MINE_ERROR_PAINT;
                        break;
                    }
                }
                else
                {
                    if (NULL == SelectObject(memoryDC, imageData.faceLost))
                    {
                        MineDebug_PrintError("Selecting face lost into memory DC\n");
                        status = MINE_ERROR_OBJECT;
                        break;
                    }

                    if (0 == BitBlt(hDC, windowData.faceRegion.left,
                                    windowData.faceRegion.top, MINE_FACE_WIDTH, 
                                    MINE_FACE_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                    {
                        MineDebug_PrintError("Copying face lost to screen: %lu\n", GetLastError());
                        status = MINE_ERROR_PAINT;
                        break;
                    }
                }
            }
            else
            {
                if (gameData.leftDown) {
                    if (NULL == SelectObject(memoryDC, imageData.faceClicked))
                    {
                        MineDebug_PrintError("Selecting face clicked into memory DC\n");
                        status = MINE_ERROR_OBJECT;
                        break;
                    }

                    if (0 == BitBlt(hDC, windowData.faceRegion.left,
                                    windowData.faceRegion.top, MINE_FACE_WIDTH, 
                                    MINE_FACE_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                    {
                        MineDebug_PrintError("Copying face clicked to screen: %lu\n", GetLastError());
                        status = MINE_ERROR_PAINT;
                        break;
                    }
                }
                else {
                    if (NULL == SelectObject(memoryDC, imageData.faceNormal))
                    {
                        MineDebug_PrintError("Selecting face normal into memory DC\n");
                        status = MINE_ERROR_OBJECT;
                        break;
                    }

                    if (0 == BitBlt(hDC, windowData.faceRegion.left,
                                    windowData.faceRegion.top, MINE_FACE_WIDTH, 
                                    MINE_FACE_HEIGHT, memoryDC, 0, 0, SRCCOPY))
                    {
                        MineDebug_PrintError("Copying face normal to screen: %lu\n", GetLastError());
                        status = MINE_ERROR_PAINT;
                        break;
                    }
                }
            }
        }

        /** If the invalid region overlaps the board, draw the overlapped tiles. */
        if (Mine_DoRectOverlap(&windowData.boardRegion, &ps.rcPaint))
        {
            for (ix = max(0, (ps.rcPaint.left-windowData.boardRegion.left)/MINE_TILE_PIXELS); 
                 ix <= min((ps.rcPaint.right-windowData.boardRegion.left)/MINE_TILE_PIXELS,
                           (int) gameData.width-1); 
                 ix++)
            {
                for (jx = max(0, (ps.rcPaint.top-windowData.boardRegion.top)/MINE_TILE_PIXELS);
                     jx <= min((ps.rcPaint.bottom-windowData.boardRegion.top)/MINE_TILE_PIXELS,
                               (int) gameData.height-1);
                     jx++)
                {
                    if (gameData.gameOver)
                    {
                        if (gameData.gameWon) //---------- Game won ----------
                        {
                            index = gameData.gameBoard[MINE_INDEX(ix, jx)];

                            //If game won, all mines are shown as flags
                            if (MINE_BOMB_VALUE == index)
                            {
                                if (NULL == SelectObject(memoryDC, imageData.flag))
                                {
                                    MineDebug_PrintError("Selecting flag into memory DC\n");
                                    status = MINE_ERROR_OBJECT;
                                    break;
                                }

                                if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                {
                                    MineDebug_PrintError("Copying flag to screen: %lu\n", GetLastError());
                                    status = MINE_ERROR_PAINT;
                                    break;
                                }
                            }
                            else
                            {
                                //If game is won, all non-mines must have been uncovered
                                if (NULL == SelectObject(memoryDC, imageData.numbers[index]))
                                {
                                    MineDebug_PrintError("Selecting numbers %i into memory DC\n", index);
                                    status = MINE_ERROR_OBJECT;
                                    break;
                                }

                                if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                {
                                    MineDebug_PrintError("Copying numbers %i to screen: %lu\n", 
                                                         index, GetLastError());
                                    status = MINE_ERROR_PAINT;
                                    break;
                                }
                            }
                        }
                        else //---------- Game lost ----------
                        {
                            if (MINE_TILE_STATUS_REVEALED == gameData.tileStatus[MINE_INDEX(ix, jx)])
                            {
                                index = gameData.gameBoard[MINE_INDEX(ix, jx)];

                                //A mine marked as revealed is shown as being hit
                                if (MINE_BOMB_VALUE == index)
                                {
                                    if (NULL == SelectObject(memoryDC, imageData.mineHit))
                                    {
                                        MineDebug_PrintError("Selecting mine hit into memory DC\n");
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS, 
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying mine hit to screen: %lu\n", GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                                else
                                {

                                    if (NULL == SelectObject(memoryDC, imageData.numbers[index]))
                                    {
                                        MineDebug_PrintError("Selecting numbers %i into memory DC\n", index);
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS,
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying numbers %i to screen: %lu\n", 
                                                             index, GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                            }
                            else if (MINE_TILE_STATUS_FLAG == gameData.tileStatus[MINE_INDEX(ix, jx)])
                            {
                                if (MINE_BOMB_VALUE == gameData.gameBoard[MINE_INDEX(ix, jx)])
                                {
                                    if (NULL == SelectObject(memoryDC, imageData.flag))
                                    {
                                        MineDebug_PrintError("Selecting flag into memory DC\n");
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS, 
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying flag to screen: %lu\n", GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                                else 
                                {
                                    //Display locations of incorrectly flagged tiles
                                    if (NULL == SelectObject(memoryDC, imageData.falseFlag))
                                    {
                                        MineDebug_PrintError("Selecting false flag into memory DC\n");
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS, 
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying false flag to screen: %lu\n", GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //If game lost, display the location of all hidden mines
                                if (MINE_BOMB_VALUE == gameData.gameBoard[MINE_INDEX(ix, jx)])
                                {
                                    if (NULL == SelectObject(memoryDC, imageData.mine))
                                    {
                                        MineDebug_PrintError("Selecting mine into memory DC\n");
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS, 
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying mine to screen: %lu\n", GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                                else 
                                {
                                    if (NULL == SelectObject(memoryDC, imageData.unclicked))
                                    {
                                        MineDebug_PrintError("Selecting unclicked into memory DC\n");
                                        status = MINE_ERROR_OBJECT;
                                        break;
                                    }

                                    if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                                    windowData.boardRegion.top+jx*MINE_TILE_PIXELS, 
                                                    MINE_TILE_PIXELS, MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                                    {
                                        MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                                        status = MINE_ERROR_PAINT;
                                        break;
                                    }
                                }
                            }                         
                        }
                    }
                    else //---------- Game still in progress ----------
                    {
                        if (MINE_TILE_STATUS_REVEALED == gameData.tileStatus[MINE_INDEX(ix, jx)])
                        {
                            index = gameData.gameBoard[MINE_INDEX(ix, jx)];

                            if (NULL == SelectObject(memoryDC, imageData.numbers[index]))
                            {
                                MineDebug_PrintError("Selecting numbers %i into memory DC\n", index);
                                status = MINE_ERROR_OBJECT;
                                break;
                            }

                            if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                            windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS, 
                                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                            {
                                MineDebug_PrintError("Copying numbers %i to screen: %lu\n", index, GetLastError());
                                status = MINE_ERROR_PAINT;
                                break;
                            }
                        }
                        else if (MINE_TILE_STATUS_FLAG == gameData.tileStatus[MINE_INDEX(ix, jx)])
                        {
                            if (NULL == SelectObject(memoryDC, imageData.flag))
                            {
                                MineDebug_PrintError("Selecting flag into memory DC\n");
                                status = MINE_ERROR_OBJECT;
                                break;
                            }

                            if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                            windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                            {
                                MineDebug_PrintError("Copying flag to screen: %lu\n", GetLastError());
                                status = MINE_ERROR_PAINT;
                                break;
                            }
                        }
                        else if (MINE_TILE_STATUS_HELD == gameData.tileStatus[MINE_INDEX(ix, jx)])
                        {
                            if (NULL == SelectObject(memoryDC, imageData.held))
                            {
                                MineDebug_PrintError("Selecting held into memory DC\n");
                                status = MINE_ERROR_OBJECT;
                                break;
                            }

                            if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                            windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS, 
                                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                            {
                                MineDebug_PrintError("Copying held to screen: %lu\n", GetLastError());
                                status = MINE_ERROR_PAINT;
                                break;
                            }
                        }
                        else
                        {
                            if (NULL == SelectObject(memoryDC, imageData.unclicked))
                            {
                                MineDebug_PrintError("Selecting unclicked into memory DC\n");
                                status = MINE_ERROR_OBJECT;
                                break;
                            }

                            if (0 == BitBlt(hDC, windowData.boardRegion.left+ix*MINE_TILE_PIXELS,
                                            windowData.boardRegion.top+jx*MINE_TILE_PIXELS, MINE_TILE_PIXELS, 
                                            MINE_TILE_PIXELS, memoryDC, 0, 0, SRCCOPY))
                            {
                                MineDebug_PrintError("Copying unclicked to screen: %lu\n", GetLastError());
                                status = MINE_ERROR_PAINT;
                                break;
                            }
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
        (void) EndPaint(hwnd, &ps);
        hDC = NULL;
    }

    if (NULL != whitePen)
    {
        if (0 == DeleteObject((HGDIOBJ) whitePen))
        {
            MineDebug_PrintWarning("Unable to delete white pen\n");
        }
        whitePen = NULL;
    }

    if (NULL != greyPen)
    {
        if (0 == DeleteObject((HGDIOBJ) greyPen))
        {
            MineDebug_PrintWarning("Unable to delete grey pen\n");
        }
        greyPen = NULL;
    }

    return status;
}

/**
    Mine_PointInRect
*//**
    Checks if a point is inside a given rectangle.

    @param[in] x     - X coordinate of point.
    @param[in] y     - Y coordinate of point.
    @param[in] pRect - Pointer to boundary rectangle.

    @return TRUE if point is in rectangle. FALSE otherwise.
*/
BOOLEAN 
Mine_PointInRect(short x, short y, _In_ PRECT pRect)
{
    BOOLEAN returnValue = TRUE;

    if (NULL == pRect)
    {
        MineDebug_PrintWarning("Input parameter to Mine_PointInRect is NULL\n");
        returnValue = FALSE;
    }
    else
    {
        if ((x < pRect->left) || (x >= pRect->right))
        {
            returnValue = FALSE;
        }

        if ((y < pRect->top) || (y >= pRect->bottom))
        {
            returnValue = FALSE;
        }
    }

    return returnValue;
}

/**
    Mine_ProcessTimer
*//**
    Check how long the game has been going and update the timer if needed.
*/
VOID 
Mine_ProcessTimer(VOID)
{
    ULONGLONG currentTime;

    //Time is based on time since game started, timer does not pause when game is minimized
    currentTime = GetTickCount64();

    //Convert time to seconds
    currentTime = (currentTime - gameData.gameStartTime) / 1000;

    if (currentTime > MINE_MAX_TIME)
    {
        currentTime = MINE_MAX_TIME;
    }

    /** If time has changed, update the timer. */
    if (gameData.time != ((UINT) currentTime))
    {
        gameData.time = ((UINT) currentTime);
        if (0 == InvalidateRect(hwnd, &(windowData.timerRegion), FALSE))
        {
            MineDebug_PrintWarning("Unable to invalidate rectangle\n");
        }
    }

    return;
}

/**
    Mine_Random
*//**
    Obtain a random integer in the range 0 to (limit-1) inclusive.

    @param[in]  limit  - Number of possible random value to return.
    @param[out] output - Pointer to a UINT to hold the random number.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR 
Mine_Random(UINT limit, _Out_ PUINT output)
{
    BOOLEAN        bFalse = FALSE;
    int            cpuidValue = 0;
    int            eflagsValue = 0;
    static BOOLEAN onboardRng = FALSE;
    int            onboardTries = 0;
    static BOOLEAN processorChecked = FALSE;
    UINT           rand = 0;
    BOOL           returnCode = 1;
    MINE_ERROR     status = MINE_ERROR_SUCCESS;
    UINT           upperbound = UINT_MAX;

    do
    {
        /** Check if processor has onboard RNG capabilities only
            the first time the function is called. */
        if (!processorChecked)
        {
            do
            {
                processorChecked = TRUE;

                //First check if CPUID call is supported
                __asm
                {
                    //Store registers that will be changed
                    push eax
                    push ecx
                    //Put EFLAGS register into EAX and also ECX
                    pushfd
                    pop eax
                    mov ecx, eax
                    //Flip bit 21, CPUID bit
                    xor eax, 0x200000
                    push eax
                    popfd
                    pushfd
                    pop eax
                    //Check if change to bit 21 was stored in EFLAGS
                    xor eax, ecx
                    mov eflagsValue, eax
                    //Restore value for EFLAGS
                    push ecx
                    popfd
                    //Restore value for EAX and ECX
                    pop ecx
                    pop eax
                }

                if (0 == eflagsValue)
                {
                    //CPUID call not supported
                    break;
                }

                //CPUID call with EAX == 0x01 is always supported,
                //don't need to check Maximum Input Value

                //Check if RDRAND call is supported
                __asm
                {
                    //Store registers that will be changed
                    push eax
                    push ebx
                    push ecx
                    push edx
                    //Call CPUID with Input Value 0x01
                    mov eax, 0x01
                    cpuid
                    //Extract feature information
                    mov cpuidValue, ecx
                    //Restore registers
                    pop edx
                    pop ecx
                    pop ebx
                    pop eax
                }

                //Bit 30 is set to 1 if RDRAND function is supported
                if (0 != (cpuidValue & (1 << 30)))
                {
                    onboardRng = TRUE;
                }
            } while (bFalse);

            //If RDRAND is not supported, set up the windows crypt provider
            if (!onboardRng)
            {
                if (0 == CryptAcquireContextW(&hCrypto, NULL, MS_DEF_PROV_W, PROV_RSA_FULL, 
                                              CRYPT_VERIFYCONTEXT | CRYPT_SILENT) || (NULL == hCrypto))
                {
                    MineDebug_PrintError("Unable to get a cryptographic context: %lu\n", GetLastError());
                    status = MINE_ERROR_CRYPT;
                    break;
                }
            }
        } 

        if (NULL == output)
        {
            MineDebug_PrintError("Parameter output is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        *output = 0;

        if (0 == limit)
        {
            MineDebug_PrintError("Parameter limit cannot be zero\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        /** Guarantee uniform distribution by having equal number of 
            possibilities from each residue class. */
        if ((UINT_MAX % limit) != (limit - 1))
        {
            upperbound = UINT_MAX - (UINT_MAX % limit) - 1;
        }

        //Use RDRAND instruction if supported
        if (onboardRng)
        {
            __asm
            {
                //Store EAX register
                push eax
                //Call RDRAND
                rdrand eax
                mov rand, eax
                //Obtain value in FLAGS register
                pushfd
                pop eax
                mov eflagsValue, eax
                //Restore EAX register
                pop eax
            }

            //Retry if rand is greater than upper bound to get even distribution
            //Carry flag is one if a valid random number was generated
            while ((rand > upperbound) || (1 != (eflagsValue & 1)))
            {
                //Try up to ten times to get a random number from the board
                if (1 != (eflagsValue & 1))
                {
                    onboardTries += 1;
                }

                if (onboardTries >= 10)
                {
                    break;
                }

                //Try again to get a random number
                __asm
                {
                    push eax
                    rdrand eax
                    mov rand, eax
                    pushfd
                    pop eax
                    mov eflagsValue, eax
                    pop eax
                }
            }

            if (onboardTries >= 10)
            {
                MineDebug_PrintError("Getting random number from processor\n");
                status = MINE_ERROR_RANDNUMBER;
                break;
            }

            *output = (rand % limit);
        }
        else //RDRAND not supported
        {
            if (0 == CryptGenRandom(hCrypto, sizeof(UINT), (BYTE*) &rand))
            {
                MineDebug_PrintError("Getting random number: %lu\n", GetLastError());
                status = MINE_ERROR_RANDNUMBER;
            }

            while (rand > upperbound)
            {
                returnCode = CryptGenRandom(hCrypto, sizeof(UINT), (BYTE*) &rand);
                if (0 == returnCode)
                {
                    break;
                }
            }
            if (0 == returnCode)
            {
                MineDebug_PrintError("Getting random number: %lu\n", GetLastError());
                status = MINE_ERROR_RANDNUMBER;
                break;
            }

            *output = (rand % limit);
        }
    } while (bFalse);

    return status;
}

/**
    Mine_RandomPerm
*//**
    Create a random ordered set of numPerm elements from a collection of numArray elements.

    @param[inout] pArray   - Pointer to the array of elements.
    @param[in]    numArray - Number of elements in the array.
    @param[in]    numPerm  - Size of ordered set to create.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_RandomPerm(_Inout_updates_to_(numArray, numPerm) PUINT pArray, 
                UINT numArray, UINT numPerm)
{
    BOOLEAN    bFalse = FALSE;
    UINT       ix = 0;
    UINT       limit = 0;
    UINT       rand = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    UINT       temp = 0;

    do
    {
        if (NULL == pArray)
        {
            MineDebug_PrintError("Parameter pArray is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        if (numPerm > numArray)
        {
            MineDebug_PrintError("Parameter numPerm must be less than or equal to numArray\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        /** Implement the Knuth shuffle to generate a random permutation. 
            Stop after numPerm elements have been fixed. */
        for (ix = 0; ix < numPerm; ix++)
        {
            limit = numArray - ix;

            status = Mine_Random(limit, &rand);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_Random: %i\n", (int) status);
                break;
            }

            temp = pArray[numArray - 1 - rand];
            pArray[numArray - 1 - rand] = pArray[ix];
            pArray[ix] = temp;
        }
    } while (bFalse);

    return status;
}

/**
    Mine_RegisterClass
*//**
    Register window class for main window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR 
Mine_RegisterClass(VOID)
{
    ATOM        atom = 0;
    BOOLEAN     bFalse = FALSE;
    MINE_ERROR  status = MINE_ERROR_SUCCESS;
    WNDCLASSEXW wcex = {0};

    do
    {
        //Load window class structure
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = 0;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;
        wcex.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_MINESWEEPER));
        if (NULL == wcex.hIcon)
        {
            MineDebug_PrintError("Loading large sized icon: %lu\n", GetLastError());
            status = MINE_ERROR_OBJECT;
            break;
        }
        wcex.hCursor = LoadCursorW(NULL, IDC_ARROWW);
        if (NULL == wcex.hCursor)
        {
            MineDebug_PrintError("Loading arrow cursor: %lu\n", GetLastError());
            status = MINE_ERROR_OBJECT;
            break;
        }
        //Make the grey colored background
        wcex.hbrBackground = CreateSolidBrush(RGB(192,192,192));
        if (NULL == wcex.hbrBackground)
        {
            MineDebug_PrintError("Loading background brush\n");
            status = MINE_ERROR_OBJECT;
            break;
        }
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINESWEEPER);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = LoadIconW(wcex.hInstance, MAKEINTRESOURCEW(IDI_SMALL));
        if (NULL == wcex.hIconSm)
        {
            MineDebug_PrintError("Loading small sized icon: %i\n", GetLastError());
            status = MINE_ERROR_OBJECT;
            break;
        }

        //Register the class
        atom = RegisterClassExW(&wcex);
        if (0 == atom)
        {
            MineDebug_PrintError("Registering window class: %lu", GetLastError());
            status = MINE_ERROR_REGISTER_CLASS;
            break;
        }
    } while (bFalse);

    //Clean up

    if (MINE_ERROR_SUCCESS != status)
    {
        if (NULL != wcex.hbrBackground)
        {
            //If window class was not registered properly need to delete brush 
            //ourselves, otherwise it is delted when the window is destroyed
            if (0 == DeleteObject((HGDIOBJ) wcex.hbrBackground))
            {
                MineDebug_PrintWarning("Unable to delete hbrBackground brush\n");
            }
            wcex.hbrBackground = NULL;
        }
    }

    return status;
}

/**
    Mine_SetRegDword
*//**
    Set a value in the Minesweeper Deluxe key to a given DWORD.

    @param[in] pValueName   - Name of the value to set.
    @param[in] newValue     - DWORD value to place in key.
    
    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetRegDword(_In_ LPWSTR pValueName, DWORD newValue)
{
    BOOLEAN    bFalse = FALSE;
    LSTATUS    lstatus = ERROR_SUCCESS;
    HKEY       registryKey = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        if (NULL == pValueName)
        {
            MineDebug_PrintError("Parameter pValueName is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        //Open a handle to the registry key
        lstatus = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Entropy\\MinesweeperDeluxe", 0,
                                    NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                                    &registryKey, NULL);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintError("Creating registry key: %li\n", lstatus);
            status = MINE_ERROR_REGISTRY;
            break;
        }

        //Set the value
        lstatus = RegSetValueExW(registryKey, pValueName, 0, REG_DWORD,
                                    (BYTE *) &(newValue), sizeof(DWORD));
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintError("Setting DWORD registry value: %li\n", lstatus);
            status = MINE_ERROR_REGISTRY;
            break;
        }       
    } while (bFalse);

    //Clean up

    if (NULL != registryKey)
    {
        lstatus = RegCloseKey(registryKey);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintWarning("Unable to close registry key: %li\n", lstatus);
        }
        registryKey = NULL;
    }

    return status;
}

/**
    Mine_SetRegString
*//**
    Set a value in the Minesweeper Deluxe key to a given string.

    @param[in] pValueName   - Name of the value to set.
    @param[in] pNewValue    - String to place in key.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetRegString(_In_ LPWSTR pValueName, _In_ LPWSTR pNewValue)
{
    BOOLEAN    bFalse = FALSE;
    size_t     cbString = 0;
    LSTATUS    lstatus = ERROR_SUCCESS;
    HKEY       registryKey = NULL;
    HRESULT    result = S_OK;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        if (NULL == pValueName)
        {
            MineDebug_PrintError("Parameter pValueName is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        if (NULL == pNewValue)
        {
            MineDebug_PrintError("Parameter pNewValue is NULL\n");
            status = MINE_ERROR_PARAMETER;
            break;
        }

        result = StringCbLengthW(pNewValue, STRSAFE_MAX_CCH*sizeof(WCHAR), &cbString);
        if (FAILED(result))
        {
            MineDebug_PrintError("Calculating string length: %li\n", result);
            status = MINE_ERROR_STRING;
            break;
        }

        //Add bytes for terminating null character
        cbString += sizeof(WCHAR);

        //Open a handle to the registry key
        lstatus = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Entropy\\MinesweeperDeluxe", 0,
                                    NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                                    &registryKey, NULL);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintError("Creating registry key: %li\n", lstatus);
            status = MINE_ERROR_REGISTRY;
            break;
        }

        //Set the value
        lstatus = RegSetValueExW(registryKey, pValueName, 0, REG_SZ,
                                    (BYTE *) pNewValue, (DWORD) cbString);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintError("Setting string registry value: %li\n", lstatus);
            status = MINE_ERROR_REGISTRY;
            break;
        }    
    } while (bFalse);

    //Clean up

    if (NULL != registryKey)
    {
        lstatus = RegCloseKey(registryKey);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintWarning("Unable to close registry key: %li\n", lstatus);
        }
        registryKey = NULL;
    }

    return status;
}

/**
    Mine_SetupGame
*//**
    Change the game settings upon the start of a new game.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR 
Mine_SetupGame(VOID)
{
    BOOLEAN    bFalse = FALSE;
    HANDLE     hHeap = NULL;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        hHeap = GetProcessHeap();
        if (NULL == hHeap)
        {
            MineDebug_PrintError("Getting process heap: %lu\n", GetLastError());
            status = MINE_ERROR_HEAP;
            break;
        }

        //Free memory for previous game board
        if (NULL != gameData.gameBoard)
        {
            if (0 == HeapFree(hHeap, 0, gameData.gameBoard))
            {
                MineDebug_PrintWarning("Unable to free game board: %lu\n", GetLastError());
            }
            gameData.gameBoard = NULL;
        }

        if (NULL != gameData.tileStatus)
        {
            if (0 == HeapFree(hHeap, 0, gameData.tileStatus))
            {
                MineDebug_PrintWarning("Unable to free tile status: %lu\n", GetLastError());
            }
            gameData.tileStatus = NULL;
        }

        gameData.gameStarted = FALSE;
        gameData.gameOver = FALSE;
        gameData.gameWon = FALSE;
        
        //Fill in board size and number of mines based on level
        switch (menuData.gameLevel)
        {
        case MINE_LEVEL_BEGINNER:
            gameData.height = MINE_BEGINNER_HEIGHT;
            gameData.width = MINE_BEGINNER_WIDTH;
            gameData.mines = MINE_BEGINNER_MINES;
            break;
        case MINE_LEVEL_INTERMEDIATE:
            gameData.height = MINE_INTERMEDIATE_HEIGHT;
            gameData.width = MINE_INTERMEDIATE_WIDTH;
            gameData.mines = MINE_INTERMEDIATE_MINES;
            break;
        case MINE_LEVEL_EXPERT:
            gameData.height = MINE_EXPERT_HEIGHT;
            gameData.width = MINE_EXPERT_WIDTH;
            gameData.mines = MINE_EXPERT_MINES;
            break;
        case MINE_LEVEL_CUSTOM:
            gameData.height = menuData.customHeight;
            gameData.width = menuData.customWidth;
            gameData.mines = menuData.customMines;
            break;
        default:
            __assume(0);
            break;
        }

        gameData.numFlagged = 0;
        gameData.numUncovered = 0;

        gameData.gameBoard = (CHAR *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY,
                                                gameData.height * gameData.width * sizeof(CHAR));
        if (NULL == gameData.gameBoard)
        {
            MineDebug_PrintError("Allocating memory for gameBoard\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        gameData.tileStatus = (CHAR *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 
                                                 gameData.height * gameData.width * sizeof(CHAR));
        if (NULL == gameData.tileStatus)
        {
            MineDebug_PrintError("Allocating memory for tileStatus\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        gameData.time = 0;

        gameData.leftDown = FALSE;
        gameData.rightDown = FALSE;

        gameData.prevGridX = -1;
        gameData.prevGridY = -1;

        gameData.horzShift = 0;
        gameData.vertShift = 0;

        //Create a new board
        status = Mine_NewRandomBoard();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_NewRandomBoard: %i\n", (int) status);
            break;
        }

        //If the number images should be random, pick a new set of images
        if (MINE_NUMBER_IMAGE_RANDOM == menuData.numberImages)
        {
            status = Mine_SetupNumberImageData();
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_SetupImageData: %i\n", (int) status);
                break;
            }
        }
    } while (bFalse);

    //Clean up (upon error only)
    if ((MINE_ERROR_SUCCESS != status) && (NULL != hHeap))
    {
        if (NULL != gameData.gameBoard)
        {
            if (0 == HeapFree(hHeap, 0, gameData.gameBoard))
            {
                MineDebug_PrintWarning("Unable to free game board: %lu\n", GetLastError());
            }
            gameData.gameBoard = NULL;
        }

        if (NULL != gameData.tileStatus)
        {
            if (0 == HeapFree(hHeap, 0, gameData.tileStatus))
            {
                MineDebug_PrintWarning("Unable to free tile status: %lu\n", GetLastError());
            }
            gameData.tileStatus = NULL;
        }
    }

    return status;
}

/**
    Mine_SetupGlobal
*//**
    Access the registry to obtain settings that persist when program is closed.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupGlobal(VOID)
{
    BOOLEAN    bFalse = FALSE;
    HANDLE     hHeap = NULL;
    LSTATUS    lstatus = ERROR_SUCCESS;
    HKEY       registryKey = NULL;
    DWORD      regType = 0;
    HRESULT    result = S_OK;
    DWORD      size = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;
    DWORD      valueFromRegistry = 0;

    do
    {
        hHeap = GetProcessHeap();
        if (NULL == hHeap)
        {
            MineDebug_PrintError("Getting process heap: %lu", GetLastError());
            status = MINE_ERROR_HEAP;
            break;
        }

        //Allocate memory for best time names
        menuData.beginnerName = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY,
                                                   (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        if (NULL == menuData.beginnerName)
        {
            MineDebug_PrintError("Allocating memory for beginnerName\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        menuData.intermediateName = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY,
                                                       (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        if (NULL == menuData.intermediateName)
        {
            MineDebug_PrintError("Allocating memory for intermediateName\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        menuData.expertName = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY,
                                                 (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        if (NULL == menuData.expertName)
        {
            MineDebug_PrintError("Allocating memory for expertName\n");
            status = MINE_ERROR_MEMORY;
            break;
        }

        //Set the default values in case of error reading registry
        menuData.gameLevel = MINE_LEVEL_BEGINNER;
        menuData.customHeight = MINE_BEGINNER_HEIGHT;
        menuData.customWidth = MINE_BEGINNER_WIDTH;
        menuData.customMines = MINE_BEGINNER_MINES;
        menuData.beginnerTime = MINE_MAX_TIME;
        result = StringCchCopyW(menuData.beginnerName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintWarning("Copying string to beginnerName: %li\n", result);
            ZeroMemory(menuData.beginnerName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        }
        menuData.intermediateTime = MINE_MAX_TIME;
        result = StringCchCopyW(menuData.intermediateName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintWarning("Copying string to intermediateName: %li\n", result);
            ZeroMemory(menuData.intermediateName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        }
        menuData.expertTime = MINE_MAX_TIME;
        result = StringCchCopyW(menuData.expertName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
        if (FAILED(result))
        {
            MineDebug_PrintWarning("Copying string to expertName: %li\n", result);
            ZeroMemory(menuData.expertName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
        }
        menuData.wrapHorz = FALSE;
        menuData.wrapVert = FALSE;
        menuData.numberImages = MINE_NUMBER_IMAGE_NORMAL;
        menuData.useMovement = FALSE;
        menuData.movementFreq = MINE_SECOND;
        menuData.movementAggressive = MINE_MOVEMENT_DEFAULT_AGGRESSIVENESS;
            
        //Open a handle to the registry key
        lstatus = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Entropy\\MinesweeperDeluxe", 0,
                                  NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                                  &registryKey, NULL);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintWarning("Uanble to create registry key: %li\n", lstatus);
            break;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve default level setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"DefaultLevel", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) && 
            ((valueFromRegistry < MINE_LEVEL_BEGINNER) || (valueFromRegistry > MINE_LEVEL_CUSTOM))))
        {
            //Set to default value
            lstatus = RegSetValueExW(registryKey, L"DefaultLevel", 0, REG_DWORD,
                                     (BYTE *) &(menuData.gameLevel), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting DefaultLevel registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing DefaultLevel registry value: %lu %li\n", 
                                   regType, lstatus);
        }
        else
        {
            menuData.gameLevel = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve custom board height setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"CustomHeight", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) && 
            ((valueFromRegistry < MINE_BEGINNER_HEIGHT) || (valueFromRegistry > MINE_MAX_HEIGHT))))
        {
            lstatus = RegSetValueExW(registryKey, L"CustomHeight", 0, REG_DWORD,
                                     (BYTE *) &(menuData.customHeight), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting CustomHeight registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing CustomHeight registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.customHeight = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve custom board width setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"CustomWidth", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            ((valueFromRegistry < MINE_BEGINNER_WIDTH) || (valueFromRegistry > MINE_MAX_WIDTH))))
        {
            lstatus = RegSetValueExW(registryKey, L"CustomWidth", 0, REG_DWORD,
                                     (BYTE *) &(menuData.customWidth), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting CustomWidth registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing CustomWidth registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.customWidth = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve custom board mine number setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"CustomMines", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || (ERROR_SUCCESS == lstatus) &&
            ((valueFromRegistry < MINE_BEGINNER_MINES) || 
             (valueFromRegistry > ((menuData.customHeight - 1)*(menuData.customWidth - 1)))))
        {
            lstatus = RegSetValueExW(registryKey, L"CustomMines", 0, REG_DWORD,
                                     (BYTE *) &(menuData.customMines), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting CustomMines registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing CustomMines registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.customMines = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve beginner best time setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"BeginnerTime", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) && 
            (valueFromRegistry > MINE_MAX_TIME)))
        {
            lstatus = RegSetValueExW(registryKey, L"BeginnerTime", 0, REG_DWORD,
                                     (BYTE *) &(menuData.beginnerTime), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting BeginnerTime registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing BeginnerTime registry value: %lu %li\n", 
                                   regType, lstatus);
        }
        else
        {
            menuData.beginnerTime = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve beginner best time name from registry. */
        size = (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR);
        lstatus = RegQueryValueExW(registryKey, L"BeginnerName", NULL, &regType,
                                   (LPBYTE) menuData.beginnerName, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"BeginnerName", 0, REG_SZ,
                                     (BYTE *) menuData.beginnerName, MINE_ANONYMOUS_SIZE);
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting BeginnerName registry value: %li\n", lstatus);
            }
        }
        else if ((REG_SZ != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing BeginnerName registry value: %lu %li\n",
                                   regType, lstatus);
            //Reset to default value since buffer was written to directly
            result = StringCchCopyW(menuData.beginnerName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
            if (FAILED(result))
            {
                MineDebug_PrintWarning("Copying string beginnerName: %li\n", result);
                //Zero out as last resort
                ZeroMemory(menuData.beginnerName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
            } 
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve intermediate best time setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"IntermediateTime", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            (valueFromRegistry > MINE_MAX_TIME)))
        {
            lstatus = RegSetValueExW(registryKey, L"IntermediateTime", 0, REG_DWORD,
                                     (BYTE *) &(menuData.intermediateTime), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting IntermediateTime registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing IntermediateTime registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.intermediateTime = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve intermediate best time name from registry. */
        size = (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR);
        lstatus = RegQueryValueExW(registryKey, L"IntermediateName", NULL, &regType,
                                   (LPBYTE) menuData.intermediateName, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"IntermediateName", 0, REG_SZ,
                                     (BYTE *) menuData.intermediateName, MINE_ANONYMOUS_SIZE);
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting IntermediateName registry value: %li\n", lstatus);
            }
        }
        else if ((REG_SZ != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing IntermediateName registry value: %lu %li\n",
                             regType, lstatus);
            result = StringCchCopyW(menuData.intermediateName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
            if (FAILED(result))
            {
                MineDebug_PrintWarning("Copying string intermediateName: %li\n", result);
                ZeroMemory(menuData.intermediateName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
            } 
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve expert best time setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"ExpertTime", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            (valueFromRegistry > MINE_MAX_TIME)))
        {
            lstatus = RegSetValueExW(registryKey, L"ExpertTime", 0, REG_DWORD,
                                     (BYTE *) &(menuData.expertTime), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting ExpertTime registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing ExpertTime registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.expertTime = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve expert best time name from registry. */
        size = (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR);
        lstatus = RegQueryValueExW(registryKey, L"ExpertName", NULL, &regType,
                                   (LPBYTE) menuData.expertName, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"ExpertName", 0, REG_SZ,
                                     (BYTE *) menuData.expertName, MINE_ANONYMOUS_SIZE);
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting ExpertName registry value: %li\n", lstatus);
            }
        }
        else if ((REG_SZ != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing ExpertName registry value: %lu %li\n",
                                   regType, lstatus);
            result = StringCchCopyW(menuData.expertName, MINE_NAME_BUFFER_CHARS, MINE_ANONYMOUS_STRING);
            if (FAILED(result))
            {
                MineDebug_PrintWarning("Copying string expertName: %li\n", result);
                ZeroMemory(menuData.expertName, (MINE_NAME_BUFFER_CHARS+1)*sizeof(WCHAR));
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve number images setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"NumberImages", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            (valueFromRegistry > MINE_NUMBER_IMAGE_BLANK)))
        {
            lstatus = RegSetValueExW(registryKey, L"NumberImages", 0, REG_DWORD,
                                (BYTE *) &(menuData.numberImages), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting NumberImages registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing NumberImages registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.numberImages = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve horizontal wrap setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"WrapHorz", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"WrapHorz", 0, REG_DWORD,
                                     (BYTE *) &(menuData.wrapHorz), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting WrapHorz registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing WrapHorz registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.wrapHorz = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve vertical wrap setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"WrapVert", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"WrapVert", 0, REG_DWORD,
                                     (BYTE *) &(menuData.wrapVert), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting WrapVert registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing WrapVert registry value: %lu %li\n", 
                                   regType, lstatus);
        }
        else
        {
            menuData.wrapVert = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve movement setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"Movement", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if (ERROR_FILE_NOT_FOUND == lstatus)
        {
            lstatus = RegSetValueExW(registryKey, L"Movement", 0, REG_DWORD,
                                     (BYTE *) &(menuData.useMovement), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting Movement registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing Movement registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.useMovement = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve movement frequency setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"MovementFrequency", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            ((valueFromRegistry < MINE_QUARTER_SECOND) || (valueFromRegistry > MINE_MAX_TIME*MINE_SECOND))))
        {
            lstatus = RegSetValueExW(registryKey, L"MovementFrequency", 0, REG_DWORD,
                                     (BYTE *) &(menuData.movementFreq), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting MovementFrequency registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing MovementFrequency registry value: %lu %li\n",
                                   regType, lstatus);
        }
        else
        {
            menuData.movementFreq = valueFromRegistry;
        }

        ///////////////////////////////////////////////////////////////////////////////////////
        /** Retrieve movement aggressiveness setting from registry. */
        size = sizeof(DWORD);
        lstatus = RegQueryValueExW(registryKey, L"MovementAggressiveness", NULL, &regType,
                                   (LPBYTE) &valueFromRegistry, &size);
        if ((ERROR_FILE_NOT_FOUND == lstatus) || ((ERROR_SUCCESS == lstatus) &&
            (valueFromRegistry < 1) || (valueFromRegistry > 10)))
        {
            lstatus = RegSetValueExW(registryKey, L"MovementAggressiveness", 0, REG_DWORD,
                                     (BYTE *) &(menuData.movementAggressive), sizeof(DWORD));
            if (ERROR_SUCCESS != lstatus)
            {
                MineDebug_PrintWarning("Setting MovementAggressiveness registry value: %li\n", lstatus);
            }
        }
        else if ((REG_DWORD != regType) || (ERROR_SUCCESS != lstatus))
        {
            MineDebug_PrintWarning("Accessing MovementAggressiveness registry value: %lu %li\n", 
                                   regType, lstatus);
        }
        else
        {
            menuData.movementAggressive = valueFromRegistry;
        }
    } while (bFalse);

    //Clean up

    if (NULL != registryKey)
    {
        lstatus = RegCloseKey(registryKey);
        if (ERROR_SUCCESS != lstatus)
        {
            MineDebug_PrintWarning("Unable to close registry key: %li\n", lstatus);
        }
        registryKey = NULL;
    }

    return status;
}

/**
    Mine_SetupImageData
*//**
    Load handles to images needed to draw window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupImageData(VOID)
{
    BOOLEAN    bFalse = FALSE;
    INT        ix = 0;
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        //Obtain handle for numbers for timer and remaining mines counter
        for (ix = 0; ix < 10; ix++)
        {
            imageData.timer[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_TIMER0+ix),
                                                       IMAGE_BITMAP, MINE_TIMER_WIDTH, MINE_TIMER_HEIGHT,
                                                       LR_DEFAULTCOLOR);     
            if (NULL == imageData.timer[ix])
            {
                MineDebug_PrintError("Loading timer %i bitmap: %lu\n", ix, GetLastError());
                status = MINE_ERROR_BITMAP;
                break;
            }
        }
        if (MINE_ERROR_SUCCESS != status)
        {
            break;
        }

        //Obtain handle for dash for remaining mines counter
        imageData.timerDash = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_TIMERDASH),
                                                   IMAGE_BITMAP, MINE_TIMER_WIDTH, MINE_TIMER_HEIGHT,
                                                   LR_DEFAULTCOLOR);
        if (NULL == imageData.timerDash)
        {
            MineDebug_PrintError("Loading timer dash bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for normal smiley face
        imageData.faceNormal = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FACENORMAL),
                                                    IMAGE_BITMAP, MINE_FACE_WIDTH, MINE_FACE_HEIGHT,
                                                    LR_DEFAULTCOLOR);
        if (NULL == imageData.faceNormal)
        {
            MineDebug_PrintError("Loading face normal bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for smiley face when button is clicked
        imageData.faceClicked = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FACECLICKED),
                                                     IMAGE_BITMAP, MINE_FACE_WIDTH, MINE_FACE_HEIGHT,
                                                     LR_DEFAULTCOLOR);
        if (NULL == imageData.faceClicked)
        {
            MineDebug_PrintError("Loading face clicked bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for smiley face when a game is won
        imageData.faceWon = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FACEWON),
                                                 IMAGE_BITMAP, MINE_FACE_WIDTH, MINE_FACE_HEIGHT,
                                                 LR_DEFAULTCOLOR);
        if (NULL == imageData.faceWon)
        {
            MineDebug_PrintError("Loading face won bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for smiley face when a game is lost
        imageData.faceLost = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FACELOST),
                                                  IMAGE_BITMAP, MINE_FACE_WIDTH, MINE_FACE_HEIGHT,
                                                  LR_DEFAULTCOLOR);
        if (NULL == imageData.faceLost)
        {
            MineDebug_PrintError("Loading face lost bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for an unclicked tile
        imageData.unclicked = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_UNCLICKED),
                                                   IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                   LR_DEFAULTCOLOR);
        if (NULL == imageData.unclicked)
        {
            MineDebug_PrintError("Loading unclicked bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for a tile that is being held
        imageData.held = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_HELD),
                                              IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                              LR_DEFAULTCOLOR);
        if (NULL == imageData.held)
        {
            MineDebug_PrintError("Loading held bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for a tile with a flag
        imageData.flag = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FLAG),
                                              IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                              LR_DEFAULTCOLOR);
        if (NULL == imageData.flag)
        {
            MineDebug_PrintError("Loading flag bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for a tile with an incorrectly placed flag, shown when game is lost
        imageData.falseFlag = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_FALSEFLAG),
                                                   IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                   LR_DEFAULTCOLOR);
        if (NULL == imageData.falseFlag)
        {
            MineDebug_PrintError("Loading false flag bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for a tile with a mine, shown when game is lost
        imageData.mine = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_MINE),
                                              IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                              LR_DEFAULTCOLOR);
        if (NULL == imageData.mine)
        {
            MineDebug_PrintError("Loading mine bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handle for a tile with a mine that was clicked, shown when game is lost
        imageData.mineHit = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_MINEHIT),
                                                 IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                 LR_DEFAULTCOLOR);
        if (NULL == imageData.mineHit)
        {
            MineDebug_PrintError("Loading mine hit bitmap: %i\n", GetLastError());
            status = MINE_ERROR_BITMAP;
            break;
        }

        //Obtain handles for the tile numbers
        status = Mine_SetupNumberImageData();
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_SetupNumberImageData: %i\n", status);
            break;
        }

    } while (bFalse);
    
    return status;
}

/**
    Mine_SetupNumberImageData
*//**
    Load handles to tile number images.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupNumberImageData(VOID)
{
    BOOLEAN    bFalse = FALSE;
    UINT       ix = 0;
    UINT       randArray[MINE_NUM_RANDOM_TILES];
    MINE_ERROR status = MINE_ERROR_SUCCESS;

    do
    {
        //Delete previously obtained handles
        for (ix = 0; ix < 9; ix++)
        {
            if (NULL != imageData.numbers[ix])
            {
                if (0 == DeleteObject((HGDIOBJ) imageData.numbers[ix]))
                {
                    MineDebug_PrintWarning("Unable to delete number %i object\n", ix);
                }

                imageData.numbers[ix] = NULL;
            }
        }

        //For random images create a random permutation and use first 8 elements
        if (MINE_NUMBER_IMAGE_RANDOM == menuData.numberImages)
        {
            for (ix = 0; ix < MINE_NUM_RANDOM_TILES; ix++)
            {
                randArray[ix] = ix;
            }

            status = Mine_RandomPerm(randArray, MINE_NUM_RANDOM_TILES, 8);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_RandomPerm: %i\n", status);
                break;
            }
        }

        for (ix = 0; ix < 9; ix++)
        {
            switch(menuData.numberImages)
            {
            //Load images for normal numbers
            case MINE_NUMBER_IMAGE_NORMAL:
                imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_CLICKED0 + ix),
                                                             IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                             LR_DEFAULTCOLOR);     
                if (NULL == imageData.numbers[ix])
                {
                    MineDebug_PrintError("Loading clicked %i bitmap: %i\n", ix, GetLastError());
                    status = MINE_ERROR_BITMAP;
                    break;
                }
                break;

            //Load images for reversed numbers
            case MINE_NUMBER_IMAGE_REVERSE:
                imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_CLICKED8 - ix),
                                                             IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                             LR_DEFAULTCOLOR);     
                if (NULL == imageData.numbers[ix])
                {
                    MineDebug_PrintError("Loading clicked %i bitmap: %i\n", 8 - ix, GetLastError());
                    status = MINE_ERROR_BITMAP;
                    break;
                }
                break;

            //Load images for solid color numbers
            case MINE_NUMBER_IMAGE_SOLID:
                imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_SOLID0+ix),
                                                             IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                             LR_DEFAULTCOLOR);     
                if (NULL == imageData.numbers[ix])
                {
                    MineDebug_PrintError("Loading solid %i bitmap: %i\n", ix, GetLastError());
                    status = MINE_ERROR_BITMAP;
                    break;
                }
                break;

            //Load random symbols for numbers
            case MINE_NUMBER_IMAGE_RANDOM:
                //Still keep a blank tile as the image for zero mines
                if (0 == ix)
                {
                    imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_CLICKED0),
                                                                 IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                                 LR_DEFAULTCOLOR);     
                    if (NULL == imageData.numbers[ix])
                    {
                        MineDebug_PrintError("Loading clicked 0 bitmap: %i\n", GetLastError());
                        status = MINE_ERROR_BITMAP;
                        break;
                    }
                }
                else
                {
                    imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, 
                                                                 MAKEINTRESOURCEW(IDB_RANDOM0+randArray[ix - 1]),
                                                                 IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                                 LR_DEFAULTCOLOR);     
                    if (NULL == imageData.numbers[ix])
                    {
                        MineDebug_PrintError("Loading random %i bitmap: %i\n", randArray[ix - 1], GetLastError());
                        status = MINE_ERROR_BITMAP;
                        break;
                    }
                }
                break;
                
            //Load blank tile image for all numbers
            case MINE_NUMBER_IMAGE_BLANK:
                imageData.numbers[ix] = (HBITMAP) LoadImageW(hInst, MAKEINTRESOURCEW(IDB_CLICKED0),
                                                             IMAGE_BITMAP, MINE_TILE_PIXELS, MINE_TILE_PIXELS,
                                                             LR_DEFAULTCOLOR);     
                if (NULL == imageData.numbers[ix])
                {
                    MineDebug_PrintError("Loading clicked 0 bitmap: %i\n", GetLastError());
                    status = MINE_ERROR_BITMAP;
                    break;
                }
                break;

            default:
                __assume(0);
                break;

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
    } while (bFalse);

    return status;
}

/**
    Mine_SetupWindow
*//**
    Store information about the coordinates of window components.
*/
VOID
Mine_SetupWindow(VOID)
{
    windowData.clientHeight = MINE_BASE_HEIGHT_PIXELS + MINE_TILE_PIXELS*gameData.height;
    windowData.clientWidth = MINE_BASE_WIDTH_PIXELS + MINE_TILE_PIXELS*gameData.width;

    /** Store coordinates for all decorative white lines. */
    //White lines along left side of window
    windowData.whiteLines[0].top = 0;
    windowData.whiteLines[0].left = 0;
    windowData.whiteLines[0].right = 0;
    windowData.whiteLines[0].bottom = (LONG) windowData.clientHeight;

    windowData.whiteLines[1].top = 0;
    windowData.whiteLines[1].left = 1;
    windowData.whiteLines[1].right = 1;
    windowData.whiteLines[1].bottom = (LONG) windowData.clientHeight;

    windowData.whiteLines[2].top = 0;
    windowData.whiteLines[2].left = 2;
    windowData.whiteLines[2].right = 2;
    windowData.whiteLines[2].bottom = (LONG) windowData.clientHeight;

    //White lines along top of window
    windowData.whiteLines[3].top = 0;
    windowData.whiteLines[3].left = 0;
    windowData.whiteLines[3].right = (LONG) windowData.clientWidth;
    windowData.whiteLines[3].bottom = 0;

    windowData.whiteLines[4].top = 1;
    windowData.whiteLines[4].left = 0;
    windowData.whiteLines[4].right = (LONG) windowData.clientWidth;
    windowData.whiteLines[4].bottom = 1;

    windowData.whiteLines[5].top = 2;
    windowData.whiteLines[5].left = 0;
    windowData.whiteLines[5].right = (LONG) windowData.clientWidth;
    windowData.whiteLines[5].bottom = 2;

    //White lines highlighting remaining mines counter
    windowData.whiteLines[6].top = 39;
    windowData.whiteLines[6].left = 17;
    windowData.whiteLines[6].right = 70;
    windowData.whiteLines[6].bottom = 39;

    windowData.whiteLines[7].top = 16;
    windowData.whiteLines[7].left = 69;
    windowData.whiteLines[7].right = 69;
    windowData.whiteLines[7].bottom = 40;

    //White lines highlighting timer
    windowData.whiteLines[8].top = 39;
    windowData.whiteLines[8].left = (LONG) windowData.clientWidth - 54;
    windowData.whiteLines[8].right = (LONG) windowData.clientWidth - 14;
    windowData.whiteLines[8].bottom = 39;

    windowData.whiteLines[9].top = 16;
    windowData.whiteLines[9].left = (LONG) windowData.clientWidth - 15;
    windowData.whiteLines[9].right = (LONG) windowData.clientWidth - 15;
    windowData.whiteLines[9].bottom = 40;

    //White lines highlighting the top banner region of the window
    windowData.whiteLines[10].top = 44;
    windowData.whiteLines[10].left = 11;
    windowData.whiteLines[10].right = (LONG) windowData.clientWidth - 5;
    windowData.whiteLines[10].bottom = 44;

    windowData.whiteLines[11].top = 45;
    windowData.whiteLines[11].left = 10;
    windowData.whiteLines[11].right = (LONG) windowData.clientWidth - 5;
    windowData.whiteLines[11].bottom = 45;

    windowData.whiteLines[12].top = 11;
    windowData.whiteLines[12].left = (LONG) windowData.clientWidth - 7;
    windowData.whiteLines[12].right = (LONG) windowData.clientWidth - 7;
    windowData.whiteLines[12].bottom = 46;

    windowData.whiteLines[13].top = 10;
    windowData.whiteLines[13].left = (LONG) windowData.clientWidth - 6;
    windowData.whiteLines[13].right = (LONG) windowData.clientWidth - 6;
    windowData.whiteLines[13].bottom = 46;

    //White lines highlighting game board
    windowData.whiteLines[14].top = (LONG) windowData.clientHeight - 8;
    windowData.whiteLines[14].left = 12;
    windowData.whiteLines[14].right = (LONG) windowData.clientWidth - 5;
    windowData.whiteLines[14].bottom = (LONG) windowData.clientHeight - 8;

    windowData.whiteLines[15].top = (LONG) windowData.clientHeight - 7;
    windowData.whiteLines[15].left = 11;
    windowData.whiteLines[15].right = (LONG) windowData.clientWidth - 5;
    windowData.whiteLines[15].bottom = (LONG) windowData.clientHeight - 7;

    windowData.whiteLines[16].top = (LONG) windowData.clientHeight - 6;
    windowData.whiteLines[16].left = 10;
    windowData.whiteLines[16].right = (LONG) windowData.clientWidth - 5;
    windowData.whiteLines[16].bottom = (LONG) windowData.clientHeight - 6;

    windowData.whiteLines[17].top = 55;
    windowData.whiteLines[17].left = (LONG) windowData.clientWidth - 8;
    windowData.whiteLines[17].right = (LONG) windowData.clientWidth - 8;
    windowData.whiteLines[17].bottom = (LONG) windowData.clientHeight - 5;

    windowData.whiteLines[18].top = 54;
    windowData.whiteLines[18].left = (LONG) windowData.clientWidth - 7;
    windowData.whiteLines[18].right = (LONG) windowData.clientWidth - 7;
    windowData.whiteLines[18].bottom = (LONG) windowData.clientHeight - 5;

    windowData.whiteLines[19].top = 53;
    windowData.whiteLines[19].left = (LONG) windowData.clientWidth - 6;
    windowData.whiteLines[19].right = (LONG) windowData.clientWidth - 6;
    windowData.whiteLines[19].bottom = (LONG) windowData.clientHeight - 5;

    /** Store coordinates for all decorative grey lines. */
    //Grey lines highlighting the top banner region of the window
    windowData.greyLines[0].top = 9;
    windowData.greyLines[0].left = 9;
    windowData.greyLines[0].right = 9;
    windowData.greyLines[0].bottom = 45;

    windowData.greyLines[1].top = 9;
    windowData.greyLines[1].left = 10;
    windowData.greyLines[1].right = 10;
    windowData.greyLines[1].bottom = 44;

    
    windowData.greyLines[2].top = 9;
    windowData.greyLines[2].left = 9;
    windowData.greyLines[2].right = (LONG) windowData.clientWidth - 6;
    windowData.greyLines[2].bottom = 9;

    windowData.greyLines[3].top = 10;
    windowData.greyLines[3].left = 9;
    windowData.greyLines[3].right = (LONG) windowData.clientWidth - 7;
    windowData.greyLines[3].bottom = 10;

    //Grey lines highlighting remaining mines counter
    windowData.greyLines[4].top = 15;
    windowData.greyLines[4].left = 16;
    windowData.greyLines[4].right = 16;
    windowData.greyLines[4].bottom = 39;

    windowData.greyLines[5].top = 15;
    windowData.greyLines[5].left = 16;
    windowData.greyLines[5].right = 69;
    windowData.greyLines[5].bottom = 15;

    //Grey lines highlighting timer
    windowData.greyLines[6].top = 15;
    windowData.greyLines[6].left = (LONG) windowData.clientWidth - 55;
    windowData.greyLines[6].right = (LONG) windowData.clientWidth - 55;
    windowData.greyLines[6].bottom = 39;

    windowData.greyLines[7].top = 15;
    windowData.greyLines[7].left = (LONG) windowData.clientWidth - 55;
    windowData.greyLines[7].right = (LONG) windowData.clientWidth - 15;
    windowData.greyLines[7].bottom = 15;

    //Grey lines highlighting game board
    windowData.greyLines[8].top = 52;
    windowData.greyLines[8].left = 9;
    windowData.greyLines[8].right = 9;
    windowData.greyLines[8].bottom = (LONG) windowData.clientHeight - 6;

    windowData.greyLines[9].top = 52;
    windowData.greyLines[9].left = 10;
    windowData.greyLines[9].right = 10;
    windowData.greyLines[9].bottom = (LONG) windowData.clientHeight - 7;

    windowData.greyLines[10].top = 52;
    windowData.greyLines[10].left = 11;
    windowData.greyLines[10].right = 11;
    windowData.greyLines[10].bottom = (LONG) windowData.clientHeight - 8;

    windowData.greyLines[11].top = 52;
    windowData.greyLines[11].left = 9;
    windowData.greyLines[11].right = (LONG) windowData.clientWidth - 6;
    windowData.greyLines[11].bottom = 52;

    windowData.greyLines[12].top = 53;
    windowData.greyLines[12].left = 9;
    windowData.greyLines[12].right = (LONG) windowData.clientWidth - 7;
    windowData.greyLines[12].bottom = 53;

    windowData.greyLines[13].top = 54;
    windowData.greyLines[13].left = 9;
    windowData.greyLines[13].right = (LONG) windowData.clientWidth - 8;
    windowData.greyLines[13].bottom = 54;

    //Store boundary of timer
    windowData.timerRegion.top = 16;
    windowData.timerRegion.left = (LONG) windowData.clientWidth - 54;
    windowData.timerRegion.right = (LONG) windowData.clientWidth - 15;
    windowData.timerRegion.bottom = 39;

    //Store boundary of remaining mines counter
    windowData.mineCountRegion.top = 16;
    windowData.mineCountRegion.left = 17;
    windowData.mineCountRegion.right = 69;
    windowData.mineCountRegion.bottom = 39;

    //Store boundary of smiley face
    windowData.faceRegion.top = 15;
    windowData.faceRegion.left = ((LONG) windowData.clientWidth / 2) - 11;
    windowData.faceRegion.right = ((LONG) windowData.clientWidth / 2) + 15;
    windowData.faceRegion.bottom = 41;

    //Store boundary of top banner region
    windowData.topBannerRegion.top = 11;
    windowData.topBannerRegion.left = 11;
    windowData.topBannerRegion.right = (LONG) windowData.clientWidth - 7;
    windowData.topBannerRegion.bottom = 44;

    //Store boundary of game board
    windowData.boardRegion.top = 55;
    windowData.boardRegion.left = 12;
    windowData.boardRegion.right = (LONG) windowData.clientWidth - 8;
    windowData.boardRegion.bottom = (LONG) windowData.clientHeight - 8;

    return;
}

/**
    WndProc
*//**
    Window procedure for main window in Minesweeper Deluxe project.

    @param[in] hWnd    - Handle to the window recieving the message.
    @param[in] message - Message to be processed.
    @param[in] wParam  - Message specific parameter.
    @param[in] lParam  - Message specific parameter.

    @return Return value depends on message that was processed.
*/
LRESULT CALLBACK
WndProc(_In_ HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR           dialogReturn = 0;
    static BOOLEAN    errorOccurred = FALSE;
    static int        functionEntryCount = 0;
    static HMENU      hMenu = NULL;
    DWORD             oldHeight = 0;
    DWORD             oldWidth = 0;
    static UINT       prevMenuGame = 0;
    static UINT       prevMenuImages = 0;
    LRESULT           returnValue = 0;
    HINSTANCE         shellExecuteReturn = 0;
    //Keep status as static variable. Set if an error is encountered processing 
    //a message, return as quit message when window is destroyed.
    static MINE_ERROR status = MINE_ERROR_SUCCESS;
    RECT              windowSize;
    UINT              wmId = 0;
    short             xMouse = 0;
    short             yMouse = 0;

    functionEntryCount += 1;

    switch (message)
    {
    /** Process WM_COMMAND message... */
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        /** Handle menu options. */
        switch (wmId)
        {
        //Start new game
        case IDM_NEW:
            status = Mine_SetupGame();
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_SetupGame: %i\n", (int) status);
                errorOccurred = TRUE;
                break;
            }
            if (0 == InvalidateRect(hWnd, NULL, FALSE))
            {
                MineDebug_PrintWarning("Unable to invalidate rectangle\n");
            }
            break;

        //Change to a non-custom level
        case IDM_BEGINNER:     /* Fall through */
        case IDM_INTERMEDIATE: /* Fall through */
        case IDM_EXPERT:
            //If level hasn't changed, do nothing
            if (wmId == prevMenuGame)
            {
                break;
            }

            //Uncheck old menu option and check new menu option
            if (-1 == CheckMenuItem(hMenu, prevMenuGame, MF_BYCOMMAND | MF_UNCHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }
            prevMenuGame = wmId;
            if (-1 == CheckMenuItem(hMenu, prevMenuGame, MF_BYCOMMAND | MF_CHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }
            
            menuData.gameLevel = ((IDM_BEGINNER == wmId) ? (DWORD) MINE_LEVEL_BEGINNER : 
                                 ((IDM_EXPERT   == wmId) ? (DWORD) MINE_LEVEL_EXPERT : 
                                                           (DWORD) MINE_LEVEL_INTERMEDIATE));

            //Store old height and width to assist in changing window size
            oldHeight = gameData.height;
            oldWidth =  gameData.width;

            //Send message to start a new game
            (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);

            //Determine the new coordinates for all the window elements
            Mine_SetupWindow();
            if (0 == GetWindowRect(hWnd, &windowSize))
            {
                status = MINE_ERROR_GET_DATA;
                MineDebug_PrintError("Getting window rectangle: %lu\n", GetLastError());
                errorOccurred = TRUE;
                break;
            }
            //Change in window size can be derived from change in board size
            windowSize.right += (gameData.width - oldWidth) * MINE_TILE_PIXELS;
            windowSize.bottom += (gameData.height - oldHeight) * MINE_TILE_PIXELS;

            //Resize board
            if (0 == MoveWindow(hWnd, windowSize.left, windowSize.top, 
                                windowSize.right - windowSize.left,
                                windowSize.bottom - windowSize.top, TRUE))
            {
                status = MINE_ERROR_RESIZE;
                MineDebug_PrintError("Moving the window: %lu\n", GetLastError());
                errorOccurred = TRUE;
                break;
            }

            //Store change in registry so it can persist when game is closed
            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"DefaultLevel", menuData.gameLevel))
            {
                MineDebug_PrintWarning("Unable to change game level in registry\n");
            }
            break;

        //Change to a custom size level
        case IDM_CUSTOM:
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCE(IDD_CUSTOM), hWnd, MineCustom_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show custom dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show custom dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }
            //Only change to custom level if OK and not cancel was selcted
            else if (IDOK == dialogReturn)
            {
                //Uncheck old menu option and check new menu option
                if (-1 == CheckMenuItem(hMenu, prevMenuGame, MF_BYCOMMAND | MF_UNCHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
                prevMenuGame = IDM_CUSTOM;
                if (-1 == CheckMenuItem(hMenu, prevMenuGame, MF_BYCOMMAND | MF_CHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
            
                menuData.gameLevel = MINE_LEVEL_CUSTOM;

                //Store old height and width to assist in changing window size
                oldHeight = gameData.height;
                oldWidth =  gameData.width;

                //Send message to start a new game
                (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);

                //Determine the new coordinates for all the window elements
                Mine_SetupWindow();
                if (0 == GetWindowRect(hWnd, &windowSize))
                {
                    status = MINE_ERROR_GET_DATA;
                    MineDebug_PrintError("Getting window rectangle: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }
                //Change in window size can be derived from change in board size
                windowSize.right += (gameData.width - oldWidth) * MINE_TILE_PIXELS;
                windowSize.bottom += (gameData.height - oldHeight) * MINE_TILE_PIXELS;

                //Resize board
                if (0 == MoveWindow(hWnd, windowSize.left, windowSize.top, 
                                    windowSize.right - windowSize.left,
                                    windowSize.bottom - windowSize.top, TRUE))
                {
                    status = MINE_ERROR_RESIZE;
                    MineDebug_PrintError("Moving the window: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }

                //Store change in registry so it can persist when game is closed
                if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"DefaultLevel", menuData.gameLevel))
                {
                    MineDebug_PrintWarning("Unable to change game level in registry\n");
                }
            }
            break;

        //Show the best times and names for finishing standard levels
        case IDM_BEST_TIMES:
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_BESTTIMES), hWnd, MineBestTimes_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show best times dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show best times dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }
            break;

        //Exit the program
        case IDM_EXIT:
            if (0 == DestroyWindow(hWnd))
            {
                MineDebug_PrintWarning("Unable to destroy window: %lu\n", GetLastError());
            }
            break;

        //Change the images displayed for the numbers
        case IDM_NORMAL:  /* Fall through */
        case IDM_REVERSE: /* Fall through */
        case IDM_SOLID:   /* Fall through */
        case IDM_RANDOM:  /* Fall through */
        case IDM_BLANK:
            //If number images haven't changed, do nothing
            if (wmId == prevMenuImages)
            {
                break;
            }

            //Uncheck old image option and check new image option
            if (-1 == CheckMenuItem(hMenu, prevMenuImages, MF_BYCOMMAND | MF_UNCHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }
            prevMenuImages = wmId;
            if (-1 == CheckMenuItem(hMenu, prevMenuImages, MF_BYCOMMAND | MF_CHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }

            menuData.numberImages = ( (IDM_NORMAL  == wmId) ? (DWORD) MINE_NUMBER_IMAGE_NORMAL  :
                                     ((IDM_REVERSE == wmId) ? (DWORD) MINE_NUMBER_IMAGE_REVERSE :
                                     ((IDM_SOLID   == wmId) ? (DWORD) MINE_NUMBER_IMAGE_SOLID   :
                                     ((IDM_RANDOM  == wmId) ? (DWORD) MINE_NUMBER_IMAGE_RANDOM  :
                                                              (DWORD) MINE_NUMBER_IMAGE_BLANK))));

            //Obtain new handles for the new number image bitmaps
            status = Mine_SetupNumberImageData();
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function Mine_SetupNumberImageData: %i\n", (int) status);
                errorOccurred = TRUE;
                break;
            }

            //Send message to start a new game
            (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);

            //Store change in registry so it can persist when game is closed
            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"NumberImages", menuData.numberImages))
            {
                MineDebug_PrintWarning("Unable to change number images in registry\n");
            }
            break;

        //Switch if the board wraps in the horizontal direction
        case IDM_HORIZONTAL:
            if (menuData.wrapHorz)
            {
                if (-1 == CheckMenuItem(hMenu, IDM_HORIZONTAL, MF_BYCOMMAND | MF_UNCHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
                menuData.wrapHorz = FALSE;
            }
            else
            {
                if (-1 == CheckMenuItem(hMenu, IDM_HORIZONTAL, MF_BYCOMMAND | MF_CHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
                menuData.wrapHorz = TRUE;
            }

            //Send message to start a new game
            (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);

            //Store change in registry so it can persist when game is closed
            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"WrapHorz", menuData.wrapHorz))
            {
                MineDebug_PrintWarning("Unable to change horizontal wrap in registry\n");
            }
            break;

        //Switch if the board wraps in the vertical direction
        case IDM_VERTICAL:
            if (menuData.wrapVert)
            {
                if (-1 == CheckMenuItem(hMenu, IDM_VERTICAL, MF_BYCOMMAND | MF_UNCHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
                menuData.wrapVert = FALSE;
            }
            else
            {
                if (-1 == CheckMenuItem(hMenu, IDM_VERTICAL, MF_BYCOMMAND | MF_CHECKED))
                {
                    MineDebug_PrintWarning("Menu item to check doesn't exist\n");
                }
                menuData.wrapVert = TRUE;
            }

            //Send message to start a new game
            (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);

            //Store change in registry so it can persist when game is closed
            if (MINE_ERROR_SUCCESS != Mine_SetRegDword(L"WrapVert", menuData.wrapVert))
            {
                MineDebug_PrintWarning("Unable to change vertical wrap in registry\n");
            }
            break;

        //Change settings for movement of mines
        case IDM_MOVEMENT:
            //Show dialog with movement options
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCE(IDD_MOVEMENT), hWnd, MineMovement_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show movement dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show movement dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }
            //Only start a new game if OK was selcted
            else if (IDOK == dialogReturn)
            {
                //Send message to start a new game
                (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);                     
            }
            break;

        //Show the online help document
        case IDM_HELP:
            shellExecuteReturn = ShellExecuteW(hwnd, L"open", L"http://www.example.com", 
                                               NULL, NULL, 0);
            if ((int) shellExecuteReturn <= 32)
            {
                MineDebug_PrintWarning("Unable to open www.example.com: %i\n", 
                                       (int) shellExecuteReturn);
            }
            break;

        //Display the about dialog
        case IDM_ABOUT:
            dialogReturn = DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, MineAbout_Dialog);
            if ((0 == dialogReturn) || (-1 == dialogReturn))
            {
                MineDebug_PrintWarning("From system trying to show about dialog: %lu\n", GetLastError());
                break;
            }
            else if (MINE_DIALOG_ERROR_OFFSET < dialogReturn)
            {
                MineDebug_PrintWarning("Trying to show about dialog: %i\n", 
                                       (int) (dialogReturn - MINE_DIALOG_ERROR_OFFSET));
                break;
            }
            break;

        //Send all other WM_COMMAND messages to the default window procedure
        default:
            returnValue = DefWindowProcW(hWnd, message, wParam, lParam);
            break;
        }
        break;

    /** Process WM_CREATE message... */
    case WM_CREATE:
        hMenu = GetMenu(hWnd);
        if (NULL == hMenu)
        {
            status = MINE_ERROR_MENU;
            MineDebug_PrintError("Getting menu handle\n");
            //Don't set errorOccurred since the window won't be created.
            returnValue = (LRESULT) -1;
            break;
        }

        /** Set check marks for game level and number images in the menu. */
        switch (menuData.gameLevel)
        {
        case MINE_LEVEL_BEGINNER:
            prevMenuGame = IDM_BEGINNER;
            break;
        case MINE_LEVEL_INTERMEDIATE:
            prevMenuGame = IDM_INTERMEDIATE;
            break;
        case MINE_LEVEL_EXPERT:
            prevMenuGame = IDM_EXPERT;
            break;
        case MINE_LEVEL_CUSTOM:
            prevMenuGame = IDM_CUSTOM;
            break;
        default:
            __assume(0);
            break;
        }

        if (-1 == CheckMenuItem(hMenu, prevMenuGame, MF_BYCOMMAND | MF_CHECKED))
        {
            MineDebug_PrintWarning("Menu item to check doesn't exist\n");
        }

        switch (menuData.numberImages)
        {
        case MINE_NUMBER_IMAGE_NORMAL:
            prevMenuImages = IDM_NORMAL;
            break;
        case MINE_NUMBER_IMAGE_REVERSE:
            prevMenuImages = IDM_REVERSE;
            break;
        case MINE_NUMBER_IMAGE_SOLID:
            prevMenuImages = IDM_SOLID;
            break;
        case MINE_NUMBER_IMAGE_RANDOM:
            prevMenuImages = IDM_RANDOM;
            break;
        case MINE_NUMBER_IMAGE_BLANK:
            prevMenuImages = IDM_BLANK;
            break;
        default:
            __assume(0);
            break;
        }

        if (-1 == CheckMenuItem(hMenu, prevMenuImages, MF_BYCOMMAND | MF_CHECKED))
        {
            MineDebug_PrintWarning("Menu item to check doesn't exist\n");
        }

        /** Set check marks for horizontal and vertical wrapping in menu. */
        if (menuData.wrapHorz)
        {
            if (-1 == CheckMenuItem(hMenu, IDM_HORIZONTAL, MF_BYCOMMAND | MF_CHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }
        }

        if (menuData.wrapVert)
        {
            if (-1 == CheckMenuItem(hMenu, IDM_VERTICAL, MF_BYCOMMAND | MF_CHECKED))
            {
                MineDebug_PrintWarning("Menu item to check doesn't exist\n");
            }
        }
        break;

    /** Process WM_DESTROY message... */
    case WM_DESTROY:

        /** Clean up timers. */
        //Timers have to be deleted while window still exists
        if (0 == KillTimer(hWnd, MINE_TIMER_CLOCK))
        {
            MineDebug_PrintWarning("Unable to kill timer: %lu\n", GetLastError());
        }

        //Movement timer exists if movementTimerCreated flag is set
        if (movementTimerCreated)
        {
            if (0 == KillTimer(hwnd, MINE_TIMER_MOVE))
            {
                MineDebug_PrintWarning("Unable to kill movement timer: %lu\n", GetLastError());
            }
        }

        if (errorOccurred && (MINE_ERROR_SUCCESS == status))
        {
            status = MINE_ERROR_UNKNOWN;
        }

        /** Destroy window and relay last set status as quit message. */
        PostQuitMessage((int) status);

        break;

    /** Process WM_KEYDOWN message... */
    case WM_KEYDOWN:

        //Store coordinates of previously held tile before coordinates shift to keep 
        //held tile on same spot of the board after the scroll
        xMouse = (short) (windowData.boardRegion.left + gameData.prevGridX*MINE_TILE_PIXELS);
        yMouse = (short) (windowData.boardRegion.top  + gameData.prevGridY*MINE_TILE_PIXELS);
        switch (wParam)
        {
         /** Scroll the board if an arrow key is pressed while in a wrap mode. */
        case VK_DOWN:
            if (menuData.wrapVert)
            {
                //Update the vertical shift amount
                gameData.vertShift = (gameData.vertShift + 1) % (LONG) gameData.height;
                if (-1 != gameData.prevGridY)
                {
                    //Translate held tile to its new grid coordinates
                    gameData.prevGridY = (gameData.prevGridY + (LONG) gameData.height - 1) % (LONG) gameData.height;
                }
                //Scroll the game board
                if (ERROR == ScrollWindowEx(hWnd, 0, (-1)*MINE_TILE_PIXELS, &windowData.boardRegion,
                                            &windowData.boardRegion, NULL, NULL, SW_INVALIDATE))
                {
                    status = MINE_ERROR_PAINT;
                    MineDebug_PrintError("Scrolling window up: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }
            }
            break;
        case VK_UP:
            if (menuData.wrapVert)
            {
                gameData.vertShift = (gameData.vertShift + (LONG) gameData.height - 1) % (LONG) gameData.height;
                if (-1 != gameData.prevGridY)
                {
                    gameData.prevGridY = (gameData.prevGridY + 1) % (LONG) gameData.height;
                }
                if (ERROR == ScrollWindowEx(hWnd, 0, MINE_TILE_PIXELS, &windowData.boardRegion,
                                            &windowData.boardRegion, NULL, NULL, SW_INVALIDATE))
                {
                    status = MINE_ERROR_PAINT;
                    MineDebug_PrintError("Scrolling window down: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }
            }
            break;
        case VK_RIGHT:
            if (menuData.wrapHorz)
            {
                gameData.horzShift = (gameData.horzShift + 1) % (LONG) gameData.width;
                if (-1 != gameData.prevGridX)
                {
                    gameData.prevGridX = (gameData.prevGridX + (LONG) gameData.width - 1) % (LONG) gameData.width;
                }
                if (ERROR == ScrollWindowEx(hWnd, (-1)*MINE_TILE_PIXELS, 0, &windowData.boardRegion,
                                            &windowData.boardRegion, NULL, NULL, SW_INVALIDATE))
                {
                    status = MINE_ERROR_PAINT;
                    MineDebug_PrintError("Scrolling window left: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }
            }
            break;
        case VK_LEFT:
            if (menuData.wrapHorz)
            {
                gameData.horzShift = (gameData.horzShift + (LONG) gameData.width - 1) % (LONG) gameData.width;
                if (-1 != gameData.prevGridX)
                {
                    gameData.prevGridX = (gameData.prevGridX + 1) % (LONG) gameData.width;
                }
                if (ERROR == ScrollWindowEx(hWnd, MINE_TILE_PIXELS, 0, &windowData.boardRegion,
                                            &windowData.boardRegion, NULL, NULL, SW_INVALIDATE))
                {
                    status = MINE_ERROR_PAINT;
                    MineDebug_PrintError("Scrolling window right: %lu\n", GetLastError());
                    errorOccurred = TRUE;
                    break;
                }
            }
        default:
            //Send all other key presses to default window procedure
            returnValue = DefWindowProcW(hWnd, message, wParam, lParam);
            break;
        }
        //If a tile is held, send a message in order to keep held tile in same spot on board
        if ((-1 != gameData.prevGridX) && (-1 != gameData.prevGridY))
        {
            (void) SendMessage(hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(xMouse, yMouse));
        }
        break;

    /** Process WM_LBUTTONDOWN message... */
    case WM_LBUTTONDOWN:
        xMouse = GET_X_LPARAM(lParam);
        yMouse = GET_Y_LPARAM(lParam);
        if ((!gameData.gameOver) && Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            /** If right button is down, start a double click. */
            if (gameData.rightDown)
            {
                status = MineMouse_StartDoubleClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_StartDoubleClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
            /** If right button is not down, start a left click. */
            else
            {
                status = MineMouse_StartLeftClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_StartLeftClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
        }

        if (0 == InvalidateRect(hWnd, &(windowData.faceRegion), FALSE))
        {
            MineDebug_PrintWarning("Unable to invalidate rectangle\n");
        }
        (void) SetCapture(hWnd);
        //Store variables for which mouse buttons are pressed instead of using wParam value
        //for proper behavior when pressing both buttons simultaneously, releasing the right,
        //and then moving the mouse to a different tile.
        gameData.leftDown = TRUE;
        break;

    /** Process WM_LBUTTONUP message... */
    case WM_LBUTTONUP:
        xMouse = GET_X_LPARAM(lParam);
        yMouse = GET_Y_LPARAM(lParam);
        /** If mouse was over face, start a new game. */
        if (Mine_PointInRect(xMouse, yMouse, &windowData.faceRegion))
        {
            (void) SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDM_NEW, 0), 0);
        }
        else if((!gameData.gameOver) && gameData.leftDown && 
                Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            /** If right button was down, process a double click. */
            if (gameData.rightDown)
            {
                status = MineMouse_ProcessDoubleClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_ProcessDoubleClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
            /** If right button was not down, process a left click. */
            else
            {
                status = MineMouse_ProcessLeftClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_ProcessLeftClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
        }

        if (0 == InvalidateRect(hWnd, &(windowData.faceRegion), FALSE))
        {
            MineDebug_PrintWarning("Unable to invalidate rectangle\n");
        }
        if (0 == ReleaseCapture())
        {
            MineDebug_PrintWarning("Unable to release mouse capture: %lu\n", GetLastError());
        }
        //Releasing either mouse button counts as releasing both mouse buttons
        gameData.leftDown = FALSE;
        gameData.rightDown = FALSE;
        break;

    /** Process WM_MOUSEMOVE message... */
    case WM_MOUSEMOVE:
        if ((!gameData.gameOver) && gameData.leftDown)
        {
            xMouse = GET_X_LPARAM(lParam);
            yMouse = GET_Y_LPARAM(lParam);
            /** If right button is down, move a double click. */
            if (gameData.rightDown)
            {
                status = MineMouse_MoveDoubleClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_MoveDoubleClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
            /** If right button is not down, move a left click. */
            else
            {
                status = MineMouse_MoveLeftClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_MoveLeftClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
        }
        break;
    
    /** Process WM_PAINT message... */
    case WM_PAINT:
        //Double check there is a region that needs to be updated
        if (0 == GetUpdateRect(hWnd, NULL, FALSE))
        {
            break;
        }

        /** Paint all elements onto the screen. */
        status = Mine_PaintScreen(hWnd);
        if (MINE_ERROR_SUCCESS != status)
        {
            MineDebug_PrintError("In function Mine_PaintScreen: %i\n", (int) status);
            errorOccurred = TRUE;
            break;
        }
        break;

    /** Process WM_RBUTTONDOWN message... */
    case WM_RBUTTONDOWN:
        xMouse = GET_X_LPARAM(lParam);
        yMouse = GET_Y_LPARAM(lParam);
        if ((!gameData.gameOver) && Mine_PointInRect(xMouse, yMouse, &windowData.boardRegion))
        {
            /** If left button is down, start a double click. */
            if (gameData.leftDown)
            {
                status = MineMouse_StartDoubleClick(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_StartDoubleClick: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
            /** If left button is not down, process a right click. */
            else
            {
                status = MineMouse_ProcessRightDown(xMouse, yMouse);
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMouse_ProcessRightDown: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
        }

        (void) SetCapture(hWnd);
        gameData.rightDown = TRUE;
        break;

    /** Process WM_RBUTTONUP message... */
    case WM_RBUTTONUP:
        /** If left button was down, process a double click. */
        if(gameData.leftDown)
        {
            xMouse = GET_X_LPARAM(lParam);
            yMouse = GET_Y_LPARAM(lParam);
            status = MineMouse_ProcessDoubleClick(xMouse, yMouse);
            if (MINE_ERROR_SUCCESS != status)
            {
                MineDebug_PrintError("In function MineMoue_ProcessDoubleClick: %i\n", (int) status);
                errorOccurred = TRUE;
                break;
            }
            gameData.leftDown = FALSE;
        }

        if (0 == InvalidateRect(hWnd, &(windowData.faceRegion), FALSE))
        {
            MineDebug_PrintWarning("Unable to invalidate rectangle\n");
        }
        if (0 == ReleaseCapture())
        {
            MineDebug_PrintWarning("Unable to release mouse capture: %lu\n", GetLastError());
        }
        gameData.rightDown = FALSE;
        break;

    /** Process WM_TIMER message... */
    case WM_TIMER:
        if (gameData.gameStarted && (!gameData.gameOver))
        {
            /** Update the clock if the time has not already reached the maximum value. */
            if ((MINE_TIMER_CLOCK == wParam) && (gameData.time < MINE_MAX_TIME))
            {
                Mine_ProcessTimer();
            }
            else if (MINE_TIMER_MOVE == wParam)
            {
                status = MineMovement_ProcessMovement();
                if (MINE_ERROR_SUCCESS != status)
                {
                    MineDebug_PrintError("In function MineMovement_ProcessMovement: %i\n", (int) status);
                    errorOccurred = TRUE;
                    break;
                }
            }
        } 
        break;

    //Process all other messages by calling the default window procedure
    default:
        returnValue = DefWindowProcW(hWnd, message, wParam, lParam);
        break;
    }

    functionEntryCount -= 1;

    //Upon error, destroy window when we are about to exit the window procedure,
    //and we are not recursively in another call of the windows procedure.
    if (errorOccurred && (0 == functionEntryCount))
    {
        //Set higher in case DestroyWindow creates a message.  Don't want to re-enter.
        functionEntryCount += 1;

        if (0 == DestroyWindow(hWnd))
        {
            MineDebug_PrintWarning("Unable to destroy window: %lu\n", GetLastError());
        }
    }

    return returnValue;
}