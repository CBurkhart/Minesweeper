/**
    @file MineDebug.cpp

    @author Craig Burkhart

    @brief Code for debugging tools for Minesweeper Deluxe project.
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
#include "MineDebug.h"

#ifdef _DEBUG
CHAR formatBuffer[MINEDEBUG_BUFFER_CHARS];
CHAR printBuffer[2*MINEDEBUG_BUFFER_CHARS]; 

/**
    MineDebug_FormatPrintToDebugConsole
*//**
    @brief Print a format string to the debug console.

    @param[in] level  - Level of severity, i.e. "WARNING" or "ERROR".
    @param[in] file   - Name of file with print statement.
    @param[in] line   - Line number of print statement.
    @param[in] format - String to be placed in debug console.
    @param[in] ...    - Extra parameters to be formatted into string.
*/
VOID 
MineDebug_FormatPrintToDebugConsole(_In_z_ LPSTR level, _In_z_ LPSTR file, INT line,
                                    _Printf_format_string_ LPSTR format, ...)
{
    va_list argList = NULL;
    BOOLEAN bFalse = FALSE;
    int     returnCode = 0;

    do
    {
        if ((NULL == level) || (NULL == file) || (NULL == format))
        {
            OutputDebugStringA("MINESWEEPER DELUX PRINT ERROR parameter in NULL\n");
            break;
        }

        ZeroMemory(formatBuffer, MINEDEBUG_BUFFER_CHARS*sizeof(CHAR));
        ZeroMemory(printBuffer, 2*MINEDEBUG_BUFFER_CHARS*sizeof(CHAR));

        va_start(argList, format);

        /** Load buffer with caller formatted error message. */
        errno = 0;
        returnCode = vsnprintf_s(formatBuffer, MINEDEBUG_BUFFER_CHARS, _TRUNCATE, format, argList);

        va_end(argList);

        if (-1 == returnCode)
        {
            //errno equlas 0 indicates the string was truncated, but was still placed in buffer
            if (0 == errno)
            {
                OutputDebugStringA("MINESWEEPER DELUX PRINT WARNING format string has been truncated\n");
            }
            else
            {
                OutputDebugStringA("MINESWEEPER DELUX PRINT ERROR printing formatted string to buffer\n");
                break;
            }
        }

        /** Format with severity, file and line information. */
        errno = 0;
        returnCode = _snprintf_s(printBuffer, 2*MINEDEBUG_BUFFER_CHARS, _TRUNCATE, 
                                 "MINESWEEPER DELUX %s (%s - %i): %s", level, file, line, formatBuffer);
        if (-1 == returnCode)
        {
            if (0 == errno)
            {
                OutputDebugStringA("MINESWEEPER DELUX PRINT WARNING print string has been truncated\n");
            }
            else
            {
                OutputDebugStringA("MINESWEEPER DELUX PRINT ERROR printing final string to buffer\n");
                break;
            }
        }

        /** Place buffer into debug console. */
        OutputDebugStringA(printBuffer);
    } while (bFalse);

    return;
}
#endif /* _DEBUG */