/* SdFs Library
 * Copyright (C) 2016..2017 by William Greiman
 *
 * This file is part of the SdFs Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the SdFs Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef PrintTemplates_h
#define PrintTemplates_h
/**
 * \file
 * \brief templates for printf
 */
#include <stdarg.h>
#include "FmtNumber.h"
/** test for digit */
#define isDigit(d) ('0' <= (d) && (d) <= '9')
/** control for supported floating formats */
#define PRINTF_USE_FLOAT 2
//-----------------------------------------------------------------------------
/** Formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] fmt format string.
 * \param[in] ap argument list.
 *
 * \return number of character printed for success else a negative value.
 */
template<typename F> 
int vfprintf(F* file, const char *fmt, va_list ap) {
#if PRINTF_USE_FLOAT
  char buf[30];
  double f;
#else 
   char buf[15]; 
#endif  // PRINTF_USE_FLOAT  
  char prefix[3];
  unsigned base;
  int n;
  int nc = 0;
  int nf;
  int nz;
  int prec;
  int width;
  size_t np;    
  size_t ns;
  size_t nw;
  long ln;
  char c;
  char plusSign;
  char* ptr;  // end of string
  char* str;  // start of string
  bool altForm;
  bool leftAdjust;
  bool isLong;
  bool zeroPad;

  while (true) {
    const char* bgn = fmt;
    while ((c = *fmt++) && c!= '%') {}
    nw = fmt - bgn - 1;
    if (nw) {
      nc += nw;
      if (nw != file->write(bgn, nw)) {
        goto fail;
      }
    }
    if (!c) break; 
    altForm = false;
    leftAdjust = false;
    np = 0;
    nz = 0;
    zeroPad = false;
    plusSign = 0;
    c = *fmt++;
    
    while (true) {
      if (c == '-') {
        leftAdjust = true;
      } else if (c == '+') {
        plusSign = '+';
      } else if (c == ' ') {
        if (plusSign == 0) {
          plusSign = ' ';
        }
      } else if (c == '0') {
        zeroPad = true;
      } else if (c == '#') {
        altForm = true;
      } else {
        break;
      }
      c = *fmt++;
    }
    
    width = 0;
	  if (isDigit(c)) {
		  while(isDigit(c)) {
		    width = 10 * width + c - '0';
		    c = *fmt++;
		  }
	  } else if (c == '*') {
		  width = va_arg(ap, int);
		  c = *fmt++;
		  if (width < 0) {
		    leftAdjust = true;
		    width = -width;
		  }
	  }
    if (leftAdjust) {
      zeroPad = false;    
    }
    
    prec = -1;
	  if (c == '.') {
      zeroPad = false;
		  prec = 0;      
		  c = *fmt++;
		  if (isDigit(c)) {
		    while(isDigit(c)) {
			    prec = 10 * prec + c - '0';
			    c = *fmt++;
		    }
		  } else if (c == '*') {
		    prec = va_arg(ap, int);
		    c = *fmt++;
		  }
	  }

    isLong = false;
    if (c == 'l' || c =='L') {
      isLong = true;
      c = *fmt++;
    }
    
    if (!c) break;
   
    str = buf + sizeof(buf);
    ptr = str;
    switch(c) {
      case 'c':
        *--str = va_arg(ap, int);
        break;
        
      case 's':
        str = va_arg(ap, char *);
        if (!str) {
          str = (char*)"(null)";
        }
        ns = strlen(str);
        ptr = str + (prec >= 0 && (size_t)prec < ns ? prec : ns);    
        break;
        
      case 'd':
      case 'i':
        ln = isLong ? va_arg(ap, long) : va_arg(ap, int);
        if (prec || ln) {
          if (ln < 0) {
            prefix[np++] = '-';
            ln = -ln;
          } else if (plusSign) {
            prefix[np++] = plusSign;            
          }
          str = fmtUnsigned(str, ln, 10, true);
          nz = prec + str - ptr;
        }
        break;
        
#if PRINTF_USE_FLOAT > 1
      case 'e':
      case 'E':
      case 'f':
      case 'F':
        f = va_arg(ap, double);
        if (f < 0) {
          f = -f;
          prefix[np++] = '-';          
        } else if (plusSign) {
          prefix[np++] = plusSign;           
        }
        str = fmtDouble(str, f, prec < 0 ? 6 : prec, altForm, c);
        break;
#elif PRINTF_USE_FLOAT > 0        
      case 'f':
      case 'F':
        f = va_arg(ap, double);
        if (f < 0) {
          f = -f;
          prefix[np++] = '-';          
        } else if (plusSign) {
          prefix[np++] = plusSign;           
        }
        str = fmtDouble(str, f, prec < 0 ? 6 : prec, altForm);
        break;
#endif  // PRINTF_USE_FLOAT 
     
      case 'o':
        base = 8;
        goto printUnsigned;
        
      case 'u':
        base = 10;
        altForm = false;
        goto printUnsigned;
        
      case 'x':
      case 'X':    
        base = 16;
        goto printUnsigned;
        
      printUnsigned:
        ln = isLong ? va_arg(ap, long) : va_arg(ap, int);
        if (prec || ln) {
          str = fmtUnsigned(str, ln, base, c == 'X');
          nz = prec + str - ptr;
        }
        if (altForm && ln) {
          if (c == 'o') {
            *--str = '0';
          } else {
            prefix[np++] = '0';
            prefix[np++] = c;            
          }
        }
        break;
        
      default:
        *--str = c;
        break;
    }
    ns = (ptr - str);
    if (nz < 0) nz = 0;
    n = ns + np + nz;
    if (width < n) {
      nc += n;
      nf = 0;
    } else {
      nc += width;
      if (zeroPad) {
        nz += width - n;
        nf = 0;
      } else {
        nf = width - n;
      }
    }
    // Do right blank padding.
    if (!leftAdjust) {
      for (; nf > 0; nf--) {
        if (1 != file->write(' ')) {
          goto fail;
        }
      }
    }
    // Don't call write if no prefix.
    if (np && np != file->write(prefix, np)) {
      goto fail;
    }
    // Do zero padding.
    for (; nz > 0; nz--) {
      if (1 != file->write('0')) {
        goto fail;
      }
    }
    // Main item.
    if (ns != file->write(str, ns)) {
      goto fail;
    }
    // Right blank padding.
    for (; nf > 0; nf--) {
      if (1 != file->write(' ')) {
        goto fail;
      }
    }
	}
  va_end(ap); 
  return nc;
 fail:
  va_end(ap);
  return -1;
}
//-----------------------------------------------------------------------------
/** Formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] fmt format string.
 *
 * \return number of character printed for success else a negative value.
 */
template<typename T>
int fprintf(T *file, const char* fmt, ...) {
  va_list ap;
	va_start(ap, fmt);  
  return vfprintf(file, fmt, ap);
}
//-----------------------------------------------------------------------------
/** Minimal formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] fmt format string.
 * \param[in] ap argument list.
 *
 * \return number of character printed for success else a negative value.
 */
template<typename F> 
int vmprintf(F* file, const char *fmt, va_list ap) {
  char buf[15];
  char* ptr;
  char* str;  
  bool isLong;
  char c;
  int nc = 0;
  size_t ns;
  long n;

  while (true) {
    const char* bgn = fmt;
    while ((c = *fmt++) && c!= '%') {}
    ns = fmt - bgn - 1;
    if (ns) {
      nc += file->write(bgn, ns);
    }
    if (!c) {
      break;
    }      
    c = *fmt++;
    if (c == 'l') {
      isLong = true;
      c = *fmt++;
    } else {
      isLong = false;
    }
    if (!c) {
      break;
    }
    ptr = str = buf + sizeof(buf);   
    switch (c) {
      case 'c':
        *--str = va_arg(ap, int);
        break;
        
      case 's':
        str = va_arg(ap, char*);
        ptr = str ? str + strlen(str) : nullptr;
        break;
        
      case 'd':
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtSigned(str, n, 10, true);
        break;
        
      case 'u':  
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtUnsigned(str, n, 10, true);
        break;
        
      case 'x':
      case 'X':    
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtUnsigned(str, n, 16, c == 'X');
        break;
        
      default:
        *--str = c;;
        break;
    }
    ns = ptr - str;
    nc += file->write(str, ns);
  }
  va_end(ap);
  return nc;  
}
//-----------------------------------------------------------------------------
/** Minimal formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] fmt format string.
 *
 * \return number of character printed for success else a negative value.
 */

template<typename T>
int mprintf(T *file, const char* fmt, ...) {
  va_list ap;
	va_start(ap, fmt);  
  return vmprintf(file, fmt, ap);
}
#if ENABLE_ARDUINO_FEATURES 
//-----------------------------------------------------------------------------
/** Minimal formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] ifsh format string using F() macro.
 * \param[in] ap argument list. 
 *
 * \return number of character printed for success else a negative value.
 */
template<typename F>
int vmprintf(F file, const __FlashStringHelper *ifsh, va_list ap) { 
  bool isLong;
  char buf[15];
  char c;
  char* ptr;
  char* str;  
  size_t ns;
  int nc = 0;
  long n;

  PGM_P fmt = reinterpret_cast<PGM_P>(ifsh);
  while (true) {
	  while ((c = pgm_read_byte(fmt++)) && c != '%') {
		  nc += file->write(c);
    }
    if (!c) {
      break;
    }
    c = pgm_read_byte(fmt++);
    if (c == 'l') {
      isLong = true;
      c = pgm_read_byte(fmt++);
    } else {
      isLong = false;
    }
    if (!c) {
      break;
    }
    ptr = str = buf + sizeof(buf);
    switch (c) {
      case 'c':
        *--str = va_arg(ap, int);
        break;
        
      case 's':
        str = va_arg(ap, char*);
        ptr = str ? str + strlen(str) : nullptr;
        break;
        
      case 'd':
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtSigned(str, n, 10, true);
        break;
        
      case 'u':  
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtUnsigned(str, n, 10, true);
        break;
        
      case 'x':
      case 'X':    
        n = isLong ? va_arg(ap, long) : va_arg(ap, int);
        str = fmtUnsigned(str, n, 16, c == 'X');
        break;
        
      default:
        *--str = c;;
        break;
    }
    ns = ptr - str;
    nc += file->write(str, ns);
  }
  va_end(ap);
  return nc;    
}
//-----------------------------------------------------------------------------
/** Minimal formatted print.
 *
 * \param[in] file destination file or device. 
 * \param[in] ifsh format string using F() macro.
 *
 * \return number of character printed for success else a negative value.
 */
template<typename F>
int mprintf(F* file, const __FlashStringHelper *ifsh, ...) {
  va_list ap;
	va_start(ap, ifsh);
  return vmprintf(file, ifsh, ap);
}
#endif  // ENABLE_ARDUINO_FEATURES 
#endif  // PrintTemplates_h
