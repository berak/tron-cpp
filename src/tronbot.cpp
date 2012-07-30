#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>  // for sleep ;(
#include "Birds.h"

int main( int argc, char **argv )
{
	srand(unsigned(time(0)));
	while ( true )
	{
		char * name = "b1rd13";
		int sock = Birds::Client("localhost",4444);
		if ( sock==-1 ) continue; // connection fail
		if ( argc>1 )
			name = argv[1];
		Birds::Write(sock, name, 0);
		Birds::Write(sock, "\r\n", 0);
		int w=10,h=10;
		char state[1024];
		memset(state,'.',1024);

		while(true) 
		{
			char *m = Birds::Read(sock);
			if ( ! m )
			{
				printf(".");
				Birds::Close(sock);
				int slp = rand() % 15000;
				Sleep(slp);
				printf("%d.\n",slp);
				break;
			}
			printf(m);
			int turn=-1,ax=0,ay=0,bx=0,by=0;
			if ( sscanf(m, "%d %d %d %d %d",&turn,&ax,&ay,&bx,&by) == 5 )
			{
				if ( turn == 0 )
				{
					w = ax;
					h = ay;
					ax = bx;
					ay = by;
					continue;
				}
				state[ax+ay*w] = '1';
				state[bx+by*w] = '2';
				int n[4] = {0};
				for ( int a=1; a<w; a++ )
					if ( state[((ax+a)%w)+ay*w] == '.' )	n[0] += 1;
					else break;
				for ( int a=1; a<w; a++ )
					if ( state[((ax-a+w)%w)+ay*w] == '.' )	n[1] += 1;
					else break;
				for ( int a=1; a<h; a++ )
					if ( state[ax+((ay+a)%h)*w] == '.' )	n[2] += 1; 
					else break;
				for ( int a=1; a<h; a++ )
					if ( state[ax+((ay-a+h)%h)*w] == '.' )	n[3] += 1;
					else break;
				char msg[4] = "_\r\n";
				int bn = 0;
				int bi = -1;
				for ( int i=0; i<4; i++ )
				{
					if ( bn < n[i] )
					{
						bn = n[i];
						bi =   i;
					}
				}
				const char * DIRS="EWSN";
				if ( bi == -1 )
					msg[0] = 'P';
				else
					msg[0] = DIRS[bi];

				Birds::Write(sock, msg, 0);
			}
		}
	}
	return 0;
}