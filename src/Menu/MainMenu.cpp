#include "MainMenu.h"
#include "../Core/Engine.h" // Pour GetInstance()->SetGameState(), Quit(), GetRenderer(), SCREEN_WIDTH/HEIGHT defines
#include "../Graphics/TextureManager.h"
#include "../Audio/AudioManager.h"
#include <SDL.h>
#include <SDL_image.h> // Pour IMG_GetError()
#include <algorithm>   // Pour std::min/max

MainMenu* MainMenu::s_Instance = nullptr;

MainMenu::MainMenu() :
    m_playHovered(false), m_aboutHovered(false), m_quitHovered(false),
    // m_playClicked(false), m_aboutClicked(false), m_quitClicked(false), // Non utilisés pour l'état
    m_volDownHovered(false), m_volUpHovered(false), m_muteToggleHovered(false),
    m_playScale(m_defaultScale), m_aboutScale(m_defaultScale), m_quitScale(m_defaultScale),
    m_volDownScale(m_defaultScale), m_volUpScale(m_defaultScale), m_muteToggleScale(m_defaultScale)
    // m_hoverScale est const float m_hoverScaleFactor maintenant
{
    SDL_Log("MainMenu::MainMenu() - Constructeur.");
    // Initialiser les rectangles ici, car SCREEN_WIDTH/HEIGHT de Engine sont disponibles
    // (en supposant que Engine est déjà initialisé ou que les defines sont globaux)
    // Idéalement, on récupère les dimensions réelles de la fenêtre via Engine.

    // Si SCREEN_WIDTH et SCREEN_HEIGHT sont des defines globaux (comme dans Engine.h)
    // ou si Engine::GetInstance() est déjà utilisable (attention au moment de la construction du singleton)
    // Pour plus de sécurité, initialisez-les dans Init() après que l'Engine soit accessible.
    // Je vais les mettre ici en supposant que les defines sont disponibles.
    // Si vous avez des erreurs de compilation ici, déplacez cette initialisation dans MainMenu::Init().

    // Dimensions des boutons principaux
    int mainButtonWidth = 300;
    int mainButtonHeight = 80;
    m_playButtonRect = { (SCREEN_WIDTH - mainButtonWidth) / 2, SCREEN_HEIGHT - 350, mainButtonWidth, mainButtonHeight };
    m_aboutButtonRect = { (SCREEN_WIDTH - mainButtonWidth) / 2, SCREEN_HEIGHT - 250, mainButtonWidth, mainButtonHeight }; // Ajusté largeur pour cohérence
    m_quitButtonRect = { (SCREEN_WIDTH - mainButtonWidth) / 2, SCREEN_HEIGHT - 150, mainButtonWidth, mainButtonHeight }; // Ajusté largeur

    // Dimensions des boutons de volume
    int volButtonSize = 60; // Augmenté la taille pour meilleure cliquabilité
    int volButtonMargin = 30;
    m_volDownRect =    { volButtonMargin, SCREEN_HEIGHT - volButtonMargin - volButtonSize * 3 - 20, volButtonSize, volButtonSize };   
    m_muteToggleRect = { volButtonMargin, SCREEN_HEIGHT - volButtonMargin - volButtonSize * 2 - 10, volButtonSize, volButtonSize };  
    m_volUpRect =      { volButtonMargin, SCREEN_HEIGHT - volButtonMargin - volButtonSize * 1 - 0,  volButtonSize, volButtonSize }; 
}

MainMenu::~MainMenu() {
    SDL_Log("MainMenu::~MainMenu() - Destructeur.");
    // Clean() est appelé par Engine::Clean() ou à la fin du programme si MainMenu est détruit.
    // Pas besoin de l'appeler ici si l'Engine gère la suppression du singleton.
    // Cependant, si MainMenu alloue des choses non gérées par TM, il faudrait un Clean() ici.
}


bool MainMenu::Init() {
    SDL_Log("MainMenu::Init() - >>> DEBUT DU CHARGEMENT DES RESSOURCES DU MENU <<<");

    // Tentative de chargement de chaque texture avec des logs détaillés
    SDL_Log("MainMenu::Init() - Chargement de 'menu_bg' depuis 'assets/Menu/menu_background.png'...");
    if (!TextureManager::GetInstance()->Load("menu_bg", "assets/Menu/menu_background.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'menu_bg'. Erreur TextureManager/IMG: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'menu_bg' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'play_btn' depuis 'assets/Menu/btn_jouer.png'...");
    if (!TextureManager::GetInstance()->Load("play_btn", "assets/Menu/btn_jouer.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'play_btn'. Erreur: %s", IMG_GetError());
        return false; // Pas besoin de nettoyer ici, TextureManager::Clean() le fera si Engine::Init échoue
    }
    SDL_Log("MainMenu::Init() - 'play_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'play_btn_hover' depuis 'assets/Menu/btn_jouer_hover.png'...");
    if (!TextureManager::GetInstance()->Load("play_btn_hover", "assets/Menu/btn_jouer_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'play_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'play_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'about_btn' depuis 'assets/Menu/btn_a_propos.png'...");
    if (!TextureManager::GetInstance()->Load("about_btn", "assets/Menu/btn_a_propos.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'about_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'about_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'about_btn_hover' depuis 'assets/Menu/btn_a_propos_hover.png'...");
    if (!TextureManager::GetInstance()->Load("about_btn_hover", "assets/Menu/btn_a_propos_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'about_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'about_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'about_screen' depuis 'assets/Menu/about_background.png'...");
    if (!TextureManager::GetInstance()->Load("about_screen", "assets/Menu/about_background.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'about_screen'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'about_screen' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'quit_btn' depuis 'assets/Menu/btn_quitter.png'...");
    if (!TextureManager::GetInstance()->Load("quit_btn", "assets/Menu/btn_quitter.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'quit_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'quit_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'quit_btn_hover' depuis 'assets/Menu/btn_quitter_hover.png'...");
    if (!TextureManager::GetInstance()->Load("quit_btn_hover", "assets/Menu/btn_quitter_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'quit_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'quit_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'vol_down_btn' depuis 'assets/Menu/vol_down.png'...");
    if (!TextureManager::GetInstance()->Load("vol_down_btn", "assets/Menu/vol_down.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'vol_down_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'vol_down_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'vol_down_btn_hover' depuis 'assets/Menu/vol_down_hover.png'...");
    if (!TextureManager::GetInstance()->Load("vol_down_btn_hover", "assets/Menu/vol_down_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'vol_down_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'vol_down_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'vol_up_btn' depuis 'assets/Menu/vol_up.png'...");
    if (!TextureManager::GetInstance()->Load("vol_up_btn", "assets/Menu/vol_up.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'vol_up_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'vol_up_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'vol_up_btn_hover' depuis 'assets/Menu/vol_up_hover.png'...");
    if (!TextureManager::GetInstance()->Load("vol_up_btn_hover", "assets/Menu/vol_up_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'vol_up_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'vol_up_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'mute_btn' depuis 'assets/Menu/mute.png'...");
    if (!TextureManager::GetInstance()->Load("mute_btn", "assets/Menu/mute.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'mute_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'mute_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'mute_btn_hover' depuis 'assets/Menu/mute_hover.png'...");
    if (!TextureManager::GetInstance()->Load("mute_btn_hover", "assets/Menu/mute_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'mute_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'mute_btn_hover' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'unmute_btn' depuis 'assets/Menu/unmute.png'...");
    if (!TextureManager::GetInstance()->Load("unmute_btn", "assets/Menu/unmute.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'unmute_btn'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'unmute_btn' charge avec succes.");

    SDL_Log("MainMenu::Init() - Chargement de 'unmute_btn_hover' depuis 'assets/Menu/unmute_hover.png'...");
    if (!TextureManager::GetInstance()->Load("unmute_btn_hover", "assets/Menu/unmute_hover.png")) {
        SDL_Log("MainMenu::Init() - ERREUR: Echec chargement 'unmute_btn_hover'. Erreur: %s", IMG_GetError());
        return false;
    }
    SDL_Log("MainMenu::Init() - 'unmute_btn_hover' charge avec succes.");


    SDL_Log("MainMenu::Init() - SUCCES: Toutes les textures du menu sont chargees !");
    return true;
}

void MainMenu::HandleEvent(SDL_Event& event) {
    int x, y; 
    SDL_GetMouseState(&x, &y); 
    SDL_Point mousePoint = {x, y};

    // Mise à jour de l'état de survol
    m_playHovered = SDL_PointInRect(&mousePoint, &m_playButtonRect);
    m_aboutHovered = SDL_PointInRect(&mousePoint, &m_aboutButtonRect);
    m_quitHovered = SDL_PointInRect(&mousePoint, &m_quitButtonRect);
    m_volDownHovered = SDL_PointInRect(&mousePoint, &m_volDownRect);
    m_volUpHovered = SDL_PointInRect(&mousePoint, &m_volUpRect);
    m_muteToggleHovered = SDL_PointInRect(&mousePoint, &m_muteToggleRect);

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) { // S'assurer que c'est le clic gauche
            if (m_playHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->SetGameState(STATE_START_SCREEN); 
                SDL_Log("MainMenu: Bouton 'Jouer' clique."); 
            } else if (m_aboutHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->SetGameState(STATE_ABOUT); 
                SDL_Log("MainMenu: Bouton 'A Propos' clique."); 
            } else if (m_quitHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->Quit(); 
                SDL_Log("MainMenu: Bouton 'Quitter' clique."); 
            } else if (m_volDownHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->DecreaseVolume(); 
            } else if (m_volUpHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->IncreaseVolume(); 
            } else if (m_muteToggleHovered) { 
                AudioManager::GetInstance()->PlaySound("click", 0); 
                Engine::GetInstance()->ToggleMute(); 
            }
        }
    }
}

void MainMenu::Update(float deltaTime) {
    // Logique d'interpolation pour l'effet de grossissement au survol
    auto updateScale = [&](float& currentScale, bool hovered) {
        float targetScale = hovered ? m_hoverScaleFactor : m_defaultScale;
        if (std::abs(currentScale - targetScale) > 0.01f) { // Évite les micro-changements
            if (currentScale < targetScale) {
                currentScale += m_scaleSpeed * deltaTime;
                if (currentScale > targetScale) currentScale = targetScale;
            } else {
                currentScale -= m_scaleSpeed * deltaTime;
                if (currentScale < targetScale) currentScale = targetScale;
            }
        } else {
            currentScale = targetScale;
        }
    };

    updateScale(m_playScale, m_playHovered);
    updateScale(m_aboutScale, m_aboutHovered);
    updateScale(m_quitScale, m_quitHovered);
    updateScale(m_volDownScale, m_volDownHovered);
    updateScale(m_volUpScale, m_volUpHovered);
    updateScale(m_muteToggleScale, m_muteToggleHovered);
}

void MainMenu::Render() {
    // S'assurer que le renderer est disponible
    SDL_Renderer* renderer = Engine::GetInstance()->GetRenderer();
    if (!renderer) {
        SDL_Log("MainMenu::Render() - ERREUR: Renderer non disponible.");
        return;
    }

    // 1. Dessiner l'arrière-plan
    TextureManager::GetInstance()->Draw("menu_bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Fonction lambda pour dessiner un bouton avec mise à l'échelle
    auto drawScaledButton = [&](const std::string& baseTextureID, const SDL_Rect& originalRect, bool isHovered, float currentScale) {
        std::string textureToDraw = baseTextureID;
        if (isHovered) {
            // Vérifier si la texture survolée existe, sinon utiliser la texture de base
            if (TextureManager::GetInstance()->IsTextureLoaded(baseTextureID + "_hover")) {
                 textureToDraw += "_hover";
            } else {
                SDL_Log("MainMenu::Render - Warning: Texture survol '%s_hover' non trouvee, utilisation de la base '%s'", baseTextureID.c_str(), baseTextureID.c_str());
            }
        }

        int scaledWidth = static_cast<int>(originalRect.w * currentScale);
        int scaledHeight = static_cast<int>(originalRect.h * currentScale);
        int posX = originalRect.x + (originalRect.w - scaledWidth) / 2;   // Centrer le bouton mis à l'échelle
        int posY = originalRect.y + (originalRect.h - scaledHeight) / 2; // Centrer le bouton mis à l'échelle
        
        TextureManager::GetInstance()->Draw(textureToDraw, posX, posY, scaledWidth, scaledHeight);
    };

    // 2. Dessiner les boutons principaux
    drawScaledButton("play_btn", m_playButtonRect, m_playHovered, m_playScale);
    drawScaledButton("about_btn", m_aboutButtonRect, m_aboutHovered, m_aboutScale);
    drawScaledButton("quit_btn", m_quitButtonRect, m_quitHovered, m_quitScale);
    
    // 3. Dessiner les boutons de volume
    drawScaledButton("vol_down_btn", m_volDownRect, m_volDownHovered, m_volDownScale);
    drawScaledButton("vol_up_btn", m_volUpRect, m_volUpHovered, m_volUpScale);

    // 4. Dessiner le bouton Mute/Unmute
    bool isMuted = Engine::GetInstance()->IsMuted();
    std::string muteBaseID = isMuted ? "unmute_btn" : "mute_btn";
    drawScaledButton(muteBaseID, m_muteToggleRect, m_muteToggleHovered, m_muteToggleScale);

    // SDL_RenderPresent est appelé par Engine::Render()
}

void MainMenu::Clean() {
    SDL_Log("MainMenu::Clean() - Nettoyage des textures du menu (via TextureManager::Drop).");
    // TextureManager::Drop est idempotent, donc appeler Drop pour des textures non chargées n'est pas une erreur.
    // Cependant, il est plus propre de ne "Drop" que ce qui a été "Load"-ed.
    // Mais pour un nettoyage général, c'est acceptable.
    TextureManager::GetInstance()->Drop("menu_bg");
    TextureManager::GetInstance()->Drop("play_btn"); 
    TextureManager::GetInstance()->Drop("play_btn_hover");
    TextureManager::GetInstance()->Drop("about_btn"); 
    TextureManager::GetInstance()->Drop("about_btn_hover");
    TextureManager::GetInstance()->Drop("quit_btn"); 
    TextureManager::GetInstance()->Drop("quit_btn_hover");
    TextureManager::GetInstance()->Drop("about_screen");
    TextureManager::GetInstance()->Drop("vol_down_btn"); 
    TextureManager::GetInstance()->Drop("vol_down_btn_hover");
    TextureManager::GetInstance()->Drop("vol_up_btn"); 
    TextureManager::GetInstance()->Drop("vol_up_btn_hover");
    TextureManager::GetInstance()->Drop("mute_btn"); 
    TextureManager::GetInstance()->Drop("mute_btn_hover");
    TextureManager::GetInstance()->Drop("unmute_btn"); 
    TextureManager::GetInstance()->Drop("unmute_btn_hover");
    SDL_Log("MainMenu::Clean() - Nettoyage termine.");
}