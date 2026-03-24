#include <game.h>

int main()
{
	//create and run game class that holds the setup of the game that class derives form my engine class
	Game game(1200, 800, "AstroidAvoidance");
	game.Run();
}