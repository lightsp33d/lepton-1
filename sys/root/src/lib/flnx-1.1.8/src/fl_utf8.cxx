
// "$Id: fl_utf8.cxx,v 1.1 2009-07-15 13:23:28 jjp Exp $"
//
// Unicode to UTF-8 conversion functions.
//
//                Copyright 2000-2003 by O'ksi'D.
//                      All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//      Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//      Neither the name of O'ksi'D nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//



#include <config.h>
#include <FL/filename.H>
#include <stdarg.h>

#if defined(WIN32)
# include <ctype.h>
# include <io.h>
# include <direct.h>
# include <windows.h>
# include <winbase.h>
# include <process.h>

extern "C" {
  int XUtf8Tolower(int ucs);
  unsigned short XUtf8IsNonSpacing(unsigned int ucs);
};

#elif defined(__APPLE__)
# include <stdio.h>
# include <time.h>
//# include <unix.h>
# include <fcntl.h>
# include <unistd.h>
# include <wchar.h>
# include <stdlib.h>

extern "C" {
  int XUtf8Tolower(int ucs);
  unsigned short XUtf8IsNonSpacing(unsigned int ucs);
};

#elif defined(NANO_X)

# include <FL/Xutf8.h>
//# include <sys/types.h>
//# include <sys/stat.h>
//# include <fcntl.h>
//# include <unistd.h>
#include "kernel/core/types.h"
#include "kernel/core/stat.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/devio.h"
#include "kernel/core/libstd.h"
#include "lib/libc/stdio/stdio.h"
#else // X-windows platform

# include <FL/Xutf8.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif // WIN32

#include <FL/fl_utf8.H>
#include <string.h>
#include <stdlib.h>

#undef fl_open

/*** NOTE : all functions are LIMITED to 24 bits Unicode values !!! ***/
/***        but only 16 bits are really used under Linux and win32  ***/


#define NBC 0xFFFF + 1

static int
Toupper(
	int ucs)
{
	long i;
	static unsigned short *table = NULL;

	if (!table) {
		table = (unsigned short*) malloc(
			sizeof(unsigned short) * (NBC));
		for (i = 0; i < NBC; i++) {
			table[i] = (unsigned short) i;
		}
		for (i = 0; i < NBC; i++) {
			int l;
			l = XUtf8Tolower(i);
			if (l != i) table[l] = (unsigned short) i;
		}

	}
	if (ucs >= NBC || ucs < 0) return ucs;
	return table[ucs];
}

/*** returns the byte length of the first UTF-8 char ***/
/*** (returns -1 if not valid) ***/
int fl_utf8len(char c)
{
  if (!(c & 0x80)) return 1;
  if (c & 0x40) {
    if (c & 0x20) {
      if (c & 0x10) {
        if (c & 0x08) {
          if (c & 0x04) {
            return 6;
          }
          return 5;
        }
        return 4;
      }
      return 3;
    }
    return 2;
  }
  return -1;
} // fl_utf8len


#if 0
int fl_utflen(
        const unsigned char     *buf,
        int                     len)
{
	unsigned int ucs;
	return fl_utf2ucs(buf, len, &ucs);
}
#endif

/**** returns the number of Unicode chars in the UTF-8 string ***/
int
fl_utf_nb_char(
	const unsigned char 	*buf,
	int 			len)
{
	int i = 0;
	int nbc = 0;
	while (i < len) {
		int cl = fl_utf8len((buf+i)[0]);//fl_utflen(buf + i, len - i);
		if (cl < 1) cl = 1;
		nbc++;
		i += cl;
	}
	return nbc;
}

/*
 * compare only the first n bytes
 * return 0 if the strings are equal;
 * return 1 if s1 is greater than s2
 * return -1 if s1 is less than s2
 */
int fl_utf_strncasecmp(const char *s1, const char *s2, int n)
{
        int i;
        int s1_l;
        int s2_l;
        char *e1, *e2; // string end pointers

        s1_l = 0;
        while (s1_l < n && s1[s1_l]) s1_l++;
        s2_l = 0;
        while (s2_l < n && s2[s2_l]) s2_l++;

        if (s1_l < s2_l) {
                return -1;
        } else if (s1_l > s2_l) {
                return 1;
        }
		e1 = (char *)&s1[s1_l]; // last char to test
		e2 = (char *)&s2[s2_l];
        for (i = 0; i < n;) {
                int l1, l2;
                unsigned int u1, u2;
                int res;

//              l1 = fl_utf2ucs((unsigned char*)s1 + i, n - i, &u1);
                u1 = fl_utf8decode(s1 + i, e1, &l1);
//              l2 = fl_utf2ucs((unsigned char*)s2 + i, n - i, &u2);
                u2 = fl_utf8decode(s2 + i, e2, &l2);
                if (l1 - l2 != 0) return l1 - l2;
                res = XUtf8Tolower(u1) - XUtf8Tolower(u2);
                if (res != 0) return res;
                if (l1 < 1) {
                        i += 1;
                } else {
                        i += l1;
                }
        }
        return 0;
}

/*
 * return 0 if the strings are equal;
 * return 1 if s1 is greater than s2
 * return -1 if s1 is less than s2
 */
int fl_utf_strcasecmp(const char *s1, const char *s2)
{
	int s1_l = strlen(s1);
	int s2_l = strlen(s2);

        if (s1_l < s2_l) {
                return -1;
        } else if (s1_l > s2_l) {
                return 1;
	}
	return fl_utf_strncasecmp(s1, s2, s1_l);
}

/*
 * return the Unicode lower case value of ucs
 */
int fl_tolower(unsigned int ucs)
{
	return XUtf8Tolower(ucs);
}

/*
 * return the Unicode upper case value of ucs
 */
int fl_toupper(unsigned int ucs)
{
	return Toupper(ucs);
}

/*
 * converts the str string to the lower case equivalent into buf.
 * Warning: to be safe buf length must be at least 3 * len
 */
int fl_utf_tolower(const unsigned char *str, int len, char *buf)
{
	int i;
	int l = 0;
	char *end = (char *)&str[len];
        for (i = 0; i < len;) {
                int l1, l2;
                unsigned int u1;

//              l1 = fl_utf2ucs((unsigned char*)str + i, len - i, &u1);
                u1 = fl_utf8decode((const char*)(str + i), end, &l1);
                l2 = fl_utf8encode((unsigned int) XUtf8Tolower(u1), buf + l);
                if (l1 < 1) {
                        i += 1;
                } else {
                        i += l1;
		}
                if (l2 < 1) {
                        l += 1;
                } else {
                        l += l2;
		}

	}
	return l;
}

/*
 * converts the str string to the upper case equivalent into buf.
 * Warning: to be safe buf length must be at least 3 * len
 */
int fl_utf_toupper(const unsigned char *str, int len, char *buf)
{
	int i;
	int l = 0;
	char *end = (char *)&str[len];
        for (i = 0; i < len;) {
                int l1, l2;
                unsigned int u1;

//              l1 = fl_utf2ucs((unsigned char*)str + i, len - i, &u1);
                u1 = fl_utf8decode((const char*)(str + i), end, &l1);
                l2 = fl_utf8encode((unsigned int) Toupper(u1), buf + l);
                if (l1 < 1) {
                        i += 1;
                } else {
                        i += l1;
		}
                if (l2 < 1) {
                        l += 1;
                } else {
                        l += l2;
		}
	}
	return l;
}

#if 0 // deprecated in favour of FLTK2's fl_utf8toa
/*
 * convert UTF-8 str to latin1
 * Warning: buf must be at least len long
 */
int fl_utf2latin1(const unsigned char *str, int len, char *buf)
{
	int i;
	int l = 0;
	char *end = (char *)&str[len];
        for (i = 0; i < len;) {
                unsigned int u1;
		int l1;

//              l1 = fl_utf2ucs((unsigned char*)str + i, len - i, &u1);
                u1 = fl_utf8decode((const char*)(str + i), end, &l1);
		if (u1 > 0xFF) u1 = '?';
		buf[l] = (char) u1;
                if (l1 < 1) {
                        i += 1;
                } else {
                        i += l1;
		}
		l++;

	}
	return l;
}
#endif

#if 0 // deprecated in favour of FLTK2's fl_utf8froma
/*
 * convert latin1 str to UTF-8
 * Warning: buf must be at least 2 * len long
 */
int fl_latin12utf(const unsigned char *str, int len, char *buf)
{
	int i;
	int l = 0;
	int l1 = 0;
        for (i = 0; i < len; i++) {
		unsigned int n = (unsigned int) str[i];
		l1 = fl_utf8encode(n, buf + l);
                if (l1 < 1) {
                        l = l + 1;
                } else {
                        l = l + l1;
		}

	}
	return l;
}
#endif

unsigned int fl_nonspacing(unsigned int ucs)
{
	return (unsigned int) XUtf8IsNonSpacing(ucs);
}

#if defined(WIN32)
static xchar *mbwbuf = NULL;
#endif

#ifdef WIN32

unsigned int fl_codepage = 0;
static char *buf = NULL;
static int buf_len = 0;
static unsigned short *wbufa = NULL;

char *fl_utf8_to_locale(const char *s, int len, UINT codepage)
{
	if (!s) return "";
	int l = 0;
//	if (buf_len < len * 2 + 1) {
//		buf_len = len * 2 + 1;
//		buf = (char*) realloc(buf, buf_len);
//		wbufa = (unsigned short*) realloc(wbufa, buf_len * sizeof(short));
//	}
	unsigned wn = fl_utf8toUtf16(s, len, NULL, 0); // Query length
	wn = wn * 2 + 1;
	if (wn >= (unsigned)buf_len) {
		buf_len = wn;
		buf = (char*) realloc(buf, buf_len);
		wbufa = (unsigned short*) realloc(wbufa, buf_len * sizeof(short));
	}
	if (codepage < 1) codepage = fl_codepage;
//	l = fl_utf2unicode((const unsigned char *)s, len, (xchar*) wbufa);
	l = fl_utf8toUtf16(s, len, wbufa, wn); // Convert string
	wbufa[l] = 0;
	buf[l] = 0;
	l = WideCharToMultiByte(codepage, 0, (WCHAR*)wbufa, l, buf, buf_len, NULL, NULL);
	if (l < 0) l = 0;
	buf[l] = 0;
	return buf;
}

char *fl_locale_to_utf8(const char *s, int len, UINT codepage)
{
	if (!s) return "";
	int l = 0;
	if (buf_len < len * 5 + 1) {
		buf_len = len * 5 + 1;
		buf = (char*) realloc(buf, buf_len);
		wbufa = (unsigned short*) realloc(wbufa, buf_len * sizeof(short));
	}
	if (codepage < 1) codepage = fl_codepage;
	buf[l] = 0;

	l = MultiByteToWideChar(codepage, 0, s, len, (WCHAR*)wbufa, buf_len);
	if (l < 0) l = 0;
	wbufa[l] = 0;
//	l = fl_unicode2utf((xchar*)wbufa, l, buf);
	l = fl_utf8fromwc(buf, buf_len, (xchar*)wbufa, l);
	buf[l] = 0;
	return buf;
}
#endif

char * fl_utf2mbcs(const char *s)
{
	if (!s) return NULL;
#if defined(WIN32)
	int l = strlen(s);
	static char *buf = NULL;

//	mbwbuf = (xchar*)realloc(mbwbuf, (l+6) * sizeof(xchar));
//	l = fl_utf2unicode((unsigned char*)s, l, mbwbuf);
//	mbwbuf[l] = 0;
	unsigned wn = fl_utf8toUtf16(s, l, NULL, 0) + 7; // Query length
	mbwbuf = (xchar*)realloc(mbwbuf, sizeof(xchar)*wn);
	l = fl_utf8toUtf16(s, l, (unsigned short *)mbwbuf, wn); // Convert string
	mbwbuf[l] = 0;

	buf = (char*)realloc(buf, l * 6 + 1);
	l = wcstombs(buf, mbwbuf, l * 6);
	buf[l] = 0;
	return buf;
#else
	return (char*) s;
#endif
}


#if 0 // deprecated in favour of FLTK2's fl_utf8from_mb
char * fl_mbcs2utf(const char *s)
{
	if (!s) return NULL;
#if defined(WIN32)
	int l = strlen(s);
	unsigned dstlen;
	static char *buf = NULL;

	mbwbuf = (xchar*)realloc(mbwbuf,(l * 6 + 6) * sizeof(xchar));
	l = mbstowcs(mbwbuf, s, l);
	dstlen = l * 6 + 1;
	buf = (char*)realloc(buf, dstlen);
//	l = fl_unicode2utf(mbwbuf, l, buf);
	l = fl_utf8fromwc(buf, dstlen, mbwbuf, l);
	buf[l] = 0;
	return buf;
#else
	return (char*) s;
#endif
} // fl_mbcs2utf
#endif

#if defined(WIN32)
static xchar *wbuf = NULL;
static xchar *wbuf1 = NULL;
#endif


char *fl_getenv(const char* v)
{
#ifdef WIN32
	int l = strlen(v);
//	static xchar* wbuf = NULL;
//	wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//	wbuf[fl_utf2unicode((const unsigned char*)v, l, wbuf)] = 0;
	unsigned wn = fl_utf8toUtf16(v, l, NULL, 0) + 1; // Query length
	wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
	wn = fl_utf8toUtf16(v, l, (unsigned short *)wbuf, wn); // Convert string
	wbuf[wn] = 0;
	xchar *ret = _wgetenv(wbuf);
	static char *buf = NULL;
	if (ret) {
		l = wcslen(ret);
		wn = fl_utf8fromwc(NULL, 0, ret, l) + 1; // query length
		buf = (char*) realloc(buf, wn);
//		buf[fl_unicode2utf(ret, l, buf)] = 0;
		wn = fl_utf8fromwc(buf, wn, ret, l); // convert string
		buf[wn] = 0;
		return buf;
	} else {
		return NULL;
	}
#else
	return getenv(v);
#endif
}

int fl_open(const char* f, int oflags, ...)
{
	int pmode;
	va_list ap;
	va_start(ap, oflags);
	pmode = va_arg (ap, int);
	va_end(ap);
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		if (pmode == -1) return _wopen(wbuf, oflags);
		else return _wopen(wbuf, oflags, pmode);
#else
	if (pmode == -1) return open(f, oflags, 0);
	else return open(f, oflags, pmode);
#endif
}

FILE *fl_fopen(const char* f, const char *mode)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		l = strlen(mode);
//		wbuf1 = (xchar*)realloc(wbuf1, sizeof(xchar) * (l+1));
//		wbuf1[fl_utf2unicode((const unsigned char*)mode, l, wbuf1)] = 0;
		wn = fl_utf8toUtf16(mode, l, NULL, 0) + 1; // Query length
		wbuf1 = (xchar*)realloc(wbuf1, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(mode, l, (unsigned short *)wbuf1, wn); // Convert string
		wbuf1[wn] = 0;
		return _wfopen(wbuf, wbuf1);
#else
	return fopen(f, (char *)mode);
#endif
}

int fl_system(const char* f)
{
#ifdef WIN32
#  ifdef __MINGW32__
	return system(fl_utf2mbcs(f));
#  else
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wsystem(wbuf);
#  endif
#else
	return system(f);
#endif
}

int fl_execvp(const char *file, char *const *argv)
{
#ifdef WIN32
#ifdef __MINGW32__
	return _execvp(fl_utf2mbcs(file), argv);
#else
		int l = strlen(file);
		int i, n, ret;
		xchar **ar;
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)file, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(file, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(file, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;

		i = 0; n = 0;
		while (argv[i]) {i++; n++;}
		ar = (xchar**) malloc(sizeof(xchar*) * (n + 1));
		i = 0;
		while (i <= n) {
			unsigned wn;
			l = strlen(argv[i]);
//			ar[i] = (xchar *)malloc(sizeof(xchar) * (l+1));
//			ar[i][fl_utf2unicode((const unsigned char*)argv[i], l, ar[i])] = 0;
			wn = fl_utf8toUtf16(argv[i], l, NULL, 0) + 1; // Query length
			ar[i] = (xchar *)malloc(sizeof(xchar)*wn);
			wn = fl_utf8toUtf16(argv[i], l, (unsigned short *)ar[i], wn); // Convert string
			ar[i][wn] = 0;
			i++;
		}
		ar[n] = NULL;
		ret = _wexecvp(wbuf, ar);
		i = 0;
		while (i <= n) {
			free(ar[i]);
			i++;
		}
		free(ar);
		return ret;
#endif
#else
	//return execvp(file, argv);
		return execv(file, (const char **)argv);
#endif
}



int fl_chmod(const char* f, int mode)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wchmod(wbuf, mode);
#else
	return chmod(f, mode);
#endif
}

int fl_access(const char* f, int mode)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _waccess(wbuf, mode);
#else
	return access(f, mode);
#endif
}


int fl_stat(const char* f, struct stat *b)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wstat(wbuf, (struct _stat*)b);
#else
	return stat(f, b);
#endif
}

char *fl_getcwd(char* b, int l)
{
	if (b == NULL) {
		b = (char*) malloc(l+1);
	}
#ifdef WIN32
		static xchar *wbuf = NULL;
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		xchar *ret = _wgetcwd(wbuf, l / 5);
		xchar *ret = _wgetcwd(wbuf, l);
		if (ret) {
			unsigned dstlen = l;
			l = wcslen(wbuf);
//			b[fl_unicode2utf(wbuf, l, b)] = 0;
			dstlen = fl_utf8fromwc(b, dstlen, wbuf, l);
			b[dstlen] = 0;
			return b;
		} else {
			return NULL;
		}
#else
	return getcwd(b, l);
#endif
}


int fl_unlink(const char* f)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wunlink(wbuf);
#else
	//return unlink(f);
		return 0;
#endif
}

int fl_mkdir(const char* f, int mode)
{
#if defined(WIN32) && !defined(__CYGWIN__)
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(short) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wmkdir(wbuf);
#else
	return mkdir(f, mode);
#endif
}


int fl_rmdir(const char* f)
{
#ifdef WIN32
		int l = strlen(f);
//		wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
//		wbuf[fl_utf2unicode((const unsigned char*)f, l, wbuf)] = 0;
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		return _wrmdir(wbuf);
#else
	return rmdir(f);
#endif
}

int fl_rename(const char* f, const char *n)
{
#ifdef WIN32
		int l = strlen(f);
		unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
		wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
		wbuf[wn] = 0;
		l = strlen(n);
		wn = fl_utf8toUtf16(n, l, NULL, 0) + 1; // Query length
		wbuf1 = (xchar*)realloc(wbuf1, sizeof(xchar)*wn);
		wn = fl_utf8toUtf16(n, l, (unsigned short *)wbuf1, wn); // Convert string
		wbuf1[wn] = 0;
		return _wrename(wbuf, wbuf1);
#else
	return rename(f, n);
#endif
}

// recursively create a path in the file system
char fl_make_path( const char *path ) {
  if (fl_access(path, 0)) {
    const char *s = strrchr( path, '/' );
    if ( !s ) return 0;
    int len = s-path;
    char *p = (char*)malloc( len+1 );
    memcpy( p, path, len );
    p[len] = 0;
    fl_make_path( p );
    free( p );
    fl_mkdir(path, 0700);
  }
  return 1;
}

// strip the filename and create a path
void fl_make_path_for_file( const char *path )
{
  const char *s = strrchr( path, '/' );
  if ( !s ) return;
  int len = s-path;
  char *p = (char*)malloc( len+1 );
  memcpy( p, path, len );
  p[len] = 0;
  fl_make_path( p );
  free( p );
}

//
// End of "$Id: fl_utf8.cxx,v 1.1 2009-07-15 13:23:28 jjp Exp $".
//

