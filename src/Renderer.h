#ifndef __Renderer_onboard__
#define __Renderer_onboard__

// fwd
struct Renderer;



struct EventHandler
{
	virtual int idle( Renderer & renderer ) = 0;	
	virtual int key( int id, int state ) = 0;	
	virtual int mouse( int id, int state, int x, int y ) = 0;	
};



struct Renderer	
{
	virtual int setCaption( const char *txt ) = 0;	
	virtual int loadSurface( const char * name ) = 0;	
	virtual int setTransparency( int surf, int colorkey ) = 0;	
	virtual int getMillis() = 0;	

	virtual int drawRect( int color, int x, int y, int w, int h ) = 0;	
	virtual int drawSurface( int id, int x, int y, int orgx=0, int orxy=0, int orgw=0, int orgh=0 ) = 0;	
	virtual int update(int x=0, int y=0, int w=0, int h=0) = 0;	

	virtual int run( EventHandler & client ) = 0;	

	virtual ~Renderer() {}
};


#endif // __Renderer_onboard__

