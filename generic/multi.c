/*
 * multi.c --
 *
 * Implementation of the part of the TclCurl extension that deals with libcurl's
 * 'multi' interface.
 *
 * Copyright (c)2002-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "multi.h"
#include <sys/time.h>

/*
 *----------------------------------------------------------------------
 *
 * Tclcurl_MultiInit --
 *
 *	This procedure initializes the 'multi' part of the package
 *
 * Results:
 *	A standard Tcl result.
 *
 *----------------------------------------------------------------------
 */

int
Tclcurl_MultiInit (Tcl_Interp *interp) {

    Tcl_CreateObjCommand (interp,"::curl::multiinit",curlInitMultiObjCmd,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCreateMultiObjCmd --
 *
 *	Looks for the first free handle (mcurl1, mcurl2,...) and creates a
 *	Tcl command for it.
 *
 * Results:
 *  A string with the name of the handle, don't forget to free it.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

char *
curlCreateMultiObjCmd (Tcl_Interp *interp,struct curlMultiObjData *curlMultiData) {
    char                *handleName;
    int                 i;
    Tcl_CmdInfo         info;
    Tcl_Command         cmdToken;

    /* We try with mcurl1, if it already exists with mcurl2, ... */
    handleName=(char *)Tcl_Alloc(10);
    for (i=1;;i++) {
        sprintf(handleName,"mcurl%d",i);
        if (!Tcl_GetCommandInfo(interp,handleName,&info)) {
            cmdToken=Tcl_CreateObjCommand(interp,handleName,curlMultiObjCmd,
                                (ClientData)curlMultiData, 
                                (Tcl_CmdDeleteProc *)curlMultiDeleteCmd);
            break;
        }
    }

    curlMultiData->token=cmdToken;

    return handleName;
}

/*
 *----------------------------------------------------------------------
 *
 * curlInitMultiObjCmd --
 *
 *	This procedure is invoked to process the "curl::multiInit" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
curlInitMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {


    Tcl_Obj                     *result;
    struct curlMultiObjData     *curlMultiData;
    char                        *multiHandleName;

    curlMultiData=(struct curlMultiObjData *)Tcl_Alloc(sizeof(struct curlMultiObjData));
    if (curlMultiData==NULL) {
        result=Tcl_NewStringObj("Couldn't allocate memory",-1);
        Tcl_SetObjResult(interp,result); 
        return TCL_ERROR;
    }

    memset(curlMultiData, 0, sizeof(struct curlMultiObjData));
    curlMultiData->interp=interp;

    curlMultiData->mcurl=curl_multi_init();

    if (curlMultiData->mcurl==NULL) {
        result=Tcl_NewStringObj("Couldn't open curl multi handle",-1);
        Tcl_SetObjResult(interp,result); 
        return TCL_ERROR;
    }

    multiHandleName=curlCreateMultiObjCmd(interp,curlMultiData);

    result=Tcl_NewStringObj(multiHandleName,-1);
    Tcl_SetObjResult(interp,result);
    Tcl_Free(multiHandleName);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiObjCmd --
 *
 *	This procedure is invoked to process the "multi curl" commands.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    struct curlMultiObjData    *curlMultiData=(struct curlMultiObjData *)clientData;
    CURLMcode                   errorCode;
    int                         tableIndex;

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"option arg ?arg?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], multiCommandTable, "option",
            TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(tableIndex) {
        case 0:
/*            fprintf(stdout,"Multi add handle\n"); */
            errorCode=curlAddMultiHandle(interp,curlMultiData->mcurl,objv[2]);
            return curlReturnCURLMcode(interp,errorCode);
            break;
        case 1:
/*            fprintf(stdout,"Multi remove handle\n"); */
            errorCode=curlRemoveMultiHandle(interp,curlMultiData->mcurl,objv[2]);
            return curlReturnCURLMcode(interp,errorCode);
            break;
        case 2:
/*            fprintf(stdout,"Multi perform\n"); */
            errorCode=curlMultiPerform(interp,curlMultiData->mcurl);
            return errorCode;
            break;
        case 3:
/*            fprintf(stdout,"Multi cleanup\n"); */
            Tcl_DeleteCommandFromToken(interp,curlMultiData->token);
            break;
        case 4:
/*            fprintf(stdout,"Multi getInfo\n"); */
            curlMultiGetInfo(interp,curlMultiData->mcurl);
            break;
        case 5:
/*            fprintf(stdout,"Multi activeTransfers\n"); */
            curlMultiActiveTransfers(interp,curlMultiData);
            break;
        case 6:
/*            fprintf(stdout,"Multi auto transfer\n");*/
            curlMultiAutoTransfer(interp,curlMultiData,objc,objv);
            break;
        case 7:
/*            fprintf(stdout,"Multi configure\n");*/
            curlMultiConfigTransfer(interp,curlMultiData,objc,objv);
            break;            
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlAddMultiHandle --
 *
 *	Adds an 'easy' curl handle to the stack of a 'multi' handle.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlMultiHandle: The handle into which we will add the easy one.
 *      objvPtr: The Tcl object with the name of the easy handle.
 *
 * Results:
 *  '0' all went well.
 *  'non-zero' in case of error.
 *----------------------------------------------------------------------
 */
CURLMcode
curlAddMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandlePtr
        ,Tcl_Obj *objvPtr) {

    struct curlObjData        *curlDataPtr;
    CURLMcode                  errorCode;


    curlDataPtr=curlGetEasyHandle(interp,objvPtr);

    if (curlOpenFiles(interp,curlDataPtr)) {
        return TCL_ERROR;
    }
    if (curlSetPostData(interp,curlDataPtr)) {
        return TCL_ERROR;
    }

    errorCode=curl_multi_add_handle(curlMultiHandlePtr,curlDataPtr->curl);

    curlEasyHandleListAdd(curlMultiHandlePtr,curlDataPtr->curl
            ,Tcl_GetString(objvPtr));

    return errorCode;
}

/*
 *----------------------------------------------------------------------
 *
 * curlRemoveMultiHandle --
 *
 *	Removes an 'easy' curl handle to the stack of a 'multi' handle.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlMultiHandle: The handle into which we will add the easy one.
 *      objvPtr: The Tcl object with the name of the easy handle.
 *
 * Results:
 *  '0' all went well.
 *  'non-zero' in case of error.
 *----------------------------------------------------------------------
 */
CURLMcode
curlRemoveMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandle
        ,Tcl_Obj *objvPtr) {
    struct curlObjData        *curlDataPtr;
    CURLMcode                  errorCode;

    curlDataPtr=curlGetEasyHandle(interp,objvPtr);
    errorCode=curl_multi_remove_handle(curlMultiHandle,curlDataPtr->curl);
    curlEasyHandleListRemove(curlMultiHandle,curlDataPtr->curl);

    curlCloseFiles(curlDataPtr);
    curlResetPostData(curlDataPtr);

    if (curlDataPtr->bodyVarName) {
        curlSetBodyVarName(interp,curlDataPtr);
    }

    return errorCode;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiPerform --
 *
 *	Invokes the 'curl_multi_perform' function to update the current
 *  transfers.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlMultiHandle: The handle of the transfer to update.
 *      objvPtr: The Tcl object with the name of the easy handle.
 *
 * Results:
        Usual Tcl result.
 *----------------------------------------------------------------------
 */
int
curlMultiPerform(Tcl_Interp *interp,CURLM *curlMultiHandlePtr) {

    CURLMcode        errorCode;
    int              runningTransfers;

    for (errorCode=-1;errorCode<0;) {   
        errorCode=curl_multi_perform(curlMultiHandlePtr,&runningTransfers);
    }

    if (errorCode==0) {
        curlReturnCURLMcode(interp,runningTransfers);
        return TCL_OK;
    }

    curlReturnCURLMcode(interp,errorCode);

    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiDeleteCmd --
 *
 *	This procedure is invoked when curl multi handle is deleted.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Cleans the curl handle and frees the memory.
 *
 *----------------------------------------------------------------------
 */
int
curlMultiDeleteCmd(ClientData clientData) {
    struct curlMultiObjData     *curlMultiData=(struct curlMultiObjData *)clientData;
    CURLM                       *curlMultiHandle=curlMultiData->mcurl;
    CURLMcode                    errorCode;
    Tcl_Interp                  *interp=curlMultiData->interp;
    struct easyHandleList       *listPtr1,*listPtr2;

    listPtr1=curlMultiData->handleListFirst;
    while (listPtr1!=NULL) {
        listPtr2=listPtr1->next;
        Tcl_Free(listPtr1->name);
        Tcl_Free((char *)listPtr1); 
        listPtr1=listPtr2;
    }
    errorCode=curl_multi_cleanup(curlMultiHandle);
    curlMultiFreeSpace(curlMultiData);
    return curlReturnCURLMcode(interp,errorCode);
}

/*
 *----------------------------------------------------------------------
 *
 * curlGetMultiInfo --
 *    Invokes the curl_multi_info_read function in libcurl to get
 *    some info about the transfer, like if they are done and
 *    things like that.
 *
 * Parameter:
 *    interp: The Tcl interpreter we are using, mainly to report errors.
 *    curlMultiHandlePtr: Pointer to the multi handle of the transfer.
 *
 * Results:
 *    Standard Tcl codes. The Tcl command will return a list with the
 *    name of the Tcl command and other info.
 *----------------------------------------------------------------------
 */
int
curlMultiGetInfo(Tcl_Interp *interp,CURLM *curlMultiHandlePtr) {
    struct CURLMsg        *multiInfo;
    int                    msgLeft;
    Tcl_Obj               *resultPtr;

    multiInfo=curl_multi_info_read(curlMultiHandlePtr, &msgLeft);
    resultPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL); 
    if (multiInfo==NULL) {
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewStringObj("",-1));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(0));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(0));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(0));
    } else {
        Tcl_ListObjAppendElement(interp,resultPtr,
            Tcl_NewStringObj(curlGetEasyName(curlMultiHandlePtr,multiInfo->easy_handle),-1));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(multiInfo->msg));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(multiInfo->data.result));
        Tcl_ListObjAppendElement(interp,resultPtr,Tcl_NewIntObj(msgLeft));
    }
    Tcl_SetObjResult(interp,resultPtr); 

    return TCL_OK;            
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiActiveTransfers --
 *    This function is used to know whether an connection is ready to
 *    transfer data. This code has been copied almost verbatim from
 *    libcurl's examples.
 *
 * Parameter:
 *    multiHandlePtr: Pointer to the multi handle of the transfer.
 *
 * Results:
 *    
 *----------------------------------------------------------------------
 */
int
curlMultiGetActiveTransfers( struct curlMultiObjData *curlMultiData) {
    struct timeval  timeout;
    int             selectCode;
    int             maxfd;

    FD_ZERO(&(curlMultiData->fdread));
    FD_ZERO(&(curlMultiData->fdwrite));
    FD_ZERO(&(curlMultiData->fdexcep));

    /* set a suitable timeout to play around with */
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    /* get file descriptors from the transfers */
    curl_multi_fdset(curlMultiData->mcurl,
            &(curlMultiData->fdread),
            &(curlMultiData->fdwrite),
            &(curlMultiData->fdexcep), &maxfd);

    selectCode = select(maxfd+1, &(curlMultiData->fdread)
            , &(curlMultiData->fdwrite), &(curlMultiData->fdexcep)
            , &timeout);

    return selectCode;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiActiveTransfers --
 *    Implements the Tcl 'active', it queries the multi handle to know
 *    if any of the connections are ready to transfer data.
 *
 * Parameter:
 *    interp: The Tcl interpreter we are using, mainly to report errors.
 *    curlMultiHandlePtr: Pointer to the multi handle of the transfer.
 *
 * Results:
 *    Standard Tcl codes. The Tcl command will return the number of
 *    transfers.
 *----------------------------------------------------------------------
 */
int
curlMultiActiveTransfers(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData) {
    int             selectCode;
    Tcl_Obj        *resultPtr;

    selectCode = curlMultiGetActiveTransfers(curlMultiData);

    if (selectCode==-1) {
        return TCL_ERROR;
    }

    resultPtr=Tcl_NewIntObj(selectCode);
    Tcl_SetObjResult(interp,resultPtr);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlGetEasyHandle --
 *
 *	Given the name of an easy curl handle (curl1,...), in a Tcl object
 *  this function  will return the pointer the 'internal' libcurl handle.
 *
 * Parameter:
 *  The Tcl object with the name.
 *
 * Results:
 *  The pointer to the libcurl handle
 *----------------------------------------------------------------------
 */
struct curlObjData *
curlGetEasyHandle(Tcl_Interp *interp,Tcl_Obj *nameObjPtr) {

    char                    *handleName;
    Tcl_CmdInfo             *infoPtr=(Tcl_CmdInfo *)Tcl_Alloc(sizeof(Tcl_CmdInfo));
    struct curlObjData      *curlDataPtr;

    handleName=Tcl_GetString(nameObjPtr);

    if (0==Tcl_GetCommandInfo(interp,handleName,infoPtr)) {
        return NULL;
    }
    curlDataPtr=(struct curlObjData *)(infoPtr->objClientData);
    Tcl_Free((char *)infoPtr);
    return curlDataPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiFreeSpace --
 *
 *	Frees the space taken by a curlMultiObjData struct.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlMultiHandle: the curl handle for which the option is set.
 *      objc and objv: The usual in Tcl.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
void
curlMultiFreeSpace(struct curlMultiObjData *curlMultiData) {

    curl_multi_cleanup(curlMultiData->mcurl);

    Tcl_Free(curlMultiData->postCommand);
    Tcl_Free((char *)curlMultiData);
}

/*
 *----------------------------------------------------------------------
 *
 * curlEasyHandleListAdd
 *	Adds an easy handle to the list of handles in a multiHandle.
 *
 *  Parameter:
 *      multiDataPtr: Pointer to the struct of the multi handle.
 *      easyHandle: The easy handle to add to the list.
 *
 * Results:
 *----------------------------------------------------------------------
 */
void
curlEasyHandleListAdd(struct curlMultiObjData *multiDataPtr,CURL *easyHandlePtr,char *name) {
    struct easyHandleList    *easyHandleListPtr;

    easyHandleListPtr=(struct easyHandleList *)Tcl_Alloc(sizeof(struct easyHandleList));
    easyHandleListPtr->curl   =easyHandlePtr;
    easyHandleListPtr->name   =curlstrdup(name);
    easyHandleListPtr->next=NULL;
    if (multiDataPtr->handleListLast==NULL) {
        multiDataPtr->handleListFirst=easyHandleListPtr;
        multiDataPtr->handleListLast =easyHandleListPtr;
    } else {
        multiDataPtr->handleListLast->next=easyHandleListPtr;
        multiDataPtr->handleListLast=easyHandleListPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * curlEasyHandleListRemove
 *	When we remove an easy handle from the multiHandle, this function
 *  will remove said handle from the linked list.
 *
 *  Parameter:
 *      multiDataPtr: Pointer to the struct of the multi handle.
 *      easyHandle: The easy handle to add to the list.
 *
 * Results:
 *----------------------------------------------------------------------
 */
void
curlEasyHandleListRemove(struct curlMultiObjData *multiDataPtr,CURL *easyHandle) {
    struct easyHandleList *listPtr1,*listPtr2;

    listPtr1=NULL;
    listPtr2=multiDataPtr->handleListFirst;
    while(listPtr2!=NULL) {
        if (listPtr2->curl==easyHandle) {
            if (listPtr1==NULL) {
                multiDataPtr->handleListFirst=listPtr2->next;
            } else {
                listPtr1->next=listPtr2->next;
            }
            if (listPtr2==multiDataPtr->handleListLast) {
                multiDataPtr->handleListLast=listPtr1;
            }
            Tcl_Free(listPtr2->name);
            Tcl_Free((char *)listPtr2);
            break;
        }
        listPtr1=listPtr2;
        listPtr2=listPtr2->next;
    }
}
/*
 *----------------------------------------------------------------------
 *
 * curlGetEasyName
 *
 *	Given the pointer to an easy handle, this function will return
 *  the name of the Tcl command.
 *
 *  Parameter:
 *      multiDataPtr: Multi handle we are using.
 *      easyHandle: The easy handle whose Tcl command we want to know.
 *
 * Results:
 *  A string with the name of the command.
 *----------------------------------------------------------------------
 */
char *
curlGetEasyName(struct curlMultiObjData *multiDataPtr,CURL *easyHandle) {
    struct easyHandleList    *listPtr;

    listPtr=multiDataPtr->handleListFirst;
    while(listPtr!=NULL) {
        if (listPtr->curl==easyHandle) {
            return listPtr->name;
        }
        listPtr=listPtr->next;
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * curlReturnCURLMcode
 *
 *	When one of the command wants to return a CURLMcode, it calls
 *  this function.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      errorCode: the value to be returned.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlReturnCURLMcode (Tcl_Interp *interp,CURLMcode errorCode) {
    Tcl_Obj        *resultPtr;

    resultPtr=Tcl_NewIntObj(errorCode);
    Tcl_SetObjResult(interp,resultPtr);

    if (errorCode>0) {
        return TCL_ERROR;
    }
    return TCL_OK;
}


/*----------------------------------------------------------------------
 *
 * curlMultiAutoTransfer --
 *
 *	Creates the event source that will take care of downloading using
 *  the multi interface driven by Tcl's event loop.
 *
 * Parameters:
 *  The usual Tcl command parameters.
 *
 * Results:
 *	Standard Tcl return code.
 *----------------------------------------------------------------------
 */

int
curlMultiAutoTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData,
        int objc,Tcl_Obj *CONST objv[]) {

    if (objc==4) {
        Tcl_Free(curlMultiData->postCommand);
        curlMultiData->postCommand=curlstrdup(Tcl_GetString(objv[3]));
    }

    Tcl_CreateEventSource((Tcl_EventSetupProc *)curlEventSetup, 
            (Tcl_EventCheckProc *)curlEventCheck, (ClientData *)curlMultiData);

    /* We have to call perform once to boot the transfer, otherwise it seems nothing
       works *shrug* */

    while(CURLM_CALL_MULTI_PERFORM ==
            curl_multi_perform(curlMultiData->mcurl,&(curlMultiData->runningTransfers))) {
    }

    return TCL_OK;
}

/*----------------------------------------------------------------------
 *
 * curlMultiConfigTrasnfer --
 *  This procedure is invoked by the user command 'configure', it reads 
 *  the options passed by the user to configure a multi handle.
 *
 * Parameters:
 *  The usual Tcl command parameters.
 *
 * Results:
 *	Standard Tcl return code.
 *----------------------------------------------------------------------
 */

int
curlMultiConfigTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData,
        int objc,Tcl_Obj *CONST objv[]) {
    int            tableIndex;
    int            i,j;

    Tcl_Obj       *resultPtr;
    char           errorMsg[500];

    for(i=2,j=3;i<objc;i=i+2,j=j+2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], multiConfigTable, "option", 
                TCL_EXACT, &tableIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
        if (i==objc-1) {
            snprintf(errorMsg,500,"Empty value for %s",multiConfigTable[tableIndex]);
            resultPtr=Tcl_NewStringObj(errorMsg,-1);
            Tcl_SetObjResult(interp,resultPtr);            
            return TCL_ERROR;
        }
        if (curlMultiSetOpts(interp,curlMultiData,objv[j],tableIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiSetOpts --
 *
 *	This procedure takes care of setting the transfer options.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlMultiHandle: the curl handle for which the option is set.
 *      objv: A pointer to the object where the data to set is stored.
 *      tableIndex: The index of the option in the options table.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlMultiSetOpts(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData,
        Tcl_Obj *CONST objv,int tableIndex) {

    switch(tableIndex) {
        case 0:
            if (SetMultiOptLong(interp,curlMultiData->mcurl,
                    CURLMOPT_PIPELINING,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 1:
            if (SetMultiOptLong(interp,curlMultiData->mcurl,
                    CURLMOPT_MAXCONNECTS,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetMultiOptLong --
 *
 *	Set the curl options that require a long
 *
 *  Parameter:
 *      interp: The interpreter we are working with.
 *      curlMultiHandle: and the multi curl handle
 *      opt: the option to set
 *      tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetMultiOptLong(Tcl_Interp *interp,CURLM *curlMultiHandle,CURLMoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    long        longNumber;
    char        *parPtr;

    if (Tcl_GetLongFromObj(interp,tclObj,&longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,multiConfigTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }
    if (curl_multi_setopt(curlMultiHandle,opt,longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,multiConfigTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *
 * curlEventSetup --
 *
 *  This function is invoked by Tcl just after curlMultiAutoTransfer and
 *  then every time just before curlEventCheck, I only use to set the
 *  maximun time without checking for events
 *
 *  NOTE: I hate having a fixed value, I will have to look into it.
 *
 * Parameters:
 *  They are passed automagically by Tcl, but I don't use them.
 *----------------------------------------------------------------------
 */

void
curlEventSetup(ClientData clientData, int flags) {
    Tcl_Time     time = {0 , 0};

    Tcl_SetMaxBlockTime(&time);
}

/*----------------------------------------------------------------------
 *
 * curlEventCheck --
 *
 *	Invoked automagically by Tcl from time to time, we check if there
 *  are any active transfer, if so we put an event in the queue so that
 *  'curl_multi_perfom' will be eventually called, if not we delete
 *  the event source.
 *
 * Parameters:
 *  They are passed automagically by Tcl.
 *----------------------------------------------------------------------
 */

void
curlEventCheck(ClientData clientData, int flags) {
    struct curlMultiObjData    *curlMultiData=(struct curlMultiObjData *)clientData;
    struct curlEvent           *curlEventPtr;
    int                         selectCode;

    selectCode=curlMultiGetActiveTransfers(curlMultiData);

    if (curlMultiData->runningTransfers==0) {
        Tcl_DeleteEventSource((Tcl_EventSetupProc *)curlEventSetup, 
                (Tcl_EventCheckProc *)curlEventCheck, (ClientData *)curlMultiData);
    } else {
        if (selectCode>=0) {
            curlEventPtr=(struct curlEvent *)Tcl_Alloc(sizeof(struct curlEvent));
            curlEventPtr->proc=curlEventProc;
            curlEventPtr->curlMultiData=curlMultiData;
            Tcl_QueueEvent((Tcl_Event *)curlEventPtr, TCL_QUEUE_TAIL);
        }
    }
}

/*----------------------------------------------------------------------
 *
 * curlEventProc --
 *
 *	Finally Tcl event loop decides it is time to transfer something.
 *
 * Parameters:
 *  They are passed automagically by Tcl.
 *----------------------------------------------------------------------
*/ 

int
curlEventProc(Tcl_Event *evPtr,int flags) {
    struct curlMultiObjData   *curlMultiData
            =(struct curlMultiObjData *)((struct curlEvent *)evPtr)->curlMultiData;
    CURLMcode                  errorCode;
    Tcl_Obj                   *tclCommandObjPtr;
    char                       tclCommand[300];

    errorCode=curl_multi_perform(curlMultiData->mcurl,&curlMultiData->runningTransfers);
    if (curlMultiData->runningTransfers==0) {
        if (curlMultiData->postCommand!=NULL) {
            snprintf(tclCommand,299,"%s",curlMultiData->postCommand);
            tclCommandObjPtr=Tcl_NewStringObj(tclCommand,-1);
            if (Tcl_EvalObjEx(curlMultiData->interp,tclCommandObjPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
/*
                fprintf(stdout,"Error invoking command\n");
                fprintf(stdout,"Error: %s\n",Tcl_GetString(Tcl_GetObjResult(curlMultiData->interp)));
*/
            }
        }
    }
    return 1;
}


