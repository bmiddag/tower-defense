// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; main.c

#include <stdlib.h>
#include <crtdbg.h>

#include "config.h"
#include "test_functions.h"
#include "game.h"

int main(int argc, char **argv)
{
	/*_CrtSetBreakAlloc(x); // Use this to seek your reported memory leaks!! */
	{
		/* Variables */
		GameState state;

		/* Initialize GameState */
		init_game_state(&state);

		if (RUN_TEST) {
			run_test_loop(&state);
		}
		else {
			run_game_loop(&state);
		}

		/* Destroy GameState */
		destroy_game_state(&state);
	}

	_CrtDumpMemoryLeaks();
	return 0;

}

