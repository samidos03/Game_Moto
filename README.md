# 🏍️ Game Moto – Jamaa El Fna

Un jeu de moto développé en C / SDL où le joueur traverse les rues de **Jamaa El Fna** à Marrakech. Ce projet est réalisé dans le cadre d’un exercice ou d’un apprentissage personnel en développement de jeux.

---

## 🎮 Fonctionnalités

- Contrôle de la moto avec le clavier  
- Gestion des collisions  
- Animation des éléments  
- Sons et effets visuels (si SDL_mixer utilisé)  
- Scène inspirée de la célèbre place Jamaa El Fna  

---

## 🛠️ Technologies utilisées

- **Langage** : C  
- **Bibliothèque** : SDL2, SDL2_image, SDL2_ttf, SDL2_mixer  
- **IDE recommandé** : Visual Studio Code  

---

## 🚀 Lancer le projet

### 1. Installer SDL2 :  
- Télécharger SDL2 (SDL2.dll, headers et librairies)  
- Lier les bibliothèques dans votre IDE  

### 2. Compiler (exemple sous Linux) :  
```bash
gcc main.c -o game -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
./game
