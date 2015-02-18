/**
    @file MineDebug.h

    @author Craig Burkhart

    @brief Header file for debugging functionality.
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

/** Number of characters in buffer that holds debug statements. */
#define MINEDEBUG_BUFFER_CHARS 500

#ifdef _DEBUG
/** Macro to print error statement only if compiled in debug mode. */
#define MineDebug_PrintError(...) \
MineDebug_FormatPrintToDebugConsole("ERROR", __FILE__, __LINE__, __VA_ARGS__);
#else /* _DEBUG */
#define MineDebug_PrintError(...) 
#endif /* _DEBUG */

#ifdef _DEBUG
/** Macro to print warning statement only if compiled in debug mode. */
#define MineDebug_PrintWarning(...) \
MineDebug_FormatPrintToDebugConsole("WARNING", __FILE__, __LINE__, __VA_ARGS__);
#else /* _DEBUG */
#define MineDebug_PrintWarning(...) 
#endif /* _DEBUG */

#ifdef _DEBUG
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
                                    _Printf_format_string_ LPSTR format, ...);
#endif /*_DEBUG */