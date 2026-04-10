#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

namespace CustomGraphics {

    // Helper function to safely set a pixel without crashing
    inline void SetPixel(sf::Image& img, int x, int y, sf::Color col) {
        // FIX: SFML 2.x requires individual unsigned ints. 
        // Using static_cast explicitly silences VS Code's strict type-checker.
        if (x >= 0 && x < (int)img.getSize().x && y >= 0 && y < (int)img.getSize().y) {
            img.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), col);
        }
    }

    // --- 1. MIDPOINT CIRCLE DRAWING ALGORITHM ---
    inline void MidpointCircle(sf::Image& img, int xc, int yc, int r, sf::Color col) {
        int x = 0;
        int y = r;
        int p = 1 - r; 

        auto plotPoints = [&](int cx, int cy, int x, int y) {
            SetPixel(img, cx + x, cy + y, col); SetPixel(img, cx - x, cy + y, col);
            SetPixel(img, cx + x, cy - y, col); SetPixel(img, cx - x, cy - y, col);
            SetPixel(img, cx + y, cy + x, col); SetPixel(img, cx - y, cy + x, col);
            SetPixel(img, cx + y, cy - x, col); SetPixel(img, cx - y, cy - x, col);
        };

        plotPoints(xc, yc, x, y);
        while (x < y) {
            x++;
            if (p < 0) {
                p += 2 * x + 1;
            } else {
                y--;
                p += 2 * (x - y) + 1;
            }
            plotPoints(xc, yc, x, y);
        }
    }

    // --- 2. BRESENHAM'S LINE DRAWING ALGORITHM ---
    inline void BresenhamLine(sf::Image& img, int x0, int y0, int x1, int y1, sf::Color col) {
        int dx = std::abs(x1 - x0);
        int dy = -std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int e2;

        while (true) {
            SetPixel(img, x0, y0, col);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    // --- GENERATE THE MAGICAL SLINGSHOT RUNE ---
    inline sf::Texture GenerateMagicRune() {
        sf::Image img;
        
        // FIX: Reverted back to SFML 2.x create(), using an explicit color 
        // to prevent namespace confusion.
        img.create(200, 200, sf::Color(0, 0, 0, 0)); 
        
        sf::Color runeColor(255, 215, 0, 220); 
        sf::Color faintColor(255, 215, 0, 80);

        int c = 100; // Center of the 200x200 image

        // Draw Thick Outer Circles
        MidpointCircle(img, c, c, 90, runeColor);
        MidpointCircle(img, c, c, 89, runeColor);
        
        // Draw Thick Inner Circles
        MidpointCircle(img, c, c, 75, runeColor);
        MidpointCircle(img, c, c, 74, runeColor);

        // Calculate 6 points on the inner circle for a Hexagram
        sf::Vector2i pts[6];
        for(int i = 0; i < 6; i++) {
            float angle = i * (3.14159f / 3.0f);
            pts[i] = {c + (int)(75 * std::cos(angle)), c + (int)(75 * std::sin(angle))};
        }

        // Draw Triangle 1 using Bresenham
        BresenhamLine(img, pts[0].x, pts[0].y, pts[2].x, pts[2].y, runeColor);
        BresenhamLine(img, pts[2].x, pts[2].y, pts[4].x, pts[4].y, runeColor);
        BresenhamLine(img, pts[4].x, pts[4].y, pts[0].x, pts[0].y, runeColor);

        // Draw Triangle 2 using Bresenham
        BresenhamLine(img, pts[1].x, pts[1].y, pts[3].x, pts[3].y, runeColor);
        BresenhamLine(img, pts[3].x, pts[3].y, pts[5].x, pts[5].y, runeColor);
        BresenhamLine(img, pts[5].x, pts[5].y, pts[1].x, pts[1].y, runeColor);

        // Draw structural lines to the center core
        for(int i = 0; i < 6; i++) {
            BresenhamLine(img, c, c, pts[i].x, pts[i].y, faintColor);
        }

        // Center magical core
        MidpointCircle(img, c, c, 15, runeColor);
        MidpointCircle(img, c, c, 14, runeColor);

        sf::Texture tex;
        tex.loadFromImage(img);
        return tex;
    }
}