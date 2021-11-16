//
// Created by vince on 20.03.2021.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <sched.h>

typedef unsigned long long measure;

static const measure tickMeasureTime = 85; // average time it take to call the getTicks() twice method
static const measure timeMeasureTime = 0; // average time it take to call the getTime() method

// I don't know what the fuck is going on here, 
// it just returns the actual processor ticks as measure and that makes me happy
measure getTicks() {
     unsigned a, d;
     asm( "cpuid" );
     asm volatile( "rdtsc" : "=a" ( a ), "=d" ( d ) );
     return ( ( (measure) a ) | ( ( (measure) d ) << 32 ) ); // returns the actual time in ticks
}

// Returns the actual milliseconds through gettimeofday()
measure getTime() {
    struct timespec currentTime;
    clock_gettime( CLOCK_MONOTONIC_RAW, &currentTime );
    return currentTime.tv_sec * 1000000000 + currentTime.tv_nsec; // returns the actual time in nanoseconds
}

// returns the time of a system call performing a 0-byte read operation
measure measureRead( bool ticks ) {
	measure start = ticks ? getTicks() : getTime(); // get local measure
	read( STDIN_FILENO, (char*) NULL, 0 ); // 0-byte read system call
	if( ticks )
		return getTicks() - start - tickMeasureTime; // return the difference between $start and the new local measure minus the average time it needs to call the getTicks() method
	else
		return getTime() - start - timeMeasureTime; // return the difference between $start and the new local measure minus the average time it needs to call the getTicks() method
}

// returns the average time it needs for a system call performing a 0-byte read operation $times times
// measured in ticks when $ticks = true or in milliseconds
measure readMeasurement( int times, bool ticks ) {
	int t = 0; // counter on how often the measure was summed onto $sum
	measure sum = 0; // sum measure until it is to big for an unsigned long long
	int r = 0; // counter on how often the average of sum was taken and summed onto $res
	measure res = 0; // sum average measures from $sum
	for( int i = 0; i < times; i++ ) { // measure $times times
		measure m = measureRead( ticks ); // take the measure
		if( ULLONG_MAX - sum < m ) { // if $sum + $m would be greater than the max value of an unsigned long long
			res = res + ( sum / t ); // sum the average of $sum onto $res
			r++; // increment counter $r
			t = 0; // reset $t
			sum = 0; // reset $sum
		}
		sum = sum + m; // sum measure onto $sum
		t++; // increment counter $t
	}
	if( sum > 0 ) { // sum the last average onto $res if $sum is not 0
		res = res + ( sum / t ); // sum the average of $sum onto $res
		r++; // increment counter $r
	}
	return res / r; // Return average of $res
}

// returns the average time it needs for a context switch
// measured in ticks when $ticks = true or in milliseconds
measure contextSwitchMeasurement( int times, bool ticks ) {
	int cPipe[2]; // writing pipe for child, reading pipe for parent
	int pPipe[2]; // writing pipe for parent, reading pipe for child
	if( pipe( cPipe ) == -1 || pipe( pPipe ) == -1 ) {
		fprintf( stderr, "Pipes failed!" );
		exit( 1 );
	}
	cpu_set_t mask; // information about the processor which is running this process
	sched_getaffinity( 0, sizeof( cpu_set_t ), &mask ); // fill the processor information
	int rc = fork();
	if( rc < 0 ) {
		fprintf( stderr, "Fork failed!" );
		exit( 1 );
	} else if( rc == 0 ) {
        // Child
		sched_setaffinity( 0, sizeof( cpu_set_t ), &mask ); // set the process to run on the processor described in $mask
		close( cPipe[0] ); // close reading end of child pipe
		close( pPipe[1] ); // close writing end of parent pipe
		char buffer[21]; // buffer for reading from pipe and parsing measures
		for( int i = -1; i < times; i++ ) { // do it $times times
			read( pPipe[0], buffer, sizeof( buffer ) ); // wait for input from parent pipe to read measure
			measure m;
			sscanf( buffer, "%llu", &m ); // parse read data into measure $m
			sprintf( buffer, "%llu", ( ticks ? getTicks() : getTime() ) - m ); // calculate parent measure minus local measure and parse it into $buffer
			write( cPipe[1], buffer, sizeof( buffer ) ); // write the difference of the two measures stored in $buffer into child pipe
		}
		close( cPipe[1] ); // close writing end of child pipe
		close( pPipe[0] ); // close reading end of parent pipe
		exit( 0 );
	} else if( rc > 0 ) {
        // Parent
        close( cPipe[1] ); // close writing end of child pipe
        close( pPipe[0] ); // close reading end of parent pipe

		int t = 0; // counter on how often the measure was summed onto $sum
		measure sum = 0; // sum measure until it is to big for an unsigned long long
		int r = 0; // counter on how often the average of sum was taken and summed onto $res
		measure res = 0; // sum average measures from $sum

		char buffer[21]; // buffer for writing to parent pipe and parsing measures from child pipe
		for( int i = -1; i < times; i++ ) { // do it $times times
			sprintf( buffer, "%llu", ticks ? getTicks() : getTime() ); // parsing local measure into buffer
			write( pPipe[1], buffer, sizeof( buffer ) ); // write buffer to parent pipe
			read( cPipe[0], buffer, sizeof( buffer ) ); // wait for input from child pipe to read measure result
			measure m; 
			sscanf( buffer, "%llu", &m ); // parse read measure result into $m
			if( i >= 0 ) { // only sum up if it wasn't the first loop, because of the initialization time of the child
				if( ULLONG_MAX - sum < m ) { // if $sum + $m would be greater than the max value of an unsigned long long
					res = res + ( sum / t ); // sum the average of $sum onto $res
					r++; // increment counter $r
					t = 0; // reset $t
					sum = 0; // reset $sum
				}
				sum = sum + m; // sum measure result onto $sum
				t++; // increment counter $t
			}
		}
		if( sum > 0 ) { // sum the last average onto $res if $sum is not 0
			res = res + ( sum / t ); // sum the average of $sum onto $res
			r++; // increment counter $r
		}
		close( cPipe[0] ); // close reading end of child pipe
		close( pPipe[1] ); // close writing end of parent pipe
        return res / r; // return average of $res
	}
	return 0;
}

// ./a1 [times] [-t]
int main( int argc, char *argv[] ) {
	if( argc < 2 ) {
		fprintf( stderr, "Too few arguments!\n" );
		exit( 1 );
	}
	int times; // the times it should measure and take the average of it
	bool ticks = false; // Wheter it should measure in processor ticks or in milliseconds
	for( int i = 1; i < argc; i++ ) { // loop through all arguments
		if( argv[i][0] == '-' ) { // if argument starts with '-'
			if( argv[i][1] == 't' ) { // if argument starts with '-t'
				ticks = true; // set measure in ticks true
			}
		} else { // if argument doesn't starts with '-'
			times = atoi( argv[i] ); // parse argument as integer into $times
			if( times == 0 ) { // error if argument couldn't be parsed as integer or times == 0
				fprintf( stderr, "Invalid times given!\n" );
				exit( 1 );
			}
		}
	}
	printf( "times: %d, ticks: %s\n", times, ticks ? "true" : "false" ); // print given arguments
	printf( "Read measurement: %llu\n", readMeasurement( times, ticks ) ); // prints the average time the system takes for a system call
	printf( "Context switch measurement: %llu\n", contextSwitchMeasurement( times, ticks ) ); // prints the average time the system takes for a context switch
	return 0;
}