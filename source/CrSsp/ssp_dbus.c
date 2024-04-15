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


#ifdef __GNUC__
#if (!defined _BUILD_ANDROID) && (!defined _NO_EXECINFO_H_)
#include <execinfo.h>
#endif
#endif

#include <sys/types.h>
#include <sys/ipc.h>

#include "ssp_global.h"
#include "ccsp_custom.h"

#include "dslh_cpeco_interface.h"
#include "dslh_cpeco_exported_api.h"
#include "ccsp_ifo_ccd.h"
#include "messagebus_interface_helper.h"

#include "ccsp_message_bus.h"
#include "ccsp_trace.h"

#include <telemetry_busmessage_sender.h>

extern PCCSP_CR_MANAGER_OBJECT                     g_pCcspCrMgr;
extern void*                                       g_pDbusHandle;
extern ULONG                                       g_ulAllocatedSizeInit;
extern ULONG                                       g_ulAllocatedSizeCurr;
extern ULONG                                       g_ulAllocatedSizePeak;
extern char*                                       pComponentName;

PDSLH_CPE_CONTROLLER_OBJECT     pDslhCpeController        = NULL;
PCCSP_CCD_INTERFACE             pSsdCcdIf                 = (PCCSP_CCD_INTERFACE        )NULL;

extern char                     g_Subsystem[32];

#define  CCSP_DATAMODEL_XML_FILE           ""


extern INT  g_iTraceLevel;
BOOL        bLogEnable;
ULONG       ulLogLevel;

static const char* CR_Introspect_msg =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<node name=\"/com/cisco/spvtg/ccsp/ComponentRegistrar\">\n"
    "    <interface name=\"com.cisco.spvtg.ccsp.ComponentRegistrar\">\n"
    "        <method name=\"registerCapabilities\">\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"component_version\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"dbus_path\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"subsystem_prefix\" direction=\"in\" />\n"
    "            <arg type=\"a(si)\" name=\"namespace\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"size\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"unregisterNamespace\">\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"namespace\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"unregisterComponent\">\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"discComponentSupportingNamespace\">\n"
    "            <arg type=\"s\" name=\"namespace\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"subsystem_prefix\" direction=\"in\" />\n"
    "            <arg type=\"a(ssiss)\" name=\"componentStruct\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"size\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"discComponentSupportingDynamicTbl\">\n"
    "            <arg type=\"s\" name=\"namespace\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"subsystem_prefix\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"out\" />\n"
    "            <arg type=\"s\" name=\"component_dbus_path\" direction=\"out\" />\n"
    "            <arg type=\"s\" name=\"remoteCR_name\" direction=\"out\" />\n"
    "            <arg type=\"s\" name=\"remoteCR_dbus_path\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"discNamespaceSupportedByComponent\">\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"in\" />\n"
	"            <arg type=\"a(si)\" name=\"namespaces\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"size\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        <method name=\"getRegisteredComponents\">\n"
    "            <arg type=\"a(sss)\" name=\"components\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"size\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"checkNamespaceDataType\">\n"
    "            <arg type=\"(si)\" name=\"namespace\" direction=\"in\" />\n"
    "            <arg type=\"s\" name=\"subsystem_prefix\" direction=\"in\" />\n"
    "            <arg type=\"b\" name=\"typeMatch\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"dumpComponentRegistry\">\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"isSystemReady\">\n"
    "            <arg type=\"b\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"requestSessionID\">\n"
    "            <arg type=\"i\" name=\"priority\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"sessionID\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <method name=\"getCurrentSessionID\">\n"
    "            <arg type=\"i\" name=\"sessionID\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"priority\" direction=\"out\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "\n"
    "        <method name=\"informEndOfSession\">\n"
    "            <arg type=\"i\" name=\"sessionID\" direction=\"in\" />\n"
    "            <arg type=\"i\" name=\"status\" direction=\"out\" />\n"
    "        </method>\n"
    "        \n"
    "        <signal name=\"systemReadySignal\">\n"
    "        </signal>\n"
    "        <signal name=\"deviceProfileChangeSignal\">\n"
    "            <arg type=\"s\" name=\"component_name\" direction=\"out\"/>\n"
    "            <arg type=\"s\" name=\"component_dbus_path\" direction=\"out\"/>\n"
    "            <arg type=\"b\" name=\"isAvailable\" direction=\"out\"/>\n"
    "        </signal>\n"
    "        <signal name=\"currentSessionIDSignal\">\n"
    "            <arg type=\"i\" name=\"priority\" direction=\"out\"/>\n"
    "            <arg type=\"i\" name=\"sessionID\" direction=\"out\"/>\n"
    "        </signal>\n"
    "    </interface>\n"
    "</node>\n"
    ;

ANSC_HANDLE
CcspCrLookforComponent
    (
        ANSC_HANDLE                 hCcspCr,
        const char*                 pCompName
    );

char*
ssp_CcdIfGetComponentName
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return g_pCcspCrMgr->pCRName;
}


ULONG
ssp_CcdIfGetComponentVersion
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return g_pCcspCrMgr->uVersion;
}


char*
ssp_CcdIfGetComponentAuthor
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return "CCSP AUSTIN TEAM";
}


ULONG
ssp_CcdIfGetComponentHealth
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    if( g_pCcspCrMgr->IsSystemReady(g_pCcspCrMgr))
    {
        return 3;  /* Green */
    }
    else
    {
        return 2; /* Yellow */
    }
}


ULONG
ssp_CcdIfGetComponentState
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    if( g_pCcspCrMgr->IsSystemReady(g_pCcspCrMgr))
    {
        return 1;  /* Running */
    }
    else
    {
        return 0;  /* Initializing */
    }
}



BOOL
ssp_CcdIfGetLoggingEnabled
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return bLogEnable;
}


ANSC_STATUS
ssp_CcdIfSetLoggingEnabled
    (
        ANSC_HANDLE                     hThisObject,
        BOOL                            bEnabled
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    bLogEnable = bEnabled;
    
    if (!bEnabled)
        AnscSetTraceLevel(CCSP_TRACE_INVALID_LEVEL);
    else
        AnscSetTraceLevel(ulLogLevel);

    return ANSC_STATUS_SUCCESS;
}

ULONG
ssp_CcdIfGetLoggingLevel
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return ulLogLevel;
}

ANSC_STATUS
ssp_CcdIfSetLoggingLevel
    (
        ANSC_HANDLE                     hThisObject,
        ULONG                           LogLevel
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    if(ulLogLevel == LogLevel) return ANSC_STATUS_SUCCESS;
    ulLogLevel = LogLevel;

    if (bLogEnable)
        AnscSetTraceLevel(ulLogLevel);        

    return ANSC_STATUS_SUCCESS;
}


ULONG
ssp_CcdIfGetMemMaxUsage
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return g_ulAllocatedSizePeak;
}


ULONG
ssp_CcdIfGetMemMinUsage
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    return g_ulAllocatedSizeInit;
}


ULONG
ssp_CcdIfGetMemConsumed
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    LONG             size = 0;

    size = AnscGetComponentMemorySize(pComponentName);

    if (size == -1 )
        size = 0;

    return size;
}


ANSC_STATUS
ssp_CcdIfApplyChanges
    (
        ANSC_HANDLE                     hThisObject
    )
{
    UNREFERENCED_PARAMETER(hThisObject);
    ANSC_STATUS                         returnStatus    = ANSC_STATUS_SUCCESS;
    /* Assume the parameter settings are committed immediately. */
    /* AnscSetTraceLevel(g_iTraceLevel); */

    return returnStatus;
}

int 
registerCapabilities
(
    const char*          component_name,
    int                  component_version,
    const char*          dbus_path,
    const char*          subsystem_prefix,
    name_spaceType_t**   name_space,
    int                  size
)
{
    return g_pCcspCrMgr->RegisterCapabilities(g_pCcspCrMgr, component_name, component_version, dbus_path, subsystem_prefix, (PVOID*)name_space, size);
}

int 
unregisterNamespace 
(
    const char*         component_name,
    const char*         name_space
)
{
    return g_pCcspCrMgr->UnregisterNamespace(g_pCcspCrMgr, component_name, name_space);
}

int 
unregisterComponent 
(
    const char*         component_name
)
{
    return g_pCcspCrMgr->UnregisterComponent(g_pCcspCrMgr, component_name);
}

int 
discComponentSupportingNamespace 
(
    const char*          name_space,
    const char*          subsystem_prefix,
    componentStruct_t*** components,
    ULONG                *val_size
)
{
    return g_pCcspCrMgr->DiscoverComponentSupportingNamespace(g_pCcspCrMgr, name_space, subsystem_prefix, FALSE, (PVOID**)components, val_size);
}

int 
discComponentSupportingDynamicTbl 
(
    const char*         name_space,
    const char*         subsystem_prefix,
    componentStruct_t** component
)
{
    return g_pCcspCrMgr->DiscoverComponentSupportingDynamicTbl(g_pCcspCrMgr, name_space, subsystem_prefix, FALSE, (PVOID*)component);
}

int 
discNamespaceSupportedByComponent 
(
    const char*         component_name,
    name_spaceType_t*** name_space,
    int*                val_size
)
{
    return g_pCcspCrMgr->GetNamespaceByComponent(g_pCcspCrMgr, component_name, (PVOID**)name_space, (ULONG*)val_size);
}

int getRegisteredComponents 
(
    registeredComponent_t***    components,
    int*                        val_size
)
{
    return g_pCcspCrMgr->GetRegisteredComponents(g_pCcspCrMgr, (PVOID**)components, (ULONG*)val_size);
}

int 
checkNamespaceDataType 
(
    const char *        name_space,
    const char*         subsystem_prefix,
    enum dataType_e     data_type,
    dbus_bool *         typeMatch
)
{
    UNREFERENCED_PARAMETER(subsystem_prefix);
    return g_pCcspCrMgr->CheckNamespaceDataType(g_pCcspCrMgr, name_space, data_type, (BOOL*)typeMatch);
}

int 
dumpComponentRegistry 
(
)
{
    return g_pCcspCrMgr->DumpObject(g_pCcspCrMgr);
}

int 
isSystemReady 
(
    dbus_bool           *val
)
{
    *val = g_pCcspCrMgr->IsSystemReady(g_pCcspCrMgr);

    return CCSP_SUCCESS;
}

int 
requestSessionID 
(
    int                 priority,
    int*                pSessionID
)
{
    return g_pCcspCrMgr->RequestSessionID(g_pCcspCrMgr, priority, (ULONG*)pSessionID);
}

int 
getCurrentSessionID 
(
    int*                pPriority,
    int*                pSessionID
)
{
    return g_pCcspCrMgr->GetCurrentSessionID(g_pCcspCrMgr, (ULONG*)pPriority, (ULONG*)pSessionID);
}

int 
informEndOfSession 
(
    int                 sessionID
)
{
    return g_pCcspCrMgr->InformEndOfSession(g_pCcspCrMgr, sessionID);
}



int freeResources
(
    int priority,
    void            *user_data
)
{
    UNREFERENCED_PARAMETER(priority);
    UNREFERENCED_PARAMETER(user_data);
    return CCSP_SUCCESS;
}


int busCheck
(
    void            *user_data
)
{
    UNREFERENCED_PARAMETER(user_data);
    return CCSP_SUCCESS;
}

int initialize
(
    void            *user_data
)
{
    UNREFERENCED_PARAMETER(user_data);
    return CCSP_SUCCESS;
}

int finalize
(
    void            *user_data
)
{
    UNREFERENCED_PARAMETER(user_data);
    return CCSP_SUCCESS;
}


DBusHandlerResult
CcspCrProcessDbusRequest
    (
        DBusConnection              *conn,
        DBusMessage                 *message,
        void                        *user_data
    )
{
    CCSP_MESSAGE_BUS_INFO *bus_info =(CCSP_MESSAGE_BUS_INFO *) user_data;
    const char *interface = dbus_message_get_interface(message);
    const char *method   = dbus_message_get_member(message);
    DBusMessage *reply;

    /* Too many trace. Disable this  -- Yan */
    /* AnscTrace("Receive Dbus Request: %s\n", method); */
    
    
    reply = dbus_message_new_method_return (message);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    /* CR related functions */
    if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("registerCapabilities", method))
    {
        DBusMessageIter iter;
        DBusMessageIter array_iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        int component_version = 0;
        dbus_int32_t result = 0;
        char * component_name = 0;
        char * dbus_path = 0;
        name_spaceType_t **val = 0;
        char *subsystem_prefix = 0;
        int param_size = 0;
        int i;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &component_name);
        }

        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_INT32)
        {
            dbus_message_iter_get_basic (&iter, &tmp);
            component_version = tmp;
        }

        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &dbus_path);
        }
        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &subsystem_prefix);
        }

        dbus_message_iter_next (&iter);
        DBUS_MESSAGE_ITER_RECURSE_SRV(&iter, &array_iter , DBUS_TYPE_ARRAY, 0, bus_info->freefunc);
        param_size = 0;
        do
        {
            int type = dbus_message_iter_get_arg_type (&array_iter);

            if (type == DBUS_TYPE_INVALID)
                break;
            param_size = param_size + 1;
        }
        while (dbus_message_iter_next (&array_iter));

        /*RDKB-6902, CID-33510, re-arranging the macro to avoid reosurce leaks as macro may return*/
        DBUS_MESSAGE_ITER_RECURSE_SRV(&iter, &array_iter , DBUS_TYPE_ARRAY, 0, bus_info->freefunc);

        if(param_size)
        {
            val = AnscAllocateMemory(sizeof(name_spaceType_t *) * param_size);

            if( val == NULL)
            {
                return DBUS_HANDLER_RESULT_HANDLED;
            }

        }

        for(i = 0; i < param_size; i++)
        {
            DBUS_MESSAGE_ITER_RECURSE_SRV(&array_iter, &struct_iter , DBUS_TYPE_STRUCT, val, bus_info->freefunc);

            val[i] = AnscAllocateMemory(sizeof(name_spaceType_t));

            if( val[i] != NULL)
            {
                if(dbus_message_iter_get_arg_type (&struct_iter) == DBUS_TYPE_STRING)
                {
                    dbus_message_iter_get_basic (&struct_iter, &val[i]->name_space);
                }
                dbus_message_iter_next (&struct_iter);
                if(dbus_message_iter_get_arg_type (&struct_iter) == DBUS_TYPE_INT32)
                {
                    dbus_message_iter_get_basic (&struct_iter, &val[i]->dataType);
                }
                dbus_message_iter_next (&array_iter);
            }
        }


        result = registerCapabilities(
                     component_name,
                     component_version,
                     dbus_path,
                     subsystem_prefix,
                     val,
                     param_size
                 );

        dbus_message_iter_init_append (reply, &iter);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        if(param_size)
        {
            for(i = 0; i < param_size; i++)
            {
                if( val[i] != NULL)
                {
                    AnscFreeMemory(val[i]);
                }
            }

            AnscFreeMemory(val);
        }

        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("unregisterNamespace", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        char * component_name = 0;
        char * name_space = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &component_name);
        }

        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &name_space);
        }

        result = unregisterNamespace(
                     component_name,
                     name_space
                 );

        dbus_message_iter_init_append (reply, &iter);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("unregisterComponent", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        char * component_name = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &component_name);
        }

        result = unregisterComponent(
                     component_name
                 );

        dbus_message_iter_init_append (reply, &iter);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("discComponentSupportingNamespace", method))
    {
        DBusMessageIter iter;
        DBusMessageIter array_iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        componentStruct_t **components =0;
        ULONG size  = 0;
        char *name_space = 0;
        char *subsystem_prefix = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &name_space);
        }

        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &subsystem_prefix);
        }
        result = discComponentSupportingNamespace(
                     name_space,
                     subsystem_prefix,
                     &components,
                     &size
                 );

        dbus_message_iter_init_append (reply, &iter);
        dbus_message_iter_open_container (&iter,
                                          DBUS_TYPE_ARRAY,
                                          "(ssiss)",
                                          &array_iter);
        if(result == CCSP_SUCCESS)
        {

            ULONG i;
            for(i = 0; i < size; i++)
            {
                dbus_message_iter_open_container (&array_iter,
                                                  DBUS_TYPE_STRUCT,
                                                  "ssiss",
                                                  &struct_iter);

                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->componentName);
                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->dbusPath);
                tmp = components[i]->type;
                dbus_message_iter_append_basic (&struct_iter, DBUS_TYPE_INT32, &tmp );
                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->remoteCR_name);
                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->remoteCR_dbus_path);

                dbus_message_iter_close_container (&array_iter,
                                                   &struct_iter);
            }
        }

        dbus_message_iter_close_container (&iter,
                                           &array_iter);


        tmp = size;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);

        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        if(result == CCSP_SUCCESS)
            free_componentStruct_t(bus_info, size, components);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("discComponentSupportingDynamicTbl", method))
    {
        DBusMessageIter iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        char * name_space = 0;
        componentStruct_t *component =0;
        char *subsystem_prefix = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &name_space);
        }

        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &subsystem_prefix);
        }
        result = discComponentSupportingDynamicTbl(
                     name_space,
                     subsystem_prefix,
                     &component
                 );

        dbus_message_iter_init_append (reply, &iter);

        if(result == CCSP_SUCCESS)
        {
           dbus_message_iter_open_container (&iter,
                                          DBUS_TYPE_STRUCT,
                                          "ssiss",
                                          &struct_iter);


            DBUS_MESSAGE_APPEND_STRING (&struct_iter, component->componentName);
            DBUS_MESSAGE_APPEND_STRING (&struct_iter, component->dbusPath);
            tmp = component->type;
            dbus_message_iter_append_basic (&struct_iter, DBUS_TYPE_INT32, &tmp );
            
            DBUS_MESSAGE_APPEND_STRING (&struct_iter, component->remoteCR_name);
            DBUS_MESSAGE_APPEND_STRING (&struct_iter, component->remoteCR_dbus_path);

            dbus_message_iter_close_container (&iter,
                                             &struct_iter);
        }

        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        if(result == CCSP_SUCCESS)
           free_componentStruct_t2(bus_info, component);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("discNamespaceSupportedByComponent", method))
    {
        DBusMessageIter iter;
        DBusMessageIter array_iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        char * component_name = 0;
        name_spaceType_t **name_space =0;
        int size  = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &component_name);
        }

        result = discNamespaceSupportedByComponent(
                     component_name,
                     &name_space,
                     &size
                 );

        dbus_message_iter_init_append (reply, &iter);

        dbus_message_iter_open_container (&iter,
                                          DBUS_TYPE_ARRAY,
                                          "(si)",
                                          &array_iter);
        if(result == CCSP_SUCCESS)
        {

            int i;
            for(i = 0; i < size; i++)
            {
                dbus_message_iter_open_container (&array_iter,
                                                  DBUS_TYPE_STRUCT,
                                                  "si",
                                                  &struct_iter);

                if (name_space[i] == NULL) {
                    AnscTrace("%s: name_space[%d] NULL\n", __func__, i);
                    return DBUS_HANDLER_RESULT_HANDLED;
                }

                DBUS_MESSAGE_APPEND_STRING (&struct_iter, name_space[i]->name_space);
		        tmp = name_space[i]->dataType;
		        dbus_message_iter_append_basic (&struct_iter, DBUS_TYPE_INT32, &tmp);

                dbus_message_iter_close_container (&array_iter,
                                                   &struct_iter);
            }
        }

        dbus_message_iter_close_container (&iter,
                                           &array_iter);

        tmp = size;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        if(result == CCSP_SUCCESS)
            free_name_spaceType_t(bus_info, size, name_space);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("getRegisteredComponents", method))
    {
        DBusMessageIter iter;
        DBusMessageIter array_iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        registeredComponent_t **components =0;
        int size  = 0;

        result = getRegisteredComponents(
                     &components,
                     &size
                 );

        dbus_message_iter_init_append (reply, &iter);
        dbus_message_iter_open_container (&iter,
                                          DBUS_TYPE_ARRAY,
                                          "(sss)",
                                          &array_iter);
        if(result == CCSP_SUCCESS)
        {

            int i;
            for(i = 0; i < size; i++)
            {
                dbus_message_iter_open_container (&array_iter,
                                                  DBUS_TYPE_STRUCT,
                                                  "sss",
                                                  &struct_iter);

                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->componentName);
                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->dbusPath);
                DBUS_MESSAGE_APPEND_STRING (&struct_iter, components[i]->subsystem_prefix);

                dbus_message_iter_close_container (&array_iter,
                                                   &struct_iter);
            }
        }

        dbus_message_iter_close_container (&iter,
                                           &array_iter);



        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        if(result == CCSP_SUCCESS)
            free_registeredComponent_t(bus_info, size, components);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("checkNamespaceDataType", method))
    {
        DBusMessageIter iter;
        DBusMessageIter struct_iter;
        dbus_int32_t tmp = 0;
        dbus_bool_t btmp = 0;
        dbus_int32_t result = 0;
        char * name_space = 0;
        enum dataType_e data_type = 0; /*RDKB-6902, CID-33263, initialize before use*/
        dbus_bool typeMatch;
        char *subsystem_prefix = 0;

        dbus_message_iter_init (message, &iter);

        DBUS_MESSAGE_ITER_RECURSE_SRV(&iter, &struct_iter , DBUS_TYPE_STRUCT, 0, bus_info->freefunc);
		
        if(dbus_message_iter_get_arg_type (&struct_iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&struct_iter, &name_space);
        }

        dbus_message_iter_next (&struct_iter);
        if(dbus_message_iter_get_arg_type (&struct_iter) == DBUS_TYPE_INT32)
        {
            dbus_message_iter_get_basic (&struct_iter, &tmp);
            data_type = tmp;
        }
		
        dbus_message_iter_next (&iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            dbus_message_iter_get_basic (&iter, &subsystem_prefix);
        }

        result = checkNamespaceDataType(
                     name_space,
                     subsystem_prefix,
                     data_type,
                     &typeMatch
                 );

        dbus_message_iter_init_append (reply, &iter);

        btmp = typeMatch;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &btmp);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("dumpComponentRegistry", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;

        result = dumpComponentRegistry(
                 );

        dbus_message_iter_init_append (reply, &iter);

        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("isSystemReady", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_bool_t btmp ;
        dbus_int32_t result = 0;
        dbus_bool Ready;

        result = isSystemReady(
                     &Ready
                 );

        dbus_message_iter_init_append (reply, &iter);

        btmp = Ready;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &btmp);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("requestSessionID", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;  /*RDKB-6902, CID-33061, initilize before use*/
        dbus_int32_t result = 0;
        int sessionID = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_INT32)
        {
            dbus_message_iter_get_basic (&iter, &tmp);
            sessionID = tmp;
        }
        result = requestSessionID(
                     tmp,
                     &sessionID
                 );

        dbus_message_iter_init_append (reply, &iter);

        tmp = sessionID;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("getCurrentSessionID", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        int sessionID = 0;
        int priority = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_INT32)
        {
            dbus_message_iter_get_basic (&iter, &tmp);
            sessionID = tmp;
        }
        result = getCurrentSessionID(
                     &priority,
                     &sessionID
                 );

        dbus_message_iter_init_append (reply, &iter);

        tmp = sessionID;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        tmp = priority;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp(CCSP_DBUS_INTERFACE_CR, interface) && !strcmp("informEndOfSession", method))
    {
        DBusMessageIter iter;
        dbus_int32_t tmp = 0;
        dbus_int32_t result = 0;
        int sessionID = 0;

        dbus_message_iter_init (message, &iter);
        if(dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_INT32)
        {
            dbus_message_iter_get_basic (&iter, &tmp);
            sessionID = tmp;
        }

        result = informEndOfSession(
                     sessionID
                 );

        dbus_message_iter_init_append (reply, &iter);

        tmp = result;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32, &tmp);
        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (!strcmp("org.freedesktop.DBus.Introspectable", interface) && !strcmp("Introspect", method))
    {
        if ( !dbus_message_append_args (reply, DBUS_TYPE_STRING, &CR_Introspect_msg, DBUS_TYPE_INVALID))
            printf ("No memory\n");

        if (!dbus_connection_send (conn, reply, NULL))
            printf ("No memory\n");

        dbus_message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;

    }

    return CcspBaseIf_base_path_message_func (conn,
            message,
            reply,
            interface,
            method,
            bus_info);
}

ANSC_STATUS
waitingForSystemReadyTask(ANSC_HANDLE  hThisObject)
{
    ULONG                           uTicks            = 1000;
    ULONG                           uWait1            = 120;  /* 2 minutes */
    ULONG                           uWait2            = 10;   /* 10 seconds */
    ULONG                           i                 = 0;
    PCCSP_COMPONENT_INFO            pCompInfo         = (PCCSP_COMPONENT_INFO)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY)NULL;

    UNREFERENCED_PARAMETER(hThisObject);
    /* wait for uWait1 seconds before checking */
    while ( i < uWait1 )
    {
        if(!g_pCcspCrMgr)  return ANSC_STATUS_SUCCESS;

        AnscSleep(uTicks);

        i ++;
    }

    /* check the system ready status */
    i = 0;

    while( TRUE )
    {
        AnscSleep(uTicks);
        i ++;

        if( g_pCcspCrMgr->bSystemReady)
        {
            return ANSC_STATUS_SUCCESS;
        }

        if( i % uWait2 != uWait2 - 1)
        {
            continue;
        }

        /* print out the first one not registered yet */
        pSLinkEntry = AnscQueueGetFirstEntry(&g_pCcspCrMgr->CompInfoQueue);

        while ( pSLinkEntry )
        {
            pCompInfo       = ACCESS_CCSP_COMPONENT_INFO(pSLinkEntry);
            pSLinkEntry     = AnscQueueGetNextEntry(pSLinkEntry);

            if( pCompInfo != NULL)
            {
                if( pCompInfo->uStatus == CCSP_Component_NotRegistered)
                {
		    AnscTrace("System Not Ready!!!! '%s' v%lu NotRegistered\n",
			     pCompInfo->pComponentName, pCompInfo->uVersion);
		    if (strstr(pCompInfo->pComponentName, "ccsp.cm"))
		    {
			t2_event_d("SYS_ERROR_CM_Not_Registered", 1);
		    }
		    else if (strstr(pCompInfo->pComponentName, "ccsp.psm"))
		    {
			t2_event_d("SYS_ERROR_PSM_Not_Registered", 1);
		    }
		    else if (strstr(pCompInfo->pComponentName, "ccsp.wifi"))
		    {
			t2_event_d("SYS_ERROR_WIFI_Not_Registered", 1);
		    }
                    break;
                }
                else if( pCompInfo->uStatus != CCSP_Component_RegSuccessful)
                {
                    AnscTrace("System Not Ready!!!! '%s' v%lu RegisterFailed\n", pCompInfo->pComponentName, pCompInfo->uVersion);
                    break;
                }
            }
        }

    }


    return ANSC_STATUS_SUCCESS;
}

int CcspCcMbi_GetHealth()
{
    extern int g_crHealth;
    return g_crHealth;
}

void CcspCrProcessRegisterCap(char* pCompName, void* user_data)
{
    PCCSP_COMPONENT_INFO            pCompInfo         = (PCCSP_COMPONENT_INFO)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY)NULL;
    (void) user_data;

    /* check whether this component is in the profile or not */
    pCompInfo = (PCCSP_COMPONENT_INFO) CcspCrLookforComponent(g_pCcspCrMgr, pCompName);

    if( pCompInfo == NULL)
    {
        AnscTraceWarning(("Unknown Registered Component:  %s\n", pCompName));
        return;
    }

    AnscTraceWarning(("Registering Component:  %s\n", pCompName));

    pCompInfo->uStatus = CCSP_Component_RegSuccessful;

    /* check whether system is ready or not */
    if(!g_pCcspCrMgr->bSystemReady)
    {
        pSLinkEntry = AnscQueueGetFirstEntry(&g_pCcspCrMgr->CompInfoQueue);

        while ( pSLinkEntry )
        {
            pCompInfo       = ACCESS_CCSP_COMPONENT_INFO(pSLinkEntry);
            pSLinkEntry     = AnscQueueGetNextEntry(pSLinkEntry);

            AnscTraceWarning(("CcspCrRegisterCapabilities - print component %s with status %lu\n",
                    pCompInfo->pComponentName, pCompInfo->uStatus));
        }

        pSLinkEntry = AnscQueueGetFirstEntry(&g_pCcspCrMgr->CompInfoQueue);

        while ( pSLinkEntry )
        {
            pCompInfo       = ACCESS_CCSP_COMPONENT_INFO(pSLinkEntry);
            pSLinkEntry     = AnscQueueGetNextEntry(pSLinkEntry);

            AnscTraceWarning(("CcspCrRegisterCapabilities - component %s status %lu\n",
                    pCompInfo->pComponentName, pCompInfo->uStatus));

            if( pCompInfo->uStatus != CCSP_Component_RegSuccessful)
            {
                break;
            }
        }

        if( pSLinkEntry == NULL)
        {
            g_pCcspCrMgr->bSystemReady = TRUE;

            /* send out System Ready event */
            AnscTraceWarning(("From CR: System is ready...\n"));

            if( g_pCcspCrMgr->SignalProc.SignalSystemReadyProc != NULL)
            {
                g_pCcspCrMgr->SignalProc.SignalSystemReadyProc(g_pCcspCrMgr->hDbusHandle);
            }
        }
    }

    return;
}

int ccspCrSystemReady ()
{
    return (int) g_pCcspCrMgr->IsSystemReady(g_pCcspCrMgr);
}

void InitDbus()
{

    CCSP_Base_Func_CB               cb;
    char                            CrName[256];
    ANSC_STATUS                     returnStatus       = ANSC_STATUS_SUCCESS;

    pComponentName = g_pCcspCrMgr->pCRName;

    g_pCcspCrMgr->SetPrefix(g_pCcspCrMgr, g_Subsystem);

    if ( g_Subsystem[0] != 0 )
    {
        _ansc_sprintf(CrName, "%s%s", g_Subsystem, g_pCcspCrMgr->pCRName);
    }
    else
    {
        AnscCopyString(CrName, g_pCcspCrMgr->pCRName);
    }

    /* CID 71933 - Unchecked return value */
    returnStatus = CCSP_Message_Bus_Init(CrName, CCSP_MSG_BUS_CFG, &g_pDbusHandle, (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
    if ( returnStatus != ANSC_STATUS_SUCCESS )
    {
        CcspTraceInfo((" !!! CCSP Message Bus Init ERROR !!!\n"));
    }

    sleep(1);

    /* set it to CR manager */
    g_pCcspCrMgr->hDbusHandle                         = g_pDbusHandle;
    g_pCcspCrMgr->SignalProc.SignalSystemReadyProc    = CcspBaseIf_SendsystemReadySignal;
    g_pCcspCrMgr->SignalProc.SignalProfileChangeProc  = CcspBaseIf_SenddeviceProfileChangeSignal;
#if 0 /* Commenting this part as sessionID is taken care of session_mgr of rbus */
    g_pCcspCrMgr->SignalProc.SignalSessionChangeProc  = CcspBaseIf_SendcurrentSessionIDSignal;
#endif

    memset(&cb, 0, sizeof(cb));

    cb.registerCaps  = CcspCrProcessRegisterCap;
    cb.isSystemReady = ccspCrSystemReady;

    cb.getParameterValues     = CcspCcMbi_GetParameterValues;
    cb.setParameterValues     = CcspCcMbi_SetParameterValues;
    cb.setCommit              = CcspCcMbi_SetCommit;
    cb.setParameterAttributes = CcspCcMbi_SetParameterAttributes;
    cb.getParameterAttributes = CcspCcMbi_GetParameterAttributes;
    cb.AddTblRow              = CcspCcMbi_AddTblRow;
    cb.DeleteTblRow           = CcspCcMbi_DeleteTblRow;
    cb.getParameterNames      = CcspCcMbi_GetParameterNames;
    cb.currentSessionIDSignal = CcspCcMbi_CurrentSessionIdSignal;

    cb.initialize             = initialize;
    cb.finalize               = finalize;
    cb.freeResources          = freeResources;
    cb.busCheck               = busCheck;
    cb.getHealth              = CcspCcMbi_GetHealth;

    CcspBaseIf_SetCallback
    (
        g_pDbusHandle,
        &cb
    );

    if(g_iTraceLevel >= CCSP_TRACE_LEVEL_EMERGENCY)
        ulLogLevel = (ULONG) g_iTraceLevel;
    bLogEnable   = TRUE;                                  
    /* Create and init data model library */
    pSsdCcdIf = (PCCSP_CCD_INTERFACE)AnscAllocateMemory(sizeof(CCSP_CCD_INTERFACE));

    AnscCopyString(pSsdCcdIf->Name, CCSP_CCD_INTERFACE_NAME);

    pSsdCcdIf->InterfaceId              = CCSP_CCD_INTERFACE_ID;
    pSsdCcdIf->hOwnerContext            = NULL;
    pSsdCcdIf->Size                     = sizeof(CCSP_CCD_INTERFACE);

    pSsdCcdIf->GetComponentName         = ssp_CcdIfGetComponentName;
    pSsdCcdIf->GetComponentVersion      = ssp_CcdIfGetComponentVersion;
    pSsdCcdIf->GetComponentAuthor       = ssp_CcdIfGetComponentAuthor;
    pSsdCcdIf->GetComponentHealth       = ssp_CcdIfGetComponentHealth;
    pSsdCcdIf->GetComponentState        = ssp_CcdIfGetComponentState;
    pSsdCcdIf->GetLoggingEnabled        = ssp_CcdIfGetLoggingEnabled;
    pSsdCcdIf->SetLoggingEnabled        = ssp_CcdIfSetLoggingEnabled;
    pSsdCcdIf->GetLoggingLevel          = ssp_CcdIfGetLoggingLevel;
    pSsdCcdIf->SetLoggingLevel          = ssp_CcdIfSetLoggingLevel;
    pSsdCcdIf->GetMemMaxUsage           = ssp_CcdIfGetMemMaxUsage;
    pSsdCcdIf->GetMemMinUsage           = ssp_CcdIfGetMemMinUsage;
    pSsdCcdIf->GetMemConsumed           = ssp_CcdIfGetMemConsumed;
    pSsdCcdIf->ApplyChanges             = ssp_CcdIfApplyChanges;

    pDslhCpeController = DslhCreateCpeController(NULL, NULL, NULL);

    pDslhCpeController->AddInterface((ANSC_HANDLE)pDslhCpeController, (ANSC_HANDLE)pSsdCcdIf);
    pDslhCpeController->SetDbusHandle((ANSC_HANDLE)pDslhCpeController, (ANSC_HANDLE)g_pDbusHandle);
    pDslhCpeController->Engage((ANSC_HANDLE)pDslhCpeController);

    pDslhCpeController->RegisterCcspDataModel
        (
            (ANSC_HANDLE)pDslhCpeController,
            g_pCcspCrMgr->pCRName,              /* CCSP CR ID */
            CCSP_DATAMODEL_XML_FILE,             /* Data Model XML file. Can be empty if only base data model supported. */
            g_pCcspCrMgr->pCRName,               /* Component Name    */
            CCSP_CR_VERSION,                     /* Component Version */
            CCSP_DBUS_PATH_CR,                   /* Component Path    */
            g_Subsystem                          /* Component Prefix  */
        );

    /* start waitingforsystemready task */
    AnscSpawnTask
        (
            waitingForSystemReadyTask,
            (ANSC_HANDLE)NULL,
            "waitingForSystemReadyTask"
        );

}

void ExitDbus()
{
    CCSP_Message_Bus_Exit(g_pDbusHandle);
}



