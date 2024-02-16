#pragma once

//#include <windows.h>

static void _RMSInfo(char * str, ...)
{     
      static char buf[1024];
      va_list args;

      va_start(args, str);
      vsprintf_s(buf, str, args);
#ifdef _WIN32
      OutputDebugStringA(buf);
#else
      fprintf(stderr, "%s", buf);
#endif
      va_end(args);
}
