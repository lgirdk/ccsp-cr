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

/*
 *  Data model export utility function in Motive ACS format
 */
#include "ansc_platform.h"

#include "ansc_ato_interface.h"
#include "ansc_ato_external_api.h"

#include "ccsp_cr_definitions.h"
#include "ccsp_cr_interface.h"
#include "ccsp_cr_profile.h"
#include "ccsp_cr_internal_api.h"
#include "ccsp_namespace_mgr.h"

#include "ccsp_base_api.h"

#define CCSP_OBJECT 12
#define CCSP_ARRAY 11
#define DM_FILE "/tmp/alldatamodel.xml"

#define ATTRIBUTE_STR "<dataModel>\n<attributes>\n<attribute>\n \
<attributeName>notification</attributeName>\n \
<attributeType>int</attributeType>\n \
<minValue>0</minValue>\n \
<maxValue>2</maxValue>\n \
</attribute>\n \
<attribute> \n \
<attributeName>accessList</attributeName> \n \
<attributeType>string</attributeType> \n \
<array>true</array> \n \
<attributeLength>64</attributeLength> \n \
</attribute>\n  \
<attribute>\n \
<attributeName>visibility</attributeName> \n \
<attributeType>string</attributeType> \n \
<attributeLength>64</attributeLength>\n  \
</attribute> </attributes>\n<parameters>\n"
#define END_XML_STR  "</parameters></dataModel>\n<baselineConfiguration/>\n </deviceType>"

extern PCCSP_CR_MANAGER_OBJECT                     g_pCcspCrMgr;
static g_NodeCount = 0;
static g_WriteNodeCount = 0;

typedef struct _STR_LIST
{
    char str[128];
    struct _STR_LIST *next;
    unsigned int type;
}STR_LIST, *PSTR_LIST;

typedef struct _TREE_NODE
{
    struct _TREE_NODE *children[128];
    struct _TREE_NODE *parent;
    unsigned int count;
    unsigned int type;
    BOOLEAN flag;
    char str[128];
}TREE_NODE, PTREE_NODE;

void printfNode(TREE_NODE *pCurPosNode)
{
    printf("paraName:%s, count:%d, type:%d\n", pCurPosNode->str, pCurPosNode->count, pCurPosNode->type);
}

void freeList( STR_LIST *pNode)
{
    STR_LIST *pTmpNode = pNode;
    while(pNode)    
    {
        pTmpNode = pNode;
        pNode = pNode->next;
        AnscFreeMemory(pTmpNode);
    }
}

void printfList( STR_LIST *pNode)
{
    while(pNode)    
    {
        printf("%s.", pNode->str);
        pNode = pNode->next;
    }
    printf("\n");
}


void WriteXmlHead(FILE *file)
{
    extern void*  g_pDbusHandle ;
    char* pParamNames[] = {"Device.DeviceInfo.Manufacturer", "Device.DeviceInfo.ManufacturerOUI", "Device.DeviceInfo.ProductClass", "Device.DeviceInfo.ModelName",  "Device.DeviceInfo.SoftwareVersion"};
    parameterValStruct_t**  ppReturnVal        = NULL;
    ULONG valCount = 0;
    fprintf(file, "%s", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(file, "%s", "<deviceType xsi:schemaLocation=\"urn:dslforum-org:hdm-0-0 deviceType.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"urn:dslforum-org:hdm-0-0\">");
    fprintf(file, "%s", "<protocol>DEVICE_PROTOCOL_DSLFTR069v1</protocol>\n");
    if(NULL == g_pDbusHandle)
        printf("g_pDbusHanle is NULL\n");
    CcspBaseIf_getParameterValues(g_pDbusHandle, "com.cisco.spvtg.ccsp.tdm", "/com/cisco/spvtg/ccsp/tdm", pParamNames, 5, &valCount, &ppReturnVal);
    //CcspCcMbi_GetParameterValues(pParamNames, 5, &valCount, &ppReturnVal, NULL);
    if(valCount < 5)
    {
        printf("Get parameter values error, the device info for xml head can't be written\n");       
    }
    else
    {
        fprintf(file, "%s%s%s\n",  "<manufacturer>",  ppReturnVal[0]->parameterValue, "</manufacturer>");
        fprintf(file, "%s%s%s\n", "<manufacturerOUI>",  ppReturnVal[1]->parameterValue,"</manufacturerOUI>");
        fprintf(file, "%s%s%s\n",  "<productClass>",  ppReturnVal[2]->parameterValue, "</productClass>");
        fprintf(file, "%s%s%s\n",  "<modelName>",  ppReturnVal[3]->parameterValue, "</modelName>");
        fprintf(file, "%s%s%s\n",  "<softwareVersion>",ppReturnVal[4]->parameterValue, "</softwareVersion>");
        free_parameterValStruct_t(g_pDbusHandle, valCount, ppReturnVal);
    }
    
    fprintf(file, "%s", ATTRIBUTE_STR);        
}

STR_LIST  *split_string(char *dataModel)
{
    STR_LIST *pStrHead = NULL;
    STR_LIST *pNode   = NULL;
    STR_LIST *pTail   = NULL;
    
    char *token  = strtok(dataModel, ".");
    while(token)
    {
        /*fix me , use ansc *** instead malloc*/
        pNode = (STR_LIST *)AnscAllocateMemory(sizeof(STR_LIST));
        strcpy(pNode->str, token);
        pNode->next = NULL;
        if(pStrHead == NULL)
        {
            pStrHead = pNode;
            pTail   = pStrHead;
        }
        else
        {
            pTail->next = pNode;
            pTail = pNode;
        }
        token = strtok(NULL, ".");
    }
    return pStrHead;    
}

void AddChildNode(TREE_NODE *parentNode, STR_LIST *pStrHead)//, unsigned int dataType)
{
    TREE_NODE *pCurPosNode = parentNode;
    TREE_NODE *pNewNode = NULL;
    STR_LIST  *pTmpNode    = pStrHead; 
    //printfNode(pCurPosNode);
    while(pTmpNode)
    {
              
        //printf("Add node %s to node %s\n", pTmpNode->str, pCurPosNode->str);
        pCurPosNode->count++;
        pNewNode = (TREE_NODE *)AnscAllocateMemory(sizeof(TREE_NODE));
        g_NodeCount++;

        memset(pNewNode, 0, sizeof(TREE_NODE));        
        strcpy(pNewNode->str, pTmpNode->str);            
        pNewNode->type = pTmpNode->type;
        pNewNode->parent = pCurPosNode;
        pNewNode->flag    = false;
        pCurPosNode->children[pCurPosNode->count-1] = pNewNode;
        pCurPosNode = pNewNode;
        pTmpNode = pTmpNode->next;
    }
    return;
}

void AddNodeToDMTree(TREE_NODE *multiTree, STR_LIST *pStrHead)//, unsigned int dataType)
{
    TREE_NODE *pCurPosNode = multiTree;
    STR_LIST  *pTmpNode    = pStrHead;    
    unsigned int index = 0;
    BOOLEAN flag = false;
   
    /*find the pos to insert node*/     
    while(pTmpNode)
    {
         if(strstr(pTmpNode->str, "{") !=NULL)
        {
            pTmpNode = pTmpNode->next;
            continue;
        }         
        if(strcmp(pTmpNode->str, pCurPosNode->str) !=0) //not match
        {
            
            AddChildNode(pCurPosNode, pTmpNode);//, dataType);
            break;
        }
        else
        {
            //printf("find the DM %s\n", pTmpNode->str);
            /*match, find the right child to continue*/
            if(pTmpNode->next != NULL)
            {     
                for(index=0; index<pCurPosNode->count; ++index)
                {
                    
                    if(0 == strcmp(pTmpNode->next->str, pCurPosNode->children[index]->str))
                    {                        
                        //pTmpNode = pTmpNode->next;
                        pCurPosNode = pCurPosNode->children[index];
                        //flag = true;
                        break;
                    }
                }                    
            }
            pTmpNode = pTmpNode->next;
        }                    
    }
}

void setTypeField(STR_LIST *pStrHead, unsigned int type)
{
    STR_LIST *pTmpNode = NULL;
    for(; pStrHead; pStrHead=pStrHead->next)
    {
        if(NULL == pStrHead->next)
        {
            pStrHead->type = type;
        }
        else if((strstr(pStrHead->next->str, "{") !=NULL) || (pStrHead->next->str[0] >'0' && pStrHead->next->str[0]<'9'))
        {
            pStrHead->type = CCSP_ARRAY;
            pTmpNode = pStrHead->next;
            pStrHead->next = pTmpNode->next;
            AnscFreeMemory(pTmpNode);            
        }
        else
        {
            pStrHead->type = CCSP_OBJECT;
        }        
    }    
}

void BuildDataModelTree(TREE_NODE *multiTree, name_spaceType_t** ppStringArray, ULONG ulSize)
{
    STR_LIST *pStrHead = NULL;
    int j ;   

    for( j = 0; j < ulSize; j ++)
    {
        if( ppStringArray[j] != NULL)
        {
            if(strstr(ppStringArray[j]->name_space, "Device") == ppStringArray[j]->name_space)
            {/*slim the string by .*/
                pStrHead = split_string(ppStringArray[j]->name_space);
                setTypeField(pStrHead, ppStringArray[j]->dataType);
                AddNodeToDMTree(multiTree, pStrHead);
                freeList(pStrHead);       
            }
            /*else
            {
                printf("%s is ignored\n",  ppStringArray[j]->name_space);
            }*/
            CcspNsMgrFreeMemory(g_pCcspCrMgr->pDeviceName, ppStringArray[j]->name_space);
            CcspNsMgrFreeMemory(g_pCcspCrMgr->pDeviceName, ppStringArray[j]);
        }
    }
}

char *typeStr[] = {"string", "int", "unsignedInt", "boolean", "dateTime", "base64", "long", "unsignedLong", "float", "double", "byte", "true", "false"};
void WriteObjectStartToXml(FILE *file, char *paraName, int type)
{
    fprintf(file,"<parameter>\n<parameterName>%s</parameterName>\n",  paraName);
    fprintf(file, "<parameterType>object</parameterType>\n");
    fprintf(file, "<array>%s</array>\n<parameters>\n", typeStr[type]);
}

void WriteSimpleVarToXml(FILE *file, char *paraName, int type)
{
    fprintf(file, "<parameter>\n<parameterName>%s</parameterName>\n<parameterType>%s</parameterType>\n</parameter>\n", paraName, typeStr[type]);
}

void WriteNodeToXml(FILE *file, TREE_NODE *pNode)
{
    if(pNode->count != 0)
        WriteObjectStartToXml(file, pNode->str, pNode->type);
    else
        WriteSimpleVarToXml(file, pNode->str, pNode->type);
    pNode->flag = true;    
}

void freeNode(TREE_NODE *pCurPosNode)
{
    if(pCurPosNode->parent != NULL)
    {
        pCurPosNode->parent->count--;
    }
    g_WriteNodeCount++;               
    AnscFreeMemory(pCurPosNode);
}

void WriteMultiTreeToXML(FILE *file, TREE_NODE *multiTree)
{
    TREE_NODE *pCurPosNode = multiTree;
    TREE_NODE *pTmpNode = NULL;    
    int index = 0;
    //printfNode(pCurPosNode);
    if(!pCurPosNode->flag)
    {
        WriteNodeToXml(file, pCurPosNode);
        if(pCurPosNode->count != 0)
        {
            for(index=pCurPosNode->count; index>0; --index)
            {
                //printf("Current node is %s, will visit count %d\n", pCurPosNode->str, index);
                WriteMultiTreeToXML(file, pCurPosNode->children[pCurPosNode->count-1]);
            }
            fprintf(file, "</parameters>\n</parameter>");
        }            /*free node*/
        freeNode(pCurPosNode);             
    }      
}

void WriteDMToXML(TREE_NODE *multiTree)
{    
    FILE *file;
    file = fopen(DM_FILE, "w");
    if(NULL == file)
    {
      printf("can't open the datamodel.xml");
      return;
    }    

    WriteXmlHead(file);
    WriteMultiTreeToXML(file, multiTree);
    fprintf(file, "%s", END_XML_STR);
    printf("Build the DM tree(%d node), Write %d ndoe to xml file\n", g_NodeCount, g_WriteNodeCount);
    fclose(file);
    return;
    
}

void InitDMTreeRoot(TREE_NODE **multiTree)
{
    TREE_NODE *pNode;
    pNode = (TREE_NODE *)AnscAllocateMemory(sizeof(TREE_NODE));
    g_NodeCount++;
    memset(pNode, 0, sizeof(TREE_NODE));
    strcpy(pNode->str, "Device");
    pNode->type = CCSP_OBJECT;
    pNode->flag = false;
    *multiTree = pNode;
}

#define OBJECT_END "</parameters>"
void GenerateDataModelXml()
{
    componentStruct_t**              ppComponentArray  = NULL;
    ULONG                            ulArraySize       = 0;
    name_spaceType_t**               ppStringArray     = NULL;

    TREE_NODE *multiTree = NULL;
    ULONG                            ulSize            = 0;
    int                              i                 = 0;
    int                              j                 = 0;

    printf("strart to generate alldatamodel.xml file\n");
    InitDMTreeRoot(&multiTree);
    g_pCcspCrMgr->GetRegisteredComponents(g_pCcspCrMgr, (PVOID**)&ppComponentArray, &ulArraySize);

    AnscTrace("The count of Registered component: %d\n", ulArraySize);
    for( i = 0 ; i < ulArraySize; i ++)
    {
        AnscTrace("#%.2d  %s -- %s\n", (i+1), ppComponentArray[i]->componentName, ppComponentArray[i]->dbusPath);

        ulSize = 0;
        g_pCcspCrMgr->GetNamespaceByComponent(g_pCcspCrMgr, (const char*)ppComponentArray[i]->componentName, (PVOID**)&ppStringArray, &ulSize);

        if( ulSize == 0)
        {
            AnscTrace(" No namespace registered.\n");
        }
        else
        {
            //printfNode(multiTree);
            BuildDataModelTree(multiTree, ppStringArray, ulSize);            
            CcspNsMgrFreeMemory(g_pCcspCrMgr->pDeviceName, ppStringArray);
        }

        /* free the memory */
        CcspNsMgrFreeMemory( g_pCcspCrMgr->pDeviceName, ppComponentArray[i]->componentName);
        CcspNsMgrFreeMemory( g_pCcspCrMgr->pDeviceName, ppComponentArray[i]->dbusPath);
        CcspNsMgrFreeMemory( g_pCcspCrMgr->pDeviceName, (PVOID)ppComponentArray[i]);
    }
    WriteDMToXML(multiTree);

    CcspNsMgrFreeMemory(g_pCcspCrMgr->pDeviceName, (PVOID)ppComponentArray);
}

