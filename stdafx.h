/**
    @file stdafx.h

    @author Craig Burkhart

    @brief Pre-compiled header file for including system headers files.
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

#include "targetver.h"

//Windows header files

/** Macro for loading only commonly used windows.h features. */
#define WIN32_LEAN_AND_MEAN

//Disable information warning for compiler inline expansion decisions
#pragma warning(disable : 4514 4710 4711)

//Disable warnings on headers from Windows
#pragma warning(push)
#pragma warning(disable : 4668 4820)

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <CommCtrl.h>
#include <wincrypt.h>

//Other system header files

#ifdef _DEBUG
#include <errno.h>
#endif /* _DEBUG */
#include <limits.h>
#include <stdarg.h>
#include <strsafe.h>

#pragma warning(pop)