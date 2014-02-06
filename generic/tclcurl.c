/*
 * tclcurl.c --
 *
 * Implementation of the TclCurl extension that creates the curl namespace
 * so that Tcl interpreters can access libcurl.
 *
 * Copyright (c) 2001-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "tclcurl.h"

#include <sys/types.h>
#include <unistd.h>

/*
 *----------------------------------------------------------------------
 *
 * Tclcurl_Init --
 *
 *  This procedure initializes the package
 *
 * Results:
 *  A standard Tcl result.
 *
 *----------------------------------------------------------------------
 */

int
Tclcurl_Init (Tcl_Interp *interp) {

    if(Tcl_InitStubs(interp,"8.1",0)==NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand (interp,"::curl::init",curlInitObjCmd,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::version",curlVersion,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::escape",curlEscape,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::unescape",curlUnescape,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::versioninfo",curlVersionInfo,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::shareinit",curlShareInitObjCmd,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::easystrerror", curlEasyStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::sharestrerror",curlShareStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::multistrerror",curlMultiStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);

    Tclcurl_MultiInit(interp);

    Tcl_PkgProvide(interp,"TclCurl","7.22.0");

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCreateObjCmd --
 *
 *  Looks for the first free handle (curl1, curl2,...) and creates a
 *  Tcl command for it.
 *
 * Results:
 *  A string with the name of the handle, don't forget to free it.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

char *
curlCreateObjCmd (Tcl_Interp *interp,struct curlObjData  *curlData) {
    char                *handleName;
    int                 i;
    Tcl_CmdInfo         info;
    Tcl_Command         cmdToken;

    /* We try with curl1, if it already exists with curl2...*/
    handleName=(char *)Tcl_Alloc(10);
    for (i=1;;i++) {
        sprintf(handleName,"curl%d",i);
        if (!Tcl_GetCommandInfo(interp,handleName,&info)) {
            cmdToken=Tcl_CreateObjCommand(interp,handleName,curlObjCmd,
                                (ClientData)curlData, 
                                (Tcl_CmdDeleteProc *)curlDeleteCmd);
            break;
        }
    }
    curlData->token=cmdToken;

    return handleName;
}

/*
 *----------------------------------------------------------------------
 *
 * curlInitObjCmd --
 *
 *  This procedure is invoked to process the "curl::init" Tcl command.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
curlInitObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {

    Tcl_Obj             *resultPtr;
    CURL                *curlHandle;
    struct curlObjData  *curlData;
    char                *handleName;

    curlData=(struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));
    if (curlData==NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't allocate memory",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    memset(curlData, 0, sizeof(struct curlObjData));
    curlData->interp=interp;

    curlHandle=curl_easy_init();
    if (curlHandle==NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't open curl handle",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    handleName=curlCreateObjCmd(interp,curlData);

    curlData->curl=curlHandle;

    resultPtr=Tcl_NewStringObj(handleName,-1);
    Tcl_SetObjResult(interp,resultPtr);
    Tcl_Free(handleName);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlObjCmd --
 *
 *  This procedure is invoked to process the "curl" commands.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    struct curlObjData     *curlData=(struct curlObjData *)clientData;
    CURL                   *curlHandle=curlData->curl;
    int                    tableIndex;

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"option arg ?arg?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], commandTable, "option",
            TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    switch(tableIndex) {
        case 0:
            if (curlSetOptsTransfer(interp,curlData,objc,objv)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case 1:
/*            fprintf(stdout,"Perform\n"); */
            if (curlPerform(interp,curlHandle,curlData)) {
                if (curlData->errorBuffer!=NULL) {
                    if (curlData->errorBufferKey==NULL) {
                        Tcl_SetVar(interp,curlData->errorBufferName,
                                curlData->errorBuffer,0);
                    } else {
                        Tcl_SetVar2(interp,curlData->errorBufferName,
                                curlData->errorBufferKey,
                                curlData->errorBuffer,0);
                    }
                }
                return TCL_ERROR;
            }
            break;
        case 2:
/*            fprintf(stdout,"Getinfo\n"); */
            if (Tcl_GetIndexFromObj(interp,objv[2],getInfoTable,
                    "getinfo option",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (curlGetInfo(interp,curlHandle,tableIndex)) {
                return TCL_ERROR;
            }
            break;
        case 3:
/*            fprintf(stdout,"Cleanup\n");  */
            Tcl_DeleteCommandFromToken(interp,curlData->token);
            break;
        case 4:
/*            fprintf(stdout,"Configure\n"); */
            if (curlConfigTransfer(interp,curlData,objc,objv)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case 5:
/*            fprintf(stdout,"DupHandle\n"); */
            if (curlDupHandle(interp,curlData,objc,objv)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case 6:
/*            fprintf(stdout,"Reset\n");     */
            if (curlResetHandle(interp,curlData)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case 7:
/*            fprintf(stdout,"Pause\n");     */
            if (curl_easy_pause(curlData->curl,CURLPAUSE_ALL)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;

        case 8:
/*            fprintf(stdout,"Resume\n");     */
            if (curl_easy_pause(curlData->curl,CURLPAUSE_CONT)==TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlDeleteCmd --
 *
 *  This procedure is invoked when curl handle is deleted.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  Cleans the curl handle and frees the memory.
 *
 *----------------------------------------------------------------------
 */
int
curlDeleteCmd(ClientData clientData) {
    struct curlObjData     *curlData=(struct curlObjData *)clientData;
    CURL                   *curlHandle=curlData->curl;

    curl_easy_cleanup(curlHandle);
    curlFreeSpace(curlData);

    Tcl_Free((char *)curlData);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlPerform --
 *
 *  Invokes the libcurl function 'curl_easy_perform'
 *
 * Parameter:
 *  interp: Pointer to the interpreter we are using.
 *  curlHandle: the curl handle for which the option is set.
 *  objc and objv: The usual in Tcl.
 *
 * Results:
 *  Standard Tcl return codes.
 *----------------------------------------------------------------------
 */
int
curlPerform(Tcl_Interp *interp,CURL *curlHandle,
            struct curlObjData *curlData) {
    int         exitCode;
    Tcl_Obj     *resultPtr;

    if (curlOpenFiles(interp,curlData)) {
        return TCL_ERROR;
    }
    if (curlSetPostData(interp,curlData)) {
        return TCL_ERROR;
    }
    exitCode=curl_easy_perform(curlHandle);
    resultPtr=Tcl_NewIntObj(exitCode);
    Tcl_SetObjResult(interp,resultPtr);
    curlCloseFiles(curlData);
    curlResetPostData(curlData);
    if (curlData->bodyVarName) {
        curlSetBodyVarName(interp,curlData);
    }
    if (curlData->command) {
        Tcl_GlobalEval(interp,curlData->command);
    }
    return exitCode;
}

/*
 *----------------------------------------------------------------------
 *
 * curlSetOptsTransfer --
 *
 *  This procedure is invoked when the user invokes the 'setopt'
 *  command, it is used to set the 'curl' options 
 *
 *  Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objc and objv: The usual in Tcl.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlSetOptsTransfer(Tcl_Interp *interp, struct curlObjData *curlData,
        int objc, Tcl_Obj *CONST objv[]) {

    int            tableIndex;

    if (Tcl_GetIndexFromObj(interp, objv[2], optionTable, "option", 
            TCL_EXACT, &tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    return  curlSetOpts(interp,curlData,objv[3],tableIndex);
}

/*
 *----------------------------------------------------------------------
 *
 * curlConfigTransfer --
 *
 *  This procedure is invoked by the user command 'configure', it reads 
 *  the options passed by the user to configure a transfer, and passes
 *  then, one by one to 'curlSetOpts'.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlHandle: the curl handle for which the option is set.
 *      objc and objv: The usual in Tcl.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlConfigTransfer(Tcl_Interp *interp, struct curlObjData *curlData,
        int objc, Tcl_Obj *CONST objv[]) {

    int            tableIndex;
    int            i,j;

    Tcl_Obj     *resultPtr;
    char        errorMsg[500];

    for(i=2,j=3;i<objc;i=i+2,j=j+2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], configTable, "option", 
                TCL_EXACT, &tableIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
        if (i==objc-1) {
            snprintf(errorMsg,500,"Empty value for %s",configTable[tableIndex]);
            resultPtr=Tcl_NewStringObj(errorMsg,-1);
            Tcl_SetObjResult(interp,resultPtr);            
            return TCL_ERROR;
        }
        if (curlSetOpts(interp,curlData,objv[j],tableIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlSetOpts --
 *
 *  This procedure takes care of setting the transfer options.
 *
 * Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objv: A pointer to the object where the data to set is stored.
 *    tableIndex: The index of the option in the options table.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlSetOpts(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *CONST objv,int tableIndex) {

    int            exitCode;
    CURL           *curlHandle=curlData->curl;
    int            i,j,k;

    Tcl_Obj        *resultObjPtr;
    Tcl_Obj        *tmpObjPtr;

    Tcl_RegExp      regExp;
    CONST char     *startPtr;
    CONST char     *endPtr;

    int             charLength;
    long            longNumber=0;
    int             intNumber;
    char           *tmpStr;
    unsigned char  *tmpUStr;

    Tcl_Obj                 **httpPostData;
    Tcl_Obj                 **protocols;
    int                       curlTableIndex,formaddError,formArrayIndex;
    struct formArrayStruct   *newFormArray;
    struct curl_forms        *formArray;
    int                       curlformBufferSize;
    size_t                    contentslen;

    unsigned long int         protocolMask;

    switch(tableIndex) {
        case 0:
            if (SetoptChar(interp,curlHandle,CURLOPT_URL,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 1:
            Tcl_Free(curlData->outFile);
            curlData->outFile=curlstrdup(Tcl_GetString(objv));
            if ((strcmp(curlData->outFile,""))&&(strcmp(curlData->outFile,"stdout"))) {
                curlData->outFlag=1;
            } else {
                curlData->outFlag=0;
                curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,stdout);
                curlData->outFile=NULL;
            }
            curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
            break;
        case 2:
            Tcl_Free(curlData->inFile);
            curlData->inFile=curlstrdup(Tcl_GetString(objv));
            if ((strcmp(curlData->inFile,""))&&(strcmp(curlData->inFile,"stdin"))) {
                curlData->inFlag=1;
            } else {
                curl_easy_setopt(curlHandle,CURLOPT_READDATA,stdin);
                curlData->inFlag=0;
                curlData->inFile=NULL;
            }
            curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
            break;
        case 3:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_USERAGENT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 4:
            if (SetoptChar(interp,curlHandle,CURLOPT_REFERER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 5:
            if (SetoptInt(interp,curlHandle,CURLOPT_VERBOSE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 6:
            if (SetoptInt(interp,curlHandle,CURLOPT_HEADER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 7:
            if (SetoptInt(interp,curlHandle,CURLOPT_NOBODY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 8:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 9:
            if (SetoptLong(interp,curlHandle,CURLOPT_PROXYPORT,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 10:
            if (SetoptInt(interp,curlHandle,CURLOPT_HTTPPROXYTUNNEL,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 11:
            if (SetoptInt(interp,curlHandle,CURLOPT_FAILONERROR,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 12:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEOUT,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 13:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOW_SPEED_LIMIT,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 14:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOW_SPEED_TIME,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 15:
            if (SetoptLong(interp,curlHandle,CURLOPT_RESUME_FROM,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 16:
            if (SetoptLong(interp,curlHandle,CURLOPT_INFILESIZE,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 17:
            if (SetoptInt(interp,curlHandle,CURLOPT_UPLOAD,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 137:
        case 18:
            if (SetoptInt(interp,curlHandle,CURLOPT_DIRLISTONLY,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 136:
        case 19:
            if (SetoptInt(interp,curlHandle,CURLOPT_APPEND,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 20:
            if (Tcl_GetIndexFromObj(interp, objv, netrcTable,
                    "netrc option",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_NETRC,curlTableIndex)) {
                curlErrorSetOpt(interp,configTable,tableIndex,netrcTable[curlTableIndex]);
                return 1;
            }
            break;
        case 21:
            if (SetoptInt(interp,curlHandle,CURLOPT_FOLLOWLOCATION,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 22:
            if (SetoptInt(interp,curlHandle,CURLOPT_TRANSFERTEXT,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            Tcl_GetIntFromObj(interp,objv,&curlData->transferText);
            break;
        case 23:
            if (SetoptInt(interp,curlHandle,CURLOPT_PUT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 24: /* The CURLOPT_MUTE option no longer does anything.*/
            break;
        case 25:
            if (SetoptChar(interp,curlHandle,CURLOPT_USERPWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 26:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYUSERPWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 27:
            if (SetoptChar(interp,curlHandle,CURLOPT_RANGE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 28:
            tmpStr=curlstrdup(Tcl_GetString(objv));
            regExp=Tcl_RegExpCompile(interp,"(.*)(?:\\()(.*)(?:\\))");
            exitCode=Tcl_RegExpExec(interp,regExp,tmpStr,tmpStr);
            switch(exitCode) {
                case -1:
                    Tcl_Free((char *)tmpStr);
                    return TCL_ERROR;
                    break;
                case 0:
                    if (*tmpStr!=0) {
                        curlData->errorBufferName=curlstrdup(tmpStr);
                    } else {
                        curlData->errorBuffer=NULL;
                    }
                    curlData->errorBufferKey=NULL;
                    break;
                case 1:
                    Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
                    charLength=endPtr-startPtr;
                    curlData->errorBufferName=Tcl_Alloc(charLength+1);
                    strncpy(curlData->errorBufferName,startPtr,charLength);
                    curlData->errorBufferName[charLength]=0;
                    Tcl_RegExpRange(regExp,2,&startPtr,&endPtr);
                    charLength=endPtr-startPtr;
                    curlData->errorBufferKey=Tcl_Alloc(charLength+1);
                    strncpy(curlData->errorBufferKey,startPtr,charLength);
                    curlData->errorBufferKey[charLength]=0;
                    break;
            }
            Tcl_Free((char *)tmpStr);
            if (curlData->errorBufferName!=NULL) {
                curlData->errorBuffer=Tcl_Alloc(CURL_ERROR_SIZE);
                if (curl_easy_setopt(curlHandle,CURLOPT_ERRORBUFFER,
                        curlData->errorBuffer)) {
                    Tcl_Free((char *)curlData->errorBuffer);
                    curlData->errorBuffer=NULL;
                    return TCL_ERROR;
                }
            } else {
                Tcl_Free(curlData->errorBuffer);
            }
            break;
        case 29:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTPGET,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 30:
            if (SetoptInt(interp,curlHandle,CURLOPT_POST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 31:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_COPYPOSTFIELDS,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 33:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_FTPPORT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 34:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 35:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIEFILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 36:
            if(SetoptsList(interp,&curlData->headerList,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"Header list invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_HTTPHEADER,curlData->headerList)) {
                curl_slist_free_all(curlData->headerList);
                curlData->headerList=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 37:
            if (Tcl_ListObjGetElements(interp,objv,&k,&httpPostData)
                    ==TCL_ERROR) {
                return TCL_ERROR;
            }
            formaddError=0;
            newFormArray=(struct formArrayStruct *)Tcl_Alloc(sizeof(struct formArrayStruct));
            formArray=(struct curl_forms *)Tcl_Alloc(k*(sizeof(struct curl_forms)));
            formArrayIndex=0;

            newFormArray->next=curlData->formArray;
            newFormArray->formArray=formArray;
            newFormArray->formHeaderList=NULL;

            for(i=0,j=0;i<k;i+=2,j+=1) {
                if (Tcl_GetIndexFromObj(interp,httpPostData[i],curlFormTable,
                        "CURLFORM option",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
                    formaddError=1;
                    break;
                }
                switch(curlTableIndex) {
                    case 0:
/*                        fprintf(stdout,"Section name: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_COPYNAME;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case 1:
/*                        fprintf(stdout,"Section contents: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        tmpStr=Tcl_GetStringFromObj(httpPostData[i+1],&curlformBufferSize);
                        formArray[formArrayIndex].option = CURLFORM_COPYCONTENTS;
                        formArray[formArrayIndex].value  = (char *)
                                memcpy(Tcl_Alloc(curlformBufferSize), tmpStr, curlformBufferSize);

                        formArrayIndex++;
                        formArray[formArrayIndex].option = CURLFORM_CONTENTSLENGTH;
                        contentslen=curlformBufferSize++;
                        formArray[formArrayIndex].value  = (char *)contentslen;
                        break;
                    case 2:
/*                        fprintf(stdout,"File name %d: %s\n",formArrayIndex,Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_FILE;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case 3:
/*                        fprintf(stdout,"Data type: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_CONTENTTYPE;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case 4:
/*                        fprintf(stdout,"ContentHeader: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_CONTENTHEADER;
                        if(SetoptsList(interp,&newFormArray->formHeaderList,httpPostData[i+1])) {
                            curlErrorSetOpt(interp,configTable,tableIndex,"Header list invalid");
                            formaddError=1;
                            return TCL_ERROR;
                        }
                        formArray[formArrayIndex].value  = (char *)newFormArray->formHeaderList;
                        break;
                    case 5:
/*                        fprintf(stdout,"FileName: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        formArray[formArrayIndex].option = CURLFORM_FILENAME;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case 6:
/*                        fprintf(stdout,"BufferName: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        formArray[formArrayIndex].option = CURLFORM_BUFFER;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case 7:
/*                        fprintf(stdout,"Buffer: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        tmpUStr=Tcl_GetByteArrayFromObj
                                (httpPostData[i+1],&curlformBufferSize);
                        formArray[formArrayIndex].option = CURLFORM_BUFFERPTR;
                        formArray[formArrayIndex].value  = (char *)
                                memcpy(Tcl_Alloc(curlformBufferSize), tmpUStr, curlformBufferSize);
                        formArrayIndex++;
                        formArray[formArrayIndex].option = CURLFORM_BUFFERLENGTH;
                        contentslen=curlformBufferSize;
                        formArray[formArrayIndex].value  = (char *)contentslen;
                        break;
                    case 8:
/*                        fprintf(stdout,"FileName: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        formArray[formArrayIndex].option = CURLFORM_FILECONTENT;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                }
                formArrayIndex++;
            }
            formArray[formArrayIndex].option=CURLFORM_END;
            curlData->formArray=newFormArray;

            if (0==formaddError) {
                formaddError=curl_formadd(&(curlData->postListFirst)
                        ,&(curlData->postListLast), CURLFORM_ARRAY, formArray
                        , CURLFORM_END);
            }
            if (formaddError!=CURL_FORMADD_OK) {
                curlResetFormArray(formArray);
                curlData->formArray=newFormArray->next;
                Tcl_Free((char *)newFormArray);
                tmpStr=Tcl_Alloc(10);
                snprintf(tmpStr,10,"%d",formaddError);
                resultObjPtr=Tcl_NewStringObj(tmpStr,-1);
                Tcl_SetObjResult(interp,resultObjPtr);
                Tcl_Free(tmpStr);
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 38:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 39:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERTPASSWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 40:
            if (Tcl_GetIndexFromObj(interp, objv, sslversion,
                "sslversion ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURL_SSLVERSION_DEFAULT;
                    break;
                case 1:
                    longNumber=CURL_SSLVERSION_TLSv1;
                    break;
                case 2:
                    longNumber=CURL_SSLVERSION_SSLv2;
                    break;
                case 3:
                    longNumber=CURL_SSLVERSION_SSLv3;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_SSLVERSION,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 41:
            if (SetoptInt(interp,curlHandle,CURLOPT_CRLF,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 42:
            if(SetoptsList(interp,&curlData->quote,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"quote list invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_QUOTE,curlData->quote)) {
                curl_slist_free_all(curlData->quote);
                curlData->quote=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 43:
            if(SetoptsList(interp,&curlData->postquote,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"postqoute invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_POSTQUOTE,curlData->postquote)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"postqoute invalid");
                curl_slist_free_all(curlData->postquote);
                curlData->postquote=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 44:
            Tcl_Free(curlData->headerFile);
            curlData->headerFile=curlstrdup(Tcl_GetString(objv));
            if ((strcmp(curlData->headerFile,""))&&(strcmp(curlData->headerFile,"stdout"))
                    &&(strcmp(curlData->headerFile,"stderr"))) {
                curlData->headerFlag=1;
            } else {
                if ((strcmp(curlData->headerFile,"stdout"))) {
                    curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stderr);
                } else {
                    curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stdout);
                }
                curlData->headerFlag=0;
                curlData->headerFile=NULL;
            }
            break;
        case 45:
            if (Tcl_GetIndexFromObj(interp, objv, timeCond,
                "time cond option",TCL_EXACT, &intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (intNumber==0) {
                longNumber=CURL_TIMECOND_IFMODSINCE;
            } else {
                longNumber=CURL_TIMECOND_IFUNMODSINCE;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_TIMECONDITION,longNumber)) {
                return TCL_ERROR;
            }
            break;
        case 46:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEVALUE,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 47:
            if (SetoptChar(interp,curlHandle,CURLOPT_CUSTOMREQUEST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 48:
            Tcl_Free(curlData->stderrFile);
            curlData->stderrFile=curlstrdup(Tcl_GetString(objv));
            if ((strcmp(curlData->stderrFile,""))&&(strcmp(curlData->stderrFile,"stdout"))
                    &&(strcmp(curlData->stderrFile,"stderr"))) {
                curlData->stderrFlag=1;
            } else {
                curlData->stderrFlag=0;
                if (strcmp(curlData->stderrFile,"stdout")) {
                    curl_easy_setopt(curlHandle,CURLOPT_STDERR,stderr);
                } else {
                    curl_easy_setopt(curlHandle,CURLOPT_STDERR,stdout);
                }
                curlData->stderrFile=NULL;
            }
            break;
        case 49:
            if (SetoptChar(interp,curlHandle,CURLOPT_INTERFACE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 50:
        case 132:
            if (SetoptChar(interp,curlHandle,CURLOPT_KRBLEVEL,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 51:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_VERIFYPEER,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 52:
            if (SetoptChar(interp,curlHandle,CURLOPT_CAINFO,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 53:
            if (SetoptLong(interp,curlHandle,CURLOPT_FILETIME,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 54:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXREDIRS,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 55:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXCONNECTS,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 56:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 57:
            if (SetoptChar(interp,curlHandle,CURLOPT_RANDOM_FILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 58:
            if (SetoptChar(interp,curlHandle,CURLOPT_EGDSOCKET,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 59:
            if (SetoptLong(interp,curlHandle,CURLOPT_CONNECTTIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 60:
            if (SetoptLong(interp,curlHandle,CURLOPT_NOPROGRESS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 61:
            if (curl_easy_setopt(curlHandle,CURLOPT_HEADERFUNCTION,
                    curlHeaderReader)) {
                return TCL_ERROR;
            }
            Tcl_Free(curlData->headerVar);
            curlData->headerVar=curlstrdup(Tcl_GetString(objv));
            if (curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,
                    (FILE *)curlData)) {
                return TCL_ERROR;
            }
            break;
        case 62:
            Tcl_Free(curlData->bodyVarName);
            curlData->bodyVarName=curlstrdup(Tcl_GetString(objv));
            if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
                    curlBodyReader)) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 63:
            Tcl_Free(curlData->progressProc);
            curlData->progressProc=curlstrdup(Tcl_GetString(objv));
            if (strcmp(curlData->progressProc,"")) {
                if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSFUNCTION,
                        curlProgressCallback)) {
                    return TCL_ERROR;
                }
                if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSDATA,
                        curlData)) {
                    return TCL_ERROR;
                }
            } else {
                if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSFUNCTION,NULL)) {
                    return TCL_ERROR;
                }
            }
            break;
        case 64:
            if (curlData->cancelTransVarName) {
                Tcl_UnlinkVar(curlData->interp,curlData->cancelTransVarName);
                Tcl_Free(curlData->cancelTransVarName);
            }
            curlData->cancelTransVarName=curlstrdup(Tcl_GetString(objv));
            Tcl_LinkVar(interp,curlData->cancelTransVarName,
                    (char *)&(curlData->cancelTrans),TCL_LINK_INT);
            break;
        case 65:
            curlData->writeProc=curlstrdup(Tcl_GetString(objv));
            curlData->outFlag=0;
            if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
                    curlWriteProcInvoke)) {
                curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,curlData)) {
                curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
                return TCL_ERROR;
            }
            break;
        case 66:
            curlData->readProc=curlstrdup(Tcl_GetString(objv));
            curlData->inFlag=0;
            if (strcmp(curlData->readProc,"")) {
                if (curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,
                        curlReadProcInvoke)) {
                    return TCL_ERROR;
                }
            } else {
                curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
                return TCL_OK;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_READDATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 67:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_VERIFYHOST,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 68:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIEJAR,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 69:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSL_CIPHER_LIST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 70:
            if (Tcl_GetIndexFromObj(interp, objv, httpVersionTable,
                "http version",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_HTTP_VERSION,
                        tableIndex)) {
                tmpStr=curlstrdup(Tcl_GetString(objv));
                curlErrorSetOpt(interp,configTable,70,tmpStr);
                Tcl_Free(tmpStr);
                return TCL_ERROR;
            }
            break;
        case 71:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_EPSV,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 72:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERTTYPE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 73:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLKEY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 74:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLKEYTYPE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 135:
        case 75:
            if (SetoptChar(interp,curlHandle,CURLOPT_KEYPASSWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 76:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLENGINE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 77:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSLENGINE_DEFAULT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 78:
            if(SetoptsList(interp,&curlData->prequote,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"pretqoute invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_PREQUOTE,curlData->prequote)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"preqoute invalid");
                curl_slist_free_all(curlData->prequote);
                curlData->prequote=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 79:
            curlData->debugProc=curlstrdup(Tcl_GetString(objv));
            if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGFUNCTION,
                    curlDebugProcInvoke)) {    
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGDATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 80:
            if (SetoptLong(interp,curlHandle,CURLOPT_DNS_CACHE_TIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 81:
            if (SetoptLong(interp,curlHandle,CURLOPT_DNS_USE_GLOBAL_CACHE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 82:
            if (SetoptLong(interp,curlHandle,CURLOPT_COOKIESESSION,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 83:
            if (SetoptChar(interp,curlHandle,CURLOPT_CAPATH,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 84:
            if (SetoptLong(interp,curlHandle,CURLOPT_BUFFERSIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 85:
            if (SetoptLong(interp,curlHandle,CURLOPT_NOSIGNAL,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 86:
            if (Tcl_GetIndexFromObj(interp, objv, encodingTable,
                "encoding",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (tableIndex==2) {
                if (curl_easy_setopt(curlHandle,CURLOPT_ACCEPT_ENCODING,"")) {
                    curlErrorSetOpt(interp,configTable,86,"all");
                    return 1;
                }
            } else {
                if (SetoptChar(interp,curlHandle,CURLOPT_ACCEPT_ENCODING,86,objv)) {
                    return TCL_ERROR;
                }
            }
            break;
        case 87:
            if (Tcl_GetIndexFromObj(interp, objv, proxyTypeTable,
                "proxy type",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(tableIndex) {
                case 0:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_HTTP);
                    break;
                case 1:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_HTTP_1_0);
                    break;
                case 2:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_SOCKS4);
                    break;
                case 3:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_SOCKS4A);
                    break;
                case 4:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_SOCKS5);
                    break;
                case 5:
                    curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                            CURLPROXY_SOCKS5_HOSTNAME);
            }
            break;
        case 88:
            if(SetoptsList(interp,&curlData->http200aliases,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"http200aliases invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_HTTP200ALIASES,curlData->http200aliases)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"http200aliases invalid");
                curl_slist_free_all(curlData->http200aliases);
                curlData->http200aliases=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 89:
            if (SetoptInt(interp,curlHandle,CURLOPT_UNRESTRICTED_AUTH
                    ,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 90:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_EPRT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 91:
            Tcl_Free(curlData->command);
            curlData->command=curlstrdup(Tcl_GetString(objv));
            break;
        case 92:
            if (Tcl_GetIndexFromObj(interp, objv, httpAuthMethods,
                "authentication method",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            curlData->anyAuthFlag=0;
            switch(intNumber) {
                case 0:
                    longNumber=CURLAUTH_BASIC;
                    break;
                case 1:
                    longNumber=CURLAUTH_DIGEST;
                    break;
                case 2:
                    longNumber=CURLAUTH_DIGEST_IE;
                    break;
                case 3:
                    longNumber=CURLAUTH_GSSNEGOTIATE;
                    break;
                case 4:
                    longNumber=CURLAUTH_NTLM;
                    break;
                case 5:
                    longNumber=CURLAUTH_ANY;
                    curlData->anyAuthFlag=1;
                    break;
                case 6:
                    longNumber=CURLAUTH_ANYSAFE;
                    break;
                case 7:
                    longNumber=CURLAUTH_NTLM_WB;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTPAUTH
                    ,tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 93:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_CREATE_MISSING_DIRS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 94:
            if (Tcl_GetIndexFromObj(interp, objv, httpAuthMethods,
                "authentication method",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLAUTH_BASIC;
                    break;
                case 1:
                    longNumber=CURLAUTH_DIGEST;
                    break;
                case 2:
                    longNumber=CURLAUTH_GSSNEGOTIATE;
                    break;
                case 3:
                    longNumber=CURLAUTH_NTLM;
                    break;
                case 5:
                    longNumber=CURLAUTH_ANYSAFE;
                    break;
                case 4:
                default:
                    longNumber=CURLAUTH_ANY;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_PROXYAUTH
                    ,tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 95:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_RESPONSE_TIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 96:
            if (Tcl_GetIndexFromObj(interp, objv, ipresolve,
                "ip version",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(curlTableIndex) {
                case 0:
                    longNumber=CURL_IPRESOLVE_WHATEVER;
                    break;
                case 1:
                    longNumber=CURL_IPRESOLVE_V4;
                    break;
                case 2:
                    longNumber=CURL_IPRESOLVE_V6;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_IPRESOLVE
                    ,tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 97:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXFILESIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 98:
            if (SetoptChar(interp,curlHandle,CURLOPT_NETRC_FILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 99:
        case 138:
            if (Tcl_GetIndexFromObj(interp, objv, ftpssl,
                "ftps method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLUSESSL_NONE;
                    break;
                case 1:
                    longNumber=CURLUSESSL_TRY;
                    break;
                case 2:
                    longNumber=CURLUSESSL_CONTROL;
                    break;
                case 3:
                    longNumber=CURLUSESSL_ALL;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_USE_SSL,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 100:
            if (SetoptSHandle(interp,curlHandle,CURLOPT_SHARE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 101:
            if (SetoptLong(interp,curlHandle,CURLOPT_PORT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 102:
            if (SetoptLong(interp,curlHandle,CURLOPT_TCP_NODELAY,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 103:
            if (SetoptLong(interp,curlHandle,CURLOPT_AUTOREFERER,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 104:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 105:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 106:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 107:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete, check '-ftpport'");
            return TCL_ERROR;
            break;
        case 108:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete, check '-ftpport'");
            return TCL_ERROR;
            break;
        case 109:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 110:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 111:
            if (Tcl_GetIndexFromObj(interp, objv, ftpsslauth,
                "ftpsslauth method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLFTPAUTH_DEFAULT;
                    break;
                case 1:
                    longNumber=CURLFTPAUTH_SSL;
                    break;
                case 2:
                    longNumber=CURLFTPAUTH_TLS;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_FTPSSLAUTH,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 112:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 113:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 114:
            if (SetoptChar(interp,curlHandle,CURLOPT_FTP_ACCOUNT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 115:
            if (SetoptLong(interp,curlHandle,CURLOPT_IGNORE_CONTENT_LENGTH,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 116:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIELIST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 117:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_SKIP_PASV_IP,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 118:
            if (Tcl_GetIndexFromObj(interp, objv, ftpfilemethod,
                "ftp file method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                case 1:
                    longNumber=1;                /* FTPFILE_MULTICWD  */
                    break;
                case 2:
                    longNumber=2;                /* FTPFILE_NOCWD     */
                    break;
                case 3:
                    longNumber=3;                /* FTPFILE_SINGLECWD */
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_FILEMETHOD,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 119:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOCALPORT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 120:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOCALPORTRANGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 121:
            if (SetoptCurlOffT(interp,curlHandle,CURLOPT_MAX_SEND_SPEED_LARGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
         case 122:
            if (SetoptCurlOffT(interp,curlHandle,CURLOPT_MAX_RECV_SPEED_LARGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 123:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_FTP_ALTERNATIVE_TO_USER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 124:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_SESSIONID_CACHE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 125:
            if (Tcl_GetIndexFromObj(interp, objv, sshauthtypes,
                "ssh auth type ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLSSH_AUTH_PUBLICKEY;
                    break;
                case 1:
                    longNumber=CURLSSH_AUTH_PASSWORD;
                    break;
                case 2:
                    longNumber=CURLSSH_AUTH_HOST;
                    break;
                case 3:
                    longNumber=CURLSSH_AUTH_KEYBOARD;
                    break;
                case 4:
                    longNumber=CURLSSH_AUTH_ANY;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_SSH_AUTH_TYPES,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 126:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_PUBLIC_KEYFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 127:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_PRIVATE_KEYFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 128:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEOUT_MS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 129:
            if (SetoptLong(interp,curlHandle,CURLOPT_CONNECTTIMEOUT_MS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 130:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTP_CONTENT_DECODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 131:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTP_TRANSFER_DECODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* 132 is together with case 50 */
        case 133:
            if (SetoptLong(interp,curlHandle,CURLOPT_NEW_FILE_PERMS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 134:
            if (SetoptLong(interp,curlHandle,CURLOPT_NEW_DIRECTORY_PERMS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* case 135 with 75, case 136 with 19, case 137 with 18 and case 138 with 99 */
        case 139:
        case 146:
            if (Tcl_GetIndexFromObj(interp, objv, postredir,
                "Postredir option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURL_REDIR_POST_301;
                    break;
                case 1:
                    longNumber=CURL_REDIR_POST_302;
                    break;
                case 2:
                    longNumber=CURL_REDIR_POST_ALL;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_POSTREDIR,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 140:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_HOST_PUBLIC_KEY_MD5,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 141:
            if (SetoptLong(interp,curlHandle,CURLOPT_PROXY_TRANSFER_MODE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 142:
            if (SetoptChar(interp,curlHandle,CURLOPT_CRLFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 143:
            if (SetoptChar(interp,curlHandle,CURLOPT_ISSUERCERT,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 144:
            if (SetoptLong(interp,curlHandle,CURLOPT_ADDRESS_SCOPE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 145:
            if (SetoptLong(interp,curlHandle,CURLOPT_CERTINFO,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* case 146 is together with 139*/
        case 147:
            if (SetoptChar(interp,curlHandle,CURLOPT_USERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 148:
            if (SetoptChar(interp,curlHandle,CURLOPT_PASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 149:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYUSERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 150:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYPASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 151:
            if (SetoptLong(interp,curlHandle,CURLOPT_TFTP_BLKSIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 152:
            if (SetoptChar(interp,curlHandle,CURLOPT_SOCKS5_GSSAPI_SERVICE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 153:
            if (SetoptLong(interp,curlHandle,CURLOPT_SOCKS5_GSSAPI_NEC,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 154:
        case 155:
            if (Tcl_ListObjGetElements(interp,objv,&j,&protocols)==TCL_ERROR) {
                return 1;
            }

            for (i=0,protocolMask=0;i<j;i++) {
                tmpStr=curlstrdup(Tcl_GetString(protocols[i]));
                if (Tcl_GetIndexFromObj(interp,protocols[i],protocolNames,
                       "protocol",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
                   return TCL_ERROR;
                }
                switch(curlTableIndex) {
                    case 0:             /* http     1 */
                        protocolMask|=CURLPROTO_HTTP;
                        break;
                    case 1:             /* https    2 */
                        protocolMask|=CURLPROTO_HTTPS;
                        break;
                    case 2:             /* ftp      4 */
                        protocolMask|=CURLPROTO_FTP;
                        break;
                    case 3:             /* ftps     8 */
                        protocolMask|=CURLPROTO_FTPS;
                        break;
                    case 4:             /* scp     16 */
                        protocolMask|=CURLPROTO_SCP;
                        break;
                    case 5:             /* sftp    32 */
                        protocolMask|=CURLPROTO_SFTP;
                        break;
                    case 6:             /* telnet  64 */
                        protocolMask|=CURLPROTO_TELNET;
                        break;
                    case 7:             /* ldap   128 */
                        protocolMask|=CURLPROTO_LDAP;
                        break;
                    case 8:             /* ldaps  256 */
                        protocolMask|=CURLPROTO_LDAPS;
                        break;
                    case 9:             /* dict   512 */
                        protocolMask|=CURLPROTO_DICT;
                        break;
                    case 10:            /* file  1024 */
                        protocolMask|=CURLPROTO_FILE;
                        break;
                    case 11:            /* tftp  2048 */
                        protocolMask|=CURLPROTO_TFTP;
                        break;
                    case 12:            /* imap  4096 */
                        protocolMask|=CURLPROTO_IMAP;
                        break;
                    case 13:            /* imaps */
                        protocolMask|=CURLPROTO_IMAPS;
                        break;
                    case 14:            /* pop3 */
                        protocolMask|=CURLPROTO_POP3;
                        break;
                    case 15:            /* pop3s */
                        protocolMask|=CURLPROTO_POP3S;
                        break;
                    case 16:            /* smtp */
                        protocolMask|=CURLPROTO_SMTP;
                        break;
                    case 17:            /* smtps */
                        protocolMask|=CURLPROTO_SMTPS;
                        break;
                    case 18:            /* rtsp */
                        protocolMask|=CURLPROTO_RTSP;
                        break;
                    case 19:            /* rtmp */
                        protocolMask|=CURLPROTO_RTMP;
                        break;
                    case 20:            /* rtmpt */
                        protocolMask|=CURLPROTO_RTMPT;
                        break;
                    case 21:            /* rtmpe */
                        protocolMask|=CURLPROTO_RTMPE;
                        break;
                    case 22:            /* rtmpte */
                        protocolMask|=CURLPROTO_RTMPTE;
                        break;
                    case 23:            /* rtmps */
                        protocolMask|=CURLPROTO_RTMPS;
                        break;
                    case 24:            /* rtmpts */
                        protocolMask|=CURLPROTO_RTMPTS;
                        break;
                    case 25:            /* gopher */
                        protocolMask|=CURLPROTO_GOPHER;
                        break;
                    case 26:            /* all   FFFF */
                        protocolMask|=CURLPROTO_ALL;
                }
            }
            tmpObjPtr=Tcl_NewLongObj(protocolMask);
            if (tableIndex==154) {
                longNumber=CURLOPT_PROTOCOLS;
            } else {
                longNumber=CURLOPT_REDIR_PROTOCOLS;
            }
            if (SetoptLong(interp,curlHandle,longNumber,tableIndex,tmpObjPtr)) {
                    return TCL_ERROR;
            }
            break;
        case 156:
            if (Tcl_GetIndexFromObj(interp, objv, ftpsslccc,
                "Clear Command Channel option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLFTPSSL_CCC_NONE;
                    break;
                case 1:
                    longNumber=CURLFTPSSL_CCC_PASSIVE;
                    break;
                case 2:
                    longNumber=CURLFTPSSL_CCC_ACTIVE;
                    break;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_SSL_CCC,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 157:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_KNOWNHOSTS,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 158:
            if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYFUNCTION,curlsshkeycallback)) {    
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYDATA,curlData)) {
                return TCL_ERROR;
            }
            curlData->sshkeycallProc=curlstrdup(Tcl_GetString(objv));
            break;
        case 159:
            if (SetoptChar(interp,curlHandle,CURLOPT_MAIL_FROM,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 160:
            if(SetoptsList(interp,&curlData->mailrcpt,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"mailrcpt invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_MAIL_RCPT,curlData->mailrcpt)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"mailrcpt invalid");
                curl_slist_free_all(curlData->mailrcpt);
                curlData->mailrcpt=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 161:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_PRET,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 162:
            if (SetoptLong(interp,curlHandle,CURLOPT_WILDCARDMATCH,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 163:
            curlData->chunkBgnProc=curlstrdup(Tcl_GetString(objv));
            if (strcmp(curlData->chunkBgnProc,"")) {
                if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,
                        curlChunkBgnProcInvoke)) {
                    return TCL_ERROR;
                }
            } else {
                curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,NULL);
                return TCL_OK;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_DATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 164:
            curlData->chunkBgnVar=curlstrdup(Tcl_GetString(objv));
            if (!strcmp(curlData->chunkBgnVar,"")) {
                curlErrorSetOpt(interp,configTable,tableIndex,"invalid var name");
                return TCL_ERROR;
            }
            break;
        case 165:
            curlData->chunkEndProc=curlstrdup(Tcl_GetString(objv));
            if (strcmp(curlData->chunkEndProc,"")) {
                if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_END_FUNCTION,
                        curlChunkEndProcInvoke)) {
                    return TCL_ERROR;
                }
            } else {
                curl_easy_setopt(curlHandle,CURLOPT_CHUNK_END_FUNCTION,NULL);
                return TCL_OK;
            }
            break;
        case 166:
            curlData->fnmatchProc=curlstrdup(Tcl_GetString(objv));
            if (strcmp(curlData->fnmatchProc,"")) {
                if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,
                        curlfnmatchProcInvoke)) {
                    return TCL_ERROR;
                }
            } else {
                curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,NULL);
                return TCL_OK;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_DATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 167:
            if(SetoptsList(interp,&curlData->resolve,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"invalid list");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_RESOLVE,curlData->resolve)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"resolve list invalid");
                curl_slist_free_all(curlData->resolve);
                curlData->resolve=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 168:
            if (SetoptChar(interp,curlHandle,CURLOPT_TLSAUTH_USERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 169:
            if (SetoptChar(interp,curlHandle,CURLOPT_TLSAUTH_PASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 170:
            if (Tcl_GetIndexFromObj(interp, objv, tlsauth,
                "TSL auth option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURL_TLSAUTH_NONE;
                    break;
                case 1:
                    longNumber=CURL_TLSAUTH_SRP;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_TLSAUTH_TYPE,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 171:
            if (SetoptLong(interp,curlHandle,CURLOPT_TRANSFER_ENCODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 172:
            if (Tcl_GetIndexFromObj(interp, objv, gssapidelegation,
                "GSS API delegation option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                    longNumber=CURLGSSAPI_DELEGATION_FLAG;
                    break;
                case 1:
                    longNumber=CURLGSSAPI_DELEGATION_POLICY_FLAG;
            }
            tmpObjPtr=Tcl_NewLongObj(longNumber);
            if (SetoptLong(interp,curlHandle,CURLOPT_GSSAPI_DELEGATION,
                        tableIndex,tmpObjPtr)) {
                return TCL_ERROR;
            }
            break;
        case 173:
            if (SetoptChar(interp,curlHandle,CURLOPT_NOPROXY,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 174:
            if(SetoptsList(interp,&curlData->telnetoptions,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"invalid list");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_TELNETOPTIONS,curlData->telnetoptions)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"telnetoptions list invalid");
                curl_slist_free_all(curlData->telnetoptions);
                curlData->telnetoptions=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptInt --
 *
 *   Sets the curl options that require an int
 *
 *  Parameter:
 *   interp: The interpreter we are working with.
 *   curlHandle: and the curl handle
 *   opt: the option to set
 *   tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptInt(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    int        intNumber;
    char       *parPtr;

    if (Tcl_GetIntFromObj(interp,tclObj,&intNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,intNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptLong --
 *
 *  Set the curl options that require a long
 *
 * Parameter:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptLong(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    long         longNumber;
    char        *parPtr;

    if (Tcl_GetLongFromObj(interp,tclObj,&longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlSetoptCurlOffT --
 *
 *  Set the curl options that require a curl_off_t, even if we really
 *  use a long to do it. (Cutting and pasting at its worst)
 *
 * Parameter:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptCurlOffT(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    long        longNumber;
    char        *parPtr;

    if (Tcl_GetLongFromObj(interp,tclObj,&longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }

    if (curl_easy_setopt(curlHandle,opt,(curl_off_t)longNumber)) {
        parPtr=curlstrdup(Tcl_GetString(tclObj));
        curlErrorSetOpt(interp,configTable,tableIndex,parPtr);
        Tcl_Free(parPtr);
        return 1;
    }

    return 0;
}


/*
 *----------------------------------------------------------------------
 *
 * SetoptChar --
 *
 *  Set the curl options that require a string
 *
 * Parameter:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptChar(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int tableIndex,Tcl_Obj *tclObj) {
    char    *optionPtr;

    optionPtr=curlstrdup(Tcl_GetString(tclObj));
    if (curl_easy_setopt(curlHandle,opt,optionPtr)) {
        curlErrorSetOpt(interp,configTable,tableIndex,optionPtr);
        Tcl_Free(optionPtr);
        return 1;
    }
    Tcl_Free(optionPtr);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptSHandle --
 *
 *  Set the curl options that require a share handle (there is only
 *  one but you never know.
 *
 * Parameter:
 *  interp: The interpreter we are working with.
 *  curlHandle: the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptSHandle(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int tableIndex,Tcl_Obj *tclObj) {

    char                    *shandleName;
    Tcl_CmdInfo             *infoPtr=(Tcl_CmdInfo *)Tcl_Alloc(sizeof(Tcl_CmdInfo));
    struct shcurlObjData    *shandleDataPtr;

    shandleName=Tcl_GetString(tclObj);
    if (0==Tcl_GetCommandInfo(interp,shandleName,infoPtr)) {
        return 1;
    }
    shandleDataPtr=(struct shcurlObjData *)(infoPtr->objClientData);
    Tcl_Free((char *)infoPtr);
    if (curl_easy_setopt(curlHandle,opt,shandleDataPtr->shandle)) {
        curlErrorSetOpt(interp,configTable,tableIndex,shandleName);
        return 1;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptsList --
 *
 *  Prepares a slist for future use.
 *
 * Parameter:
 *  slistPtr: Pointer to the slist to prepare.
 *  objv: Tcl object with a list of the data.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *----------------------------------------------------------------------
 */
int
SetoptsList(Tcl_Interp *interp,struct curl_slist **slistPtr,
        Tcl_Obj *CONST objv) {
    int         i,headerNumber;
    Tcl_Obj     **headers;

    if (slistPtr!=NULL) {
        curl_slist_free_all(*slistPtr);
        *slistPtr=NULL;
    }

    if (Tcl_ListObjGetElements(interp,objv,&headerNumber,&headers)
            ==TCL_ERROR) {
        return 1;
    }

    for (i=0;i<headerNumber;i++) {
       *slistPtr=curl_slist_append(*slistPtr,Tcl_GetString(headers[i]));
        if (slistPtr==NULL) {
            return 1;
        }
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlErrorSetOpt --
 *
 *  When an error happens when setting an option, this function
 *  takes cares of reporting it
 *
 * Parameter:
 *  interp: Pointer to the interpreter we are using.
 *  option: The index of the option in 'optionTable'
 *  parPtr: String with the parameter we wanted to set the option to.
 *----------------------------------------------------------------------
 */

void
curlErrorSetOpt(Tcl_Interp *interp,CONST char **configTable, int option,
        CONST char *parPtr) {
    Tcl_Obj     *resultPtr;
    char        errorMsg[500];

    snprintf(errorMsg,500,"setting option %s: %s",configTable[option],parPtr);
    resultPtr=Tcl_NewStringObj(errorMsg,-1);
    Tcl_SetObjResult(interp,resultPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * curlHeaderVar --
 *
 *  This is the function that will be invoked if the user wants to put
 *  the headers into a variable
 *
 * Parameter:
 *  header: string with the header line.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Returns
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *-----------------------------------------------------------------------
 */
size_t
curlHeaderReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {

    char                *header=ptr;
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_RegExp           regExp;

    CONST char          *startPtr;
    CONST char          *endPtr;

    char                *headerName;
    char                *headerContent;
    char                *httpStatus;

    int                  match,charLength;

    regExp=Tcl_RegExpCompile(curlData->interp,"(.*?)(?::\\s*)(.*?)(\\r*)(\\n)");
    match=Tcl_RegExpExec(curlData->interp,regExp,header,header);

    if (match) {
        Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        headerName=Tcl_Alloc(charLength+1);
        strncpy(headerName,startPtr,charLength);
        headerName[charLength]=0;

        Tcl_RegExpRange(regExp,2,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        headerContent=Tcl_Alloc(charLength+1);
        strncpy(headerContent,startPtr,charLength);
        headerContent[charLength]=0;
        /* There may be multiple 'Set-Cookie' headers, so we use a list */
        if (Tcl_StringCaseMatch(headerName,"Set-Cookie",1)) {
            Tcl_SetVar2(curlData->interp,curlData->headerVar,headerName, \
                    headerContent,TCL_LIST_ELEMENT|TCL_APPEND_VALUE);
        } else {
            Tcl_SetVar2(curlData->interp,curlData->headerVar,headerName,
                    headerContent,0);
        }
    }
    regExp=Tcl_RegExpCompile(curlData->interp,"(^(HTTP|http)[^\r]+)(\r*)(\n)");
    match=Tcl_RegExpExec(curlData->interp,regExp,header,header);
    if (match) {
        Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        httpStatus=Tcl_Alloc(charLength+1);
        strncpy(httpStatus,startPtr,charLength);
        httpStatus[charLength]=0;

        Tcl_SetVar2(curlData->interp,curlData->headerVar,"http",
                httpStatus,0);
    }
    return size*nmemb;
}

/*
 *----------------------------------------------------------------------
 *
 * curlBodyReader --
 *
 *  This is the function that will be invoked as a callback while 
 *  transferring the body of a request into a Tcl variable.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameter:
 *  header: string with the header line.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Returns
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *-----------------------------------------------------------------------
 */
size_t
curlBodyReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {

    register int realsize = size * nmemb;
    struct MemoryStruct *mem=&(((struct curlObjData *)curlDataPtr)->bodyVar);

    mem->memory = (char *)Tcl_Realloc(mem->memory,mem->size + realsize);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
    }
    return realsize;
}

/*
 *----------------------------------------------------------------------
 *
 * curlProgressCallback --
 *
 *  This is the function that will be invoked as a callback during a  
 *  transfer.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameter:
 *  clientData: The curlData struct for the transfer.
 *  dltotal: Total amount of bytes to download.
 *  dlnow: Bytes downloaded so far.
 *  ultotal: Total amount of bytes to upload.
 *  ulnow: Bytes uploaded so far.
 *
 * Returns
 *  Returning a non-zero value will make 'libcurl' abort the transfer
 *  and return 'CURLE_ABORTED_BY_CALLBACK'.
 *-----------------------------------------------------------------------
 */
int
curlProgressCallback(void *clientData,double dltotal,double dlnow,
        double ultotal,double ulnow) {

    struct curlObjData    *curlData=(struct curlObjData *)clientData;
    Tcl_Obj               *tclProcPtr;
    char                   tclCommand[300];

    snprintf(tclCommand,299,"%s %f %f %f %f",curlData->progressProc,dltotal,
            dlnow,ultotal,ulnow);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);
    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return -1;
        }
    }
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return -1;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlWriteProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to write the recieved data.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameter:
 *  ptr: A pointer to the data.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  data read.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Returns
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *-----------------------------------------------------------------------
 */
size_t
curlWriteProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {
    register int realsize = size * nmemb;
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj             *objv[2];

    objv[0]=Tcl_NewStringObj(curlData->writeProc,-1);
    objv[1]=Tcl_NewByteArrayObj(ptr,realsize);
    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return -1;
        }
    }
    if (Tcl_EvalObjv(curlData->interp,2,objv,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return -1;
    }
    return realsize;
}

/*
 *----------------------------------------------------------------------
 *
 * curlReadProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to read the data to send.
 *
 * Parameter:
 *  header: string with the header line.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Returns
 *  The number of bytes actually read or CURL_READFUNC_ABORT in case
 *  of error, in which case 'libcurl' will abort the transfer.
 *-----------------------------------------------------------------------
 */
size_t
curlReadProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {
    register int realsize = size * nmemb;
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj             *tclProcPtr;
    Tcl_Obj             *readDataPtr;
    char                 tclCommand[300];
    unsigned char       *readBytes;
    int                  sizeRead;

    snprintf(tclCommand,300,"%s %d",curlData->readProc,realsize);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return CURL_READFUNC_ABORT;
        }
    }
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return CURL_READFUNC_ABORT;
    }
    readDataPtr=Tcl_GetObjResult(curlData->interp);
    readBytes=Tcl_GetByteArrayFromObj(readDataPtr,&sizeRead);
    memcpy(ptr,readBytes,sizeRead);

    return sizeRead;
}

/*
 *----------------------------------------------------------------------
 *
 * curlChunkBgnProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to process every wildcard matching file
 *  on a ftp transfer.
 *
 * Parameter:
 *  transfer_info: a curl_fileinfo structure about the file.
 *  curlData: A pointer to the curlData structure for the transfer.
 *  remains: number of chunks remaining.
 *-----------------------------------------------------------------------
 */
long
curlChunkBgnProcInvoke (const void *transfer_info, void *curlDataPtr, int remains) {
    struct curlObjData             *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                        *tclProcPtr;
    char                            tclCommand[300];
    int                             i;
    const struct curl_fileinfo     *fileinfoPtr=(const struct curl_fileinfo *)transfer_info;

    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    if (curlData->chunkBgnVar==NULL) {
        curlData->chunkBgnVar=curlstrdup("fileData");
    }

    Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filename",
            fileinfoPtr->filename,0);
    
    switch(fileinfoPtr->filetype) {
        case 0:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "file",0);
            break;
        case 1:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "directory",0);
            break;
        case 2:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "symlink",0);
            break;
        case 3:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "device block",0);
            break;
        case 4:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "device char",0);
            break;
        case 5:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "named pipe",0);
            break;
        case 6:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "socket",0);
            break;
        case 7:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "door",0);
            break;
        case 8:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype",
                    "error",0);
            break;
    }
    
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"time",
            Tcl_NewLongObj(fileinfoPtr->time),0);    

    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"perm",
            Tcl_NewIntObj(fileinfoPtr->perm),0);    

    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"uid",
            Tcl_NewIntObj(fileinfoPtr->uid),0);    
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"gid",
            Tcl_NewIntObj(fileinfoPtr->gid),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"size",
            Tcl_NewLongObj(fileinfoPtr->size),0);    
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"hardlinks",
            Tcl_NewIntObj(fileinfoPtr->hardlinks),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"flags",
            Tcl_NewIntObj(fileinfoPtr->flags),0);

    snprintf(tclCommand,300,"%s %d",curlData->chunkBgnProc,remains);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }
    switch(i) {
        case 0:
            return CURL_CHUNK_BGN_FUNC_OK;
        case 1:
            return CURL_CHUNK_BGN_FUNC_SKIP;
    }
    return CURL_CHUNK_BGN_FUNC_FAIL;
}

/*
 *----------------------------------------------------------------------
 *
 * curlChunkEndProcInvoke --
 *
 *  This is the function that will be invoked every time a file has
 *  been downloaded or skipped, it does little more than called the
 *  given proc.
 *
 * Parameter:
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Returns
 *-----------------------------------------------------------------------
 */
long
curlChunkEndProcInvoke (void *curlDataPtr) {

    struct curlObjData      *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                 *tclProcPtr;
    char                     tclCommand[300];
    int                      i;

    snprintf(tclCommand,300,"%s",curlData->chunkEndProc);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return CURL_CHUNK_END_FUNC_FAIL;
    }

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_CHUNK_END_FUNC_FAIL;
    }
    if (i==1) {
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }
    return CURL_CHUNK_END_FUNC_OK;    
}

/*
 *----------------------------------------------------------------------
 *
 * curlfnmatchProcInvoke --
 *
 *  This is the function that will be invoked to tell whether a filename
 *  matches a pattern when doing a 'wildcard' download. It invokes a Tcl
 *  proc to do the actual work.
 *
 * Parameter:
 *  curlData: A pointer to the curlData structure for the transfer.
 *  pattern: The pattern to match.
 *  filename: The file name to be matched.
 *-----------------------------------------------------------------------
 */
int curlfnmatchProcInvoke(void *curlDataPtr, const char *pattern, const char *filename) {

    struct curlObjData      *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                 *tclProcPtr;
    char                     tclCommand[500];
    int                      i;

    snprintf(tclCommand,500,"%s %s %s",curlData->fnmatchProc,pattern,filename);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        return CURL_FNMATCHFUNC_FAIL;
    }

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_FNMATCHFUNC_FAIL;
    }
    switch(i) {
        case 0:
            return CURL_FNMATCHFUNC_MATCH;
        case 1:
            return CURL_FNMATCHFUNC_NOMATCH;
    }
    return CURL_FNMATCHFUNC_FAIL;
}

/*
 *----------------------------------------------------------------------
 *
 * curlshkeyextract --
 *
 *  Out of one of libcurl's ssh key struct, this function will return a 
 *  Tcl_Obj with a list, the first element is the type ok key, the second
 *  the key itself.
 *
 * Parameter:
 *  interp: The interp need to deal with the objects.
 *  key: a curl_khkey struct with the key.
 *
 * Returns
 *  The object with the list.
 *-----------------------------------------------------------------------
 */
Tcl_Obj *
curlsshkeyextract(Tcl_Interp *interp,const struct curl_khkey *key) {

    Tcl_Obj         *keyObjPtr;

    keyObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);

    switch(key->keytype) {
        case CURLKHTYPE_RSA1:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("rsa1",-1));
            break;
        case CURLKHTYPE_RSA:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("rsa",-1));
            break;
        case CURLKHTYPE_DSS:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("dss",-1));
            break;
        default:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("unknnown",-1));
            break;
    }
    Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj(key->key,-1));

    return keyObjPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * curlshkeycallback --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to decide about this new ssh host
 *
 * Parameter:
 *  curl: curl's easy handle for the connection.
 *  knownkey:    The key from the hosts_file.
 *  foundkey:    The key from the remote site.
 *  match:       What libcurl thinks about how they match
 *  curlDataPtr: Points to the structure with all the TclCurl data
 *               for the connection.
 *
 * Returns
 *  A libcurl return code so that libcurl knows what to do.
 *-----------------------------------------------------------------------
 */
size_t
curlsshkeycallback(CURL *curl ,const struct curl_khkey *knownkey,
        const struct curl_khkey *foundkey, enum curl_khmatch match,void *curlDataPtr) {

    struct curlObjData  *tclcurlDataPtr=(struct curlObjData *)curlDataPtr;
    Tcl_Interp          *interp;

    Tcl_Obj             *objv[4];
    Tcl_Obj             *returnObjPtr;

    int                  action;

    interp=tclcurlDataPtr->interp;

    objv[0]=Tcl_NewStringObj(tclcurlDataPtr->sshkeycallProc,-1);
    objv[1]=curlsshkeyextract(interp,knownkey);
    objv[2]=curlsshkeyextract(interp,foundkey);

    switch(match) {
        case CURLKHMATCH_OK:
            objv[3]=Tcl_NewStringObj("match",-1);
            break;
        case CURLKHMATCH_MISMATCH:
            objv[3]=Tcl_NewStringObj("mismatch",-1);
            break;
        case CURLKHMATCH_MISSING:
            objv[3]=Tcl_NewStringObj("missing",-1);
            break;
        case CURLKHMATCH_LAST:
            objv[3]=Tcl_NewStringObj("error",-1);
    }

    if (Tcl_EvalObjv(interp,4,objv,TCL_EVAL_GLOBAL)!=TCL_OK)      {return CURLKHSTAT_REJECT;}

    returnObjPtr=Tcl_GetObjResult(interp);

    if (Tcl_GetIntFromObj(interp,returnObjPtr,&action)!=TCL_OK)   {return CURLKHSTAT_REJECT;}

    switch(action) {
        case 0:
            return CURLKHSTAT_FINE_ADD_TO_FILE;
        case 1:
            return CURLKHSTAT_FINE;
        case 2:
            return CURLKHSTAT_REJECT;
        case 3:
            return CURLKHSTAT_DEFER;
    }
    return CURLKHSTAT_REJECT;
}

/*
 *----------------------------------------------------------------------
 *
 * curlDebugProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to write the debug data produce by
 *  the verbose option.
 *
 *  Parameter:
 *   curlHandle: A pointer to the handle for the transfer.
 *   infoType: Integer with the type of data.
 *   dataPtr: the data passed to the procedure.
 *   curlDataPtr: ointer to the curlData structure for the transfer.
 *
 *  Returns
 *   The number of bytes actually written or -1 in case of error, in
 *   which case 'libcurl' will abort the transfer.
 *-----------------------------------------------------------------------
 */
int
curlDebugProcInvoke(CURL *curlHandle, curl_infotype infoType,
        char * dataPtr, size_t size, void  *curlDataPtr) {
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj             *tclProcPtr;
    Tcl_Obj             *objv[3];
    char                tclCommand[300];

    snprintf(tclCommand,300,"%s %d %d",curlData->debugProc,infoType,size);
    tclProcPtr=Tcl_NewStringObj(tclCommand,-1);

    objv[0]=Tcl_NewStringObj(curlData->debugProc,-1);
    objv[1]=Tcl_NewIntObj(infoType);
    objv[2]=Tcl_NewByteArrayObj((CONST unsigned char *)dataPtr,size);

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return -1;
        }
    }

    Tcl_EvalObjv(curlData->interp,3,objv,TCL_EVAL_GLOBAL);

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlGetInfo --
 *
 *  Invokes the 'curl_easy_getinfo' function in libcurl.
 *
 * Parameter:
 *
 * Results:
 *   0 if all went well.
 *   The CURLcode for the error.
 *----------------------------------------------------------------------
 */
CURLcode
curlGetInfo(Tcl_Interp *interp,CURL *curlHandle,int tableIndex) {
    char                    *charPtr;
    long                     longNumber;
    double                   doubleNumber;
    struct curl_slist       *slistPtr;
    struct curl_certinfo    *certinfoPtr=NULL;
    int                      i;

    CURLcode    exitCode;

    Tcl_Obj    *resultObjPtr;

    switch(tableIndex) {
        case 0:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_EFFECTIVE_URL,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 1:
        case 2:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_RESPONSE_CODE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 3:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_FILETIME,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 4:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_TOTAL_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 5:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_NAMELOOKUP_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 6:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONNECT_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 7:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PRETRANSFER_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 8:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SIZE_UPLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 9:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SIZE_DOWNLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 10:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SPEED_DOWNLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 11:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SPEED_UPLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 12:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_HEADER_SIZE,
                    &longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 13:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REQUEST_SIZE,
                    &longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 14:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SSL_VERIFYRESULT,
                    &longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 15:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 16:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_UPLOAD,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 17:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_STARTTRANSFER_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 18:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_TYPE,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 19:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 20:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_COUNT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 21:
        case 22:
            if (tableIndex==21) {
                exitCode=curl_easy_getinfo(curlHandle,CURLINFO_HTTPAUTH_AVAIL,&longNumber);
            } else {
                exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PROXYAUTH_AVAIL,&longNumber);
            }
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            if (longNumber&CURLAUTH_BASIC) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("basic",-1));
            }
            if (longNumber&CURLAUTH_DIGEST) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("digest",-1));
            }
            if (longNumber&CURLAUTH_GSSNEGOTIATE) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("gssnegotiate",-1));
            }
            if (longNumber&CURLAUTH_NTLM) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("NTLM",-1));
            }
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 23:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_OS_ERRNO,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 24:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_NUM_CONNECTS,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 25:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_SSL_ENGINES,&slistPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            while(slistPtr!=NULL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj(slistPtr->data,-1));
                slistPtr=slistPtr->next;
            }
            curl_slist_free_all(slistPtr);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 26:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_HTTP_CONNECTCODE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 27:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_COOKIELIST,&slistPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            while(slistPtr!=NULL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj(slistPtr->data,-1));
                slistPtr=slistPtr->next;
            }
            curl_slist_free_all(slistPtr);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 28:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_FTP_ENTRY_PATH,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 29:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_URL,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 30:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PRIMARY_IP,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 31:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_APPCONNECT_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 32:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CERTINFO,certinfoPtr);
            if (exitCode) {
                return exitCode;
            }
            charPtr=(char *)Tcl_Alloc(3);
            sprintf(charPtr,"%d",certinfoPtr->num_of_certs);
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(charPtr,-1));
            Tcl_Free(charPtr);
            for(i=0; i < certinfoPtr->num_of_certs; i++) {
                for(slistPtr = certinfoPtr->certinfo[i]; slistPtr; slistPtr=slistPtr->next) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(slistPtr->data,-1));
                }
            }
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 33:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_CONDITION_UNMET,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 34:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_PRIMARY_PORT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 35:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_LOCAL_IP,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 36:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_LOCAL_PORT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
    }
    return 0;            
}

/*
 *----------------------------------------------------------------------
 *
 * curlFreeSpace --
 *
 *    Frees the space taken by a curlObjData struct either because we are
 *    deleting the handle or reseting it.
 *
 *  Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objc and objv: The usual in Tcl.
 *
 * Results:
 *    A standard Tcl result.
 *----------------------------------------------------------------------
 */
void
curlFreeSpace(struct curlObjData *curlData) {

    curl_slist_free_all(curlData->headerList);
    curl_slist_free_all(curlData->quote);
    curl_slist_free_all(curlData->prequote);
    curl_slist_free_all(curlData->postquote);

    Tcl_Free(curlData->outFile);
    Tcl_Free(curlData->inFile);
    Tcl_Free(curlData->proxy);
    Tcl_Free(curlData->errorBuffer);
    Tcl_Free(curlData->errorBufferName);
    Tcl_Free(curlData->errorBufferKey);
    Tcl_Free(curlData->stderrFile);
    Tcl_Free(curlData->randomFile);
    Tcl_Free(curlData->headerVar);
    Tcl_Free(curlData->bodyVarName);
    if (curlData->bodyVar.memory) {
        Tcl_Free(curlData->bodyVar.memory);
    }
    Tcl_Free(curlData->progressProc);
    if (curlData->cancelTransVarName) {
        Tcl_UnlinkVar(curlData->interp,curlData->cancelTransVarName);
        Tcl_Free(curlData->cancelTransVarName);
    }
    Tcl_Free(curlData->writeProc);
    Tcl_Free(curlData->readProc);
    Tcl_Free(curlData->debugProc);
    curl_slist_free_all(curlData->http200aliases);
    Tcl_Free(curlData->sshkeycallProc);
    curl_slist_free_all(curlData->mailrcpt);
    Tcl_Free(curlData->chunkBgnProc);
    Tcl_Free(curlData->chunkBgnVar);
    Tcl_Free(curlData->chunkEndProc);
    Tcl_Free(curlData->fnmatchProc);
    curl_slist_free_all(curlData->resolve);
    curl_slist_free_all(curlData->telnetoptions);

    Tcl_Free(curlData->command);
}

/*
 *----------------------------------------------------------------------
 *
 * curlDupHandle --
 *
 *  This function is invoked by the 'duphandle' command, it will 
 *  create a duplicate of the given handle.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlDupHandle(Tcl_Interp *interp, struct curlObjData *curlData,
        int objc, Tcl_Obj *CONST objv[]) {

    CURL                *newCurlHandle;
    Tcl_Obj             *result;
    struct curlObjData  *newCurlData;
    char                *handleName;

    newCurlHandle=curl_easy_duphandle(curlData->curl);
    if (newCurlHandle==NULL) {
        result=Tcl_NewStringObj("Couldn't create new handle.",-1);
        Tcl_SetObjResult(interp,result);
        return TCL_ERROR;
    }

    newCurlData=(struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));    

    curlCopyCurlData(curlData,newCurlData);

    handleName=curlCreateObjCmd(interp,newCurlData);

    newCurlData->curl=newCurlHandle;

    result=Tcl_NewStringObj(handleName,-1);
    Tcl_SetObjResult(interp,result);
    Tcl_Free(handleName);

    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * curlResetHandle --
 *
 *  This function is invoked by the 'reset' command, it reset all the
 *  options in the handle to the state it had when 'init' was invoked.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlResetHandle(Tcl_Interp *interp, struct curlObjData *curlData)  {
    struct curlObjData   *tmpPtr=
                    (struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));

    tmpPtr->curl       = curlData->curl;
    tmpPtr->token      = curlData->token;
    tmpPtr->shareToken = curlData->shareToken;
    tmpPtr->interp     = curlData->interp;
    
    curlFreeSpace(curlData);
    memset(curlData, 0, sizeof(struct curlObjData));

    curlData->curl       = tmpPtr->curl;
    curlData->token      = tmpPtr->token;
    curlData->shareToken = tmpPtr->shareToken;
    curlData->interp     = tmpPtr->interp;

    curl_easy_reset(curlData->curl);

    Tcl_Free((char *)tmpPtr);

    return TCL_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * curlVersion --
 *
 *	This procedure is invoked to process the "curl::init" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
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
curlVersion (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    Tcl_Obj     *versionPtr;
    char        tclversion[200];

    sprintf(tclversion,"TclCurl Version %s (%s)",TclCurlVersion,
                                                 curl_version());
    versionPtr=Tcl_NewStringObj(tclversion,-1);
    Tcl_SetObjResult(interp,versionPtr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlEscape --
 *
 *  This function is invoked to process the "curl::escape" Tcl command.
 *  See the user documentation for details on what it does.
 *
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlEscape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    Tcl_Obj        *resultObj;
    char           *escapedStr;

    escapedStr=curl_easy_escape(NULL,Tcl_GetString(objv[1]),0);

    if(!escapedStr) {
        resultObj=Tcl_NewStringObj("curl::escape bad parameter",-1);
        Tcl_SetObjResult(interp,resultObj);
        return TCL_ERROR;
    }
    resultObj=Tcl_NewStringObj(escapedStr,-1);
    Tcl_SetObjResult(interp,resultObj);
    curl_free(escapedStr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlUnescape --
 *
 *  This function is invoked to process the "curl::Unescape" Tcl command.
 *  See the user documentation for details on what it does.
 *
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlUnescape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    Tcl_Obj        *resultObj;
    char           *unescapedStr;

    unescapedStr=curl_easy_unescape(NULL,Tcl_GetString(objv[1]),0,NULL);
    if(!unescapedStr) {
        resultObj=Tcl_NewStringObj("curl::unescape bad parameter",-1);
        Tcl_SetObjResult(interp,resultObj);
        return TCL_ERROR;
    }
    resultObj=Tcl_NewStringObj(unescapedStr,-1);
    Tcl_SetObjResult(interp,resultObj);
    curl_free(unescapedStr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlVersionInfo --
 *
 *  This function invokes 'curl_version_info' to query how 'libcurl' was
 *  compiled.
 *
 * Parameters:
 *  The standard parameters for Tcl commands, but nothing is used.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlVersionInfo (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    int                            tableIndex;
    int                            i;
    curl_version_info_data        *infoPtr;
    Tcl_Obj                       *resultObjPtr=NULL;
    char                           tmp[7];

    if (objc!=2) {
        resultObjPtr=Tcl_NewStringObj("usage: curl::versioninfo -option",-1);
        Tcl_SetObjResult(interp,resultObjPtr); 
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], versionInfoTable, "option",
            TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    infoPtr=curl_version_info(CURLVERSION_NOW);

    switch(tableIndex) {
        case 0:
            resultObjPtr=Tcl_NewStringObj(infoPtr->version,-1);
            break;
        case 1:
            sprintf(tmp,"%X",infoPtr->version_num);
            resultObjPtr=Tcl_NewStringObj(tmp,-1);
            break;
        case 2:
            resultObjPtr=Tcl_NewStringObj(infoPtr->host,-1);
            break;
        case 3:
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            if (infoPtr->features&CURL_VERSION_IPV6) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("IPV6",-1));
            }
            if (infoPtr->features&CURL_VERSION_KERBEROS4) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("KERBEROS4",-1));
            }
            if (infoPtr->features&CURL_VERSION_SSL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("SSL",-1));
            }
            if (infoPtr->features&CURL_VERSION_LIBZ) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("LIBZ",-1));
            }
            if (infoPtr->features&CURL_VERSION_NTLM) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("NTLM",-1));
            }
            if (infoPtr->features&CURL_VERSION_GSSNEGOTIATE) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("GSSNEGOTIATE",-1));
            }
            if (infoPtr->features&CURL_VERSION_DEBUG) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("DEBUG",-1));
            }
            if (infoPtr->features&CURL_VERSION_ASYNCHDNS) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("ASYNCHDNS",-1));
            }
            if (infoPtr->features&CURL_VERSION_SPNEGO) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("SPNEGO",-1));
            }
            if (infoPtr->features&CURL_VERSION_LARGEFILE) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("LARGEFILE",-1));
            }
            if (infoPtr->features&CURL_VERSION_IDN) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("IDN",-1));
            }
            if (infoPtr->features&CURL_VERSION_SSPI) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("SSPI",-1));
            }
            break;
            if (infoPtr->features&CURL_VERSION_CONV) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj("CONV",-1));
            }
        case 4:
            resultObjPtr=Tcl_NewStringObj(infoPtr->ssl_version,-1);
            break;
        case 5:
            resultObjPtr=Tcl_NewLongObj(infoPtr->ssl_version_num);
            break;
        case 6:
            resultObjPtr=Tcl_NewStringObj(infoPtr->libz_version,-1);
            break;
        case 7:
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            for(i=0;;i++) {
                if (infoPtr->protocols[i]!=NULL) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr
                            ,Tcl_NewStringObj(infoPtr->protocols[i],-1));
                } else {
                    break;
                }
            }
    }

    Tcl_SetObjResult(interp,resultObjPtr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCopyCurlData --
 *
 *  This function copies the contents of a curlData struct into another.
 *
 * Parameters:
 *  curlDataOld: The original one.
 *  curlDataNew: The new one
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlCopyCurlData (struct curlObjData *curlDataOld,
                      struct curlObjData *curlDataNew) {

    /* This takes care of the int and long values */
    memcpy(curlDataNew, curlDataOld, sizeof(struct curlObjData));

    /* Some of the data doesn't get copied */

    curlDataNew->headerList=NULL;
    curlDataNew->quote=NULL;
    curlDataNew->prequote=NULL;
    curlDataNew->postquote=NULL;
    curlDataNew->formArray=NULL;
    curlDataNew->postListFirst=NULL;
    curlDataNew->postListLast=NULL;
    curlDataNew->formArray=NULL;
    curlDataNew->outHandle=NULL;
    curlDataNew->outFlag=0;
    curlDataNew->inHandle=NULL;
    curlDataNew->inFlag=0;
    curlDataNew->headerHandle=NULL;
    curlDataNew->headerFlag=0;
    curlDataNew->stderrHandle=NULL;
    curlDataNew->stderrFlag=0;
    curlDataNew->http200aliases=NULL;
    curlDataNew->mailrcpt=NULL;
    curlDataNew->resolve=NULL;
    curlDataNew->telnetoptions=NULL;

    /* The strings need a special treatment. */

    curlDataNew->outFile=curlstrdup(curlDataOld->outFile);
    curlDataNew->inFile=curlstrdup(curlDataOld->inFile);
    curlDataNew->proxy=curlstrdup(curlDataOld->proxy);
    curlDataNew->errorBuffer=curlstrdup(curlDataOld->errorBuffer);
    curlDataNew->errorBufferName=curlstrdup(curlDataOld->errorBufferName);
    curlDataNew->errorBufferKey=curlstrdup(curlDataOld->errorBufferKey);
    curlDataNew->headerFile=curlstrdup(curlDataOld->headerFile);
    curlDataNew->stderrFile=curlstrdup(curlDataOld->stderrFile);
    curlDataNew->randomFile=curlstrdup(curlDataOld->randomFile);
    curlDataNew->headerVar=curlstrdup(curlDataOld->headerVar);
    curlDataNew->bodyVarName=curlstrdup(curlDataOld->bodyVarName);
    curlDataNew->progressProc=curlstrdup(curlDataOld->progressProc);
    curlDataNew->cancelTransVarName=curlstrdup(curlDataOld->cancelTransVarName);
    curlDataNew->writeProc=curlstrdup(curlDataOld->writeProc);
    curlDataNew->readProc=curlstrdup(curlDataOld->readProc);
    curlDataNew->debugProc=curlstrdup(curlDataOld->debugProc);
    curlDataNew->command=curlstrdup(curlDataOld->command);
    curlDataNew->sshkeycallProc=curlstrdup(curlDataOld->sshkeycallProc);
    curlDataNew->chunkBgnProc=curlstrdup(curlDataOld->chunkBgnProc);
    curlDataNew->chunkBgnVar=curlstrdup(curlDataOld->chunkBgnVar);
    curlDataNew->chunkEndProc=curlstrdup(curlDataOld->chunkEndProc);
    curlDataNew->fnmatchProc=curlstrdup(curlDataOld->fnmatchProc);
    
    curlDataNew->bodyVar.memory=(char *)Tcl_Alloc(curlDataOld->bodyVar.size);
    memcpy(curlDataNew->bodyVar.memory,curlDataOld->bodyVar.memory
            ,curlDataOld->bodyVar.size);
    curlDataNew->bodyVar.size=curlDataOld->bodyVar.size;

    return TCL_OK;
}

/*----------------------------------------------------------------------
 *
 * curlOpenFiles --
 *
 *  Before doing a transfer with the easy interface or adding an easy
 *  handle to a multi one, this function takes care of opening all
 *  necessary files for the transfer.
 *
 * Parameter:
 *  curlData: The pointer to the struct with the transfer data.
 *
 * Results:
 *  '0' all went well, '1' in case of error.
 *----------------------------------------------------------------------
 */
int
curlOpenFiles(Tcl_Interp *interp,struct curlObjData *curlData) {

    if (curlData->outFlag) {
        if (curlOpenFile(interp,curlData->outFile,&(curlData->outHandle),1,
                curlData->transferText)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_WRITEDATA,curlData->outHandle);
    }
    if (curlData->inFlag) {
        if (curlOpenFile(interp,curlData->inFile,&(curlData->inHandle),0,
                curlData->transferText)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_READDATA,curlData->inHandle);
        if (curlData->anyAuthFlag) {
            curl_easy_setopt(curlData->curl, CURLOPT_SEEKFUNCTION, curlseek);
            curl_easy_setopt(curlData->curl, CURLOPT_SEEKDATA, curlData->inHandle);
        }
    }
    if (curlData->headerFlag) {
        if (curlOpenFile(interp,curlData->headerFile,&(curlData->headerHandle),1,1)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_HEADERDATA,curlData->headerHandle);
    }
    if (curlData->stderrFlag) {
        if (curlOpenFile(interp,curlData->stderrFile,&(curlData->stderrHandle),1,1)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_STDERR,curlData->stderrHandle);
    }
    return 0;
}

/*----------------------------------------------------------------------
 *
 * curlCloseFiles --
 *
 *  Closes the files opened during a transfer.
 *
 * Parameter:
 *  curlData: The pointer to the struct with the transfer data.
 *
 *----------------------------------------------------------------------
 */
void
curlCloseFiles(struct curlObjData *curlData) {
    if (curlData->outHandle!=NULL) {
        fclose(curlData->outHandle);
        curlData->outHandle=NULL;
    }
    if (curlData->inHandle!=NULL) {
        fclose(curlData->inHandle);
        curlData->inHandle=NULL;
    }
    if (curlData->headerHandle!=NULL) {
        fclose(curlData->headerHandle);
        curlData->headerHandle=NULL;
    }
    if (curlData->stderrHandle!=NULL) {
        fclose(curlData->stderrHandle);
        curlData->stderrHandle=NULL;
    }
}

/*----------------------------------------------------------------------
 *
 * curlOpenFile --
 *
 *  Opens a file to be used during a transfer.
 *
 * Parameter:
 *  fileName: name of the file.
 *  handle: the handle for the file
 *  writing: '0' if reading, '1' if writing.
 *  text:    '0' if binary, '1' if text.
 *
 * Results:
 *  '0' all went well, '1' in case of error.
 *----------------------------------------------------------------------
 */
int
curlOpenFile(Tcl_Interp *interp,char *fileName, FILE **handle, int writing, int text) {
    Tcl_Obj        *resultObjPtr;
    char            errorMsg[300];

    if (*handle!=NULL) {
        fclose(*handle);
    }
    if (writing==1) {
        if (text==1) {
            *handle=fopen(fileName,"w");
        } else {
            *handle=fopen(fileName,"wb");
        }
    } else {
        if (text==1) {
            *handle=fopen(fileName,"r");
        } else {
            *handle=fopen(fileName,"rb");
        }
    }
    if (*handle==NULL) {
        snprintf(errorMsg,300,"Couldn't open file %s.",fileName);
        resultObjPtr=Tcl_NewStringObj(errorMsg,-1);
        Tcl_SetObjResult(interp,resultObjPtr);
        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *
 * curlseek --
 *
 *  When the user requests the 'any' auth, libcurl may need
 *  to send the PUT/POST data more than once and thus may need to ask
 *  the app to "rewind" the read data stream to start.
 *
 *----------------------------------------------------------------------
 */

int
curlseek(void *instream, curl_off_t offset, int origin)
{
    if(-1 == fseek((FILE *)instream, 0, origin)) {
          return CURLIOE_FAILRESTART;
    }
    return CURLIOE_OK;
}

/*----------------------------------------------------------------------
 *
 * curlSetPostData --
 *
 *  In case there is going to be a post transfer, this function sets the
 *  data that is going to be posted.
 *
 * Parameter:
 *  interp: Tcl interpreter we are using.
 *  curlData: A pointer to the struct with the transfer data.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlSetPostData(Tcl_Interp *interp,struct curlObjData *curlDataPtr) {
    Tcl_Obj        *errorMsgObjPtr;

    if (curlDataPtr->postListFirst!=NULL) {
        if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_HTTPPOST,curlDataPtr->postListFirst)) {
            curl_formfree(curlDataPtr->postListFirst);
            errorMsgObjPtr=Tcl_NewStringObj("Error setting the data to post",-1);
            Tcl_SetObjResult(interp,errorMsgObjPtr);
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

/*----------------------------------------------------------------------
 *
 * curlResetPostData --
 *
 *  After performing a transfer, this function is invoked to erease the
 *  posr data.
 *
 * Parameter:
 *  curlData: A pointer to the struct with the transfer data.
 *----------------------------------------------------------------------
 */
void 
curlResetPostData(struct curlObjData *curlDataPtr) {
    struct formArrayStruct       *tmpPtr;

    if (curlDataPtr->postListFirst) {
        curl_formfree(curlDataPtr->postListFirst);
        curlDataPtr->postListFirst=NULL;
        curlDataPtr->postListLast=NULL;
        curl_easy_setopt(curlDataPtr->curl,CURLOPT_HTTPPOST,NULL);

        while(curlDataPtr->formArray!=NULL) {
            if (curlDataPtr->formArray->formHeaderList!=NULL) {
                curl_slist_free_all(curlDataPtr->formArray->formHeaderList);
                curlDataPtr->formArray->formHeaderList=NULL;
            }
            curlResetFormArray(curlDataPtr->formArray->formArray);
            tmpPtr=curlDataPtr->formArray->next;
            Tcl_Free((char *)curlDataPtr->formArray);
            curlDataPtr->formArray=tmpPtr;
        }
    }
}
/*----------------------------------------------------------------------
 *
 * curlResetFormArray --
 *
 *  Cleans the contents of the formArray, it is done after a transfer or
 *  if 'curl_formadd' returns an error.
 *
 * Parameter:
 *  formArray: A pointer to the array to clean up.
 *----------------------------------------------------------------------
 */
void 
curlResetFormArray(struct curl_forms *formArray) {
    int        i;

    for (i=0;formArray[i].option!=CURLFORM_END;i++) {
        switch (formArray[i].option) {
            case CURLFORM_COPYNAME:
            case CURLFORM_COPYCONTENTS:
            case CURLFORM_FILE:
            case CURLFORM_CONTENTTYPE:
            case CURLFORM_FILENAME:
            case CURLFORM_FILECONTENT:
            case CURLFORM_BUFFER:
            case CURLFORM_BUFFERPTR:
                Tcl_Free((char *)(formArray[i].value));
                break;
            default:
                break;
        } 
    }
    Tcl_Free((char *)formArray);
}

/*----------------------------------------------------------------------
 *
 * curlSetBodyVarName --
 *
 *  After performing a transfer, this function is invoked to set the 
 *  body of the recieved transfer into a user defined Tcl variable.
 *
 * Parameter:
 *  interp: The Tcl interpreter we are using.
 *  curlData: A pointer to the struct with the transfer data.
 *----------------------------------------------------------------------
 */
void 
curlSetBodyVarName(Tcl_Interp *interp,struct curlObjData *curlDataPtr) {
    Tcl_Obj    *bodyVarNameObjPtr, *bodyVarObjPtr;

    bodyVarNameObjPtr=Tcl_NewStringObj(curlDataPtr->bodyVarName,-1);
    bodyVarObjPtr=Tcl_NewByteArrayObj((unsigned char *)curlDataPtr->bodyVar.memory,
            curlDataPtr->bodyVar.size);

    Tcl_ObjSetVar2(interp,bodyVarNameObjPtr,(Tcl_Obj *)NULL,bodyVarObjPtr,0);

    Tcl_Free(curlDataPtr->bodyVar.memory);
    curlDataPtr->bodyVar.memory=NULL;
    curlDataPtr->bodyVar.size=0;
}

/*----------------------------------------------------------------------
 *
 * curlstrdup --
 *   The same as strdup, but won't seg fault if the string to copy is NULL.
 *
 * Parameter:
 *   old: The original one.
 *
 * Results:
 *   Returns a pointer to the new string.
 *----------------------------------------------------------------------
 */
char
*curlstrdup (char *old) {
    char    *tmpPtr;

    if (old==NULL) {
        return NULL;
    }
    tmpPtr=Tcl_Alloc(strlen(old)+1);
    strcpy(tmpPtr,old);

    return tmpPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareInitObjCmd --
 *
 *  Looks for the first free share handle (scurl1, scurl2,...) and
 *  creates a Tcl command for it.
 *
 * Results:
 *  A string with the name of the handle, don't forget to free it.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

char *
curlCreateShareObjCmd (Tcl_Interp *interp,struct shcurlObjData  *shcurlData) {
    char                *shandleName;
    int                 i;
    Tcl_CmdInfo         info;
    Tcl_Command         cmdToken;

    /* We try with scurl1, if it already exists with scurl2...*/
    shandleName=(char *)Tcl_Alloc(10);
    for (i=1;;i++) {
        sprintf(shandleName,"scurl%d",i);
        if (!Tcl_GetCommandInfo(interp,shandleName,&info)) {
            cmdToken=Tcl_CreateObjCommand(interp,shandleName,curlShareObjCmd,
                                (ClientData)shcurlData, 
                                (Tcl_CmdDeleteProc *)curlCleanUpShareCmd);
            break;
        }
    }
    shcurlData->token=cmdToken;

    return shandleName;
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareInitObjCmd --
 *
 *  This procedure is invoked to process the "curl::shareinit" Tcl command.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
curlShareInitObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {

    Tcl_Obj               *resultPtr;
    CURL                  *shcurlHandle;
    struct shcurlObjData  *shcurlData;
    char                  *shandleName;

    shcurlData=(struct shcurlObjData *)Tcl_Alloc(sizeof(struct shcurlObjData));
    if (shcurlData==NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't allocate memory",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    memset(shcurlData, 0, sizeof(struct shcurlObjData));

    shcurlHandle=curl_share_init();
    if (shcurlHandle==NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't create share handle",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    shandleName=curlCreateShareObjCmd(interp,shcurlData);

    shcurlData->shandle=shcurlHandle;

    resultPtr=Tcl_NewStringObj(shandleName,-1);
    Tcl_SetObjResult(interp,resultPtr);
    Tcl_Free(shandleName);

#ifdef TCL_THREADS
    curl_share_setopt(shcurlHandle, CURLSHOPT_LOCKFUNC, curlShareLockFunc);
    curl_share_setopt(shcurlHandle, CURLSHOPT_LOCKFUNC, curlShareUnLockFunc);
#endif

    return TCL_OK;
}

#ifdef TCL_THREADS
/*
 *----------------------------------------------------------------------
 *
 * curlShareLockFunc --
 *
 *  This will be the function invoked by libcurl when it wants to lock
 *  some data for the share interface.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

void
curlShareLockFunc (CURL *handle, curl_lock_data data, curl_lock_access access
        , void *userptr) {

    switch(data) {
        CURL_LOCK_DATA_COOKIE:
            Tcl_MutexLock(&cookieLock);
            break;
        CURL_LOCK_DATA_DNS:
            Tcl_MutexLock(&dnsLock);
            break;
        CURL_LOCK_DATA_SSL_SESSION:
            Tcl_MutexLock(&sslLock);
            break;
        CURL_LOCK_DATA_CONNECT:
            Tcl_MutexLock(&connectLock);
            break;
        default:
            /* Prevent useless compile warnings */
            break;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareUnLockFunc --
 *
 *  This will be the function invoked by libcurl when it wants to unlock
 *  the previously locked data.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
void
curlShareUnLockFunc(CURL *handle, curl_lock_data data, void *userptr) {

    switch(data) {
        CURL_LOCK_DATA_COOKIE:
            Tcl_MutexUnlock(&cookieLock);
            break;
        CURL_LOCK_DATA_DNS:
            Tcl_MutexUnlock(&dnsLock);
            break;
        CURL_LOCK_DATA_SSL_SESSION:
            Tcl_MutexUnlock(&sslLock);
            break;
        CURL_LOCK_DATA_CONNECT:
            Tcl_MutexUnlock(&connectLock);
            break;
        default:
            break;
    }
}

#endif

/*
 *----------------------------------------------------------------------
 *
 * curlShareObjCmd --
 *
 *   This procedure is invoked to process the "share curl" commands.
 *   See the user documentation for details on what it does.
 *
 * Results:
 *   A standard Tcl result.
 *
 * Side effects:
 *   See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlShareObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *CONST objv[]) {

    struct shcurlObjData     *shcurlData=(struct shcurlObjData *)clientData;
    CURLSH                   *shcurlHandle=shcurlData->shandle;
    int                       tableIndex, dataIndex;
    int                       dataToLock=0;

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"option arg ?arg?");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], shareCmd, "option",TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    switch(tableIndex) {
        case 0:
        case 1:
            if (Tcl_GetIndexFromObj(interp, objv[2], lockData,
                "data to lock ",TCL_EXACT,&dataIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(dataIndex) {
                case 0:
                    dataToLock=CURL_LOCK_DATA_COOKIE;
                    break;
                case 1:
                    dataToLock=CURL_LOCK_DATA_DNS;
                    break;
            }
            if (tableIndex==0) {
                curl_share_setopt(shcurlHandle, CURLSHOPT_SHARE,   dataToLock);
            } else {
                curl_share_setopt(shcurlHandle, CURLSHOPT_UNSHARE, dataToLock);
            }
            break;
        case 2:
            Tcl_DeleteCommandFromToken(interp,shcurlData->token);
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCleanUpShareCmd --
 *
 *   This procedure is invoked when curl share handle is deleted.
 *
 * Results:
 *   A standard Tcl result.
 *
 * Side effects:
 *   Cleans the curl share handle and frees the memory.
 *
 *----------------------------------------------------------------------
 */
int
curlCleanUpShareCmd(ClientData clientData) {
    struct shcurlObjData     *shcurlData=(struct shcurlObjData *)clientData;
    CURLSH                   *shcurlHandle=shcurlData->shandle;

    curl_share_cleanup(shcurlHandle);
    Tcl_Free((char *)shcurlData);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlErrorStrings --
 *
 *  All the commands to return the error string from the error code have
 *  this function in common.
 *
 * Results:
 *  '0': All went well.
 *  '1': The error code didn't make sense.
 *----------------------------------------------------------------------
 */
int
curlErrorStrings (Tcl_Interp *interp, Tcl_Obj *CONST objv,int type) {

    Tcl_Obj               *resultPtr;
    int                    errorCode;
    char                   errorMsg[500];

    if (Tcl_GetIntFromObj(interp,objv,&errorCode)) {
        snprintf(errorMsg,500,"Invalid error code: %s",Tcl_GetString(objv));
        resultPtr=Tcl_NewStringObj(errorMsg,-1);
        Tcl_SetObjResult(interp,resultPtr);
        return 1;
    }
    switch(type) {
        case 0:
            resultPtr=Tcl_NewStringObj(curl_easy_strerror(errorCode),-1);
            break;
        case 1:
            resultPtr=Tcl_NewStringObj(curl_share_strerror(errorCode),-1);
            break;
        case 2:
            resultPtr=Tcl_NewStringObj(curl_multi_strerror(errorCode),-1);
            break;
        default:
            resultPtr=Tcl_NewStringObj("You're kidding,right?",-1);
    }
    Tcl_SetObjResult(interp,resultPtr);

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlEasyStringError --
 *
 *  This function is invoked to process the "curl::easystrerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlEasyStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],0)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareStringError --
 *
 *  This function is invoked to process the "curl::sharestrerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlShareStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],1)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiStringError --
 *
 *  This function is invoked to process the "curl::multirerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlMultiStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *CONST objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],2)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}
