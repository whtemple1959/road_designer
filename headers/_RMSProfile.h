// Author: Ryan Schmidt  (rms@unknownroad.com)  http://www.unknownroad.com
// Copyright (c) 2006. All Rights Reserved.
// The SketchFramework Library is supplied "AS IS". The Author disclaims all warranties, expressed or implied, including, 
// without limitation, the warranties of merchantability and of fitness for any purpose. The Author assume no liability for 
// direct, indirect, incidental, special, exemplary, or consequential damages, which may result from the use of the 
// SketchFramework Library, even if advised of the possibility of such damage. Permission is hereby granted to use, copy, 
// modify, and distribute this source code, or portions hereof, for any purpose, without fee, subject to the following restrictions:
// 1) The origin of this source code must not be misrepresented.
// 2) This Copyright notice may not be removed or altered from any source or altered source distribution.
// The Author specifically permits, without fee, the use of this source code as a component in commercial products.

static LARGE_INTEGER _RMSTUNE_starts[16];
static LARGE_INTEGER _RMSTUNE_ends[16];

static LARGE_INTEGER _RMSTUNE_accums[16];

static void _RMSTUNE_start(int i)
{
      QueryPerformanceCounter(&_RMSTUNE_starts[i]);
}

static void _RMSTUNE_end(int i)
{
      QueryPerformanceCounter(&_RMSTUNE_ends[i]);
}

static void _RMSTUNE_accum_init(int i)
{
      memset(&_RMSTUNE_accums[i], 0, sizeof(LARGE_INTEGER));
}

static void _RMSTUNE_accum(int i)
{
      _RMSTUNE_accums[i].QuadPart += (_RMSTUNE_ends[i].QuadPart - _RMSTUNE_starts[i].QuadPart);
}

static double _RMSTUNE_time(int i)
{
      LARGE_INTEGER freq;
      QueryPerformanceFrequency(&freq);
      return (double)( (_RMSTUNE_ends[i].QuadPart - _RMSTUNE_starts[i].QuadPart)) / (double)freq.QuadPart;
}

static double _RMSTUNE_accum_time(int i)
{
      LARGE_INTEGER freq;
      QueryPerformanceFrequency(&freq);
      return (double)( _RMSTUNE_accums[i].QuadPart) / (double)freq.QuadPart;
}

/*
static void _RMSTUNE_winprint(double timeval, char * str)
{
      char buf[256];
      sprintf_s(buf, "%f \n", timeval);
      OutputDebugString(str);
      OutputDebugString(buf);
}
*/