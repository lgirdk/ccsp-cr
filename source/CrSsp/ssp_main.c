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

#ifdef _ANSC_LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#endif

#include "ssp_global.h"

PCCSP_CR_MANAGER_OBJECT                     g_pCcspCrMgr            = NULL;
void*                                       g_pDbusHandle           = NULL;
ULONG                                       g_ulAllocatedSizeInit   = 0;
char                                        g_Subsystem[32]         = {0};

#ifndef WIN32
extern ULONG                                g_ulAllocatedSizeCurr;
#else
ULONG                                       g_ulAllocatedSizeCurr = 0;
#endif

extern char*                                pComponentName;

#define  CCSP_COMMON_COMPONENT_HEALTH_Red                   1
#define  CCSP_COMMON_COMPONENT_HEALTH_Yellow                2
#define  CCSP_COMMON_COMPONENT_HEALTH_Green                 3
int                                         g_crHealth = CCSP_COMMON_COMPONENT_HEALTH_Red;

/*
 *  For export data models in Motive compliant format
 */
BOOLEAN                                     g_exportAllDM = false;

int  cmd_dispatch(int  command)
{    
    switch ( command )
    {
        case    'e' :

                if ( !g_pCcspCrMgr)
                {
                    g_pCcspCrMgr = (PCCSP_CR_MANAGER_OBJECT)CcspCreateCR();
                    InitDbus();

                    g_crHealth = CCSP_COMMON_COMPONENT_HEALTH_Green;

                    g_ulAllocatedSizeInit = g_ulAllocatedSizeCurr;
                }
                else
                {
                    AnscTrace("CR is already created and engaged.\n");
                }

                break;

        case    'c' :

                if ( g_pCcspCrMgr )
                {
                    ExitDbus();
                    CcspFreeCR((ANSC_HANDLE)g_pCcspCrMgr);

                    g_pCcspCrMgr = NULL;
                }
                else
                {
                    AnscTrace("CR is not created.\n");
                }

                break;

        case    'd' :

                if ( g_pCcspCrMgr )
                {
                    g_pCcspCrMgr->DumpObject(g_pCcspCrMgr);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }

                break;

        case    'm' :

//                AnscTraceMemoryUsage();

                break;

        case    't' :

                AnscTraceMemoryTable();

                break;

        case    'r':     /* AnscTrace("'r'     --------  \"Register Namespace\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    CRRegisterTest();
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    'u':     /* AnscTrace("'u'     --------  \"Unregister Namespace\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    CRUnregisterTest();
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    'v':     /* AnscTrace("'v'     --------  \"Discover Namespace\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    CRDiscoverTest();
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    '1':     /*         AnscTrace("GetParmeterNames from 'Device.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("Device.", TRUE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    '2':     /*         AnscTrace("GetParmeterNames from 'com.cisco.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("com.cisco.", TRUE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;
                
        case    '3':     /*         AnscTrace("GetParmeterValues from 'Device.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("Device.", FALSE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    '4':     /*         AnscTrace("GetParmeterValues from 'com.cisco.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("com.cisco.", FALSE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    '5':     /*         AnscTrace("GetParmeterNames from 'com.cisco.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("", TRUE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    '6':     /*         AnscTrace("GetParmeterValues from 'com.cisco.'\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    BaseApiTest("", FALSE);
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;
                
        case    'y':   /*  AnscTrace("'y'     --------  \"Get Component Array\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    CRComponentTest();
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    'k':   /*  AnscTrace("'k'     --------  \"Check Namespace DataType\"\n"); */

                if ( g_pCcspCrMgr )
                {
                    CRCheckDataTypeTest();
                }
                else
                {
                    AnscTrace("CR is still not created.\n");
                }
                break;

        case    's':  /*   AnscTrace("'s'     --------  \"Session control test\"\n"); */
               
                if ( g_pCcspCrMgr )
                {
                    CRSessionTest();
                }
                else
                {
                    AnscTrace("CR is still not created. Click 'e' to create it.\n");
                }

                break;

        case    'b':   /*  AnscTrace("'b'     --------  \"Batch Test\"\n"); */

                if ( !g_pCcspCrMgr)
                {
                    g_pCcspCrMgr = (PCCSP_CR_MANAGER_OBJECT)CcspCreateCR();
                    InitDbus();
                    g_ulAllocatedSizeInit = g_ulAllocatedSizeCurr;

                }

                CRBatchTest();

                break;

        case    '0':

                g_pCcspCrMgr->SignalProc.SignalSystemReadyProc(g_pCcspCrMgr->hDbusHandle);

                break;

        default :

                break;
    }

    return  0;
}


int  gather_info()
{
	AnscTrace("\n\n");
	AnscTrace("        ***************************************************************\n");
    AnscTrace("        ***                                                         ***\n");
    AnscTrace("        ***   Common Component Software Platform (CCSP)             ***\n");
    AnscTrace("        ***   Component Registrar (CR) Application                  ***\n");
    AnscTrace("        ***                                                         ***\n");
    AnscTrace("        ***   Cisco Systems, Inc., All Rights Reserved.             ***\n");
    AnscTrace("        ***                                                         ***\n");
    AnscTrace("        ***************************************************************\n");
    AnscTrace("\n\n");

    AnscTrace("\n=============================================\nCCSP CR Menu:\n");

    AnscTrace("'e'     --------  \"Engage the program\"\n");
    AnscTrace("'c'     --------  \"Cancel the program\"\n");
    AnscTrace("'m'     --------  \"Memory Check\"\n");
    AnscTrace("'t'     --------  \"Memory Trace\"\n");
    AnscTrace("'d'     --------  \"Dump CR information\"\n");
    AnscTrace("'r'     --------  \"Register Namespace\"\n");
    AnscTrace("'u'     --------  \"Unregister Namespace\"\n");
    AnscTrace("'v'     --------  \"Discover Namespace\"\n");
    AnscTrace("'y'     --------  \"Get Component Array\"\n");
    AnscTrace("'k'     --------  \"Check Namespace DataType\"\n");
    AnscTrace("'s'     --------  \"Session control test\"\n");
    AnscTrace("'b'     --------  \"Batch Test\"\n");
    AnscTrace("'1'     --------  \"GetParameterNames from 'Device.'\"\n");
    AnscTrace("'2'     --------  \"GetParmaeterNames from 'com.cisco.'\"\n");
    AnscTrace("'3'     --------  \"GetParameterValues from 'Device.'\"\n");
    AnscTrace("'4'     --------  \"GetParmaeterValues from 'com.cisco.'\"\n");
    AnscTrace("'5'     --------  \"GetParameterNames (all)\"\n");
    AnscTrace("'6'     --------  \"GetParmaeterValues (all)\"\n");
    AnscTrace("'q'     --------  \"Quit\"\n");
    AnscTrace("=============================================\n\n");

    return  0;
}

static void _print_stack_backtrace(void)
{
#ifdef __GNUC__
#if (!defined _BUILD_ANDROID) && (!defined _NO_EXECINFO_H_)
        void* tracePtrs[100];
        char** funcNames = NULL;
        int i, count = 0;

        count = backtrace( tracePtrs, 100 );
        backtrace_symbols_fd( tracePtrs, count, 2 );

        funcNames = backtrace_symbols( tracePtrs, count );

        if ( funcNames ) {
            // Print the stack trace
            for( i = 0; i < count; i++ )
                printf("%s\n", funcNames[i] );

            // Free the string pointers
            free( funcNames );
        }
#endif
#endif
}

void sig_handler(int sig)
{
    if ( sig == SIGINT ) {
        signal(SIGINT, sig_handler); /* reset it to this function */
        CcspTraceInfo(("SIGINT received!\n"));
        exit(0);
    }
    else if ( sig == SIGUSR1 ) {
        signal(SIGUSR1, sig_handler); /* reset it to this function */
        CcspTraceInfo(("SIGUSR1 received!\n"));
    }
    else if ( sig == SIGUSR2 ) {
        CcspTraceInfo(("SIGUSR2 received!\n"));
        /* DH  Very ugly reference of functions -- generate the complete DM XML file*/
        GenerateDataModelXml();
    }
    else if ( sig == SIGCHLD ) {
        signal(SIGCHLD, sig_handler); /* reset it to this function */
        CcspTraceInfo(("SIGCHLD received!\n"));
    }
    else if ( sig == SIGPIPE ) {
        signal(SIGPIPE, sig_handler); /* reset it to this function */
        CcspTraceInfo(("SIGPIPE received!\n"));
    }
    else if ( sig == SIGTERM )
    {
        CcspTraceInfo(("SIGTERM received!\n"));
        exit(0);
    }
    else if ( sig == SIGKILL )
    {
        CcspTraceInfo(("SIGKILL received!\n"));
        exit(0);
    }
    else {
        /* get stack trace first */
        _print_stack_backtrace();
        CcspTraceInfo(("Signal %d received, exiting!\n", sig));
        exit(0);
    }
}


#if defined(_ANSC_LINUX)
static void daemonize(void) {
	int fd;
	switch (fork()) {
	case 0:
		break;
	case -1:
		// Error
		AnscTrace("Error daemonizing (fork)! %d - %s\n", errno, strerror(
				errno));
		exit(0);
		break;
	default:
		_exit(0);
	}

	if (setsid() < 	0) {
		AnscTrace("Error demonizing (setsid)! %d - %s\n", errno, strerror(errno));
		exit(0);
	}

    /*
     *  What is the point to change current directory?
     *
    chdir("/");
     */

#ifndef  _DEBUG

	fd = open("/dev/null", O_RDONLY);
	if (fd != 0) {
		dup2(fd, 0);
		close(fd);
	}
	fd = open("/dev/null", O_WRONLY);
	if (fd != 1) {
		dup2(fd, 1);
		close(fd);
	}
	fd = open("/dev/null", O_WRONLY);
	if (fd != 2) {
		dup2(fd, 2);
		close(fd);
	}
#endif
}
#endif

int main(int argc, char* argv[])
{
	int                             cmdChar = 0;
    int                             idx     = 0;
    BOOL                            bRunAsDaemon       = TRUE;

    pComponentName = CCSP_DBUS_INTERFACE_CR;

#if defined(_DEBUG) || defined(_COSA_SIM_)
    AnscSetTraceLevel(CCSP_TRACE_LEVEL_INFO);
#endif

    srand(time(0));

    for (idx = 1; idx < argc; idx++)
    {
        if ( (strcmp(argv[idx], "-subsys") == 0) )
        {
            AnscCopyString(g_Subsystem, argv[idx+1]);
        }
        else if ( strcmp(argv[idx], "-c" ) == 0 )
        {
            bRunAsDaemon = FALSE;
        }
        else if (strcmp(argv[idx], "-eDM") == 0)
        {
            g_exportAllDM = true;
        }
    }

#if  defined(_ANSC_WINDOWSNT)

    AnscStartupSocketWrapper(NULL);

    gather_info();

    cmd_dispatch('e');


    while ( cmdChar != 'q' )
    {
        AnscTrace("\n=============================================\nCCSP CR Menu:\n");

	    AnscTrace("'e'     --------  \"Engage the program\"\n");
	    AnscTrace("'c'     --------  \"Cancel the program\"\n");
	    AnscTrace("'m'     --------  \"Memory Check\"\n");
	    AnscTrace("'t'     --------  \"Memory Trace\"\n");
	    AnscTrace("'d'     --------  \"Dump CR information\"\n");
	    AnscTrace("'r'     --------  \"Register Namespace\"\n");
	    AnscTrace("'u'     --------  \"Unregister Namespace\"\n");
	    AnscTrace("'v'     --------  \"Discover Namespace\"\n");
	    AnscTrace("'y'     --------  \"Get Component Array\"\n");
	    AnscTrace("'k'     --------  \"Check Namespace DataType\"\n");
	    AnscTrace("'s'     --------  \"Session control test\"\n");
	    AnscTrace("'b'     --------  \"Batch Test\"\n");
	    AnscTrace("'0'     --------  \"Send systemReadySignal\"\n");
	    AnscTrace("'1'     --------  \"GetParameterNames from 'Device.'\"\n");
	    AnscTrace("'2'     --------  \"GetParmaeterNames from 'com.cisco.'\"\n");
	    AnscTrace("'3'     --------  \"GetParameterValues from 'Device.'\"\n");
	    AnscTrace("'4'     --------  \"GetParmaeterValues from 'com.cisco.'\"\n");
	    AnscTrace("'5'     --------  \"GetParameterNames (all)\"\n");
	    AnscTrace("'6'     --------  \"GetParmaeterValues (all)\"\n");
	    AnscTrace("'q'     --------  \"Quit\"\n");
	    AnscTrace("=============================================\n\n");

        cmdChar = getchar();
        fflush(stdin);

        cmd_dispatch(cmdChar);
    }


#elif defined(_ANSC_LINUX)
    if ( bRunAsDaemon )
        daemonize();

    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    signal(SIGSEGV, sig_handler);
    signal(SIGBUS, sig_handler);
    signal(SIGKILL, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGHUP, sig_handler);
    signal(SIGPIPE, SIG_IGN);

    gather_info();

    cmd_dispatch('e');


    if ( bRunAsDaemon )
    {
		while (1)
			sleep(30);
    }
    else 
    {
		while ( cmdChar != 'q' )
		{
			cmdChar = getchar();
			cmd_dispatch(cmdChar);
		}
    }
#endif

    if ( g_pCcspCrMgr )
    {
        ExitDbus();
        CcspFreeCR((ANSC_HANDLE)g_pCcspCrMgr);
        g_pCcspCrMgr = NULL;
    }

    return 0;
}

