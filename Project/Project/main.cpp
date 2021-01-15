#include <iostream>
#include <string>
#include <algorithm>
using namespace std;
#include "Header.h"
#include "jogo.h"


int main()
{
	// Usar o ficheiro do header

	Jogo game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	return 0;
}