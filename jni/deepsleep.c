//
// Deepsleep forces device to deep sleep by accessing /sys/power/state
// if it seems to be appropriate
//
// Version:  0.2
// Date: 18. Aug 2014
// Author Martin Zwicknagl: martin.zwicknagl@kirchbichl.net
//
//  This source code is distributed under the terms of
//   GNU General Public License
//   See LICENSE file for details
//
////////////////////////////////////////////////////////////////////////////

#define VERSION "0.2, 19. Aug. 2014"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROG_NAME   "DeepSleep"

#define true ( 1 == 1 )
#define false ( 1 == 0 )


#define SYS_PWER_STATE "/sys/power/state"
#define CPUFREQ_GOVERNOR              "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define ONDEMAND_UPTHRESHOLD          "/sys/devices/system/cpu/cpufreq/ondemand/up_threshold"
#define ONDEMAND_SAMPLING_DOWN_FACTOR "/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor"
#define ONDEMAND_IGNORE_NICE_LOAD     "/sys/devices/system/cpu/cpufreq/ondemand/ignore_nice_load"

#define ADB_OPEN        "/mnt/extsd/adb_open"
#define ADB_PROPERTY    "persist.service.adb.enable"

#define BOOT_COMPLETE  "sys.boot_completed"


#define POWER_STATE_STRING "mPowerState"
#define POWER_STATE_SLEEP  "mPowerState=0"

#define SLEEP_CMD "mem"
#define WAKE_CMD "on"


#define MAX_BUFFER 1024

#define sleeping 0
#define awake    1


// POLL_TIME is in us
#define POLL_TIME  750000


//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, PROG_NAME, __VA_ARGS__

char propValue[255];


void errorMsg( char *str )
{
    printf("error: %s\n", str );
  // LOGI( "error: ", str );
}

int fexist( char *filename )
{
    int res;
    FILE *fp = fopen( filename, "r" );
    if( fp )
    { // exist
        res = true;
        fclose(fp);
    }
    else
    { // doesnt exist
        res = false;
    }

    return res;
}



void getProp( char* property, char* propValue )
{
   FILE *pipe_reader;
   char str[255];
   sprintf( str, "getprop %s", property );
   if ((pipe_reader = popen ( str, "r")) == NULL)
   {
      errorMsg("Fehler bei popen ...\n");
      exit( 1 );
   }

   fgets( propValue, 255, pipe_reader );

   pclose (pipe_reader);
}

void setProp( char* property, char* propValue )
{
   FILE *pipe_reader;
   char str[255];
   sprintf( str, "setprop %s %s", property, propValue );
   if ((pipe_reader = popen ( str, "r")) == NULL)
   {
      errorMsg("Fehler bei popen ...\n");
      exit( 1 );
   }

   /*
   fgets( propValue, 255, pipe_reader );
*/
   pclose (pipe_reader);
}




void adbOpen()
{
    if ( fexist( ADB_OPEN ) )
    {
        printf(" open ADB\n");
        setProp( ADB_PROPERTY, "1" );
    }
    else
    {
        printf(" do NOT open ADB\n");
    }
    return;
}



void setCpuGovernor( char *governor )
{
    if ( strlen(governor) == 0 )
    {
        errorMsg( "no governor set");
    }


   FILE *fp = fopen( CPUFREQ_GOVERNOR,"w"); // read mode

   if( fp == NULL )
   {
      errorMsg("ERROR opening "CPUFREQ_GOVERNOR"\n" );
      exit(-1);
   }

   fprintf( fp, "%s", governor );

   fclose(fp);
   return;

}


void genericSysFsWrite( char *file, char* data )
{
   FILE *fp = fopen( file, "w"); // read mode

   if( fp == NULL )
   {
      errorMsg( "ERROR opening" );
      errorMsg( file );
      errorMsg( "\n" );
      exit(-1);
   }

   printf( "write %s -> %s\n", data, file );

   fprintf( fp, "%s", data );

   fclose( fp );
   return;
}


void setCpu( int dummy )
{
    setCpuGovernor( "ondemand" );
    genericSysFsWrite( ONDEMAND_UPTHRESHOLD, "66" );
    genericSysFsWrite( ONDEMAND_SAMPLING_DOWN_FACTOR, "2" );
    genericSysFsWrite( ONDEMAND_IGNORE_NICE_LOAD, "0" );
}


void deepSleep()
{

   FILE *fp = fopen( SYS_PWER_STATE,"w"); // read mode

   if( fp == NULL )
   {
      errorMsg("ERROR opening"SYS_PWER_STATE"\n" );
      exit(-1);
   }

   fprintf( fp, "%s", SLEEP_CMD );

   fclose(fp);
   return;
}

void wakeUp()
{
   FILE *fp = fopen( SYS_PWER_STATE,"w"); // read mode

   if( fp == NULL )
   {
      errorMsg( "ERROR opening"SYS_PWER_STATE"\n" );
      exit(-1);
   }

   fprintf(fp, "%s",WAKE_CMD );

   fclose(fp);
   return;
}

int getState( int *isOn )
{
    int status;
    *isOn = sleeping;

   FILE *pipe_reader;
   if ((pipe_reader = popen ("dumpsys power", "r")) == NULL)
   {
      errorMsg("Fehler bei popen ...\n");
      exit( 1 );
   }


   char str[ MAX_BUFFER ];
   while ( !feof( pipe_reader ) )
   {
        fgets( str, MAX_BUFFER, pipe_reader );

//        printf( "%s", str );

        if ( strstr( str, POWER_STATE_STRING ) != NULL )
        {
            if ( strstr( str, POWER_STATE_SLEEP ) != NULL )
                *isOn = sleeping;
            else
                *isOn = awake;
            break;
        }
   }

   pclose (pipe_reader);

   return (status);
}




int main( int argc, char* argv[])
{
    int oldStatus, status;

    printf(" This ist the first C-Version of deepsleep\n");
    printf(" Martin Zwicknagl, put to GPL\n");
    printf(" Version %s\n", VERSION );

    printf(" wait for boot completed\n");

    do
    {
        getProp( BOOT_COMPLETE, propValue);
        sleep(1);
    }
    while ( strncmp( propValue, "1", 1 ) != 0 );


    printf(" set CPU governor \n");
    setCpu( 0 );

    printf("open adb if /mnt/extsd/adb_open exits\n");
    adbOpen();

    printf(" start power save\n");
    getState( &status );

   // getCacheDir();
    do
    {
        oldStatus = status;
        getState( &status);

        printf( "old: %u   new %u\n", oldStatus, status );

        if ( status == sleeping  && oldStatus == awake)
        {
            deepSleep();
        }
        else if ( status == awake && oldStatus == sleeping )
            wakeUp();

        usleep( POLL_TIME );
    }
    while ( true );


    return 0;
}
