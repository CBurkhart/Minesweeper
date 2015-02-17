/**
    @file Mine.h

    @author Craig Burkhart

    @brief Header file for Minesweeper Deluxe project.
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

#include "resource.h"
#include "stdafx.h"

//--------------------------------------------------------------
//    Macros
//--------------------------------------------------------------

/** Numeric identifier for beginner level. */
#define MINE_LEVEL_BEGINNER     1
/** Numeric idetifier for intermediate level. */
#define MINE_LEVEL_INTERMEDIATE 2
/** Numeric identifier for expert level. */
#define MINE_LEVEL_EXPERT       3
/** Numeric identifier for custom sized level. */
#define MINE_LEVEL_CUSTOM       4

/** Height (in tiles) of beginner level. */
#define MINE_BEGINNER_HEIGHT 9
/** Width (in tiles) of beginner level. */
#define MINE_BEGINNER_WIDTH  9
/** Number of mines in beginner level. */
#define MINE_BEGINNER_MINES 10

/** Height (in tiles) of intermediate level. */
#define MINE_INTERMEDIATE_HEIGHT 16
/** Widht (in tiles) of intermediate level. */
#define MINE_INTERMEDIATE_WIDTH  16
/** Number of mines in intermediate level. */
#define MINE_INTERMEDIATE_MINES  40

/** Height (in tiles) of expert level. */
#define MINE_EXPERT_HEIGHT 16
/** Width (in tiles) of expert level. */
#define MINE_EXPERT_WIDTH  30
/** Number of mines in expert level. */
#define MINE_EXPERT_MINES  99

//Maximum size for custom level is twice the 
//maximum from the original minewsweeper game
/** Maximum height (in tiles) for custom level. */
#define MINE_MAX_HEIGHT 48
/** Maximum width (in tiles) for custom level. */
#define MINE_MAX_WIDTH  60 

/** Maximum number of seconds on the timer. */
#define MINE_MAX_TIME 999

/** Maximum number of characters in high score name .*/
#define MINE_NAME_BUFFER_CHARS 15
/** Number of characters needed to display best time info. */
#define MINE_BESTTIME_BUFFER_CHARS 12

/** Wide string version of IDC_ARROW. */
#define IDC_ARROWW MAKEINTRESOURCEW(32512)
/** Maximum number of characters for string resources. */
#define MINE_LOADSTRING_MAX_CHARS 20

/** Default high score name. */
#define MINE_ANONYMOUS_STRING L"Anonymous"
/** Number of bytes in wide string "Anonymous". */
#define MINE_ANONYMOUS_SIZE 20

/** Numeric identifier indicating tile is a mine. */
#define MINE_BOMB_VALUE ((CHAR) -1)

/** Number of pixels in height and width of a tile. */
#define MINE_TILE_PIXELS  16 
/** Number of pixels in height of a timer number. */
#define MINE_TIMER_HEIGHT 23
/** Number of pixels in width of a timer number. */
#define MINE_TIMER_WIDTH  13
/** Number of pixels in height of smiley face. */
#define MINE_FACE_HEIGHT  26
/** Number of pixels in width of smiley face. */
#define MINE_FACE_WIDTH   26

/** Extra pixels in window width beyond size of board. */
#define MINE_BASE_WIDTH_PIXELS  20
/** Extra pixels in window height beyond size of board. */
#define MINE_BASE_HEIGHT_PIXELS 63

/** Number of milliseconds between clock update times. */
#define MINE_CLOCK_UPDATE_TIME 250

/** Identifier for clock timer. */
#define MINE_TIMER_CLOCK 1
/** Identifier for movement timer. */
#define MINE_TIMER_MOVE  2

/** Numeric identifier indicating tile is unclicked. */
#define MINE_TILE_STATUS_NORMAL   0
/** Numeric identifier indicating tile has been revealed. */
#define MINE_TILE_STATUS_REVEALED 1
/** Numeric identifier indicating tile is flagged. */
#define MINE_TILE_STATUS_FLAG     2
/** Numeric identifier indicating tile is currently held down. */
#define MINE_TILE_STATUS_HELD     3

/** Number of white lines needed to draw game window. */
#define MINE_NUM_WHITE_LINES 20
/** Number of grey lines needed to draw game window. */
#define MINE_NUM_GREY_LINES  14

/** Numeric identifier for normal number images. */
#define MINE_NUMBER_IMAGE_NORMAL  0
/** Numeric identifier for reversed number images. */
#define MINE_NUMBER_IMAGE_REVERSE 1 
/** Numeric identifier for solid color number images. */
#define MINE_NUMBER_IMAGE_SOLID   2
/** Numeric identifier for random symbol number images. */
#define MINE_NUMBER_IMAGE_RANDOM  3
/** Numeric identifier for blank number images. */
#define MINE_NUMBER_IMAGE_BLANK   4

/** Number of symbols for random number images. */
#define MINE_NUM_RANDOM_TILES 16

/** Number of milliseconds in a second. */
#define MINE_SECOND 1000
/** Number of milliseconds in a quarter of a second. */
#define MINE_QUARTER_SECOND 250

/** Default movement aggressiveness value. */
#define MINE_MOVEMENT_DEFAULT_AGGRESSIVENESS 5

/** Offset for returning a MINE_ERROR from a dialog window. */
#define MINE_DIALOG_ERROR_OFFSET 200

/** Macro to convert x and y grid position of tile into array index. */
#define MINE_INDEX(x,y) (((((LONG)(x))+gameData.horzShift)%((LONG)gameData.width))+\
   (((((LONG)(y))+gameData.vertShift)%((LONG)gameData.height))*((LONG)gameData.width)))

//--------------------------------------------------------------
//    Enumerations
//--------------------------------------------------------------

enum _MINE_ERROR
{
    /** Function completed successfully. */
    MINE_ERROR_SUCCESS = 0,
    /** Error loading keyboard accelerators. */
    MINE_ERROR_ACCELERATORS,
    /** Error loading a bitmap. */
    MINE_ERROR_BITMAP,
    /** Error in a common control. */
    MINE_ERROR_CONTROL,
    /** Error creating a new window. */
    MINE_ERROR_CREATE_WINDOW,
    /** Error related to cryptographic context. */
    MINE_ERROR_CRYPT,
    /** Error obtaing a handle to a device context. */
    MINE_ERROR_DC,
    /** Error obtaining information from Windows OS. */
    MINE_ERROR_GET_DATA,
    /** Error related to the process heap. */
    MINE_ERROR_HEAP,
    /** Error allocating memory. */
    MINE_ERROR_MEMORY,
    /** Error getting a handle to the menu. */ 
    MINE_ERROR_MENU,
    /** Error getting the next window message. */
    MINE_ERROR_MESSAGE,
    /** Error loading a resource. */
    MINE_ERROR_OBJECT,
    /** Error due to invalid parameter passed to a function. */
    MINE_ERROR_PARAMETER,
    /** Error displaying the window on the screen. */
    MINE_ERROR_PAINT,
    /** Error generating a random number. */
    MINE_ERROR_RANDNUMBER,
    /** Error registering a window class. */
    MINE_ERROR_REGISTER_CLASS,
    /** Error opening or manipulating the registry. */
    MINE_ERROR_REGISTRY,
    /** Error resizing the window. */
    MINE_ERROR_RESIZE,
    /** Error getting a resource. */
    MINE_ERROR_RESOURCE,
    /** Error manipulating a string. */
    MINE_ERROR_STRING,
    /** Error of unknown origin in windows procedure. */
    MINE_ERROR_UNKNOWN
};

//--------------------------------------------------------------
//    Structures
//--------------------------------------------------------------

struct _MINE_GLOBAL_SETTINGS
{
    /** Level of game being played. */
    DWORD  gameLevel;
    /** Height (in tiles) of custom level. */
    DWORD  customHeight;
    /** Width (in tiles) of custom level. */
    DWORD  customWidth;
    /** Number of mines in custom level. */
    DWORD  customMines;
    /** Best time for solving beginner level. */
    DWORD  beginnerTime;
    /** Name of best time holder for beginner level. */
    LPWSTR beginnerName;
    /** Best time for solving intermediate level. */
    DWORD  intermediateTime;
    /** Name of best time holder for intermediate level. */
    LPWSTR intermediateName;
    /** Best time for solving expert level. */
    DWORD  expertTime;
    /** Name of best time holder for expert level. */
    LPWSTR expertName;
    /** Flag to determine if board is wrapped horizontally. */
    DWORD  wrapHorz;
    /** Flag to determine if board is wrapped vertically. */
    DWORD  wrapVert;
    /** Type of images to use for numbers. */
    DWORD  numberImages;
    /** Flag to determine if mines move. */
    DWORD  useMovement;
    /** Mine movement frequency in milliseconds. */
    DWORD  movementFreq;
    /** Mine movement aggressiveness. */
    DWORD  movementAggressive;
};

struct _MINE_GAME_SETTINGS
{
    /** Height (in tiles) of game board. */
    DWORD     height;
    /** Width (in tiles) of game board. */
    DWORD     width;
    /** Number of mines in game. */
    DWORD     mines;
    /** Number of flags currently placed on board. */
    DWORD     numFlagged;
    /** Number of tiles that have been uncovered as non-mines. */
    DWORD     numUncovered;
    /** Array of board showing mine locations and numbers. */
    CHAR*     gameBoard;
    /** Array of board showing clicked/flagged status. */
    CHAR*     tileStatus;
    /** Time (in seconds) game has been played. */
    UINT      time;
    /** Time (in milliseconds since computer start) of game start. */
    ULONGLONG gameStartTime;
    /** X position (in grid coordinates) of last mouse press. */
    LONG      prevGridX;
    /** Y position (in grid coordinates) or last mouse press. */
    LONG      prevGridY;
    /** Horizontal shift due to arrow key presses. */
    LONG      horzShift;
    /** Vertical shift due to arrow key presses. */
    LONG      vertShift;
    /** Flag for if the game has been started. */
    BOOLEAN   gameStarted;
    /** Flag for if the game has been finished. */
    BOOLEAN   gameOver;
    /** Flag for if the game was won. */
    BOOLEAN   gameWon;
    /** Flag for if the left mouse button is pressed. */
    BOOLEAN   leftDown;
    /** Flag for if the right mouse button is pressed. */
    BOOLEAN   rightDown;
    /** Reserved padding. */
    CHAR      reserved[3];
};

struct _MINE_WINDOW_SETTINGS
{
    /** Height (in pixels) of client area. */
    DWORD clientHeight;
    /** Width (in pixels) of client area. */
    DWORD clientWidth;
    /** Position of white lines. */
    RECT  whiteLines[MINE_NUM_WHITE_LINES];
    /** Position of grey lines. */
    RECT  greyLines[MINE_NUM_GREY_LINES];
    /** Location of timer. */
    RECT  timerRegion;
    /** Location of remaining mine counter. */
    RECT  mineCountRegion;
    /** Location of smiley face. */
    RECT  faceRegion;
    /** Location of entire top section. */
    RECT  topBannerRegion;
    /** Location of game board. */
    RECT  boardRegion;
};

struct _MINE_IMAGE_STORAGE
{
    /** Images for timer numbers. */
    HBITMAP timer[10];
    /** Dash for negative remainng mine count. */
    HBITMAP timerDash;
    /** Normal smiley face. */
    HBITMAP faceNormal;
    /** Face when left mouse button is clicked. */
    HBITMAP faceClicked;
    /** Face when game has been won. */
    HBITMAP faceWon;
    /** Face when game has been lost. */
    HBITMAP faceLost;
    /** Images for numbers in game board. */
    HBITMAP numbers[9];
    /** Image for unclicked tile in game board. */
    HBITMAP unclicked;
    /** Image for tile that is being held. */
    HBITMAP held;
    /** Image for flagged tile. */
    HBITMAP flag;
    /** Image for a flag that was incorrectly placed. */
    HBITMAP falseFlag;
    /** Image for a mine when a game is lost. */
    HBITMAP mine;
    /** Image for a mine that was clicked. */
    HBITMAP mineHit;
};

//--------------------------------------------------------------
//    Typedefs
//--------------------------------------------------------------

/** The error code returned by a function. */
typedef _Return_type_success_(return == MINE_ERROR_SUCCESS)\
        enum _MINE_ERROR MINE_ERROR;

/** Structure containing global settings from game menu. */
typedef struct _MINE_GLOBAL_SETTINGS MINE_GLOBAL_SETTINGS;

/** Structure containg state of specific game being played. */
typedef struct _MINE_GAME_SETTINGS MINE_GAME_SETTINGS; 

/** Location of regions in window. */
typedef struct _MINE_WINDOW_SETTINGS MINE_WINDOW_SETTINGS;

/** Structure containing handles to bitmaps. */
typedef struct _MINE_IMAGE_STORAGE MINE_IMAGE_STORAGE;

//--------------------------------------------------------------
//    Global Variable Externs
//--------------------------------------------------------------

extern MINE_GAME_SETTINGS   gameData;
extern HINSTANCE            hInst;
extern HWND                 hwnd;
extern MINE_IMAGE_STORAGE   imageData;
extern MINE_GLOBAL_SETTINGS menuData;
extern BOOLEAN              movementTimerCreated;
extern MINE_WINDOW_SETTINGS windowData;

//--------------------------------------------------------------
//    Function Prototypes
//--------------------------------------------------------------

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
Mine_AssignNumbers(LONG xGridMin, LONG xGridMax, LONG yGridMin, LONG yGridMax);

/**
    Mine_Cleanup
*//**
    Free all memory and delete all objects remaining at program close.
*/
VOID
Mine_Cleanup(VOID);

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
Mine_DoRectOverlap(_In_ PRECT pRect1, _In_ PRECT pRect2);

/**
    Mine_GameWon
*//**
    Process a win of the game.
*/
VOID
Mine_GameWon(VOID);

/**
    Mine_InitInstance
*//**
    Create and display an instance of the main window.

    @param[in] nCmdShow - How window should be shown.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
Mine_InitInstance(int nCmdShow);

/**
    Mine_IsRectSubset
*//**
    Check if a rectangle is a subset of the boundary rectangle.

    @param[in] pBoundary - Boundary rectangle.
    @param[in] pRect     - Rectangle to check.

    @return TRUE if rectangle is a subset. FALSE otherwise.
*/
BOOLEAN
Mine_IsRectSubset(_In_ PRECT pBoundary, _In_ PRECT pRect);

/**
    Mine_NewRandomBoard
*//**
    Assign the correct number of mines randomly to a new game board.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
Mine_NewRandomBoard(VOID);

/**
    Mine_PaintScreen
*//**
    Draw everything in the main window.

    @param[in] hwnd - Handle to the main window.

    @return Mine error code (MINE_ERROR_SUCCESS on success).
*/
MINE_ERROR
Mine_PaintScreen(_In_ HWND hwnd);

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
Mine_PointInRect(short x, short y, _In_ PRECT pRect);

/**
    Mine_ProcessTimer
*//**
    Check how long the game has been going and update the timer if needed.
*/
VOID
Mine_ProcessTimer(VOID);

/**
    Mine_Random
*//**
    Obtain a random integer in the range 0 to (limit-1) inclusive.

    @param[in]  limit  - Number of possible random value to return.
    @param[out] output - Pointer to a UINT to hold the random number.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_Random(UINT limit, _Out_ PUINT output);

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
                UINT numArray, UINT numPerm);

/**
    Mine_RegisterClass
*//**
    Register window class for main window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_RegisterClass(VOID);

/**
    Mine_SetRegDword
*//**
    Set a value in the Minesweeper Deluxe key to a given DWORD.

    @param[in] pValueName   - Name of the value to set.
    @param[in] newValue     - DWORD value to place in key.
    
    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetRegDword(_In_ LPWSTR pValueName, DWORD newValue);

/**
    Mine_SetRegString
*//**
    Set a value in the Minesweeper Deluxe key to a given string.

    @param[in] pValueName   - Name of the value to set.
    @param[in] pNewValue    - String to place in key.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetRegString(_In_ LPWSTR pValueName, _In_ LPWSTR pNewValue);

/**
    Mine_SetupGame
*//**
    Change the game settings upon the start of a new game.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupGame(VOID);

/**
    Mine_SetupGlobal
*//**
    Access the registry to obtain settings that persist when program is closed.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupGlobal(VOID);

/**
    Mine_SetupImageData
*//**
    Load handles to images needed to draw window.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupImageData(VOID);

/**
    Mine_SetupNumberImageData
*//**
    Load handles to tile number images.

    @return Mine error code (MINE_ERROR_SUCCESS upon success).
*/
MINE_ERROR
Mine_SetupNumberImageData(VOID);

/**
    Mine_SetupWindow
*//**
    Store information about the coordinates of window components.
*/
VOID
Mine_SetupWindow(VOID);

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
WndProc(_In_ HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);