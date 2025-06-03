#include "SDL.h"
#include "src/Core/Engine.h"

int main(int argc, char** argv) {
  SDL_Log("Starting Game...");

  bool initSuccess = Engine::GetInstance()->Init();
  SDL_Log("Init returned: %d", initSuccess);

  if (!initSuccess) {
    SDL_Log("Engine initialization failed!");
    return -1;
  }

  SDL_Log("Entering Main Loop...");
  while (Engine::GetInstance()->IsRunning()) {
    SDL_Log("Loop iteration...");
    Engine::GetInstance()->Events();
    Engine::GetInstance()->Update();
    Engine::GetInstance()->Render();
  }
  SDL_Log("Exited Main Loop.");

  Engine::GetInstance()->Clean();
  SDL_Log("Game Shut Down Gracefully.");
  return 0;
}
 