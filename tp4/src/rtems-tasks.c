/*  Test_task
 *
 *  This routine serves as a test task.  It verifies the basic task
 *  switching capabilities of the executive.
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: tasks.c,v 1.7.2.1 2000/05/05 12:58:06 joel Exp $
 */

#include <rtems.h>
#include "rtems-task.h"
/* functions */

rtems_task Init(
  rtems_task_argument argument
);

rtems_task Test_task(
  rtems_task_argument argument
);

rtems_task initialize_producer(
	rtems_task_argument unused
);

void test(char* s, uint32_t i);


/* global variables */

/*
 *  Keep the names and IDs in global variables so another task can use them.
 */

extern rtems_id   Task_id[ 4 ];         /* array of task ids */
extern rtems_name Task_name[ 4 ];       /* array of task names */
float imagettes[ 100 ][ 36 ];			/* array of pictures 6x6 */
unsigned int windows_max_number = 10;

//static uint32_t buffer[1];


/* configuration information */

#include <bsp.h> /* for device driver prototypes */
#include <windows-producer.h>

#define MESSAGE_QUEUE_COUNT 3
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS 5

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

//define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
#define CONFIGURE_MAXIMUM_TIMERS 10


#include <rtems/confdefs.h>

/*
 *  Handy macros and static inline functions
 */

/*
 *  Macro to hide the ugliness of printing the time.
 */

#define print_time(_s1, _tb, _s2) \
  do { \
    printf( "%s%02d:%02d:%02d   %02d/%02d/%04d%s", \
       _s1, (_tb)->hour, (_tb)->minute, (_tb)->second, \
       (_tb)->month, (_tb)->day, (_tb)->year, _s2 ); \
    fflush(stdout); \
  } while ( 0 )

/*
 *  Macro to print an task name that is composed of ASCII characters.
 *
 */

#define put_name( _name, _crlf ) \
  do { \
    uint32_t c0, c1, c2, c3; \
    \
    c0 = ((_name) >> 24) & 0xff; \
    c1 = ((_name) >> 16) & 0xff; \
    c2 = ((_name) >> 8) & 0xff; \
    c3 = (_name) & 0xff; \
    putchar( (char)c0 ); \
    if ( c1 ) putchar( (char)c1 ); \
    if ( c2 ) putchar( (char)c2 ); \
    if ( c3 ) putchar( (char)c3 ); \
    if ( (_crlf) ) \
      putchar( '\n' ); \
  } while (0)

/*
 *  static inline routine to make obtaining ticks per second easier.
 */

static inline uint32_t get_ticks_per_second( void )
{
  rtems_interval ticks_per_second;
  (void) rtems_clock_get( RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second );  return ticks_per_second;
}


/*
 *  This allows us to view the "Test_task" instantiations as a set
 *  of numbered tasks by eliminating the number of application
 *  tasks created.
 *
 *  In reality, this is too complex for the purposes of this
 *  example.  It would have been easier to pass a task argument. :)
 *  But it shows how rtems_id's can sometimes be used.
 */

#define task_number( tid ) \
  ( rtems_object_id_get_index( tid ) - \
     rtems_configuration_get_rtems_api_configuration()->number_of_initialization_tasks )


#include <stdio.h>
#include <stdlib.h>

/*
 *  Keep the names and IDs in global variables so another task can use them.
 */

rtems_id   Task_id[ 4 ];         /* array of task ids */
rtems_name Task_name[ 4 ];       /* array of task names */
rtems_id timer_id;

static size_t size;
rtems_id timer_id;
rtems_id task_id_1;
rtems_id message_queue_id_1;

static uint32_t data = 0;


rtems_task Init(
  rtems_task_argument argument
)
{

  rtems_status_code status;
  rtems_time_of_day time;

  puts( "\n\n***** CLOCK TICK TEST *****" );

  time.year   = 1988;
  time.month  = 12;
  time.day    = 31;
  time.hour   = 9;
  time.minute = 0;
  time.second = 0;
  time.ticks  = 0;

  status = rtems_clock_set( &time );


  Task_name[ 1 ] = rtems_build_name( 'T', 'A', '1', ' ' );
  Task_name[ 2 ] = rtems_build_name( 'T', 'A', '2', ' ' );
  Task_name[ 3 ] = rtems_build_name( 'T', 'A', '3', ' ' );

  status = rtems_timer_create(rtems_build_name('T','I','M',1),&timer_id);

  status = rtems_timer_fire_after(timer_id, get_ticks_per_second(), time_fun, 0);


  status = rtems_task_create(
    Task_name[ 1 ], 1, 10240 , RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &task_id_1
  );

  status = rtems_message_queue_create(rtems_build_name('M','S','Q', '1'),
		  MESSAGE_QUEUE_COUNT, CONFIGURE_MAXIMUM_MESSAGE_QUEUES,
		  RTEMS_LOCAL | RTEMS_PRIORITY,
		  &message_queue_id_1
  );


  status = rtems_task_start( task_id_1, initialize_producer, 1 );
  status = rtems_task_delete( RTEMS_SELF );
  puts("avant bien envoyé");
}

#include <stdio.h>

void time_fun(){


	rtems_status_code stat;

	data++;

	stat = rtems_message_queue_send(
			  message_queue_id_1, &data,
			  sizeof(uint32_t)
	  );

	puts("bien envoyé");
	stat = rtems_timer_fire_after(
			timer_id, get_ticks_per_second(),
			time_fun, 0
	);
	puts("apres bien envoyé");
	//test("time_fun", stat);

}

rtems_task initialize_producer(rtems_task_argument unused) {
	rtems_status_code stat;
	size_t size;
	Windows_producer wp;
	uint32_t time;


	// init(&wp,&imagettes,windows_max_number);

	for( ; ; ) {
		// produce_images(&wp);

		//rtems_task_wake_after(50);
		stat = rtems_message_queue_receive(message_queue_id_1, &data, &size, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
		puts("avant bien envoye");
		if (stat == RTEMS_SUCCESSFUL) {
			printf("message from MSQ1 = %d", data);
			//test("init_producer",time);
		}
		//stat = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_SINCE_BOOT, &time);

	}
	return unused;
}

void test(char* test_name, uint32_t time){

}



