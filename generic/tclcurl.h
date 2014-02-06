/*
 * tclcurl.h --
 *
 * Header file for the TclCurl extension to enable Tcl interpreters
 * to access libcurl.
 *
 * Copyright (c) 2001-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#if (defined(WIN32) || defined(_WIN32))
#define CURL_STATICLIB 1
#endif

#include <curl/curl.h>
#include <curl/easy.h>
#include <tcl.h>
#include <tclDecls.h>
#include <stdio.h>
#include <string.h>

#define _MPRINTF_REPLACE
#include <curl/mprintf.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_tclcurl should be undefined for Unix.
 * Actually I don't use this, but it was in TEA so I keep in case
 * I ever understand what it is for.
 */

#ifdef BUILD_tclcurl
#undef  TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif

#define TclCurlVersion "7.22.0"

/*
 * This struct will contain the data of a transfer if the user wants
 * to put the body into a Tcl variable
 */
struct MemoryStruct {
    char   *memory;
    size_t size;
};

/* 
 * Struct that will be used for a linked list with all the
 * data for a post
 */
struct formArrayStruct {
    struct curl_forms       *formArray;
    struct curl_slist       *formHeaderList;
    struct formArrayStruct  *next;
};

struct curlObjData {
    CURL                   *curl;
    Tcl_Command             token;
    Tcl_Command             shareToken;
    Tcl_Interp             *interp;
    struct curl_slist      *headerList;
    struct curl_slist      *quote;
    struct curl_slist      *prequote;
    struct curl_slist      *postquote;
    struct curl_httppost   *postListFirst;
    struct curl_httppost   *postListLast;
    struct formArrayStruct *formArray;
    char                   *outFile;
    FILE                   *outHandle;
    int                     outFlag;
    char                   *inFile;
    FILE                   *inHandle;
    int                     inFlag;
    char                   *proxy;
    int                     transferText;
    char                   *errorBuffer;
    char                   *errorBufferName;
    char                   *errorBufferKey;
    char                   *headerFile;
    FILE                   *headerHandle;
    int                     headerFlag;
    char                   *stderrFile;
    FILE                   *stderrHandle;
    int                     stderrFlag;
    char                   *randomFile;
    char                   *headerVar;
    char                   *bodyVarName;
    struct MemoryStruct     bodyVar;
    char                   *progressProc;
    char                   *cancelTransVarName;
    int                     cancelTrans;
    char                   *writeProc;
    char                   *readProc;
    char                   *debugProc;
    struct curl_slist      *http200aliases;
    char                   *command;
    int                     anyAuthFlag;
    char                   *sshkeycallProc;
    struct curl_slist      *mailrcpt;
    char                   *chunkBgnProc;
    char                   *chunkBgnVar;
    char                   *chunkEndProc;
    char                   *fnmatchProc;
    struct curl_slist      *resolve;
    struct curl_slist      *telnetoptions;
};

struct shcurlObjData {
    Tcl_Command           token;
    CURLSH               *shandle;
};

#ifndef multi_h

CONST static char *commandTable[] = {
    "setopt",
    "perform",
    "getinfo",
    "cleanup",
    "configure",
    "duphandle",
    "reset",
    "pause",
    "resume",
    (char *) NULL
};

CONST static char *optionTable[] = {
    "CURLOPT_URL",           "CURLOPT_FILE",            "CURLOPT_READDATA",
    "CURLOPT_USERAGENT",     "CURLOPT_REFERER",         "CURLOPT_VERBOSE",
    "CURLOPT_HEADER",        "CURLOPT_NOBODY",          "CURLOPT_PROXY",
    "CURLOPT_PROXYPORT",     "CURLOPT_HTTPPROXYTUNNEL", "CURLOPT_FAILONERROR",
    "CURLOPT_TIMEOUT",       "CURLOPT_LOW_SPEED_LIMIT", "CURLOPT_LOW_SPEED_TIME",
    "CURLOPT_RESUME_FROM",   "CURLOPT_INFILESIZE",      "CURLOPT_UPLOAD",
    "CURLOPT_FTPLISTONLY",   "CURLOPT_FTPAPPEND",       "CURLOPT_NETRC",
    "CURLOPT_FOLLOWLOCATION","CURLOPT_TRANSFERTEXT",    "CURLOPT_PUT",
    "CURLOPT_MUTE",          "CURLOPT_USERPWD",         "CURLOPT_PROXYUSERPWD",
    "CURLOPT_RANGE",         "CURLOPT_ERRORBUFFER",     "CURLOPT_HTTPGET",
    "CURLOPT_POST",          "CURLOPT_POSTFIELDS",      "CURLOPT_POSTFIELDSIZE",
    "CURLOPT_FTPPORT",       "CURLOPT_COOKIE",          "CURLOPT_COOKIEFILE",
    "CURLOPT_HTTPHEADER",    "CURLOPT_HTTPPOST",        "CURLOPT_SSLCERT",
    "CURLOPT_SSLCERTPASSWD", "CURLOPT_SSLVERSION",      "CURLOPT_CRLF",
    "CURLOPT_QUOTE",         "CURLOPT_POSTQUOTE",       "CURLOPT_WRITEHEADER",
    "CURLOPT_TIMECONDITION", "CURLOPT_TIMEVALUE",       "CURLOPT_CUSTOMREQUEST",
    "CURLOPT_STDERR",        "CURLOPT_INTERFACE",       "CURLOPT_KRB4LEVEL",
    "CURLOPT_SSL_VERIFYPEER","CURLOPT_CAINFO",          "CURLOPT_FILETIME",
    "CURLOPT_MAXREDIRS",     "CURLOPT_MAXCONNECTS",     "CURLOPT_CLOSEPOLICY",
    "CURLOPT_RANDOM_FILE",   "CURLOPT_EGDSOCKET",       "CURLOPT_CONNECTTIMEOUT",
    "CURLOPT_NOPROGRESS",    "CURLOPT_HEADERVAR",       "CURLOPT_BODYVAR",
    "CURLOPT_PROGRESSPROC","CURLOPT_CANCELTRANSVARNAME","CURLOPT_WRITEPROC",
    "CURLOPT_READPROC",      "CURLOPT_SSL_VERIFYHOST",  "CURLOPT_COOKIEJAR",
    "CURLOPT_SSL_CIPHER_LIST","CURLOPT_HTTP_VERSION",   "CURLOPT_FTP_USE_EPSV",
    "CURLOPT_SSLCERTTYPE",    "CURLOPT_SSLKEY",         "CURLOPT_SSLKEYTYPE",
    "CURLOPT_SSLKEYPASSWD",   "CURLOPT_SSL_ENGINE",     "CURLOPT_SSL_ENGINEDEFAULT",
    "CURLOPT_PREQUOTE",       "CURLOPT_DEBUGPROC",      "CURLOPT_DNS_CACHE_TIMEOUT",
    "CURLOPT_DNS_USE_GLOBAL_CACHE", "CURLOPT_COOKIESESSION","CURLOPT_CAPATH",
    "CURLOPT_BUFFERSIZE",     "CURLOPT_NOSIGNAL",       "CURLOPT_ENCODING",
    "CURLOPT_PROXYTYPE",      "CURLOPT_HTTP200ALIASES", "CURLOPT_UNRESTRICTED_AUTH",
    "CURLOPT_FTP_USE_EPRT",   "CURLOPT_NOSUCHOPTION",   "CURLOPT_HTTPAUTH",
    "CURLOPT_FTP_CREATE_MISSING_DIRS",                  "CURLOPT_PROXYAUTH",
    "CURLOPT_FTP_RESPONSE_TIMEOUT",                     "CURLOPT_IPRESOLVE",
    "CURLOPT_MAXFILESIZE",    "CURLOPT_NETRC_FILE",     "CURLOPT_FTP_SSL",
    "CURLOPT_SHARE",          "CURLOPT_PORT",           "CURLOPT_TCP_NODELAY",
    "CURLOPT_AUTOREFERER",    "CURLOPT_SOURCE_HOST",    "CURLOPT_SOURCE_USERPWD",
    "CURLOPT_SOURCE_PATH",    "CURLOPT_SOURCE_PORT",    "CURLOPT_PASV_HOST",
    "CURLOPT_SOURCE_PREQUOTE","CURLOPT_SOURCE_POSTQUOTE",
    "CURLOPT_FTPSSLAUTH",     "CURLOPT_SOURCE_URL",     "CURLOPT_SOURCE_QUOTE",
    "CURLOPT_FTP_ACCOUNT",    "CURLOPT_IGNORE_CONTENT_LENGTH",
    "CURLOPT_COOKIELIST",     "CURLOPT_FTP_SKIP_PASV_IP",
    "CURLOPT_FTP_FILEMETHOD", "CURLOPT_LOCALPORT",      "CURLOPT_LOCALPORTRANGE",
    "CURLOPT_MAX_SEND_SPEED_LARGE",                     "CURLOPT_MAX_RECV_SPEED_LARGE",
    "CURLOPT_FTP_ALTERNATIVE_TO_USER",                  "CURLOPT_SSL_SESSIONID_CACHE",
    "CURLOPT_SSH_AUTH_TYPES",                           "CURLOPT_SSH_PUBLIC_KEYFILE",
    "CURLOPT_SSH_PRIVATE_KEYFILE",                      "CURLOPT_TIMEOUT_MS",
    "CURLOPT_CONNECTTIMEOUT_MS",                        "CURLOPT_HTTP_CONTENT_DECODING",
    "CURLOPT_HTTP_TRANSFER_DECODING",                   "CURLOPT_KRBLEVEL",
    "CURLOPT_NEW_FILE_PERMS",                           "CURLOPT_NEW_DIRECTORY_PERMS",
    "CURLOPT_KEYPASSWD",      "CURLOPT_APPEND",         "CURLOPT_DIRLISTONLY",
    "CURLOPT_USE_SSL",        "CURLOPT_POST301",        "CURLOPT_SSH_HOST_PUBLIC_KEY_MD5",
    "CURLOPT_PROXY_TRANSFER_MODE",                      "CURLOPT_CRLFILE",
    "CURLOPT_ISSUERCERT",     "CURLOPT_ADDRESS_SCOPE",  "CURLOPT_CERTINFO",
    "CURLOPT_POSTREDIR",      "CURLOPT_USERNAME",       "CURLOPT_PASSWORD",
    "CURLOPT_PROXYUSERNAME",  "CURLOPT_PROXYPASSWORD",  "CURLOPT_TFTP_BLKSIZE",
    "CURLOPT_SOCKS5_GSSAPI_SERVICE",                    "CURLOPT_SOCKS5_GSSAPI_NEC",
    "CURLOPT_PROTOCOLS",      "CURLOPT_REDIR_PROTOCOLS","CURLOPT_FTP_SSL_CC",
    "CURLOPT_SSH_KNOWNHOSTS", "CURLOPT_SSH_KEYFUNCTION","CURLOPT_MAIL_FROM",
    "CURLOPT_MAIL_RCPT",      "CURLOPT_FTP_USE_PRET",   "CURLOPT_WILDCARDMATCH",
    "CURLOPT_CHUNK_BGN_PROC", "CURLOPT_CHUNK_BGN_VAR",  "CURLOPT_CHUNK_END_PROC",
    "CURLOPT_FNMATCH_PROC",   "CURLOPT_RESOLVE",        "CURLOPT_TLSAUTH_USERNAME",
    "CURLOPT_TLSAUTH_PASSWORD","CURLOPT_GSSAPI_DELEGATION", "CURLOPT_NOPROXY",
    "CURLOPT_TELNETOPTIONS",
    (char *)NULL
};

CONST static char *configTable[] = {
    "-url",               "-file",               "-infile",
    "-useragent",         "-referer",            "-verbose",
    "-header",            "-nobody",             "-proxy",
    "-proxyport",         "-httpproxytunnel",    "-failonerror",
    "-timeout",           "-lowspeedlimit",      "-lowspeedtime",
    "-resumefrom",        "-infilesize",         "-upload",
    "-ftplistonly",       "-ftpappend",          "-netrc",
    "-followlocation",    "-transfertext",       "-put",
    "-mute",              "-userpwd",            "-proxyuserpwd",
    "-range",             "-errorbuffer",        "-httpget",
    "-post",              "-postfields",         "-postfieldssize",
    "-ftpport",           "-cookie",             "-cookiefile",
    "-httpheader",        "-httppost",           "-sslcert",
    "-sslcertpasswd",     "-sslversion",         "-crlf",
    "-quote",             "-postquote",          "-writeheader",
    "-timecondition",     "-timevalue",          "-customrequest",
    "-stderr",            "-interface",          "-krb4level",
    "-sslverifypeer",     "-cainfo",             "-filetime",
    "-maxredirs",         "-maxconnects",        "-closepolicy",
    "-randomfile",        "-egdsocket",          "-connecttimeout",
    "-noprogress",        "-headervar",          "-bodyvar",
    "-progressproc",      "-canceltransvarname", "-writeproc",
    "-readproc",          "-sslverifyhost",      "-cookiejar",
    "-sslcipherlist",     "-httpversion",        "-ftpuseepsv",
    "-sslcerttype",       "-sslkey",             "-sslkeytype",
    "-sslkeypasswd",      "-sslengine",          "-sslenginedefault",
    "-prequote",          "-debugproc",          "-dnscachetimeout",
    "-dnsuseglobalcache", "-cookiesession",      "-capath",
    "-buffersize",        "-nosignal",           "-encoding",
    "-proxytype",         "-http200aliases",     "-unrestrictedauth",
    "-ftpuseeprt",        "-command",            "-httpauth",
    "-ftpcreatemissingdirs",                     "-proxyauth",
    "-ftpresponsetimeout",                       "-ipresolve",
    "-maxfilesize",       "-netrcfile",          "-ftpssl",
    "-share",             "-port",               "-tcpnodelay",
    "-autoreferer",       "-sourcehost",         "-sourceuserpwd",
    "-sourcepath",        "-sourceport",         "-pasvhost",
    "-sourceprequote",    "-sourcepostquote",    "-ftpsslauth",
    "-sourceurl",         "-sourcequote",        "-ftpaccount",
    "-ignorecontentlength",                      "-cookielist",
    "-ftpskippasvip",     "-ftpfilemethod",      "-localport",
    "-localportrange",
    "-maxsendspeed",                             "-maxrecvspeed",
    "-ftpalternativetouser",                     "-sslsessionidcache",
    "-sshauthtypes",      "-sshpublickeyfile",   "-sshprivatekeyfile",
    "-timeoutms",         "-connecttimeoutms",   "-contentdecoding",
    "-transferdecoding",  "-krblevel",           "-newfileperms",
    "-newdirectoryperms", "-keypasswd",          "-append",
    "-dirlistonly",       "-usessl",             "-post301",
    "-sshhostpublickeymd5",                      "-proxytransfermode",
    "-crlfile",           "-issuercert",         "-addressscope",
    "-certinfo",          "-postredir",          "-username",
    "-password",          "-proxyuser",          "-proxypassword",
    "-tftpblksize",       "-socks5gssapiservice","-socks5gssapinec",
    "-protocols",         "-redirprotocols",     "-ftpsslcc",
    "-sshknownhosts",     "-sshkeyproc",         "-mailfrom",
    "-mailrcpt",          "-ftpusepret",         "-wildcardmatch",
    "-chunkbgnproc",      "-chunkbgnvar",        "-chunkendproc",
    "-fnmatchproc",       "-resolve",            "-tlsauthusername",
    "-tlsauthpassword",   "-gssapidelegation",   "-noproxy",
    "-telnetoptions",
    (char *) NULL
};

CONST static char    *timeCond[] = {
    "ifmodsince", "ifunmodsince",
    (char *)NULL
};

CONST static char    *getInfoTable[]={
    "effectiveurl",   "httpcode",       "responsecode",
    "filetime",       "totaltime",      "namelookuptime",
    "connecttime",    "pretransfertime","sizeupload",
    "sizedownload",   "speeddownload",  "speedupload",
    "headersize",     "requestsize",    "sslverifyresult",
    "contentlengthdownload",            "contentlengthupload",
    "starttransfertime",                "contenttype",
    "redirecttime",   "redirectcount",  "httpauthavail",
    "proxyauthavail", "oserrno",        "numconnects",
    "sslengines",     "httpconnectcode","cookielist",
    "ftpentrypath",   "redirecturl",    "primaryip",
    "appconnecttime", "certinfo",       "conditionunmet",
    "primaryport",    "localip",        "localport",
    (char *)NULL
};

CONST static char   *curlFormTable[]={
    "name",  "contents", "file", "contenttype", "contentheader", "filename",
    "bufferName", "buffer", "filecontent", (char *)NULL
};

CONST static char   *httpVersionTable[] = {
    "none", "1.0", "1.1", (char *)NULL
};

CONST static char *netrcTable[] = {
    "optional", "ignored", "required", (char *)NULL
};

CONST static char *encodingTable[] = {
    "identity", "deflated", "all", (char *)NULL
};

CONST static char *versionInfoTable[] = {
    "-version",    "-versionnum",    "-host",         "-features",
    "-sslversion", "-sslversionnum", "-libzversion",
    "-protocols",  (char *)NULL
};

CONST static char *proxyTypeTable[] = {
    "http", "http1.0", "socks4", "socks4a", "socks5", "socks5h", (char *)NULL
};

CONST static char *httpAuthMethods[] = {
    "basic", "digest", "digestie", "gssnegotiate", "ntlm", "any", "anysafe", "ntlmwb",(char *)NULL
};

CONST static char *ipresolve[] = {
    "whatever", "v4", "v6", (char *)NULL
};

CONST static char *ftpssl[] = {
    "nope", "try", "control", "all", (char *)NULL
};

CONST static char *shareCmd[] = {
    "share", "unshare", "cleanup", (char *)NULL
};

CONST static char *lockData[] = {
    "cookies", "dns", (char *)NULL
};

CONST static char *ftpsslauth[] = {
    "default", "ssl", "tls", (char *)NULL
};

CONST static char *ftpsslccc[] = {
    "none", "passive", "active", (char *)NULL
};

CONST static char *sslversion[] = {
    "default", "tlsv1", "sslv2", "sslv3", (char *)NULL
};

CONST static char *ftpfilemethod[] = {
    "default", "multicwd", "nocwd", "singlecwd", (char *)NULL
};

CONST static char *sshauthtypes[] = {
    "publickey", "password", "host", "keyboard", "any", (char *)NULL
};

CONST static char *postredir[] = {
    "301", "302", "all", (char *)NULL
};

CONST static char *protocolNames[] = {
    "http", "https", "ftp", "ftps", "scp", "sftp", "telnet", "ldap",
    "ldaps","dict",  "file","tftp", "all", "imap", "imaps", "pop3",
    "pop3s", "smtp", "smtps", "rtsp", "rtmp", "rtmpt", "rtmpe", 
    "rtmpte", "rtmps", "rtmpts", "gopher", (char*)NULL
};

CONST static char *tlsauth[] = {
    "none", "srp", (char *)NULL
};

CONST static char *gssapidelegation[] = {
    "flag", "policyflag", (char *) NULL
};

int curlseek(void *instream, curl_off_t offset, int origin);

int Tclcurl_MultiInit (Tcl_Interp *interp);

#endif

int Tclcurl_Init(Tcl_Interp *interp);

char *curlCreateObjCmd(Tcl_Interp *interp,struct curlObjData  *curlData);
int curlInitObjCmd(ClientData clientData, Tcl_Interp *interp, int objc,
        Tcl_Obj *CONST objv[]);
int curlObjCmd(ClientData clientData, Tcl_Interp *interp, int objc,
        Tcl_Obj *CONST objv[]);
int curlDeleteCmd(ClientData clientData);

int curlPerform(Tcl_Interp *interp,CURL *curlHandle,struct curlObjData *curlData);

int curlSetOptsTransfer(Tcl_Interp *interp, struct curlObjData *curlData,int objc,
        Tcl_Obj *CONST objv[]);

int curlConfigTransfer(Tcl_Interp *interp, struct curlObjData *curlData,int objc,
        Tcl_Obj *CONST objv[]);


int curlDupHandle(Tcl_Interp *interp, struct curlObjData *curlData,int objc,
        Tcl_Obj *CONST objv[]);

int curlResetHandle(Tcl_Interp *interp, struct curlObjData *curlData);

int curlSetOpts(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *CONST objv,int tableIndex);

int SetoptInt(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,
            Tcl_Obj *tclObj);
int SetoptLong(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,
            Tcl_Obj *tclObj);
int SetoptCurlOffT(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,
            Tcl_Obj *tclObj);
int SetoptChar(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
            int tableIndex,Tcl_Obj *tclObj);
int SetoptSHandle(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj);
int SetoptsList(Tcl_Interp *interp,struct curl_slist **slistPtr,Tcl_Obj *CONST objv);

CURLcode curlGetInfo(Tcl_Interp *interp,CURL *curlHandle,int tableIndex);

void curlFreeSpace(struct curlObjData *curlData);

void curlErrorSetOpt(Tcl_Interp *interp,CONST char **configTable, int option,CONST char *parPtr);

size_t curlHeaderReader(void *ptr,size_t size,size_t nmemb,FILE *stream);

size_t curlBodyReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);

int curlProgressCallback(void *clientp,double dltotal,double dlnow,
        double ultotal,double ulnow);

size_t curlWriteProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);
size_t curlReadProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);

long curlChunkBgnProcInvoke (const void *transfer_info, void *curlDataPtr, int remains);
long curlChunkEndProcInvoke (void *curlDataPtr);
int curlfnmatchProcInvoke(void *curlDataPtr, const char *pattern, const char *filename);

/* Puts a ssh key into a Tcl object */
Tcl_Obj *curlsshkeyextract(Tcl_Interp *interp,const struct curl_khkey *key);

/* Function that will be invoked by libcurl to see what the user wants to
   do about the new ssh host */
size_t curlsshkeycallback(CURL *easy,                        /* easy handle */
                          const struct curl_khkey *knownkey, /* known */
                          const struct curl_khkey *foundkey, /* found */
                          enum curl_khmatch,                 /* libcurl's view on the keys */
                          void *curlData);

int curlDebugProcInvoke(CURL *curlHandle, curl_infotype infoType,
        char * dataPtr, size_t size, void  *curlData);

int curlVersion (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]);

int curlEscape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]);

int curlUnescape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]);

int curlVersionInfo (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]);

int curlCopyCurlData (struct curlObjData *curlDataOld,
                      struct curlObjData *curlDataNew);

int curlOpenFile(Tcl_Interp *interp,char *fileName, FILE **handle, int writing, int text);

int  curlOpenFiles (Tcl_Interp *interp,struct curlObjData *curlData);
void curlCloseFiles(struct curlObjData *curlData);

int curlSetPostData(Tcl_Interp *interp,struct curlObjData *curlData);
void curlResetPostData(struct curlObjData *curlDataPtr);
void curlResetFormArray(struct curl_forms *formArray);

void curlSetBodyVarName(Tcl_Interp *interp,struct curlObjData *curlDataPtr);

char *curlstrdup (char *old);


char *curlCreateShareObjCmd (Tcl_Interp *interp,struct shcurlObjData  *shcurlData);
int curlShareInitObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);
int curlShareObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);
int curlCleanUpShareCmd(ClientData clientData);

#ifdef TCL_THREADS
    TCL_DECLARE_MUTEX(cookieLock)
    TCL_DECLARE_MUTEX(dnsLock)
    TCL_DECLARE_MUTEX(sslLock)
    TCL_DECLARE_MUTEX(connectLock)

    void curlShareLockFunc (CURL *handle, curl_lock_data data
            , curl_lock_access access, void *userptr);
    void curlShareUnLockFunc(CURL *handle, curl_lock_data data, void *userptr);
#endif

int curlErrorStrings (Tcl_Interp *interp, Tcl_Obj *CONST objv,int type);
int curlEasyStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);
int curlShareStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);
int curlMultiStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);

#ifdef  __cplusplus
}
#endif
