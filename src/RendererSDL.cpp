#include "Renderer.h"
#include "SDL.h"    


#include <iostream> 

using std::cout;    
using std::cerr;    
using std::endl;    

#include <stdio.h>
#include <map>
using std::map;

class RendererSdl : public Renderer
{
	typedef map<int,SDL_Surface*> SurfMap;
	typedef SurfMap::iterator SurfIter;

	SurfMap surfs;
	SDL_Surface* screen;
	int _id;

	void clearSurfs()
	{
		for ( SurfIter it=surfs.begin(); it != surfs.end(); ++it )
		{
			SDL_Surface* s = it->second;
			if ( s ) 
			{
				SDL_FreeSurface( s );
			}
		}
		surfs.clear();
	}


public:
	RendererSdl( int scrWidth=400, int scrHeight=400, const char *iconpath="ico.bmp" )
		: screen(0)
		, _id(1)
	{
		if (SDL_Init(SDL_INIT_VIDEO)<0)	
		{
			cerr<< "Video initialization failed: " << SDL_GetError() << endl;;
			SDL_Quit();
			return;
		}
		if ( iconpath )
		{
			SDL_Surface *icon = SDL_LoadBMP(iconpath);
			SDL_WM_SetIcon(icon, NULL);
		}
		Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF;
		if (scrWidth==0) flags = flags|SDL_FULLSCREEN;

		screen = SDL_SetVideoMode(scrWidth, scrHeight, 32, flags);
		if (screen!=NULL) 
		{
			SDL_FillRect(screen,NULL,0);
		}
	}

	~RendererSdl()
	{
		clearSurfs();
	}

	int getMillis()
	{
		return SDL_GetTicks();
	}

	int drawRect( int color, int x, int y, int w, int h ) 
	{
		if ( ! screen )
			return 0;
		SDL_Rect rect = {x,y,w,h};
		SDL_FillRect(screen,&rect,color);
		return 1;
	}
	int drawSurface( int id, int x, int y, int orgx=0, int orgy=0, int orgw=0, int orgh=0 ) 
	{
		if ( ! screen )
			return 0;
		SDL_Surface *image = surfs[id];
		if ( ! image )
			return 0;
		SDL_Rect org = {orgx,orgy,orgw,orgh};
		SDL_Rect rd  = {x, y, orgw,orgh};
		if ( SDL_BlitSurface(image, (orgx||orgw?&org:NULL), screen, &rd) < 0 )
		{
			fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
			return 0;
		}
		return 1;
	}

	int update(int x=0, int y=0, int w=0, int h=0) 
	{
		SDL_UpdateRect(screen, x,y,w,h);
		return 1;
	}

	int setCaption( const char *txt ) 
	{
		SDL_WM_SetCaption(txt,0);
		return 1;
	}

	int setTransparency( int surf, int colorkey ) 
	{
		SDL_Surface * img = ( surf==0 ? screen : surfs[surf] );
		if ( ! img )
			return 0;
		int flag = SDL_SRCCOLORKEY;
		SDL_SetColorKey(img, flag, colorkey);
		return 1;
	}

	int loadSurface( const char * name )
	{
		SDL_Surface *image = SDL_LoadBMP(name);
		if (image == NULL) {
			fprintf(stderr, "Couldn't load %s: %s\n", name, SDL_GetError());
			return 0;
		}
		surfs[_id] = image;
		return _id++;
	}

	int run( EventHandler & client ) 
	{
		if ( ! screen )
			return 0;
		SDL_Event event;

		while ( true ) 
		{
			if ( SDL_PollEvent(&event) )
			{
				if (event.type==SDL_QUIT) 
					break;
				if (event.type==SDL_KEYDOWN||event.type==SDL_KEYUP) 
				{
					if (event.key.keysym.sym==SDLK_ESCAPE) 
						break;
					client.key( event.key.keysym.scancode, event.key.state );
				}
				if (event.type==SDL_MOUSEBUTTONDOWN || event.type==SDL_MOUSEMOTION) 
					client.mouse( event.button.button, event.button.state, event.button.x, event.button.y  );
			}
			if ( ! client.idle(*this) ) 
				break;;
		}
		return 1;
	}
};


Renderer * createRendererSdl(int scrWidth=400, int scrHeight=400, const char *iconpath="ico.bmp")
{
	return new RendererSdl(scrWidth, scrHeight, iconpath);
}
