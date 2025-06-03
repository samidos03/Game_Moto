#include "Player.h"
#include "../Graphics/TextureManager.h"
#include "../Core/Engine.h"
#include <SDL.h>
#include <cmath>
#include <algorithm>

Player::Player() :
    m_x(0.0f),
    m_currentY(0.0f),
    m_targetY(0.0f),
    m_currentLane(0),
    m_numLanes(0),
    m_laneChangeAnimSpeed(700.0f),
    m_speed(0.0f),
    m_acceleration(280.0f),
    m_braking(1000.0f),
    m_drag(0.90f),
    m_maxSpeed(380.0f),
    m_minSpeed(0.0f),
    m_textureId(""),
    m_width(0),
    m_height(0),
    m_isSlowed(false),
    m_slowedStartTime(0),
    m_initialMaxSpeed(380.0f)
{}

bool Player::load(std::string textureId, float startX, const std::vector<float>& laneYPositions) {
    if (textureId.empty()) {
        SDL_Log("Player::load - Error: Empty texture ID provided.");
        return false;
    }
    if (!TextureManager::GetInstance()->QueryTexture(textureId, &m_width, &m_height)) {
        SDL_Log("Player::load - Failed to query texture ID '%s'", textureId.c_str());
        m_width = m_height = 0;
        return false;
    }
    m_textureId = textureId;
    m_initialMaxSpeed = m_maxSpeed;
    reset(startX, laneYPositions);
    SDL_Log("Player loaded with Texture ID: %s, Size: %dx%d, Lanes: %d, Initial Lane: %d",
            m_textureId.c_str(), m_width, m_height, m_numLanes, m_currentLane);
    return true;
}

void Player::reset(float startX, const std::vector<float>& laneYPositions) {
     m_x = startX;
     m_speed = 0.0f;
     m_laneYPositions = laneYPositions;
     m_numLanes = static_cast<int>(m_laneYPositions.size());
     m_isSlowed = false;
     m_slowedStartTime = 0;
     m_maxSpeed = m_initialMaxSpeed;

     if (m_numLanes > 0) {
        m_currentLane = m_numLanes / 2;
        setLane(m_currentLane);
        m_currentY = m_targetY;
     } else {
        SDL_Log("Warning: No lane positions provided during Player::reset\n");

        m_currentY = 300;
        m_targetY = m_currentY;
        m_currentLane = -1;
     }
}

void Player::IncreaseMaxSpeed(float amount, float absoluteMax) {
    m_maxSpeed += amount;
    if (m_maxSpeed > absoluteMax) {
        m_maxSpeed = absoluteMax;
    }
     SDL_Log("Player Max Speed Increased! New max speed: %.2f", m_maxSpeed);
}


void Player::ApplySpeedPenalty() {
    if (!m_isSlowed) {
        SDL_Log("Applying speed penalty!");
        m_isSlowed = true;
        m_slowedStartTime = SDL_GetTicks();
        m_speed = m_penaltySpeed;
    }
}

SDL_Rect Player::GetCollider() const {
    SDL_Rect collider;
    collider.x = static_cast<int>(m_x - m_width / 2.0f);
    collider.y = static_cast<int>(m_currentY - m_height / 2.0f);
    collider.w = m_width;
    collider.h = m_height;
    return collider;
}


void Player::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (m_currentLane > 0) {
                    setLane(m_currentLane - 1);
                }
                break;
            case SDLK_DOWN:
                 if (m_currentLane < m_numLanes - 1) {
                    setLane(m_currentLane + 1);
                }
                break;
            default:
                break;
        }
    }
}

void Player::update(float deltaTime) {
    if (m_isSlowed) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - m_slowedStartTime >= m_penaltyDuration) {
            SDL_Log("Speed penalty ended.");
            m_isSlowed = false;
        }
    }

    if (!m_isSlowed) {
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        bool braking = keyState[SDL_SCANCODE_LEFT];

        if (braking) {
            m_speed -= m_braking * deltaTime;
        } else {
            m_speed += m_acceleration * deltaTime;
        }

        if (m_speed > 0 && m_drag < 1.0f && m_drag >= 0.0f) {
             m_speed *= pow(m_drag, deltaTime);
        }

        m_speed = std::max(m_minSpeed, std::min(m_speed, m_maxSpeed));


         if (braking && std::abs(m_speed) < 1.0f) {
              m_speed = 0.0f;
         }
         else if (!braking && std::abs(m_speed) < 0.5f) {
             m_speed = 0.0f;
         }
    } else {
        m_speed = m_penaltySpeed;
    }


    if (m_numLanes > 0) {
        if (std::abs(m_currentY - m_targetY) > 0.5f) {
            float direction = (m_targetY > m_currentY) ? 1.0f : -1.0f;
            float distanceToMove = m_laneChangeAnimSpeed * deltaTime;
            m_currentY += direction * distanceToMove;

            if ((direction > 0 && m_currentY > m_targetY) || (direction < 0 && m_currentY < m_targetY)) {
                m_currentY = m_targetY;
            }
        } else {
            m_currentY = m_targetY;
        }
    }
}

void Player::draw() {
    if (m_width > 0 && m_height > 0 && !m_textureId.empty()) {
        int drawX = static_cast<int>(m_x - m_width / 2.0f);
        int drawY = static_cast<int>(m_currentY - m_height / 2.0f);
        TextureManager::GetInstance()->Draw(m_textureId, drawX, drawY, m_width, m_height);
    }
}

float Player::getSpeed() const {
    return m_speed;
}

void Player::setLane(int laneIndex) {
    if (m_numLanes > 0 && laneIndex >= 0 && laneIndex < m_numLanes) {
        m_currentLane = laneIndex;
        m_targetY = m_laneYPositions[m_currentLane];
    } else {
         SDL_Log("Player::setLane - Warning: Invalid lane index %d requested.", laneIndex);
    }
}
