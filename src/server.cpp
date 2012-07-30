#include <time.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include <map>
#include <vector>
#include "Birds.h"
#include "db.h"

//
// cl /EHsc server.cpp birds.cpp sqlite.cpp ws2_32.lib sqlite.lib
//

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


struct TronGame 
{
	struct Player 
	{
		int sock;
		int x,y;
		int alive;
		char way[600];
		char name[64];
		char from_player[600];
		char to_player[600];
		Player() 
			: sock(0)
			, x(0)
			, y(0)
			, alive(0)
		{
			reset(0);
		}
		void  reset(const char*mess)
		{
			alive = false;
			x = y = 0;
			if ( sock )
			{
				if ( mess ) Birds::Write(sock, (char*)mess, 0);
				Birds::Close(sock);
				sock=0;
			}
			memset(from_player,'P',600);
			memset(to_player,0,600);
			memset(way,0,600);
			memset(name,0,64);
		}
	} player[2];

	struct Board 
	{
		int w,h;
		char state[1024];

		Board() : w(10),h(10) {clear('.');}
		void clear(char c='.') {memset(state,c,1024);}

		char & at(int x, int y)
		{
			return state[y*w+x];
		}
		void print()
		{
			for ( int i=0; i<h; i++ )
			{
				for ( int j=0; j<w; j++ )
				{
					putc(state[i*w+j],stdout);
				}
				putc('\n',stdout);
			}
		}
	} board;
	
	int inval;
	int turn;
	int _id;
	
	TronGame() 
		: turn(-2)
		, inval(0)
	{
		static int __id=0;
		_id = __id++;
	}
	

	int id(int pl_sock)
	{
		if ( player[0].sock==pl_sock ) return 0;
		if ( player[1].sock==pl_sock ) return 1;
		return -1;
	}

	void join( int pl_sock, const char * pl_name )
	{
		if ( !(pl_name  && pl_name[0]) ) return;
		if ( id(pl_sock) !=-1 ) return;
		if ( id(0)       ==-1 ) return;
		if ( !strcmp(player[0].name,pl_name) ) return;
		if ( !strcmp(player[1].name,pl_name) ) return;
		int i = player[0].sock ? 1 : 0;
		player[i].sock=pl_sock;
		player[i].alive=1;
		strcpy(player[i].name,pl_name);
		turn ++;
	}

	void leave(int pl_sock)
	{
		if ( turn > 0 )
		{
			player[0].reset("# game cancelled\r\n");
			player[1].reset("# game cancelled\r\n");
			board.clear();
			turn = -2;
			return;
		}
		int i=id(pl_sock);
		if ( i != -1 )
		{
			player[i].reset(0);
			turn --;
		}
	}

	void from_player( int id, const char * mess )
	{
		strcpy( player[id].from_player,mess);
	}
	int move( int p, int dir, int bound )
	{
		return ((p + dir + bound) % bound);
	}
	bool move_player( int id )
	{
		if ( turn<0 ) return false;

		Player & p = player[id];
		char * mess = p.from_player;
		if ( !mess ) return false;

		bool moved = false;
		int fux=p.x, fuy=p.y;
		if ( mess[0] == 'N' )	fuy=move(p.y, -1, board.h); 
		if ( mess[0] == 'S' )	fuy=move(p.y,  1, board.h); 
		if ( mess[0] == 'E' )	fux=move(p.x,  1, board.w); 
		if ( mess[0] == 'W' )	fux=move(p.x, -1, board.w); 
		if ( fux != p.x || fuy != p.y )
		{
			char & futp = board.at(fux,fuy);
			if ( futp == '.' )
			{
				futp = id+'1';
				p.x = fux;
				p.y = fuy;
			} 
			else
			{
				p.alive = 0;
			}
			moved = true;
		}
		int l = strlen(p.way);
		if ( strchr("NESW",mess[0]) )
		{
			p.way[l] = mess[0];
		}
		else
		{
			p.way[l] = 'P';
			moved = false;
		}
			
		board.at(p.x,p.y) = id+'1';
		p.from_player[0]='P';
		return moved;
	}

	int eval()
	{
		if ( player[0].alive && player[1].alive )
		{
			int n0=0, n1=0;
			for ( int i=0; i<1024; i++ )
			{
				n0 += (strchr("WESN",player[0].way[i])!=0);
				n1 += (strchr("WESN",player[1].way[i])!=0);
			}
			player[0].alive = (n0 > n1);
			player[1].alive = (n1 > n0);
		}
		if ( player[0].alive && !player[1].alive )
			return 0;
		if ( !player[0].alive && player[1].alive )
			return 1;
		if ( !player[0].alive && !player[1].alive )
			return 2;
		return 3;
	}

	void game_over()
	{
		printf("~ game_%02d  %2d %2d : %2d %2d : %2d %2d : %2d %2d : %2d : %-8s vs %-8s\r\n", _id, board.w, board.h, player[0].x,player[1].x,player[0].y,player[1].y,player[0].alive,player[1].alive, turn, player[0].name, player[1].name);

		char mess[2][124];
		sprintf(mess[0],"# %s against %s\r\n", (player[0].alive?"won":"lost"), player[1].name);
		sprintf(mess[1],"# %s against %s\r\n", (player[1].alive?"won":"lost"), player[0].name);
		player[0].reset( mess[0] );
		player[1].reset( mess[1] );
		board.clear();
		turn = -2;
	}

	bool doTurn()
	{
		//if ( turn >= -1 && player[0].sock )
		//	printf("%d:%d] %s:%d %s:%d\n",turn,_id,player[0].name,player[0].alive,player[1].name,player[1].alive);

		if ( turn < 0 )
			return true;
		if ( turn == 0 )
		{
			inval = 0;
			board.h = 7 + rand()%10;
			board.w = 7 + rand()%10;
			do {
				player[0].x = rand() % (board.w-1);
				player[0].y = rand() % (board.h-1);
				player[1].x = rand() % (board.w-1);
				player[1].y = rand() % (board.h-1);
			} while ( (player[0].x==player[1].x) && (player[0].y==player[1].y) );

			board.at(player[0].x,player[0].y) = '1';
			board.at(player[1].x,player[1].y) = '2';

			sprintf(player[0].to_player, "# game with %s\r\n%d %d %d %d %d\r\n", player[1].name, turn, board.w, board.h, player[0].x,player[0].y );
			sprintf(player[1].to_player, "# game with %s\r\n%d %d %d %d %d\r\n", player[0].name, turn, board.w, board.h, player[1].x,player[1].y );
			
			printf("# game_%02d  %2d %2d : %2d %2d : %2d %2d :            : %-8s vs %-8s \r\n", _id, board.w, board.h, player[0].x,player[1].x,player[0].y,player[1].y, player[0].name, player[1].name);
		}
		else if ( turn > 0 )
		{
			bool l = move_player(0);
			bool r = move_player(1);
			if ( (!l) && (!r) )
				inval ++;
			else inval = 0;

			sprintf(player[0].to_player, "%d %d %d %d %d\r\n", turn, player[0].x, player[0].y, player[1].x, player[1].y );
			sprintf(player[1].to_player, "%d %d %d %d %d\r\n", turn, player[1].x, player[1].y, player[0].x, player[0].y );		
			//board.print();
		}
			

		Birds::Write( player[0].sock, player[0].to_player, 0 );
		Birds::Write( player[1].sock, player[1].to_player, 0 );

		turn ++;
		if ( (turn>99) || (!player[0].alive) || (!player[1].alive) || (inval>5) )
		{
			return false;
		}
		
		return true;
	}

	bool running() {return turn>=0;}
	
};



struct GameServer
{
	TronGame games[16];
	typedef std::map<int,TronGame*> ClientMap;
	typedef ClientMap::iterator ClientIter;
	typedef std::map<int,std::string> LobbyMap;
	typedef LobbyMap::iterator LobbyIter;
	ClientMap cli;
	LobbyMap lobby;
	Sqlite sqlite;
	int ticks;
	
	GameServer()
		: ticks(0)
	{
		sqlite.open( "tron1.sqlite", "","","");
		sqlite.query("create table player(name text, won numeric, games numeric, score numeric );");
		sqlite.query("create table replay(stamp text, p0 text, p1 text, w0 text, w1 text, a0 numeric, a1 numeric, turns numeric);");
	}

	void addReplay( TronGame::Player & l,  TronGame::Player & r, int t)
	{
		char sql[2048];
		sprintf(sql,"insert into replay values( \"%d\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d );", unsigned(time(0)),l.name, r.name,l.way,r.way,l.alive,r.alive, t);
		sqlite.query(sql);
	}
	void addGame(std::string name, bool won)
	{
		char sql[699];
		sprintf(sql,"select count(*) from player where name='%s';", name.c_str() );
		sqlite.query(sql);
		if ( sqlite.numRows()>0 )
		{
			if ( atoi(sqlite.getItem(0,0)) < 1 )
			{
				sprintf(sql,"insert into player values( \"%s\", %d, 1, 0.0 );", name.c_str(), won);
				sqlite.query(sql);
			} 
			else
			{
				if ( won )
					sprintf(sql,"update player set games=games+1, won=won+1, score=100*won/games where name='%s';", name.c_str() );
				else
					sprintf(sql,"update player set games=games+1, score=100*won/games where name='%s';", name.c_str() );
				sqlite.query(sql);
			}
		}		
	}

	TronGame * get( int sock )
	{
		ClientIter it = cli.find(sock);
		if ( it == cli.end() )
			return 0;
		return (it->second);
	}

	TronGame * nextFree( int sock )
	{
		//int r = rand() %16;
		for ( int i=0; i<16; i++ )
		{
			int j = i; //(i+r)%16;
			if ( ! games[j].running() )
			{
				TronGame *tron = &(games[j]);
				return tron;
			}
		}
		return 0;
	}

	void popLobby( TronGame * tron )
	{
		LobbyIter lob = lobby.begin();
		if ( lob == lobby.end() ) return;
		tron->join(lob->first,lob->second.c_str());
		cli[lob->first]=tron;
		lobby.erase(lob);
	}

	void addLobby( int pl_sock, const char * msg=0 )
	{
		if ( msg )
			lobby[pl_sock] = msg;

		if ( lobby.size() >= 2 )
		{
			TronGame * tron = nextFree(pl_sock);
			if ( tron )
			{
				popLobby(tron);
				popLobby(tron);
			}
		}
	}

	void remove(int sock)
	{
		ClientIter it = cli.find(sock);
		if ( it != cli.end() )
			cli.erase(it);
		LobbyIter li = lobby.find(sock);
		if ( li != lobby.end() )
			lobby.erase(li);
	}

	void doTurn()
	{
		int ngamesrun=0;
		for ( int i=0; i<16; i++ )
		{
			bool ok = games[i].doTurn();
			if ( ! ok )
			{
				int res = games[i].eval();
				addGame(games[i].player[0].name,games[i].player[0].alive!=0);
				addGame(games[i].player[1].name,games[i].player[1].alive!=0);
				addReplay(games[i].player[0],games[i].player[1], games[i].turn);
				games[i].game_over();
			}
			ngamesrun += games[i].running();
		}
		ticks ++;
		if ( ticks % 20 == 0 )
			printf("%-4d\t%2d games, %2d players, %2d lobby.\r\n", ticks, ngamesrun, cli.size(), lobby.size() );
	}
} server;


int callback(int sock,char * mess)
{
	
	//if ( sock!=-1) printf("%d:> %s\n", sock, mess );
	if ( !strncmp(mess,".idle",5) )
	{
		server.doTurn();
		
		return 1;
	}
	if ( !strncmp(mess,".join",5) )
	{
		Birds::Write( sock, "# welcome to the lobby.\r\n", 0 );
		return 1;
	}

	if ( sock < 0 ) return 0;
	TronGame * gp = server.get(sock);

	if ( !strncmp(mess,".left",5) )
	{
		if ( gp ) gp->leave(sock);
		server.remove(sock);
		return 1;
	}

	if ( gp )
	{
		TronGame & game = *gp;	
		int id = game.id(sock);
		if ( mess[0] == 'N' )	game.from_player(id,"N");
		else
		if ( mess[0] == 'E' )	game.from_player(id,"E");
		else
		if ( mess[0] == 'S' )	game.from_player(id,"S");
		else
		if ( mess[0] == 'W' )	game.from_player(id,"W");
		else
		if ( mess[0] == 'P' )	game.from_player(id,"P");

		return 1;
	}

	char * ms = trim(mess);
	if ( strlen(ms)>0 )
		server.addLobby( sock, ms );

	return 1;
}


int main( int argc, char **argv )
{
	int serv = Birds::Server( 4444 );

	Birds::Select( serv, 200000, callback );
    return 0;
}
