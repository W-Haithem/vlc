/*****************************************************************************
 * getaddrinfo.c: getaddrinfo/getnameinfo replacement functions
 *****************************************************************************
 * Copyright (C) 2005 the VideoLAN team
 * Copyright (C) 2002-2007 Rémi Denis-Courmont
 * $Id$
 *
 * Author: Rémi Denis-Courmont <rem # videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc/vlc.h>

#include <stddef.h> /* size_t */
#include <string.h> /* strlen(), memcpy(), memset(), strchr() */
#include <stdlib.h> /* malloc(), free(), strtoul() */
#include <errno.h>

#ifdef HAVE_SYS_TYPES_H
#   include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#   include <arpa/inet.h>
#endif
#ifdef HAVE_NETINET_IN_H
#   include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#include <vlc_network.h>

#ifndef NO_ADDRESS
#   define NO_ADDRESS  NO_DATA
#endif
#ifndef INADDR_NONE
#   define INADDR_NONE 0xFFFFFFFF
#endif
#ifndef AF_UNSPEC
#   define AF_UNSPEC   0
#endif


#ifndef HAVE_GAI_STRERROR
static struct
{
    int code;
    const char *msg;
} const __gai_errlist[] =
{
    { 0,              "Error 0" },
    { EAI_BADFLAGS,   "Invalid flag used" },
    { EAI_NONAME,     "Host or service not found" },
    { EAI_AGAIN,      "Temporary name service failure" },
    { EAI_FAIL,       "Non-recoverable name service failure" },
    { EAI_NODATA,     "No data for host name" },
    { EAI_FAMILY,     "Unsupported address family" },
    { EAI_SOCKTYPE,   "Unsupported socket type" },
    { EAI_SERVICE,    "Incompatible service for socket type" },
    { EAI_ADDRFAMILY, "Unavailable address family for host name" },
    { EAI_MEMORY,     "Memory allocation failure" },
    { EAI_OVERFLOW,   "Buffer overflow" },
    { EAI_SYSTEM,     "System error" },
    { 0,              NULL }
};

static const char *__gai_unknownerr = "Unrecognized error number";

/****************************************************************************
 * Converts an EAI_* error code into human readable english text.
 ****************************************************************************/
const char *vlc_gai_strerror (int errnum)
{
    for (unsigned i = 0; __gai_errlist[i].msg != NULL; i++)
        if (errnum == __gai_errlist[i].code)
            return __gai_errlist[i].msg;

    return __gai_unknownerr;
}
#else /* ifndef HAVE_GAI_STRERROR */
const char *vlc_gai_strerror (int errnum)
{
    return gai_strerror (errnum);
}
#endif

#ifndef HAVE_GETNAMEINFO
#define _NI_MASK (NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|\
                  NI_DGRAM)
/*
 * getnameinfo() non-thread-safe IPv4-only implementation,
 * Address-family-independant address to hostname translation
 * (reverse DNS lookup in case of IPv4).
 *
 * This is meant for use on old IP-enabled systems that are not IPv6-aware,
 * and probably do not have getnameinfo(), but have the old gethostbyaddr()
 * function.
 *
 * GNU C library 2.0.x is known to lack this function, even though it defines
 * getaddrinfo().
 */
#ifdef WIN32
static int WSAAPI
getnameinfo (const struct sockaddr *sa, socklen_t salen,
             char *host, DWORD hostlen, char *serv, DWORD servlen, int flags)
#else
static int
getnameinfo (const struct sockaddr *sa, socklen_t salen,
             char *host, int hostlen, char *serv, int servlen, int flags)
#endif
{
    if (((size_t)salen < sizeof (struct sockaddr_in))
     || (sa->sa_family != AF_INET))
        return EAI_FAMILY;
    else if (flags & (~_NI_MASK))
        return EAI_BADFLAGS;
    else
    {
        const struct sockaddr_in *addr;

        addr = (const struct sockaddr_in *)sa;

        if (host != NULL)
        {
            /* host name resolution */
            if (!(flags & NI_NUMERICHOST))
            {
                if (flags & NI_NAMEREQD)
                    return EAI_NONAME;
            }

            /* inet_ntoa() is not thread-safe, do not use it */
            uint32_t ipv4 = ntohl (addr->sin_addr.s_addr);

            if (snprintf (host, hostlen, "%u.%u.%u.%u", ipv4 >> 24,
                          (ipv4 >> 16) & 0xff, (ipv4 >> 8) & 0xff,
                          ipv4 & 0xff) >= (int)hostlen)
                return EAI_OVERFLOW;
        }

        if (serv != NULL)
        {
            if (snprintf (serv, servlen, "%u",
                          (unsigned int)ntohs (addr->sin_port)) >= (int)servlen)
                return EAI_OVERFLOW;
        }
    }
    return 0;
}

#endif /* if !HAVE_GETNAMEINFO */

#ifndef HAVE_GETADDRINFO
#define _AI_MASK (AI_PASSIVE|AI_CANONNAME|AI_NUMERICHOST)
/*
 * Converts the current herrno error value into an EAI_* error code.
 * That error code is normally returned by getnameinfo() or getaddrinfo().
 */
static int
gai_error_from_herrno (void)
{
    switch (h_errno)
    {
        case HOST_NOT_FOUND:
            return EAI_NONAME;

        case NO_ADDRESS:
# if (NO_ADDRESS != NO_DATA)
        case NO_DATA:
# endif
            return EAI_NODATA;

        case NO_RECOVERY:
            return EAI_FAIL;

        case TRY_AGAIN:
            return EAI_AGAIN;
    }
    return EAI_SYSTEM;
}

/*
 * This functions must be used to free the memory allocated by getaddrinfo().
 */
#ifdef WIN32
static void WSAAPI freeaddrinfo (struct addrinfo *res)
#else
static void freeaddrinfo (struct addrinfo *res)
#endif
{
    if (res != NULL)
    {
        if (res->ai_canonname != NULL)
            free (res->ai_canonname);
        if (res->ai_addr != NULL)
            free (res->ai_addr);
        if (res->ai_next != NULL)
            free (res->ai_next);
        free (res);
    }
}


/*
 * Internal function that builds an addrinfo struct.
 */
static struct addrinfo *
makeaddrinfo (int af, int type, int proto,
              const struct sockaddr *addr, size_t addrlen,
              const char *canonname)
{
    struct addrinfo *res;

    res = (struct addrinfo *)malloc (sizeof (struct addrinfo));
    if (res != NULL)
    {
        res->ai_flags = 0;
        res->ai_family = af;
        res->ai_socktype = type;
        res->ai_protocol = proto;
        res->ai_addrlen = addrlen;
        res->ai_addr = malloc (addrlen);
        res->ai_canonname = NULL;
        res->ai_next = NULL;

        if (res->ai_addr != NULL)
        {
            memcpy (res->ai_addr, addr, addrlen);

            if (canonname != NULL)
            {
                res->ai_canonname = strdup (canonname);
                if (res->ai_canonname != NULL)
                    return res; /* success ! */
            }
            else
                return res;
        }
    }
    /* failsafe */
    vlc_freeaddrinfo (res);
    return NULL;
}


static struct addrinfo *
makeipv4info (int type, int proto, u_long ip, u_short port, const char *name)
{
    struct sockaddr_in addr;

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
# ifdef HAVE_SA_LEN
    addr.sin_len = sizeof (addr);
# endif
    addr.sin_port = port;
    addr.sin_addr.s_addr = ip;

    return makeaddrinfo (AF_INET, type, proto,
                         (struct sockaddr*)&addr, sizeof (addr), name);
}


/*
 * getaddrinfo() non-thread-safe IPv4-only implementation
 * Address-family-independant hostname to address resolution.
 *
 * This is meant for IPv6-unaware systems that do probably not provide
 * getaddrinfo(), but still have old function gethostbyname().
 *
 * Only UDP and TCP over IPv4 are supported here.
 */
#ifdef WIN32
static int WSAAPI
getaddrinfo (const char *node, const char *service,
             const struct addrinfo *hints, struct addrinfo **res)
#else
static int
getaddrinfo (const char *node, const char *service,
             const struct addrinfo *hints, struct addrinfo **res)
#endif
{
    struct addrinfo *info;
    u_long ip;
    u_short port;
    int protocol = 0, flags = 0;
    const char *name = NULL;

#ifdef WIN32
    /*
     * Maybe you knew already that Winsock does not handle TCP/RST packets
     * properly, so that when a TCP connection fails, it will wait until it
     * times out even if the remote host did return a TCP/RST. However, it
     * still sees the TCP/RST as the error code is 10061 instead of 10060.
     * Basically, we have the stupid brainfucked behavior with DNS queries...
     * When the recursive DNS server returns an error, Winsock waits about
     * 2 seconds before it returns to the callers, even though it should know
     * that is pointless. I'd like to know how come this hasn't been fixed
     * for the past decade, or maybe not.
     *
     * Anyway, this is causing a severe delay when the SAP listener tries
     * to resolve more than ten IPv6 numeric addresses. Modern systems will
     * eventually realize that it is an IPv6 address, and won't try to resolve
     * it as a IPv4 address via the Domain Name Service. Old systems
     * (including Windows XP without the IPv6 stack) will not. It is normally
     * not an issue as the DNS server usually returns an error very quickly.
     * But it IS a severe issue on Windows, given the bug explained above.
     * So here comes one more bug-to-bug Windows compatibility fix.
     */
    if ((node != NULL) && (strchr (node, ':') != NULL))
       return EAI_NONAME;
#endif

    if (hints != NULL)
    {
        flags = hints->ai_flags;

        if (flags & ~_AI_MASK)
            return EAI_BADFLAGS;
        /* only accept AF_INET and AF_UNSPEC */
        if (hints->ai_family && (hints->ai_family != AF_INET))
            return EAI_FAMILY;

        /* protocol sanity check */
        switch (hints->ai_socktype)
        {
            case SOCK_STREAM:
                protocol = IPPROTO_TCP;
                break;

            case SOCK_DGRAM:
                protocol = IPPROTO_UDP;
                break;

#ifndef SOCK_RAW
            case SOCK_RAW:
#endif
            case 0:
                break;

            default:
                return EAI_SOCKTYPE;
        }
        if (hints->ai_protocol && protocol
         && (protocol != hints->ai_protocol))
            return EAI_SERVICE;
    }

    *res = NULL;

    /* default values */
    if (node == NULL)
    {
        if (flags & AI_PASSIVE)
            ip = htonl (INADDR_ANY);
        else
            ip = htonl (INADDR_LOOPBACK);
    }
    else
    if ((ip = inet_addr (node)) == INADDR_NONE)
    {
        struct hostent *entry = NULL;

        /* hostname resolution */
        if (!(flags & AI_NUMERICHOST))
            entry = gethostbyname (node);

        if (entry == NULL)
            return gai_error_from_herrno ();

        if ((entry->h_length != 4) || (entry->h_addrtype != AF_INET))
            return EAI_FAMILY;

        ip = *((u_long *) entry->h_addr);
        if (flags & AI_CANONNAME)
            name = entry->h_name;
    }

    if ((flags & AI_CANONNAME) && (name == NULL))
        name = node;

    /* service resolution */
    if (service == NULL)
        port = 0;
    else
    {
        unsigned long d;
        char *end;

        d = strtoul (service, &end, 0);
        if (end[0] || (d > 65535u))
            return EAI_SERVICE;

        port = htons ((u_short)d);
    }

    /* building results... */
    if ((!protocol) || (protocol == IPPROTO_UDP))
    {
        info = makeipv4info (SOCK_DGRAM, IPPROTO_UDP, ip, port, name);
        if (info == NULL)
        {
            errno = ENOMEM;
            return EAI_SYSTEM;
        }
        if (flags & AI_PASSIVE)
            info->ai_flags |= AI_PASSIVE;
        *res = info;
    }
    if ((!protocol) || (protocol == IPPROTO_TCP))
    {
        info = makeipv4info (SOCK_STREAM, IPPROTO_TCP, ip, port, name);
        if (info == NULL)
        {
            errno = ENOMEM;
            return EAI_SYSTEM;
        }
        info->ai_next = *res;
        if (flags & AI_PASSIVE)
            info->ai_flags |= AI_PASSIVE;
        *res = info;
    }

    return 0;
}
#endif /* if !HAVE_GETADDRINFO */

#if defined( WIN32 ) && !defined( UNDER_CE )
    /*
     * Here is the kind of kludge you need to keep binary compatibility among
     * varying OS versions...
     */
typedef int (WSAAPI * GETNAMEINFO) ( const struct sockaddr FAR *, socklen_t,
                                           char FAR *, DWORD, char FAR *, DWORD, int );
typedef int (WSAAPI * GETADDRINFO) (const char FAR *, const char FAR *,
                                          const struct addrinfo FAR *,
                                          struct addrinfo FAR * FAR *);

typedef void (WSAAPI * FREEADDRINFO) ( struct addrinfo FAR * );

static int WSAAPI _ws2_getnameinfo_bind ( const struct sockaddr FAR *, socklen_t,
                                           char FAR *, DWORD, char FAR *, DWORD, int );
static int WSAAPI _ws2_getaddrinfo_bind (const char FAR *, const char FAR *,
                                          const struct addrinfo FAR *,
                                          struct addrinfo FAR * FAR *);

static GETNAMEINFO ws2_getnameinfo = _ws2_getnameinfo_bind;
static GETADDRINFO ws2_getaddrinfo = _ws2_getaddrinfo_bind;
static FREEADDRINFO ws2_freeaddrinfo;

static FARPROC ws2_find_api (LPCTSTR name)
{
    FARPROC f = NULL;

    HMODULE m = GetModuleHandle (TEXT("WS2_32"));
    if (m != NULL)
        f = GetProcAddress (m, name);

    if (f == NULL)
    {
        /* Windows 2K IPv6 preview */
        m = LoadLibrary (TEXT("WSHIP6"));
        if (m != NULL)
            f = GetProcAddress (m, name);
    }

    return f;
}

static WSAAPI int _ws2_getnameinfo_bind( const struct sockaddr FAR * sa, socklen_t salen,
               char FAR *host, DWORD hostlen, char FAR *serv, DWORD servlen, int flags )
{
    GETNAMEINFO entry = (GETNAMEINFO)ws2_find_api (TEXT("getnameinfo"));
    int result;

    if (entry == NULL)
    {
        /* not found, use replacement API instead */
    entry = getnameinfo;

    }
    /* call API before replacing function pointer to avoid crash */
    result = entry (sa, salen, host, hostlen, serv, servlen, flags);
    ws2_getnameinfo = entry;
    return result;
}
#undef getnameinfo
#define getnameinfo ws2_getnameinfo

static WSAAPI int _ws2_getaddrinfo_bind(const char FAR *node, const char FAR *service,
               const struct addrinfo FAR *hints, struct addrinfo FAR * FAR *res)
{
    GETADDRINFO entry;
    FREEADDRINFO freentry;
    int result;

    entry = (GETADDRINFO)ws2_find_api (TEXT("getaddrinfo"));
    freentry = (FREEADDRINFO)ws2_find_api (TEXT("freeaddrinfo"));

    if ((entry == NULL) ||  (freentry == NULL))
    {
        /* not found, use replacement API instead */
    entry = getaddrinfo;
    freentry = freeaddrinfo;
    }
    /* call API before replacing function pointer to avoid crash */
    result = entry (node, service, hints, res);
    ws2_freeaddrinfo = freentry;
    ws2_getaddrinfo = entry;
    return result;
}
#undef getaddrinfo
#undef freeaddrinfo
#define getaddrinfo ws2_getaddrinfo
#define freeaddrinfo ws2_freeaddrinfo
#define HAVE_GETADDRINFO
#endif



int vlc_getnameinfo( const struct sockaddr *sa, int salen,
                     char *host, int hostlen, int *portnum, int flags )
{
    char psz_servbuf[6], *psz_serv;
    int i_servlen, i_val;

    flags |= NI_NUMERICSERV;
    if( portnum != NULL )
    {
        psz_serv = psz_servbuf;
        i_servlen = sizeof( psz_servbuf );
    }
    else
    {
        psz_serv = NULL;
        i_servlen = 0;
    }

    i_val = getnameinfo(sa, salen, host, hostlen, psz_serv, i_servlen, flags);

    if( portnum != NULL )
        *portnum = atoi( psz_serv );

    return i_val;
}


int vlc_getaddrinfo( vlc_object_t *p_this, const char *node,
                     int i_port, const struct addrinfo *p_hints,
                     struct addrinfo **res )
{
    struct addrinfo hints;
    char psz_buf[NI_MAXHOST], *psz_node, psz_service[6];

    /*
     * In VLC, we always use port number as integer rather than strings
     * for historical reasons (and portability).
     */
    if( ( i_port > 65535 ) || ( i_port < 0 ) )
    {
        msg_Err( p_this, "invalid port number %d specified", i_port );
        return EAI_SERVICE;
    }

    /* cannot overflow */
    snprintf( psz_service, 6, "%d", i_port );

    /* Check if we have to force ipv4 or ipv6 */
    memset (&hints, 0, sizeof (hints));
    if (p_hints != NULL)
    {
        const int safe_flags =
            AI_PASSIVE |
            AI_CANONNAME |
            AI_NUMERICHOST |
#ifdef AI_NUMERICSERV
            AI_NUMERICSERV |
#endif
#ifdef AI_ALL
            AI_ALL |
#endif
#ifdef AI_ADDRCONFIG
            AI_ADDRCONFIG |
#endif
#ifdef AI_V4MAPPED
            AI_V4MAPPED |
#endif
            0;

        hints.ai_family = p_hints->ai_family;
        hints.ai_socktype = p_hints->ai_socktype;
        hints.ai_protocol = p_hints->ai_protocol;
        /* Unfortunately, some flags chang the layout of struct addrinfo, so
         * they cannot be copied blindly from p_hints to &hints. Therefore, we
         * only copy flags that we know for sure are "safe".
         */
        hints.ai_flags = p_hints->ai_flags & safe_flags;
    }

#ifdef AI_NUMERICSERV
    /* We only ever use port *numbers* */
    hints.ai_flags |= AI_NUMERICSERV;
#endif

    if( hints.ai_family == AF_UNSPEC )
    {
        vlc_value_t val;

        var_Create( p_this, "ipv4", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );
        var_Get( p_this, "ipv4", &val );
        if( val.b_bool )
            hints.ai_family = AF_INET;

#ifdef AF_INET6
        var_Create( p_this, "ipv6", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );
        var_Get( p_this, "ipv6", &val );
        if( val.b_bool )
            hints.ai_family = AF_INET6;
#endif
    }

    /*
     * VLC extensions :
     * - accept "" as NULL
     * - ignore square brackets
     */
    if( ( node == NULL ) || (node[0] == '\0' ) )
    {
        psz_node = NULL;
    }
    else
    {
        strlcpy( psz_buf, node, NI_MAXHOST );

        psz_node = psz_buf;

        if( psz_buf[0] == '[' )
        {
            char *ptr;

            ptr = strrchr( psz_buf, ']' );
            if( ( ptr != NULL ) && (ptr[1] == '\0' ) )
            {
                *ptr = '\0';
                psz_node++;
            }
        }
    }

#ifdef WIN32
    /*
     * Winsock tries to resolve numerical IPv4 addresses as AAAA
     * and IPv6 addresses as A... There comes the bug-to-bug fix.
     */
    if ((hints.ai_flags & AI_NUMERICHOST) == 0)
    {
        hints.ai_flags |= AI_NUMERICHOST;

        if (getaddrinfo (psz_node, psz_service, &hints, res) == 0)
            return 0;

        hints.ai_flags &= ~AI_NUMERICHOST;
    }
#endif
#if defined (HAVE_GETADDRINFO)
# ifdef AI_IDN
    /* Run-time I18n Domain Names support */
    static bool b_idn = true; /* beware of thread-safety */

    if (b_idn)
    {
        hints.ai_flags |= AI_IDN;
        int ret = getaddrinfo (psz_node, psz_service, &hints, res);

        if (ret != EAI_BADFLAGS)
            return ret;

        /* IDN not available: disable and retry without it */
        hints.ai_flags &= ~AI_IDN;
        b_idn = false;
        msg_Info (p_this, "International Domain Names not supported");
    }
# endif
    return getaddrinfo (psz_node, psz_service, &hints, res);
#else
    int ret;
    vlc_value_t lock;

    var_Create (p_this->p_libvlc, "getaddrinfo_mutex", VLC_VAR_MUTEX);
    var_Get (p_this->p_libvlc, "getaddrinfo_mutex", &lock);
    vlc_mutex_lock (lock.p_address);

    ret = getaddrinfo (psz_node, psz_service, &hints, res);
    vlc_mutex_unlock (lock.p_address);
    return ret;
#endif
}


void vlc_freeaddrinfo( struct addrinfo *infos )
{
    freeaddrinfo (infos);
}
