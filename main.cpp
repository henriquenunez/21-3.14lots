#include "song.hpp"

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_AUDIO);

    Population a_pop(5);
    //a_pop.playPopulation();

    while(1)
    {
	a_pop.evaluate();
	a_pop.elitism();
    }

    return 0;
}

