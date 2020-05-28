/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/


/**********************************************************************

    module: ccsp_cr_base.c

        For Common Component Software Platform (CCSP) Development

    ---------------------------------------------------------------

    description:

        This module implements basic CR functions

        *   CcspCreateCR
        *   CcspFreeCR
        *   CcspCrAllocateMemory
        *   CcspCrFreeMemory
        *   CcspCrCloneString

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Bin Zhu 

    ---------------------------------------------------------------

    revision:

        06/06/2011    initial revision.

**********************************************************************/

#include "ansc_platform.h"

#include "ansc_ato_interface.h"
#include "ansc_ato_external_api.h"

#include "ccsp_cr_definitions.h"
#include "ccsp_cr_interface.h"
#include "ccsp_cr_profile.h"
#include "ccsp_cr_internal_api.h"

#include "ccsp_namespace_mgr.h"

#ifndef WIN32
#include "ccsp_trace.h"
#endif

/**********************************************************************

    prototype:

        ANSC_HANDLE
        CcspCreateCR
            (
            );

    description:

        This function is called to create CCSP CR compoent

    argument:   None

    return:     The CR handle;

**********************************************************************/
ANSC_HANDLE
CcspCreateCR
    (
    )
{
    PCCSP_CR_MANAGER_OBJECT             pThisObject = (PCCSP_CR_MANAGER_OBJECT)NULL;
    PCCSP_NAMESPACE_MGR_OBJECT          pNameMgr    = (PCCSP_NAMESPACE_MGR_OBJECT)NULL;
    
    pThisObject = (PCCSP_CR_MANAGER_OBJECT)
        CcspCrAllocateMemory(sizeof(CCSP_CR_MANAGER_OBJECT));

    if( pThisObject == NULL)
    {
        return NULL;
    }

    /*
     * Initialize the common variables and functions.
     */
    pThisObject->pDeviceName                        = CcspCrCloneString(CCSP_CR_NAME);
    pThisObject->pCRName                            = CcspCrCloneString(CCSP_CR_NAME);
    pThisObject->pPrefix                            = NULL;
    pThisObject->uVersion                           = 1;
    pThisObject->bSystemReady                       = FALSE;
    pThisObject->hCcspNamespaceMgr                  = NULL;
    pThisObject->uPriority                          = 0;
    pThisObject->uSessionID                         = 0;
    pThisObject->bInSession                         = FALSE;
    pThisObject->hDbusHandle                        = NULL;

    AnscQueueInitializeHeader(&pThisObject->CompInfoQueue);
    AnscQueueInitializeHeader(&pThisObject->UnknowCompInfoQueue);
    AnscQueueInitializeHeader(&pThisObject->RemoteCRQueue);

    pThisObject->LoadDeviceProfile                  = CcspCrLoadDeviceProfile;
    pThisObject->CleanAll                           = CcspCrCleanAll;
    pThisObject->DumpObject                         = CcspCrDumpObject;
    pThisObject->IsSystemReady                      = CcspCrIsSystemReady;
    pThisObject->RegisterCapabilities               = CcspCrRegisterCapabilities;
    pThisObject->DiscoverComponentSupportingNamespace
                                                    = CcspCrDiscoverComponentSupportingNamespace;
    pThisObject->DiscoverComponentSupportingDynamicTbl
                                                    = CcspCrDiscoverComponentSupportingDynamicTbl;
    pThisObject->UnregisterNamespace                = CcspCrUnregisterNamespace;
    pThisObject->UnregisterComponent                = CcspCrUnregisterComponent;
    pThisObject->CheckNamespaceDataType             = CcspCrCheckNamespaceDataType;
    pThisObject->RequestSessionID                   = CcspCrRequestSessionID;
    pThisObject->GetCurrentSessionID                = CcspCrGetCurrentSessionID;
    pThisObject->InformEndOfSession                 = CcspCrInformEndOfSession;
    pThisObject->AfterComponentLost                 = CcspCrAfterComponentLost;
    pThisObject->GetRegisteredComponents            = CcspCrGetRegisteredComponents;
    pThisObject->GetNamespaceByComponent            = CcspCrGetNamespaceByComponent;
    pThisObject->SetPrefix                          = CcspCrSetPrefix;

    /* init CCSP NamespaceMgr */
    pNameMgr = (PCCSP_NAMESPACE_MGR_OBJECT)CcspCreateNamespaceMgr(CCSP_CR_NAME);

    pThisObject->hCcspNamespaceMgr = (ANSC_HANDLE)pNameMgr;

    if( pNameMgr == NULL)
    {
        AnscTrace(("CR failed to create namespace manager. Memory Error!!!\n"));
    }

    /* load from profile */
    if( !pThisObject->LoadDeviceProfile(pThisObject))
    {
        AnscTrace(("CR failed to load Device Profile.\n"));
    }

    return (ANSC_HANDLE)pThisObject;
}


/**********************************************************************

    prototype:

        void
        CcspFreeCR
            (
                ANSC_HANDLE                                 hCcspCr
            );

    description:

        This function is called to release the CCSP CR handle;

    argument:   ANSC_HANDLE                                 hCcspCr
                the handle of CCSP CR component;

    return:     None

**********************************************************************/
void
CcspFreeCR
    (
        ANSC_HANDLE                     hCcspCr
    )
{
    PCCSP_CR_MANAGER_OBJECT             pThisObject = (PCCSP_CR_MANAGER_OBJECT)hCcspCr;

    if( pThisObject != NULL)
    {
        pThisObject->CleanAll(pThisObject);

        CcspCrFreeMemory((PVOID)hCcspCr);
    }
}


/**********************************************************************

    prototype:

        PVOID
        CcspCrAllocateMemory
            (
                ULONG               uSize
            );

    description:

        This function is called to allocate specified memory in CR component;

    argument:   ULONG               uSize
                The size of requested memory

    return:     The memory handle allocated;

**********************************************************************/
PVOID
CcspCrAllocateMemory
  (
        ULONG                       uSize
  )
{
    /* To monitor the memory usage, we need to add "Component Name" in memory calls */

    return  AnscAllocateMemory(uSize);
}


/**********************************************************************

    prototype:

        void
        CcspCrFreeMemory
            (
                PVOID               pMemory
            );

    description:

        This function is called to release the memory in CR component;

    argument:   PVOID               pMemory
                The memory block to be released.

    return:     None

**********************************************************************/
void
CcspCrFreeMemory
  (
        PVOID                       pMemory
  )
{
    /* To monitor the memory usage, we need to add "Component Name" in memory calls */

    AnscFreeMemory(pMemory);
}


/**********************************************************************

    prototype:

        char*
        CcspCrCloneString
            (
                const char*                 pString
            );

    description:

        This function is called to clone a string;

    argument:   const char*                 pString
                The source string;

    return:     The cloned string

**********************************************************************/
char*
CcspCrCloneString
  (
        const char*                 pString
  )
{
    char*                       pNewString  = NULL;
    ULONG                       ulSize      = 0;

    if( pString == NULL)
    {
        return NULL;
    }

    ulSize = AnscSizeOfString(pString) + 1;  /*RDKB-6903, CID-33341,null check and use*/

    if( ulSize == 16 || ulSize == 13)
    {
        AnscTrace("CcspCrCloneString with ulSize == %d, '%s'\n", ulSize, pString);
    }

    pNewString = (char*)CcspCrAllocateMemory(ulSize);

    if( pNewString != NULL)
    {
        AnscCopyString(pNewString, (char*)pString);
    }

    return pNewString;
}
