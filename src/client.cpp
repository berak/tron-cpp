#include <stdio.h>
#include <string.h>
#include "Birds.h"
#include "Renderer.h"


#include <iostream> 

using std::cout;    
using std::cerr;    
using std::endl;    


char *trim(char *s)
{
	int l = strlen(s);
	char *end = &s[l-1];
	while( *end=='\r' || *end=='\n' )
	{
		*end = 0;
		end --;
	}
	return s;
}

//
// cl client.cpp birds.cpp RendererSdl.cpp /EHsc /Iinclude ws2_32.lib SDL.lib
//
struct Game : EventHandler
{
	int pic1, pic2, font,dead;
	int mx,my;
	int sock;
	int cmd;
	int w,h;
	int iLost;
	int oLost;
	int frame,turn,ax,ay,bx,by;
	const char * name;
	char state[1024];
	
	Game(const char * name = "trooon") 
		: pic1(0),pic2(0),dead(0)
		, mx(0),my(0)
		, sock(0)
		, cmd(0)
		, w(10)
		, h(10)
		, frame(0)
		, turn(-1)
		, ax(0)
		, ay(0)
		, bx(0)
		, by(1)
		, name(name)
	{
		iLost=0;
		oLost=0;
		memset(state,'.',1024);
	}

	void drawText(const char * txt, int x, int y, Renderer & renderer)
	{
		int lw = 16;
		int n = strlen(txt);
		for ( int i=0; i<n; i++ )
		{
			int ox = (int(txt[i])%16) * 16;
			int oy = (int(txt[i])/16) * 16;
			renderer.drawSurface( font, x,y, ox,oy,16,16 );
			x += 10;
		}
	}

	inline int min(int a, int b ) 
	{
		return a<b?a:b;
	}
	virtual int idle(Renderer & renderer)
	{	
		if ( sock )
		{
			if ( Birds::Select(sock,100000) > 0 )
			{
				char * m = Birds::Read(sock);
				if ( ! m )
				{
					Birds::Close(sock);
					sock = 0;
				} else {
					cerr << m;
					while ( *m )
					{
						if ( m[0] == '#')
						{
							drawText(trim(m+2),60,300,renderer);
							if ( !strncmp(m,"# game",6) )
							{
								iLost=oLost=0;
								renderer.setCaption(m+1);
							}
							if ( !strncmp(m,"# won",5) )
							{
								oLost=1;
							}
							if ( !strncmp(m,"# lost",6) )
							{
								iLost=1;
							}
						}
						else
						if ( sscanf(m, "%d %d %d %d %d",&turn,&ax,&ay,&bx,&by) == 5 )
						{
							if ( turn == 0 )
							{
								w = ax;
								h = ay;
								ax = bx;
								ay = by;
							}
							cerr << m;
						}
						char *e = strchr(m,'\n');
						if ( e && e>m )
						{
							m = e;
						}
						else 
							break;
					}
				}
			}
		}
		if ( frame == 0 )
		{
			renderer.setCaption(name);

			font = renderer.loadSurface("mv_boli.bmp");
			//renderer.setTransparency(font,0);

			dead = renderer.loadSurface("dead.bmp");
			renderer.setTransparency(dead,0);

			pic1 = renderer.loadSurface("p1.bmp");
			renderer.setTransparency(pic1,0xff00ff);

			pic2 = renderer.loadSurface("p2.bmp");
			renderer.setTransparency(pic2,0);
		}

		if ( turn <= 0 )
		{
			// clear
			renderer.drawRect(0, 0,0, -1,-1);
		}
		else
		{
			int tile = min(400/w,400/h);
			if ( turn == 1 )
			{
				bool on = 0;
				int c[2] = {0,0x333333};
				for ( int j=0; j<h; j++) {
					for ( int i=0; i<w; i++) {
						renderer.drawRect(c[on],i*tile,j*tile, tile,tile);
						on = ! on;
					}
					if ( w%2==0 ) on = ! on;
				}
			}
			renderer.drawSurface( pic1, ax*tile, ay*tile, 0,0, min(40,tile), min(40,tile) );
			if(iLost) renderer.drawSurface( dead, ax*tile, ay*tile );
			renderer.drawSurface( pic2, bx*tile, by*tile, 0,0, min(40,tile), min(40,tile) );
			if(oLost) renderer.drawSurface( dead, bx*tile, by*tile );
		}
		
		renderer.update();

		frame ++;

		return 1;
	}

	virtual int key( int id, int state ) 	
	{
		if ( id==46 && state==1 && sock==0 ) {
			sock = Birds::Client("localhost",4444);
			if ( sock==-1 ) return 1; // connection fail
			char msg[200];
			sprintf(msg, "%s\r\n",name);
			Birds::Write(sock, msg, 0);
			iLost=0;
			oLost=0;
			turn = -1;
		}
		if ( sock>0 && state==1 )
		{
			if ( id==17 || id==72 ) Birds::Write(sock, "N\r\n", 0);
			if ( id==30 || id==75 ) Birds::Write(sock, "W\r\n", 0);
			if ( id==31 || id==80 ) Birds::Write(sock, "S\r\n", 0);
			if ( id==32 || id==77 ) Birds::Write(sock, "E\r\n", 0);
		}
		cerr << "key " << id << " " << state << endl;
		return 1;
	}
	virtual int mouse( int id, int state, int x, int y )
	{
		mx=x;
		my=y;
		//cerr << "mouse " << id << " " << state << " " << x << " " << y << endl;
		return 1;
	}
};

extern Renderer * createRendererSdl(int scrWidth=400, int scrHeight=400, const char *iconpath="ico.bmp");


//#include <windows.h>
//int WINAPI WinMain(HINSTANCE hinst, HINSTANCE previnst, LPSTR cmdline, int cmdshow) {
int main(int argc, char **argv) {

	char *name = "troooon";
	if ( argc>1 ) name = argv[1];
	Game game(name);
	Renderer * renderer = createRendererSdl(400,400);
	renderer->run( game );

	delete renderer;
	return 0;
}
