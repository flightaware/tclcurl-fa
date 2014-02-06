/*
 * multi.h --
 *
 * Header file for the part of the TclCurl extension that deals with libcurl's
 * 'multi' interface.
 *
 * Copyright (c) 2002-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#define multi_h
#include "tclcurl.h"


#ifdef  __cplusplus
extern "C" {
#endif 

struct easyHandleList {
    CURL                    *curl;
    char                    *name;
    struct easyHandleList   *next;
};

struct curlMultiObjData {
    CURLM                 *mcurl;
    Tcl_Command            token;
    Tcl_Interp            *interp;
    struct easyHandleList *handleListFirst;
    struct easyHandleList *handleListLast;
    fd_set                 fdread;
    fd_set                 fdwrite;
    fd_set                 fdexcep;
    int                    runningTransfers;
    char                  *postCommand;    
};

struct curlEvent {
    Tcl_EventProc           *proc;
    struct Tcl_Event        *nextPtr;
    struct curlMultiObjData *curlMultiData;
};

CONST static char *multiCommandTable[] = {
    "addhandle",
    "removehandle",
    "perform",
    "cleanup",
    "getinfo",
    "active",
    "auto",
    "configure",
    (char *) NULL
};

CONST static char *multiConfigTable[] = {
    "-pipelining", "-maxconnects",
    (char *)NULL
};

char *curlCreateMultiObjCmd (Tcl_Interp *interp,struct curlMultiObjData *curlMultiData);

int Tclcurl_MultiInit (Tcl_Interp *interp);

int curlMultiDeleteCmd(ClientData clientData);

int curlInitMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]);

int curlMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]);

CURLMcode curlAddMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandle
        ,Tcl_Obj *objvPtr);

CURLMcode curlRemoveMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandle
        ,Tcl_Obj *objvPtr);

int curlMultiPerform(Tcl_Interp *interp,CURLM *curlMultiHandle);

int curlMultiGetInfo(Tcl_Interp *interp,CURLM *curlMultiHandlePtr);

int curlMultiGetActiveTransfers( struct curlMultiObjData *curlMultiData);
int curlMultiActiveTransfers(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData);

struct curlObjData *curlGetEasyHandle(Tcl_Interp *interp,Tcl_Obj *nameObjPtr);

void curlMultiFreeSpace(struct curlMultiObjData *curlMultiData);

int curlReturnCURLMcode(Tcl_Interp *interp,CURLMcode errorCode);

void curlEasyHandleListAdd(struct curlMultiObjData *multiDataPtr,CURL *easyHandle,char *name);
void curlEasyHandleListRemove(struct curlMultiObjData *multiDataPtr,CURL *easyHandle);
char *curlGetEasyName(struct curlMultiObjData *multiDataPtr,CURL *easyHandle);

int curlMultiAutoTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData, int objc,Tcl_Obj *CONST objv[]);
int curlMultiSetOpts(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData,Tcl_Obj *CONST objv,int tableIndex);
int SetMultiOptLong(Tcl_Interp *interp,CURLM *curlMultiHandle,CURLMoption opt,
        int tableIndex,Tcl_Obj *tclObj);

int curlMultiConfigTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData, int objc,Tcl_Obj *CONST objv[]);

void curlEventSetup(ClientData clientData, int flags);

void curlEventCheck(ClientData clientData, int flags);

int curlEventProc(Tcl_Event *evPtr,int flags);

#ifdef  __cplusplus
}

#endif
