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
#include <semaphore.h>
#include <fcntl.h>

#include "ssp_global.h"
#include "syscfg/syscfg.h"
#include "cap.h"
#include "telemetry_busmessage_sender.h"

static cap_user appcaps;

bool g_dropStatus = false;

#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif
#define DEBUG_INI_NAME  "/etc/debug.ini"

#define SUBSYS_LEN 32

PCCSP_CR_MANAGER_OBJECT                     g_pCcspCrMgr            = NULL;
void*                                       g_pDbusHandle           = NULL;
ULONG                                       g_ulAllocatedSizeInit   = 0;
char                                        g_Subsystem[SUBSYS_LEN] = {0};

extern ULONG                                g_ulAllocatedSizeCurr;

extern char*                                pComponentName;

#define  CCSP_COMMON_COMPONENT_HEALTH_Red                   1
#define  CCSP_COMMON_COMPONENT_HEALTH_Yellow                2
#define  CCSP_COMMON_COMPONENT_HEALTH_Green                 3
int                                         g_crHealth = CCSP_COMMON_COMPONENT_HEALTH_Red;

/*
 *  For export data models in Motive compliant format
 */
BOOLEAN                                     g_exportAllDM = false;

static int cmd_dispatch (int command)
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


static int gather_info (void)
{
	AnscTrace("\n\n");
	AnscTrace("        ***************************************************************\n");
    AnscTrace("        ***                                                         ***\n");
    AnscTrace("        ***   Common Component Software Platform (CCSP)             ***\n");
    AnscTrace("        ***   Component Registrar (CR) Application                  ***\n");
    AnscTrace("        ***                                                         ***\n");
    AnscTrace("        ***   Copyright Cisco Systems, Inc. under Apache 2.0	   ***\n");
    AnscTrace("        ***      as per component's LICENSE file.         	   ***\n");
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

#ifndef INCLUDE_BREAKPAD
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

static void sig_handler(int sig)
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
#endif

static void drop_root()
{ 
  appcaps.caps = NULL;
  appcaps.user_name = NULL;
  bool ret = false;
  ret = isBlocklisted();
  if(ret)
  {
    AnscTrace("NonRoot feature is disabled\n");
  }
  else
  {
    AnscTrace("NonRoot feature is enabled, dropping root privileges for CcspCr process\n");
    init_capability();
    drop_root_caps(&appcaps);
    update_process_caps(&appcaps);
    read_capability(&appcaps);
  }
}

static void* waitforsyscfgReady(void *arg)
{
  UNREFERENCED_PARAMETER(arg);
  #define TIME_INTERVAL 2000
  #define MAX_WAIT_TIME 90
  int times = 0;
  pthread_detach(pthread_self());
  while(times++ < MAX_WAIT_TIME)    {
        if ( 0 != syscfg_init( ) )    {
             CCSP_Msg_SleepInMilliSeconds(TIME_INTERVAL);
        }
        else {
             g_dropStatus = true;
             break;
        }
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int cmdChar = 0;
    int idx     = 0;
    BOOL bRunAsDaemon = TRUE;
    char cmd[1024] = {0};
    FILE *fd = NULL;
    int rc;
    sem_t *sem = NULL;

    // Buffer characters till newline for stdout and stderr
    setlinebuf(stdout);
    setlinebuf(stderr);

    pComponentName = CCSP_DBUS_INTERFACE_CR;
#ifdef FEATURE_SUPPORT_RDKLOG
    RDK_LOGGER_INIT();
#endif

#if defined(_DEBUG) || defined(_COSA_SIM_)
    AnscSetTraceLevel(CCSP_TRACE_LEVEL_INFO);
#endif

    srand(time(0));

    for (idx = 1; idx < argc; idx++)
    {
        if ( (strcmp(argv[idx], "-subsys") == 0) )
        {
	    /* CID-137568 fix */
	    if(strlen(argv[idx+1]) < SUBSYS_LEN)
	    {
		AnscCopyString(g_Subsystem, argv[idx+1]);
	    }
	    else
	    {
	        AnscTrace("subsys length error \n");
		return 1;
	    }
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

/*demonizing*/
    if ( bRunAsDaemon )
    {

		 /* initialize semaphores for shared processes */
		 sem = sem_open ("pSemCr", O_CREAT | O_EXCL, 0644, 0);
		 if(SEM_FAILED == sem)
		 {
		 	AnscTrace("Failed to create semaphore %d - %s\n", errno, strerror(errno));
		 	_exit(1);
		 }
		 /* name of semaphore is "pSemCr", semaphore is reached using this name */
		 sem_unlink ("pSemCr");
		 /* unlink prevents the semaphore existing forever */
		 /* if a crash occurs during the execution         */
		 AnscTrace("Semaphore initialization Done!!\n");
		
		 rc = fork();
		 if(rc == 0 ){
		 	AnscTrace("Demonizing Done!!\n");
		 	}
		 else if(rc == -1){

			AnscTrace("Demonizing Error (fork)! %d - %s\n", errno, strerror(errno));
			exit(0);
		 	}
		 else
		 	{
				sem_wait (sem);
				sem_close(sem);
		 		_exit(0);
		 	}
	
		if (setsid() <	0) {
			AnscTrace("Demonizing Error (setsid)! %d - %s\n", errno, strerror(errno));
			exit(0);
		}
	
#ifndef  _DEBUG
                int FileDescriptor;
		FileDescriptor = open("/dev/null", O_RDONLY);
		if (FileDescriptor != 0) {
			dup2(FileDescriptor, 0);
			close(FileDescriptor);
		}
		FileDescriptor = open("/dev/null", O_WRONLY);
		if (FileDescriptor != 1) {
			dup2(FileDescriptor, 1);
			close(FileDescriptor);
		}
		FileDescriptor = open("/dev/null", O_WRONLY);
		if (FileDescriptor != 2) {
			dup2(FileDescriptor, 2);
			close(FileDescriptor);
		}

#endif
	}

	fd = fopen("/var/tmp/CcspCrSsp.pid", "w+");
    if ( !fd )
    {
        AnscTrace("Create /var/tmp/CcspCrSsp.pid error. \n");
        return 1;
    }
    else
    {
        sprintf(cmd, "%d", getpid());
        fputs(cmd, fd);
        fclose(fd);
    }

#ifdef INCLUDE_BREAKPAD
    breakpad_ExceptionHandler();
#else
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

#endif
    t2_init("ccsp-cr");

    if(CCSP_Msg_IsRbus_enabled())
    {
        if(CRRbusOpen() != 0)
        {
            AnscTrace("CRRbusOpen failed\n");
            return 1;
        }
    }
    else
    {
        gather_info();

        cmd_dispatch('e');
    }

	system("touch /tmp/cr_initialized");
    pthread_t EvtThreadId;
    pthread_create(&EvtThreadId, NULL, &waitforsyscfgReady, NULL);


    if ( bRunAsDaemon )
    {
		sem_post (sem);
		sem_close(sem);
		while (1) {
		       if (g_dropStatus) {
                            g_dropStatus = false;
                            drop_root();
			}
			sleep(30);
		}
    }
    else
    {
		while ( cmdChar != 'q' )
		{
			cmdChar = getchar();

            if(!CCSP_Msg_IsRbus_enabled())
            {
			    cmd_dispatch(cmdChar);
            }
		}
    }

    if(CCSP_Msg_IsRbus_enabled())
    {
        CRRbusClose();
    }

    if ( g_pCcspCrMgr )
    {
        ExitDbus();
        CcspFreeCR((ANSC_HANDLE)g_pCcspCrMgr);
        g_pCcspCrMgr = NULL;
    }

    return 0;
}

