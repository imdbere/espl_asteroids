#include "player.h"
#include "tools.h"

void updatePlayer(struct player* player, int joyX, int joyY)
{
    float shipMaxSpeed = 2;
    
    addToVec(&player->speed, (pointf) {joyX / 2000.0, joyY / 2000.0});
    clampVec(&player->speed, shipMaxSpeed);
    
    addToVec(&player->position, player->speed);
    wrapScreen(&player->position);

    player->isThrusting = joyX != 0 || joyY != 0;
    if (player->isThrusting)
    {
        float angleRad = atan2f(-joyY, joyX);
        float t = 0.1;

        player->angleRad = lerp_angle(player->angleRad, angleRad, t);
    }

    player->flameLength = max(abs(joyX), abs(joyY)) * 12 / 127.0;
}

void drawPlayer(struct player* player)
{
    int pointCount = 6;

    point points[] = {{-8, 0}, {8, 0}, {0, 20}, {-5, 0}, {5, 0}, {0, -player->flameLength - 5}};
    for (int i = 0; i < pointCount; i++)
    {
        pointf rotatedPoint = rotatePoint(points[i], player->angleRad - M_PI / 2); // Player has a default rotation of 90°
        points[i] = (point){(int) rotatedPoint.x, (int) rotatedPoint.y};
    }

    gdispFillConvexPoly(player->position.x, player->position.y, points, 3, White);
    if (player->isThrusting)
        gdispFillConvexPoly(player->position.x, player->position.y, points + 3, 3, Orange);
}