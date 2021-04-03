
// Author: 	Stephan Meesters
// Date:	April 2, 2021
// Version: 1.0
//
// Implementation of Conway's Game of Life using C++ (STD11) and SDL
//
// Possible improvements: allocate std::vector<std::vector<T>> on heap,
//                        use multi-threading with std::shared_ptr
//

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <SDL.h>
#include <vector>
#include <tuple>
#include <cmath>
#include <random>
#include <string>
#include <cstdarg>

#define WINDOW_WIDTH_DEFAULT 800
#define WINDOW_HEIGHT_DEFAULT 600
#define GRID_WIDTH_DEFAULT 80
#define GRID_HEIGHT_DEFAULT 60
#define SPARSENESS_DEFAULT 2
#define FPS_DEFAULT 30

/* Conway's Game of Life process */
template <class T>
class Conway
{
public:
    
    typedef std::tuple<short, short> Coord;
    
    /* constructor */
    Conway(int width, int height):gridWidth(width), gridHeight(height)
    {
        // allocate std::vectors
        std::vector<std::vector<T>> grid1(width, std::vector<T>(height, 0));
        std::vector<std::vector<T>> grid2(width, std::vector<T>(height, 0));
        newGrid = std::move(grid1);
        oldGrid = std::move(grid2);

        // define neighbor search coordinates
        coords.push_back(std::make_tuple(0,1));
        coords.push_back(std::make_tuple(0,-1));
        coords.push_back(std::make_tuple(1,0));
        coords.push_back(std::make_tuple(-1,0));
        coords.push_back(std::make_tuple(1,1));
        coords.push_back(std::make_tuple(-1,-1));
        coords.push_back(std::make_tuple(1,-1));
        coords.push_back(std::make_tuple(-1,1));
    }
    ~Conway(){};
    
    /* initialize grid with random values */
    void randomInitialization(short sparseness)
    {
        auto gen = std::bind(std::uniform_int_distribution<>(0,sparseness),std::default_random_engine());
        for(int i = 0; i<gridWidth; i++)
        {
            for(int j = 0; j<gridHeight; j++)
            {
                short b = gen();
                if(b == 0)
                {
                    newGrid[i][j] = 1;
                    oldGrid[i][j] = 1;
                }
                else
                {
                    newGrid[i][j] = 0;
                    oldGrid[i][j] = 0;
                }
            }
        }
    }

    /* return a reference of the grid */
    std::vector< std::vector<T> >& fullGrid()
    {
        return newGrid;
    }
    
    /* verify that grid coordinates are within bounds */
    void verifyCoordBounds(Coord& cc)
    {
        // reflect coordinates if out of bounds
        short& x = std::get<0>(cc);
        short& y = std::get<1>(cc);
        
        if(x < 0)
        {
            x = gridWidth-1 + x;
        }
        else if(x >= gridWidth)
        {
            x = x % gridWidth;
        }
        if(y < 0)
        {
            y = gridHeight-1 + y;
        }
        else if(y >= gridHeight)
        {
            y = y % gridHeight;
        }
    }

    /* read a pixel in the grid */
    T readGrid(Coord& cc)
    {
        return oldGrid[std::get<0>(cc)][std::get<1>(cc)];
    }

    /* write a pixel in the grid */
    void writeGrid(Coord& cc, T val)
    {
        newGrid[std::get<0>(cc)][std::get<1>(cc)] = val;
    }

    /* main update loop */
    void update()
    {
        // swap old and new grids
        oldGrid.swap(newGrid);

        // update grid
        for(int i = 0; i<this->gridWidth; i++)
        {
            for(int j =0; j<this->gridHeight; j++)
            {
                Coord idx = std::make_tuple(i,j);
                short numLiveNeighbors = 0;
                T isAlive = readGrid(idx);

                // calculate number of neighbors
                for(auto const& cc: coords)
                {
                    Coord nIdx = std::make_tuple(i+std::get<0>(cc), j+std::get<1>(cc));
                    verifyCoordBounds(nIdx);    // verify neighbor coordinates
                                                // use mirroring of out of bounds
                    if(readGrid(nIdx) == 1)
                    {
                        numLiveNeighbors++;
                    }
                }
                
                // Algorithm:
                // Any live cell with two or three live neighbours survives.
                if(isAlive && (numLiveNeighbors == 2 || numLiveNeighbors == 3))
                {
                    writeGrid(idx,1);
                }
                
                // Any dead cell with three live neighbours becomes a live cell.
                else if(!isAlive && numLiveNeighbors == 3)
                {
                    writeGrid(idx,1);
                }
                
                // All other live cells die in the next generation.
                else if(isAlive)
                {
                    writeGrid(idx,0);
                }
                
                // Similarly, all other dead cells stay dead.
                else
                {
                    writeGrid(idx,0);
                }
            }
        }
    }
                          
private:

    std::vector<Coord> coords;
    std::vector< std::vector<T> > newGrid;
    std::vector< std::vector<T> > oldGrid;

    int gridWidth, gridHeight;
};

/* GUI class */
class GUI
{
public:
    
    enum CallbackType
    {
        CALLBACK_NOACTION,
        CALLBACK_QUIT,
        CALLBACK_RESET
    };

    /* create a new window of a certain size */
	static GUI* createWithDimensions(int width, int height)
	{
		GUI *pRet = new (std::nothrow) GUI();
		if (pRet && pRet->initWithSize(width, height)) {
			m_pInstance = pRet;
			return pRet;
		}
		else {
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
		return nullptr;
	}

    /* listen to quit of keyboard eventss */
	CallbackType pollEvents()
	{
        while( SDL_PollEvent( &event ) != 0 )
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    return CALLBACK_QUIT;
                    
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_r)
                    {
                        return CALLBACK_RESET;
                    }
                    break;
            }
        }
		return CALLBACK_NOACTION;
	}

    /* clear the screen */
	void clear()
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);	    
	}

    /* draw all pixels */
	template <class T>
	void drawGrid(std::vector< std::vector<T> >& grid, int width, int height)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		for(int i = 0; i < width; i++)
		{
			for(int j = 0; j<height; j++)
			{
				if(grid[i][j] == 1)
				{
					SDL_RenderDrawPoint(renderer, i, j);
				}
			}
		}
	}

    /* show the result to the screen */
	void present()
	{
		SDL_RenderPresent(renderer);
	}
    
    /* set the pixel scaling ratio's */
    void setScale(float scaleX, float scaleY)
    {
        SDL_RenderSetScale(renderer, scaleX, scaleY);
    }
    
    /* set the title of the window */
    void setWindowTitle(std::string title)
    {
        SDL_SetWindowTitle(window, title.c_str());
    }

    /* clean up */
	~GUI()
	{
		SDL_DestroyRenderer(renderer);
    	SDL_DestroyWindow(window);
    	SDL_Quit();
    	m_pInstance = nullptr;
	}

	GUI(GUI const&) = delete;             // Copy construct
	GUI(GUI&&) = delete;                  // Move construct
	GUI& operator=(GUI const&) = delete;  // Copy assign
	GUI& operator=(GUI &&) = delete;      // Move assign

private:

	GUI(){};

    /* initialize the window */
	bool initWithSize(int width, int height)
	{
		this->screenWidth = width;
		this->screenHeight = height;

		if(SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return false;
		}
		if(SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer) != 0)
		{
			SDL_Log("Unable to create window and renderer: %s", SDL_GetError());
			return false;
		}
   		return true;
	}

	static GUI* m_pInstance;    // singleton
	int screenWidth, screenHeight;

	SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
};
GUI* GUI::m_pInstance = nullptr;

/* String formatting function for std::string */
// Taken from:
// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/3742999
inline std::string format(const char* fmt, ...)
{
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

/* Main entry point */
int main(int argc, char *argv[])
{
    // default values
    int screenWidth = WINDOW_WIDTH_DEFAULT;
    int screenHeight = WINDOW_HEIGHT_DEFAULT;
    int gridWidth = GRID_WIDTH_DEFAULT;
    int gridHeight = GRID_HEIGHT_DEFAULT;
    short sparseness = SPARSENESS_DEFAULT;
    short fps = FPS_DEFAULT;
    
	// parse arguments, adjust values
    if(argc == 7)
    {
        fps = atoi(argv[6]);
    }
    if(argc >= 6)
    {
        sparseness = atoi(argv[5]);
    }
    if(argc >= 5)
    {
        gridWidth = atoi(argv[3]);
        gridHeight = atoi(argv[4]);
    }
    if(argc >= 3)
    {
        screenWidth = atoi(argv[1]);
        screenHeight = atoi(argv[2]);
    }
    else
    {
        printf("usage: Conway [window width] [window height] [grid width] [grid height] [sparseness] [fps]\n");
        printf("e.g.: Conway %d %d %d %d %d %d\n", WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT, GRID_WIDTH_DEFAULT, GRID_HEIGHT_DEFAULT, SPARSENESS_DEFAULT, FPS_DEFAULT);
    }

	// create GUI object as singleton
	auto screen = GUI::createWithDimensions(screenWidth, screenHeight);
	if(screen == nullptr)
	{
		return 0;
	}
    screen->setScale(screenWidth/gridWidth, screenHeight/gridHeight);

	// create Conway Game of Life process
    Conway<char> conway(gridWidth, gridHeight);
    conway.randomInitialization(sparseness);
    
    // loop
    int frame_time = std::round((1.0 / (float)fps)*1000);
    uint32_t startTime, currTime;
    std::vector<float> elapsedTimes(5, 0.0);
    while(1)
    {
        switch(screen->pollEvents())
        {
            // is close window pressed?
            case GUI::CALLBACK_QUIT:
                goto exitapp;
                break;
            // is the mouse pressed?
            case GUI::CALLBACK_RESET:
                conway.randomInitialization(sparseness); // reset the game
                break;
            // no action
            case GUI::CALLBACK_NOACTION:
                break;
        }
        
        // update the Conway way of life, measure the execution time
        startTime = SDL_GetTicks();
    	conway.update();
        currTime = SDL_GetTicks();
        std::rotate(elapsedTimes.rbegin(), elapsedTimes.rbegin() + 1, elapsedTimes.rend());
        elapsedTimes[0] = currTime - startTime;
        
        // update the visuals
    	screen->clear();
    	screen->drawGrid(conway.fullGrid(), gridWidth, gridHeight);
    	screen->present();
        screen->setWindowTitle(format("Conway's Game of Life. Press R to reset. Average computation time: %.1f ms",std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0)/5.0));
        
        // delay until next loop
        SDL_Delay(frame_time);
    }
    
exitapp:
    
    // clean up
    delete screen;
    
    // finish
    return EXIT_SUCCESS;
}
