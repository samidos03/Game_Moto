#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "../Audio/AudioManager.h"
#include "../Graphics/TextureManager.h"
#include "../Menu/MainMenu.h"
#include "../Objects/Player.h" 
#include "Engine.h"

Engine* Engine::s_Instance = nullptr;

void Engine::ApplyMasterVolume() {
  if (m_isMuted) {
    AudioManager::GetInstance()->SetMusicVolume(0);
    AudioManager::GetInstance()->SetAllSoundsVolume(0);
  } else {
    AudioManager::GetInstance()->SetMusicVolume(m_currentMasterVolume);
    AudioManager::GetInstance()->SetAllSoundsVolume(m_currentMasterVolume);
  }
}

void Engine::IncreaseVolume() {
  if (m_isMuted) {
    ToggleMute();
  }
  m_currentMasterVolume += VOLUME_STEP;
  if (m_currentMasterVolume > VOLUME_MAX) {
    m_currentMasterVolume = VOLUME_MAX;
  }
  SDL_Log("Volume Increased to %d", m_currentMasterVolume);
  ApplyMasterVolume();
}

void Engine::DecreaseVolume() {
  if (m_isMuted) {
    ToggleMute();
  }
  m_currentMasterVolume -= VOLUME_STEP;
  if (m_currentMasterVolume < 0) {
    m_currentMasterVolume = 0;
  }
  SDL_Log("Volume Decreased to %d", m_currentMasterVolume);
  ApplyMasterVolume();
}

void Engine::ToggleMute() {
  m_isMuted = !m_isMuted;
  if (m_isMuted) {
    m_volumeBeforeMute = m_currentMasterVolume;
    m_currentMasterVolume = 0;
    SDL_Log("Volume Muted");
  } else {
    m_currentMasterVolume = m_volumeBeforeMute;
    SDL_Log("Volume Unmuted to %d", m_currentMasterVolume);
  }
  ApplyMasterVolume();
}

bool Engine::Init() {
  SDL_Log("Engine::Init() - Initialisation du moteur...");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Engine::Init() - ECHEC: Initialisation SDL: %s", SDL_GetError());
    return false;
  }
  SDL_Log("Engine::Init() - SDL initialise.");

  if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
    SDL_Log("Engine::Init() - ECHEC: Initialisation SDL_image: %s", IMG_GetError());
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - SDL_image initialise.");

  if (TTF_Init() == -1) {
    SDL_Log("Engine::Init() - ECHEC: Initialisation SDL_ttf: %s", TTF_GetError());
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - SDL_ttf initialise.");

  if (!AudioManager::GetInstance()->Init()) {
    SDL_Log("Engine::Init() - ECHEC: Initialisation AudioManager!"); 
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - AudioManager initialise.");

  m_Window = SDL_CreateWindow("Moto Game",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); 
  
  if (m_Window == nullptr) {
    SDL_Log("Engine::Init() - ECHEC: Creation de la fenetre: %s", SDL_GetError());
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - Fenetre creee.");

  m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (m_Renderer == nullptr) {
    SDL_Log("Engine::Init() - ECHEC: Creation du renderer: %s", SDL_GetError());
    SDL_DestroyWindow(m_Window);
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - Renderer cree.");
  SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND); 

  SDL_Log("Engine::Init() - Initialisation de MainMenu...");
  if (!MainMenu::GetInstance()->Init()) { // Ceci est l'appel qui échouait initialement
    SDL_Log("Engine::Init() - ERREUR CRITIQUE: Echec de l'initialisation du menu principal. Verifiez les logs de MainMenu::Init().");
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false; 
  }
  SDL_Log("Engine::Init() - MainMenu initialise avec succes.");

  SDL_Log("Engine::Init() - Chargement des textures globales...");
  if (!TextureManager::GetInstance()->Load("background", "assets/Background1.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'background'"); return false; 
  }
  if (!TextureManager::GetInstance()->Load("track", "assets/Track.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'track'"); return false;
  }
  if (!TextureManager::GetInstance()->Load("player", "assets/player_bike.png")) { // Utilisé par Player::load
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'player'"); return false;
  }
  if (!TextureManager::GetInstance()->Load("start", "assets/timer/start.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'start'"); return false;
  }
  m_timerTextures.push_back("start");
  for (int i = 0; i <= 60; i++) {
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << i;
    std::string timerName = ss.str();
    std::string path = "assets/timer/" + timerName + ".png";
    if (!TextureManager::GetInstance()->Load(timerName, path)) {
      SDL_Log("Engine::Init() - ECHEC: Chargement texture timer '%s'", timerName.c_str()); return false;
    }
    m_timerTextures.push_back(timerName);
  }
  if (!TextureManager::GetInstance()->Load("end", "assets/timer/end.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'end'"); return false;
  }
  m_timerTextures.push_back("end");
  if (!TextureManager::GetInstance()->Load("gameover", "assets/game_over.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'gameover'"); return false;
  }
  if (!TextureManager::GetInstance()->Load("win", "assets/win.png")) {
     SDL_Log("Engine::Init() - ECHEC: Chargement texture 'win'"); return false;
  }
  SDL_Log("Engine::Init() - Textures globales chargees.");

  SDL_Log("Engine::Init() - Chargement des textures d'obstacles...");
  m_obstacleTextureIds.clear();
  std::vector<std::pair<std::string, std::string>> obstaclesToLoad = {
      {"obstacle1", "assets/obstacle1.png"}, {"obstacle2", "assets/obstacle2.png"},
      {"obstacle3", "assets/obstacle3.png"}, {"obstacle4", "assets/obstacle4.png"}
  };
  bool firstObstacleLoaded = false;
  for (const auto& obsData : obstaclesToLoad) {
    if (!TextureManager::GetInstance()->Load(obsData.first, obsData.second)) {
        SDL_Log("Engine::Init() - Attention: Echec chargement texture obstacle '%s'. On continue si d'autres se chargent.", obsData.first.c_str());
        continue; 
    }
    m_obstacleTextureIds.push_back(obsData.first);
    if (!firstObstacleLoaded) {
      // Récupérer la largeur/hauteur du premier obstacle chargé pour dimensionner les autres (supposition)
      TextureManager::GetInstance()->QueryTexture(obsData.first, &m_obstacleTextureWidth, &m_obstacleTextureHeight);
      if (m_obstacleTextureWidth > 0) firstObstacleLoaded = true;
    }
  }
  if (m_obstacleTextureIds.empty() || !firstObstacleLoaded) {
    SDL_Log("Engine::Init() - ERREUR CRITIQUE: Aucune texture d'obstacle n'a pu etre chargee ou dimensions invalides.");
    TextureManager::GetInstance()->Clean(); 
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - Textures d'obstacles chargees. %zu types disponibles.", m_obstacleTextureIds.size());


  SDL_Log("Engine::Init() - Chargement des ressources audio...");
  if (!AudioManager::GetInstance()->LoadMusic("menu_music", "assets/audio/menu_theme.ogg")) { SDL_Log("Engine::Init() - Echec chargement musique 'menu_music'"); }
  if (!AudioManager::GetInstance()->LoadMusic("game_music", "assets/audio/game_loop.ogg")) { SDL_Log("Engine::Init() - Echec chargement musique 'game_music'"); }
  if (!AudioManager::GetInstance()->LoadSound("click", "assets/audio/button_click.wav")) { SDL_Log("Engine::Init() - Echec chargement son 'click'"); }
  if (!AudioManager::GetInstance()->LoadSound("crash", "assets/audio/player_crash.wav")) { SDL_Log("Engine::Init() - Echec chargement son 'crash'"); }
  if (!AudioManager::GetInstance()->LoadSound("win", "assets/audio/level_win.wav")) { SDL_Log("Engine::Init() - Echec chargement son 'win'"); }
  if (!AudioManager::GetInstance()->LoadSound("lose", "assets/audio/game_over.wav")) { SDL_Log("Engine::Init() - Echec chargement son 'lose'"); }
  if (!AudioManager::GetInstance()->LoadSound("countdown", "assets/audio/timer_tick.wav")) { SDL_Log("Engine::Init() - Echec chargement son 'countdown'"); }
  SDL_Log("Engine::Init() - Ressources audio chargees (ou tentatives effectuees).");

  SDL_Log("Engine::Init() - Chargement de la police UI...");
  m_uiFont = TTF_OpenFont("assets/Swansea-q3pd.ttf", 24); 
  if (m_uiFont == nullptr) {
    SDL_Log("Engine::Init() - ECHEC: Chargement police UI 'assets/Swansea-q3pd.ttf': %s", TTF_GetError());
    TextureManager::GetInstance()->Clean();
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - Police UI chargee.");

  SDL_Log("Engine::Init() - Initialisation du joueur et des positions des voies...");
  float laneHeight = TRACK_HEIGHT / 3.0f;
  m_laneYPositions.clear(); // S'assurer qu'il est vide avant de remplir
  // m_laneYPositions stockera le Y du CENTRE de chaque voie.
  m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight * 0.5f);
  m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight * 1.5f);
  m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight * 2.5f);
  
  m_Player = new Player(); 
  float playerStartX = 150.0f;
  if (!m_Player || !m_Player->load("player", playerStartX, m_laneYPositions)) {
    SDL_Log("Engine::Init() - ECHEC: Initialisation du joueur.");
    if (m_Player) delete m_Player;
    m_Player = nullptr;
    TTF_CloseFont(m_uiFont);
    TextureManager::GetInstance()->Clean();
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    AudioManager::GetInstance()->Clean();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Log("Engine::Init() - Joueur initialise.");

  SDL_Log("Engine::Init() - Creation du message 'Retour au menu'...");
  SDL_Color textColor = { 0, 0, 0, 255 }; 
  SDL_Surface* surface = TTF_RenderText_Blended(m_uiFont, "Appuyez sur Echap pour retourner au menu", textColor); 
  if (surface) {
    m_returnPromptTexture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    if (m_returnPromptTexture) {
      m_returnPromptRect.w = surface->w;
      m_returnPromptRect.h = surface->h;
      m_returnPromptRect.x = (SCREEN_WIDTH - m_returnPromptRect.w) / 2;
      m_returnPromptRect.y = SCREEN_HEIGHT - m_returnPromptRect.h - 20; 
    } else {
      SDL_Log("Engine::Init() - Attention: Echec creation texture 'Retour au menu': %s", SDL_GetError());
    }
    SDL_FreeSurface(surface);
  } else {
    SDL_Log("Engine::Init() - Attention: Echec creation surface 'Retour au menu': %s", TTF_GetError());
  }
  SDL_Log("Engine::Init() - Message 'Retour au menu' cree (ou tentative effectuee).");

  m_lastTick = SDL_GetTicks();
  m_deltaTime = 0.0f;
  m_BackgroundScrollX = 0.0f;
  m_IsRunning = true;
  m_remainingSeconds = 60; 
  m_lastSecondUpdate = SDL_GetTicks();
  // m_gameOverStartTime est initialisé dans SetGameState
  m_showGameOverScreen = false;
  m_totalDistanceTraveled = 0.0f;
  m_timeSinceLastSpawn = 1.0f; 
  m_obstacleSpawnInterval = 2.3f; 
  m_lastDifficultyIncreaseTime = SDL_GetTicks();
  m_lastMaxSpeedIncreaseTime = SDL_GetTicks();
  m_distanceTexture = nullptr;
  m_lastDisplayedDistance = -1;
  m_lastCountdownSecondPlayed = -1;
  m_showReturnPrompt = false;
  // m_endScreenStartTime est initialisé dans SetGameState

  ApplyMasterVolume();
  SetGameState(STATE_MAIN_MENU); 

  SDL_Log("Engine::Init() - Initialisation du moteur terminee avec succes !");
  return true;
}

void Engine::SetGameState(GameState newState) {
  if (m_gameState == newState) return;

  GameState oldState = m_gameState;
  m_gameState = newState;
  SDL_Log("Engine::SetGameState - Changement d'etat de %d a %d", oldState, newState);
  m_showReturnPrompt = false;

  switch (newState) {
    case STATE_MAIN_MENU:
      if (oldState == STATE_GAME_OVER || oldState == STATE_WIN || oldState == STATE_PLAYING || oldState == STATE_START_SCREEN) {
          AudioManager::GetInstance()->StopMusic(); // S'assurer que la musique du jeu est arrêtée
          AudioManager::GetInstance()->PlayMusic("menu_music", -1); // Puis jouer celle du menu
          
          m_obstacles.clear();
          m_totalDistanceTraveled = 0.0f;
          if (m_Player) {
              if (m_laneYPositions.empty()) { // Sécurité si SetGameState est appelé avant que Init ait tout rempli
                   float laneHeight_temp = TRACK_HEIGHT / 3.0f;
                   m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight_temp * 0.5f);
                   m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight_temp * 1.5f);
                   m_laneYPositions.push_back(TRACK_Y_POSITION + laneHeight_temp * 2.5f);
              }
              m_Player->reset(150.0f, m_laneYPositions);
          }
      } else if (!Mix_PlayingMusic()){ 
         AudioManager::GetInstance()->PlayMusic("menu_music", -1);
      }
      break;
    case STATE_ABOUT:
      // Si on vient du jeu, arrêter la musique du jeu et jouer celle du menu
      if (oldState == STATE_PLAYING || oldState == STATE_START_SCREEN) {
          AudioManager::GetInstance()->StopMusic();
          AudioManager::GetInstance()->PlayMusic("menu_music", -1);
      } else if (!Mix_PlayingMusic()) { // Si on vient d'un autre état (ex: menu principal) et que rien ne joue
          AudioManager::GetInstance()->PlayMusic("menu_music", -1);
      }
      break;
    case STATE_START_SCREEN: 
      AudioManager::GetInstance()->StopMusic();
      m_remainingSeconds = 60;
      // m_gameOverStartTime = 0; // Déjà initialisé
      m_showGameOverScreen = false;
      m_obstacles.clear();
      m_timeSinceLastSpawn = 1.0f;
      m_obstacleSpawnInterval = 2.3f; 
      m_lastDifficultyIncreaseTime = SDL_GetTicks();
      m_lastMaxSpeedIncreaseTime = SDL_GetTicks();
      m_totalDistanceTraveled = 0.0f;
      m_lastDisplayedDistance = -1;
      if (m_distanceTexture) {
        SDL_DestroyTexture(m_distanceTexture);
        m_distanceTexture = nullptr;
      }
      if (m_Player) {
        m_Player->reset(150.0f, m_laneYPositions); 
      }
      m_lastCountdownSecondPlayed = -1; 
      SDL_Log("Engine::SetGameState - Etat du jeu reinitialise pour STATE_START_SCREEN.");
      break;
    case STATE_PLAYING:
      AudioManager::GetInstance()->StopMusic(); // S'assurer que toute musique précédente est arrêtée
      AudioManager::GetInstance()->PlayMusic("game_music", -1);
      m_lastSecondUpdate = SDL_GetTicks(); 
      m_lastCountdownSecondPlayed = -1;
      SDL_Log("Engine::SetGameState - Passage a STATE_PLAYING, musique de jeu lancee.");
      break;
    case STATE_GAME_OVER:
      AudioManager::GetInstance()->StopMusic();
      AudioManager::GetInstance()->PlaySound("lose", 0);
      m_gameOverStartTime = SDL_GetTicks(); // Pour le délai avant d'afficher l'écran gameover
      m_endScreenStartTime = SDL_GetTicks(); // Pour le délai d'affichage du message "Retour"
      SDL_Log("Engine::SetGameState - Passage a STATE_GAME_OVER.");
      break;
    case STATE_WIN:
      AudioManager::GetInstance()->StopMusic();
      AudioManager::GetInstance()->PlaySound("win", 0);
      m_endScreenStartTime = SDL_GetTicks();
      SDL_Log("Engine::SetGameState - Passage a STATE_WIN.");
      break;
  }
}

void Engine::SpawnObstacle() {
  int activeCount = 0;
  for (const auto& obs : m_obstacles) {
      if (obs.isActive) ++activeCount;
  }
  
  const int maxOnScreenObstacles = 2; 
  if (activeCount >= maxOnScreenObstacles) {
      return;
  }

  if (m_obstacleTextureIds.empty()) {
      SDL_Log("Engine::SpawnObstacle - Aucune texture d'obstacle disponible pour le spawn.");
      return;
  }
  if (m_laneYPositions.empty()) {
      SDL_Log("Engine::SpawnObstacle - Aucune position de voie disponible.");
      return;
  }


  int remainingSlots = maxOnScreenObstacles - activeCount;
  std::vector<int> availableLaneIndices(m_laneYPositions.size());
  std::iota(availableLaneIndices.begin(), availableLaneIndices.end(), 0); 

  if (availableLaneIndices.empty()) {
      SDL_Log("Engine::SpawnObstacle - Aucune voie disponible pour spawner.");
      return;
  }

  std::uniform_int_distribution<int> chanceDist(1, 100);
  bool spawnTwo = (availableLaneIndices.size() >= 2) && (chanceDist(m_rng) <= m_doubleSpawnChance);
  int numToSpawn = spawnTwo ? 2 : 1;
  numToSpawn = std::min(numToSpawn, remainingSlots);
  numToSpawn = std::min(numToSpawn, static_cast<int>(availableLaneIndices.size()));


  std::uniform_int_distribution<int> textureDist(0, static_cast<int>(m_obstacleTextureIds.size()) - 1);

  for (int i = 0; i < numToSpawn; ++i) {
      if (availableLaneIndices.empty()) break; 

      std::uniform_int_distribution<int> laneDist(0, static_cast<int>(availableLaneIndices.size()) - 1);
      int randomLaneListIndex = laneDist(m_rng);
      int actualLaneIndex = availableLaneIndices[randomLaneListIndex];

      Obstacle newObs;
      newObs.textureId = m_obstacleTextureIds[textureDist(m_rng)];
      
      newObs.collider.w = m_obstacleTextureWidth; 
      newObs.collider.h = m_obstacleTextureHeight;
      
      newObs.collider.x = SCREEN_WIDTH + 50; 
      newObs.collider.y = static_cast<int>(m_laneYPositions[actualLaneIndex] - (newObs.collider.h / 2.0f));
      newObs.isActive = true;
      m_obstacles.push_back(newObs);
      SDL_Log("Engine::SpawnObstacle - Obstacle '%s' spawne sur la voie %d (x:%d, y:%d)", newObs.textureId.c_str(), actualLaneIndex, newObs.collider.x, newObs.collider.y);

      availableLaneIndices.erase(availableLaneIndices.begin() + randomLaneListIndex); 
  }
}

void Engine::Update() {
  Uint32 currentTick = SDL_GetTicks();
  m_deltaTime = (currentTick - m_lastTick) / 1000.0f;
  m_lastTick = currentTick;
  if (m_deltaTime > 0.05f) m_deltaTime = 0.05f; 

  if (m_gameState == STATE_MAIN_MENU) {
    MainMenu::GetInstance()->Update(m_deltaTime); 
  } else if (m_gameState == STATE_ABOUT) {
    // Rien à faire ici, géré par Events
  } else if (m_gameState == STATE_START_SCREEN) {
    // Rien à faire ici, géré par Render/Events
  } else if (m_gameState == STATE_PLAYING) {
    if (m_Player) m_Player->update(m_deltaTime);

    float playerSpeed = m_Player ? m_Player->getSpeed() : 0.0f; 
    float scrollAmount = playerSpeed * m_deltaTime; 

    m_BackgroundScrollX -= scrollAmount;
    if (m_BackgroundScrollX <= -SCREEN_WIDTH) { 
        m_BackgroundScrollX += SCREEN_WIDTH; 
    }

    m_totalDistanceTraveled += playerSpeed * m_deltaTime; 
    int currentDisplayedDistance = static_cast<int>(m_totalDistanceTraveled / 10.0f);
    if (currentDisplayedDistance != m_lastDisplayedDistance) {
      if (m_distanceTexture != nullptr) {
        SDL_DestroyTexture(m_distanceTexture);
        m_distanceTexture = nullptr;
      }
      std::stringstream ssDistance;
      ssDistance << "Distance: " << currentDisplayedDistance << " m / " << static_cast<int>(WIN_DISTANCE / 10.0f) << " m";
      SDL_Color textColor = { 0, 0, 0, 255 }; 
      if (m_uiFont) {
        SDL_Surface* surface = TTF_RenderText_Blended(m_uiFont, ssDistance.str().c_str(), textColor); 
        if (surface) {
          m_distanceTexture = SDL_CreateTextureFromSurface(m_Renderer, surface);
          if (m_distanceTexture) {
            SDL_QueryTexture(m_distanceTexture, NULL, NULL, &m_distanceRect.w, &m_distanceRect.h);
          } else {
            SDL_Log("Engine::Update - Echec creation texture distance: %s", SDL_GetError());
          }
          SDL_FreeSurface(surface);
        } else {
          SDL_Log("Engine::Update - Echec creation surface distance: %s", TTF_GetError());
        }
      }
      m_lastDisplayedDistance = currentDisplayedDistance;
    }

    if (currentTick - m_lastDifficultyIncreaseTime >= m_difficultyIncreaseInterval) {
      m_obstacleSpawnInterval -= m_spawnIntervalReduction;
      if (m_obstacleSpawnInterval < m_minSpawnInterval) {
        m_obstacleSpawnInterval = m_minSpawnInterval;
      }
      m_lastDifficultyIncreaseTime = currentTick;
      SDL_Log("Engine::Update - Difficulte augmentee! Nouvel interval de spawn: %.2fs", m_obstacleSpawnInterval);
    }

    if (currentTick - m_lastMaxSpeedIncreaseTime >= m_maxSpeedIncreaseInterval) {
      if (m_Player) m_Player->IncreaseMaxSpeed(m_maxSpeedIncreaseAmount, m_absoluteMaxPlayerSpeed);
      m_lastMaxSpeedIncreaseTime = currentTick;
    }

    m_timeSinceLastSpawn += m_deltaTime;
    if (m_timeSinceLastSpawn >= m_obstacleSpawnInterval) {
      SpawnObstacle();
      m_timeSinceLastSpawn = 0.0f; 
    }

    SDL_Rect playerFullCollider = m_Player ? m_Player->GetCollider() : SDL_Rect{0,0,0,0};
    bool collisionProcessedThisFrame = false;

    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ) {
      if (!it->isActive) { 
          it = m_obstacles.erase(it);
          continue;
      }

      it->collider.x -= static_cast<int>(scrollAmount); 

      float reductionFactor = 0.7f; 
      SDL_Rect playerCollisionBox;
      playerCollisionBox.w = static_cast<int>(playerFullCollider.w * reductionFactor);
      playerCollisionBox.h = static_cast<int>(playerFullCollider.h * reductionFactor);
      playerCollisionBox.x = playerFullCollider.x + (playerFullCollider.w - playerCollisionBox.w) / 2;
      playerCollisionBox.y = playerFullCollider.y + (playerFullCollider.h - playerCollisionBox.h) / 2;

      SDL_Rect obstacleCollisionBox;
      obstacleCollisionBox.w = static_cast<int>(it->collider.w * reductionFactor);
      obstacleCollisionBox.h = static_cast<int>(it->collider.h * reductionFactor);
      obstacleCollisionBox.x = it->collider.x + (it->collider.w - obstacleCollisionBox.w) / 2;
      obstacleCollisionBox.y = it->collider.y + (it->collider.h - obstacleCollisionBox.h) / 2;

      if (m_Player && SDL_HasIntersection(&playerCollisionBox, &obstacleCollisionBox) && !collisionProcessedThisFrame) {
        SDL_Log("Engine::Update - Collision detectee avec obstacle '%s'!", it->textureId.c_str());
        AudioManager::GetInstance()->PlaySound("crash", 0);
        m_Player->ApplySpeedPenalty(); 

        it = m_obstacles.erase(it); 
        collisionProcessedThisFrame = true; 
      } else if (it->collider.x + it->collider.w < 0) { 
        it = m_obstacles.erase(it);
      } else {
        ++it;
      }
    }

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - m_lastSecondUpdate >= 1000 && m_remainingSeconds > 0) {
      m_remainingSeconds--;
      m_lastSecondUpdate = currentTime; 
      SDL_Log("Engine::Update - Temps restant: %d s", m_remainingSeconds);

      if (m_remainingSeconds <= 10 && m_remainingSeconds > 0) {
        if (m_lastCountdownSecondPlayed != m_remainingSeconds) { 
          AudioManager::GetInstance()->PlaySound("countdown", 0);
          m_lastCountdownSecondPlayed = m_remainingSeconds;
        }
      }
      if (m_remainingSeconds == 0) {
        SDL_Log("Engine::Update - TEMPS ECOULE! Game Over.");
        SetGameState(STATE_GAME_OVER); 
      }
    }

    if (m_totalDistanceTraveled >= WIN_DISTANCE) {
      SDL_Log("Engine::Update - CONDITION DE VICTOIRE ATTEINTE! Distance: %.2f", m_totalDistanceTraveled);
      SetGameState(STATE_WIN);
    }

  } else if (m_gameState == STATE_GAME_OVER) {
    if (!m_showGameOverScreen) { 
      Uint32 currentTime = SDL_GetTicks();
      if (currentTime - m_gameOverStartTime >= 2000) { 
        SDL_Log("Engine::Update - Affichage de l'ecran Game Over.");
        m_showGameOverScreen = true; 
      }
    }
    if (m_showGameOverScreen && !m_showReturnPrompt && SDL_GetTicks() - m_endScreenStartTime >= RETURN_PROMPT_DELAY) {
        m_showReturnPrompt = true;
    }
  } else if (m_gameState == STATE_WIN) {
     if (!m_showReturnPrompt && SDL_GetTicks() - m_endScreenStartTime >= RETURN_PROMPT_DELAY) {
        m_showReturnPrompt = true;
    }
  }
}

void Engine::Render() {
  SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255); 
  SDL_RenderClear(m_Renderer);

  if (m_gameState == STATE_MAIN_MENU) {
    MainMenu::GetInstance()->Render(); 
  } else if (m_gameState == STATE_ABOUT) {
    SDL_SetRenderDrawColor(m_Renderer, 20, 20, 50, 255); 
    SDL_RenderClear(m_Renderer);
    if (TextureManager::GetInstance()->QueryTexture("about_screen", nullptr, nullptr)) { 
        TextureManager::GetInstance()->Draw("about_screen", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    } else {
        SDL_Log("Engine::Render - Texture 'about_screen' non trouvee pour l'etat STATE_ABOUT.");
    }
    if (m_returnPromptTexture) { 
        SDL_Rect aboutPromptRect = m_returnPromptRect; 
        aboutPromptRect.y = SCREEN_HEIGHT - 100; 
        aboutPromptRect.x = (SCREEN_WIDTH - aboutPromptRect.w) / 2;
        SDL_RenderCopy(m_Renderer, m_returnPromptTexture, NULL, &aboutPromptRect);
    }
  } else if (m_gameState == STATE_GAME_OVER) {
    if (m_showGameOverScreen) {
      SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255); 
      SDL_RenderClear(m_Renderer);
      int imgW = 0, imgH = 0;
      if (TextureManager::GetInstance()->QueryTexture("gameover", &imgW, &imgH)) {
        int imgX = (SCREEN_WIDTH - imgW) / 2;
        int imgY = (SCREEN_HEIGHT - imgH) / 2;
        TextureManager::GetInstance()->Draw("gameover", imgX, imgY, imgW, imgH);
      } else {
        SDL_Log("Engine::Render - Attention: Texture 'gameover' introuvable.");
      }
      if (m_showReturnPrompt && m_returnPromptTexture) {
        SDL_RenderCopy(m_Renderer, m_returnPromptTexture, NULL, &m_returnPromptRect);
      }
    } else { 
      int bgScrollInt = static_cast<int>(m_BackgroundScrollX);
      TextureManager::GetInstance()->Draw("background", bgScrollInt, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 0.6f); 
      TextureManager::GetInstance()->Draw("background", bgScrollInt + SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 0.6f);
      TextureManager::GetInstance()->Draw("track", 0, static_cast<int>(TRACK_Y_POSITION), SCREEN_WIDTH, static_cast<int>(TRACK_HEIGHT));
      if (m_Player) m_Player->draw();
      for (const auto& obs : m_obstacles) {
        if (obs.isActive) TextureManager::GetInstance()->Draw(obs.textureId, obs.collider.x, obs.collider.y, obs.collider.w, obs.collider.h);
      }
      if (TextureManager::GetInstance()->QueryTexture("00", nullptr, nullptr)) {
        TextureManager::GetInstance()->Draw("00", m_timerRect.x, m_timerRect.y, m_timerRect.w, m_timerRect.h);
      }
      if (m_distanceTexture) SDL_RenderCopy(m_Renderer, m_distanceTexture, NULL, &m_distanceRect);
    }
  } else if (m_gameState == STATE_WIN) {
    SDL_SetRenderDrawColor(m_Renderer, 100, 180, 255, 255); 
    SDL_RenderClear(m_Renderer);
    int imgW = 0, imgH = 0;
    if (TextureManager::GetInstance()->QueryTexture("win", &imgW, &imgH)) {
      int imgX = (SCREEN_WIDTH - imgW) / 2;
      int imgY = (SCREEN_HEIGHT - imgH) / 2;
      TextureManager::GetInstance()->Draw("win", imgX, imgY, imgW, imgH);
    } else {
      SDL_Log("Engine::Render - Attention: Texture 'win' introuvable.");
    }
    if (m_showReturnPrompt && m_returnPromptTexture) {
      SDL_RenderCopy(m_Renderer, m_returnPromptTexture, NULL, &m_returnPromptRect);
    }
  } else { 
    int bgScrollInt = static_cast<int>(m_BackgroundScrollX);
    TextureManager::GetInstance()->Draw("background", bgScrollInt, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 0.6f); 
    TextureManager::GetInstance()->Draw("background", bgScrollInt + SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 0.6f); 

    TextureManager::GetInstance()->Draw("track", 0, static_cast<int>(TRACK_Y_POSITION), SCREEN_WIDTH, static_cast<int>(TRACK_HEIGHT));

    if (m_gameState == STATE_PLAYING) {
      for (const auto& obs : m_obstacles) {
        if (obs.isActive) TextureManager::GetInstance()->Draw(obs.textureId, obs.collider.x, obs.collider.y, obs.collider.w, obs.collider.h);
      }
    }

    if (m_Player) m_Player->draw();

    if (m_gameState == STATE_START_SCREEN) {
      if (TextureManager::GetInstance()->QueryTexture("start", nullptr, nullptr)) {
          TextureManager::GetInstance()->Draw("start", m_timerRect.x, m_timerRect.y, m_timerRect.w, m_timerRect.h);
      } else {
          SDL_Log("Engine::Render - Texture 'start' non trouvee pour STATE_START_SCREEN.");
      }
    } else if (m_gameState == STATE_PLAYING) {
      std::string currentTimerTextureId = "end"; 
      if (m_remainingSeconds > 0 && m_remainingSeconds <= 60) {
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << m_remainingSeconds;
        currentTimerTextureId = ss.str();
      } else if (m_remainingSeconds > 60) { 
          currentTimerTextureId = "start"; 
      }
      
      if (TextureManager::GetInstance()->QueryTexture(currentTimerTextureId, nullptr, nullptr)) {
          TextureManager::GetInstance()->Draw(currentTimerTextureId, m_timerRect.x, m_timerRect.y, m_timerRect.w, m_timerRect.h);
      } else {
          SDL_Log("Engine::Render - Texture timer '%s' non trouvee.", currentTimerTextureId.c_str());
          if (TextureManager::GetInstance()->QueryTexture("00", nullptr, nullptr)) {
            TextureManager::GetInstance()->Draw("00", m_timerRect.x, m_timerRect.y, m_timerRect.w, m_timerRect.h);
          }
      }
    }

    if (m_distanceTexture != nullptr) {
      SDL_RenderCopy(m_Renderer, m_distanceTexture, NULL, &m_distanceRect);
    }
  }

  SDL_RenderPresent(m_Renderer);
}

void Engine::Events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      Quit(); 
      return; 
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_m: ToggleMute(); break;
            case SDLK_PAGEUP: IncreaseVolume(); break; 
            case SDLK_PAGEDOWN: DecreaseVolume(); break; 
        }
    }

    switch (m_gameState) {
      case STATE_MAIN_MENU:
        MainMenu::GetInstance()->HandleEvent(event); 
        break;
      case STATE_ABOUT:
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
          SDL_Log("Engine::Events - STATE_ABOUT: Echap appuye, retour au menu principal.");
          SetGameState(STATE_MAIN_MENU);
        }
        break;
      case STATE_START_SCREEN:
        if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) ) {
          SDL_Log("Engine::Events - STATE_START_SCREEN: Touche de demarrage appuyee, passage a STATE_PLAYING.");
          SetGameState(STATE_PLAYING); 
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            SDL_Log("Engine::Events - STATE_START_SCREEN: Echap appuye, retour au menu principal.");
            SetGameState(STATE_MAIN_MENU);
        }
        break;
      case STATE_PLAYING:
        if (m_Player && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
          m_Player->handleEvent(event);
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
          SDL_Log("Engine::Events - STATE_PLAYING: Echap appuye, retour au menu principal.");
          SetGameState(STATE_MAIN_MENU); 
        }
        break;
      case STATE_GAME_OVER:
      case STATE_WIN:
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_r || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                SDL_Log("Engine::Events - STATE_GAME_OVER/WIN: Touche 'R' (ou Entree/Espace) appuyee, redemarrage.");
                SetGameState(STATE_START_SCREEN); 
            } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                SDL_Log("Engine::Events - STATE_GAME_OVER/WIN: Echap appuye, retour au menu principal.");
                SetGameState(STATE_MAIN_MENU);
            }
        }
        break;
    }
  }
}

bool Engine::Clean() {
  SDL_Log("Engine::Clean() - Nettoyage du moteur...");

  if (m_returnPromptTexture) {
    SDL_DestroyTexture(m_returnPromptTexture);
    m_returnPromptTexture = nullptr;
    SDL_Log("Engine::Clean() - Texture 'Retour au menu' detruite.");
  }
  if (m_distanceTexture != nullptr) {
    SDL_DestroyTexture(m_distanceTexture);
    m_distanceTexture = nullptr;
    SDL_Log("Engine::Clean() - Texture 'Distance' detruite.");
  }
  m_obstacles.clear();
  m_obstacleTextureIds.clear();
  m_timerTextures.clear(); // Les textures elles-mêmes sont gérées par TextureManager
  SDL_Log("Engine::Clean() - Vecteurs d'obstacles et de textures timer vides.");

  if (m_uiFont != nullptr) {
    TTF_CloseFont(m_uiFont);
    m_uiFont = nullptr;
    SDL_Log("Engine::Clean() - Police UI fermee.");
  }
  
  if (m_Player) {
    delete m_Player; 
    m_Player = nullptr;
    SDL_Log("Engine::Clean() - Joueur detruit.");
  }

  // MainMenu est un singleton, il devrait se nettoyer lui-même ou vous pourriez le faire ici
  // MainMenu::GetInstance()->Clean(); // Si MainMenu a une méthode Clean publique
  // delete MainMenu::GetInstance(); // Si vous voulez supprimer l'instance du singleton

  TextureManager::GetInstance()->Clean(); 
  SDL_Log("Engine::Clean() - TextureManager nettoye.");

  AudioManager::GetInstance()->Clean(); 
  SDL_Log("Engine::Clean() - AudioManager nettoye.");

  if (m_Renderer) {
    SDL_DestroyRenderer(m_Renderer);
    m_Renderer = nullptr;
    SDL_Log("Engine::Clean() - Renderer detruit.");
  }
  if (m_Window) {
    SDL_DestroyWindow(m_Window);
    m_Window = nullptr;
    SDL_Log("Engine::Clean() - Fenetre detruite.");
  }

  TTF_Quit();
  SDL_Log("Engine::Clean() - SDL_ttf quitte.");
  IMG_Quit();
  SDL_Log("Engine::Clean() - SDL_image quitte.");
  SDL_Quit();
  SDL_Log("Engine::Clean() - SDL quitte.");
  
  SDL_Log("Engine::Clean() - Nettoyage termine.");
  return true;
}

void Engine::Quit() {
  m_IsRunning = false; 
  SDL_Log("Engine::Quit() - Moteur marque pour fermeture.");
}