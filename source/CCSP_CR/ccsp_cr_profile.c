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

    module: ccsp_cr_profile.c

        For Common Component Software Platform (CCSP) Development

    ---------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    ---------------------------------------------------------------

    description:

        This module implements device profile related functions for 
        CR (Component Registrar) development.

        *  CcspCrLoadDeviceProfile
        *  CcspCrLoadComponentProfile
        *  CcspCrLoadRemoteCRInfo

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

#include "ccsp_base_api.h"

#include "ansc_xml_dom_parser_interface.h"
#include "ansc_xml_dom_parser_external_api.h"
#include "ansc_xml_dom_parser_status.h"

#include "ccsp_namespace_mgr.h"

/* define default CR device profile name */
#define CCSP_CR_DEVICE_PROFILE_XML_FILE             "cr-deviceprofile.xml"

/**********************************************************************

    prototype:

        BOOL
        CcspCrLoadDeviceProfile
            (
                ANSC_HANDLE                                 hCcspCr
            );

    description:

        This function is called to load device profile for CR.

    argument:   ANSC_HANDLE                                 hCcspCr
                the handle of CCSP CR component;

    return:     Succeeded or not;

**********************************************************************/
BOOL
CcspCrLoadDeviceProfile
    (
        ANSC_HANDLE                                 hCcspCr
    )
{
    PCCSP_CR_MANAGER_OBJECT         pMyObject          = (PCCSP_CR_MANAGER_OBJECT)hCcspCr;
    PCCSP_NAMESPACE_MGR_OBJECT      pNSMgr             = (PCCSP_NAMESPACE_MGR_OBJECT)pMyObject->hCcspNamespaceMgr;
    ANSC_HANDLE                     pFileHandle        = NULL;
    char*                           pXMLContent        = NULL;
    char*                           pBackContent       = NULL;
    ULONG                           uXMLLength         = 0;
    PANSC_XML_DOM_NODE_OBJECT       pXmlNode           = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PANSC_XML_DOM_NODE_OBJECT       pListNode          = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode         = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    BOOL                            bStatus            = TRUE;
    ULONG                           uFileLength        = 0;
    ULONG                           uBufferSize        = 0;
    char                            buffer[512]        = { 0 };
    ULONG                           uLength            = 511;
    PCCSP_COMPONENT_INFO            pCompInfo          = (PCCSP_COMPONENT_INFO)NULL;

    /* load from the file */
    pFileHandle =
        AnscOpenFile
        (
            CCSP_CR_DEVICE_PROFILE_XML_FILE,
            ANSC_FILE_O_BINARY | ANSC_FILE_O_RDONLY,
            ANSC_FILE_S_IREAD
        );

    if( pFileHandle == NULL)
    {
        AnscTrace("Failed to load the file : '%s'\n", CCSP_CR_DEVICE_PROFILE_XML_FILE);

        return FALSE;
    }

    uFileLength = AnscGetFileSize( pFileHandle);

    pXMLContent = (char*)AnscAllocateMemory( uFileLength + 8);

    if( pXMLContent == NULL)
    {
        return FALSE;
    }

    uBufferSize = uFileLength + 8;

    if( AnscReadFile( pFileHandle, pXMLContent, &uBufferSize) != ANSC_STATUS_SUCCESS)
    {
        AnscFreeMemory(pXMLContent);

        return FALSE;
    }

    if( pFileHandle != NULL)
    {
        AnscCloseFile(pFileHandle);
    }

    /* parse the XML content */
    pBackContent = pXMLContent;

    pXmlNode = (PANSC_XML_DOM_NODE_OBJECT)
        AnscXmlDomParseString((ANSC_HANDLE)NULL, (PCHAR*)&pXMLContent, uBufferSize);

    AnscFreeMemory(pBackContent);

    if( pXmlNode == NULL)
    {
        AnscTraceWarning(("Failed to parse the CR profile file.\n"));

        return FALSE;
    }

    /* load CR name */
    pChildNode  = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetChildByName(pXmlNode, CCSP_CR_XML_NODE_crName); 

    if( pChildNode != NULL && pChildNode->GetDataString(pChildNode, NULL, buffer, &uLength) == ANSC_STATUS_SUCCESS && uLength > 0)
    {
        pMyObject->pCRName = AnscCloneString(buffer);
    }
    else
    {
        pMyObject->pCRName = AnscCloneString(CCSP_CR_NAME);
    }

    AnscTraceWarning(("CR Name: %s\n", pMyObject->pCRName));

#if 0
    /* load prefix name */
    /* Prefix will be set from command line instead */
    uLength            = 511;
    AnscZeroMemory(buffer, 512);

    pChildNode  = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetChildByName(pXmlNode, CCSP_CR_XML_NODE_prefix); 

    if( pChildNode != NULL && pChildNode->GetDataString(pChildNode, NULL, buffer, &uLength) == ANSC_STATUS_SUCCESS && uLength > 0)
    {
        pMyObject->pPrefix = AnscCloneString(buffer);

        pNSMgr->SubsysPrefix = AnscCloneString(buffer);

        AnscTraceWarning(("CR Prefix: %s\n", pMyObject->pPrefix));
    }
#endif

    /* get remote cr array */
    pListNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetChildByName(pXmlNode, CCSP_CR_XML_NODE_remote);

    if( pListNode != NULL)
    {
        pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		    AnscXmlDomNodeGetHeadChild(pListNode);

        while(pChildNode != NULL)
        {
            /* load remote cr information */
            if(!CcspCrLoadRemoteCRInfo(hCcspCr, (ANSC_HANDLE)pChildNode))
            {
                AnscTraceWarning(("Failed to load remote cr infor.\n"));
            }

            pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		        AnscXmlDomNodeGetNextChild(pListNode, pChildNode);
        }
    }


    /* get the component array node */
    pListNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetChildByName(pXmlNode, CCSP_CR_XML_NODE_components);

    if( pListNode != NULL)
    {
        pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		    AnscXmlDomNodeGetHeadChild(pListNode);

        while(pChildNode != NULL)
        {
            /* load component information */
            if(!CcspCrLoadComponentProfile(hCcspCr, (ANSC_HANDLE)pChildNode))
            {
                AnscTraceWarning(("Failed to load component profile.\n"));
            }

            pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		        AnscXmlDomNodeGetNextChild(pListNode, pChildNode);
        }
    }

    if( pXmlNode != NULL)
    {
        pXmlNode->Remove(pXmlNode);
    }


    /* create a Component Info for CR itself */
    pCompInfo = (PCCSP_COMPONENT_INFO)CcspCrAllocateMemory(sizeof(CCSP_COMPONENT_INFO));

    if( pCompInfo != NULL)
    {
        pCompInfo->pComponentName = CcspCrCloneString(CCSP_CR_NAME);
        pCompInfo->uVersion       = CCSP_CR_VERSION;
        pCompInfo->uStatus        = CCSP_Component_NotRegistered;

        AnscQueuePushEntry(&pMyObject->CompInfoQueue, &pCompInfo->Linkage);
    }

    return bStatus;
}


/**********************************************************************

    prototype:

        BOOL
        CcspCrLoadComponentProfile
            (
                ANSC_HANDLE                                 hCcspCr,
                ANSC_HANDLE                                 hXmlHandle
            );

    description:

        This function is called to load single component profile.

    argument:   ANSC_HANDLE                                 hCcspCr
                the handle of CCSP CR component;

                ANSC_HANDLE                                 hXmlHandle
                the handle of component XML handle in profile;

    return:     Succeeded or not;

**********************************************************************/
BOOL
CcspCrLoadComponentProfile
    (
        ANSC_HANDLE                                 hCcspCr,
        ANSC_HANDLE                                 hXmlHandle
    )
{
    PCCSP_CR_MANAGER_OBJECT         pMyObject         = (PCCSP_CR_MANAGER_OBJECT)hCcspCr;
    PANSC_XML_DOM_NODE_OBJECT       pObjectNode       = (PANSC_XML_DOM_NODE_OBJECT)hXmlHandle;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode        = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PCCSP_COMPONENT_INFO            pCompInfo         = (PCCSP_COMPONENT_INFO)NULL;
    char                            buffer[512]       = { 0 };
    ULONG                           uLength           = 512;
    ULONG                           uVersion          = 0;

    /* get the name */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
        pObjectNode->GetChildByName(pObjectNode, CCSP_CR_XML_NODE_component_name);
     
    if( pChildNode == NULL || pChildNode->GetDataString(pChildNode, NULL, buffer, &uLength) != ANSC_STATUS_SUCCESS || uLength == 0)
    {
        AnscTraceWarning(("Failed to load component name.\n"));
        
        return FALSE;
    }

    /* get the version */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
        pObjectNode->GetChildByName(pObjectNode, CCSP_CR_XML_NODE_component_version);

    if( pChildNode == NULL || pChildNode->GetDataUlong(pChildNode, NULL, &uVersion) != ANSC_STATUS_SUCCESS)
    {
        AnscTraceWarning(("Failed to load component version.\n"));
        
        return FALSE;
    }

    /* create a Component Info and add it */
    pCompInfo = (PCCSP_COMPONENT_INFO)CcspCrAllocateMemory(sizeof(CCSP_COMPONENT_INFO));

    if( pCompInfo != NULL)
    {
        pCompInfo->pComponentName = CcspCrCloneString(buffer);
        pCompInfo->uVersion       = uVersion;
        pCompInfo->uStatus        = CCSP_Component_NotRegistered;

        AnscQueuePushEntry(&pMyObject->CompInfoQueue, &pCompInfo->Linkage);
    }

    return TRUE;
}

/**********************************************************************

    prototype:

        BOOL
        CcspCrLoadRemoteCRInfo
            (
                ANSC_HANDLE                                 hCcspCr,
                ANSC_HANDLE                                 hXmlHandle
            );

    description:

        This function is called to load single remote cr information

    argument:   ANSC_HANDLE                                 hCcspCr
                the handle of CCSP CR component;

                ANSC_HANDLE                                 hXmlHandle
                the XML handle of remote CR in profile;

    return:     Succeeded or not;

**********************************************************************/
BOOL
CcspCrLoadRemoteCRInfo
    (
        ANSC_HANDLE                                 hCcspCr,
        ANSC_HANDLE                                 hXmlHandle
    )
{
    PCCSP_CR_MANAGER_OBJECT         pMyObject         = (PCCSP_CR_MANAGER_OBJECT)hCcspCr;
    PANSC_XML_DOM_NODE_OBJECT       pObjectNode       = (PANSC_XML_DOM_NODE_OBJECT)hXmlHandle;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode        = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PCCSP_REMOTE_CRINFO             pCRInfo           = (PCCSP_REMOTE_CRINFO)NULL;
    char                            buffer1[512]       = { 0 };
    char                            buffer2[512]       = { 0 };
    ULONG                           uLength           = 512;

    /* get the prefix */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
        pObjectNode->GetChildByName(pObjectNode, CCSP_CR_XML_NODE_cr_prefix);
     
    if( pChildNode == NULL || pChildNode->GetDataString(pChildNode, NULL, buffer1, &uLength) != ANSC_STATUS_SUCCESS || uLength == 0)
    {
        AnscTraceWarning(("Empty or invalid 'prefix' name.\n"));
        
        return FALSE;
    }

    uLength = 512;
    /* get the id */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
        pObjectNode->GetChildByName(pObjectNode, CCSP_CR_XML_NODE_cr_id);
     
    if( pChildNode == NULL || pChildNode->GetDataString(pChildNode, NULL, buffer2, &uLength) != ANSC_STATUS_SUCCESS || uLength == 0)
    {
        AnscTraceWarning(("Empty or invalid 'id' name.\n"));
        
        return FALSE;
    }

    /* create a remote CR info */
    pCRInfo = (PCCSP_REMOTE_CRINFO)CcspCrAllocateMemory(sizeof(CCSP_REMOTE_CRINFO));

    if( pCRInfo != NULL)
    {
        pCRInfo->pPrefix        = CcspCrCloneString(buffer1);
        pCRInfo->pID            = CcspCrCloneString(buffer2);

        AnscQueuePushEntry(&pMyObject->RemoteCRQueue, &pCRInfo->Linkage);
    }

    return TRUE;
}
