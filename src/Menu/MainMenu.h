#ifndef MAINMENU_H
#define MAINMENU_H

#include "SDL.h"
// Pas besoin d'inclure Engine.h ici si SCREEN_WIDTH/HEIGHT sont des defines globaux
// ou si vous les passez en paramètre. Pour l'instant, on suppose qu'ils sont accessibles
// via Engine::GetInstance() ou des defines comme dans votre Engine.h.
// Pour simplifier, je vais utiliser les defines que vous avez dans Engine.h.
// Si Engine.h définit SCREEN_WIDTH et SCREEN_HEIGHT, il doit être inclus AVANT ou ici.
// Cependant, il est préférable de ne pas dépendre des defines d'Engine dans le .h de MainMenu.
// Idéalement, les dimensions seraient passées ou récupérées dynamiquement.
// Pour cet exemple, je vais supposer que SCREEN_WIDTH et SCREEN_HEIGHT sont définis
// quelque part globalement ou via une inclusion (ex: un "GameDefines.h").
// Si ce n'est pas le cas, vous aurez des erreurs de compilation ici.
// Ou, mieux, récupérez-les de l'Engine dans le .cpp.

// Pour éviter une dépendance directe à Engine.h dans MainMenu.h pour SCREEN_WIDTH/HEIGHT
// Vous pouvez les déclarer comme extern si définis ailleurs, ou les obtenir dans le .cpp
// Pour l'instant, je vais les utiliser en supposant qu'ils sont accessibles.
// Si vous avez #define SCREEN_WIDTH 1920 dans Engine.h, et que Engine.h est inclus
// avant MainMenu.h dans vos .cpp, cela fonctionnera.

// Dimensions (si vous ne les récupérez pas de l'Engine dans le .h)
// Ces valeurs doivent correspondre à celles utilisées par votre Engine
#define MENU_SCREEN_WIDTH 1920
#define MENU_SCREEN_HEIGHT 1080


class MainMenu {
public:
    static MainMenu* GetInstance() {
        // Simplification du singleton pour éviter les conditions de concurrence
        if (s_Instance == nullptr) {
            s_Instance = new MainMenu();
        }
        return s_Instance;
    }

    bool Init();
    void HandleEvent(SDL_Event& event);
    void Update(float deltaTime);
    void Render();
    void Clean();

private:
    MainMenu(); // Constructeur privé
    ~MainMenu(); // Destructeur pour appeler Clean

    static MainMenu* s_Instance;
                
    // Coordonnées et dimensions des boutons
    // Il est préférable d'initialiser les valeurs dans le constructeur ou Init()
    // plutôt que directement dans la déclaration de la classe si elles dépendent de SCREEN_WIDTH/HEIGHT.
    SDL_Rect m_playButtonRect;
    SDL_Rect m_aboutButtonRect;
    SDL_Rect m_quitButtonRect;
    
    SDL_Rect m_volDownRect;
    SDL_Rect m_muteToggleRect;
    SDL_Rect m_volUpRect;

    // États de survol et d'animation
    bool m_playHovered, m_aboutHovered, m_quitHovered;
    // m_playClicked, m_aboutClicked, m_quitClicked; // Semblent non utilisés pour l'état, juste pour l'action
    float m_playScale;
    float m_aboutScale;
    float m_quitScale;

    bool m_volDownHovered, m_volUpHovered, m_muteToggleHovered;
    float m_volDownScale;
    float m_volUpScale;
    float m_muteToggleScale;

    const float m_defaultScale = 1.0f; // Échelle par défaut
    const float m_hoverScaleFactor = 1.1f; // Facteur d'agrandissement au survol
    const float m_scaleSpeed = 8.0f; // Vitesse d'animation de l'échelle
};

#endif // MAINMENU_H