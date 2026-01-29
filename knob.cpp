#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

extern "C" {
    #include <raylib.h>
}

typedef uint32_t u32;

// --------------------------------------------------
// Global UI focus
// --------------------------------------------------
class Knob;
static Knob* gActiveKnob = nullptr;

// --------------------------------------------------
// Utility
// --------------------------------------------------
static Color Darken(Color c, float f) {
    return Color{
        (unsigned char)(c.r * f),
        (unsigned char)(c.g * f),
        (unsigned char)(c.b * f),
        c.a
    };
}

// --------------------------------------------------
// Knob
// --------------------------------------------------
class Knob {
public:
    Knob(Vector2 pos, float r, bool lin=false)
        : position(pos), radius(r), sqradius(r*r), linear(lin){
            ring = minring;
            glow = minglow;
        }

    void draw() {
        update();

        bool hasFocus = (gActiveKnob != nullptr);
        bool active   = (gActiveKnob == this);
        bool muted    = hasFocus && !active;

        drawBody(active, muted);
        drawIndicator(active, muted);
        drawValue(muted);
    }

    u32 value() const { return kvalue; }

private:
    // Geometry
    Vector2 position{};
    float radius{};
    float sqradius{};

    // State
    float angle = MIN_ANGLE;
    u32   kvalue = 0;
    bool  hovered = false;
    bool  dragging = false;

    // Style
    Color minring{247, 115, 115, 255};
    Color minglow{248, 139, 139, 110};
    Color maxring{115, 247, 115, 255};
    Color maxglow{176, 248, 139, 110};
    Color ring;
    Color glow;

    Color base{60, 60, 60, 255};
    Color indicator{235, 235, 235, 255};


    // Usable arc
    static constexpr float MIN_ANGLE = PI * 0.25f;  // 45°
    static constexpr float MAX_ANGLE = PI * 1.75f;  // 315°
    static constexpr float RANGE     = MAX_ANGLE - MIN_ANGLE;

    // Linear Variant
    Vector2 dposition{};
    bool linear = false;
    float dangle = 0.0f;
private:
    void update() {
        Vector2 m = GetMousePosition();
        float dx = m.x - position.x;
        float dy = m.y - position.y;

        hovered = (dx*dx + dy*dy) <= sqradius * 1.3f;

        // Mouse press → steal focus
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gActiveKnob = this;
            dragging = true;
            dposition = m;
            dangle = angle;
        }

        // Mouse release → release focus
        if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragging = false;
            if (gActiveKnob == this)
                gActiveKnob = nullptr;
        }

        // --- Angle update (accept / ignore, NO CLAMP) ---
        if (dragging && gActiveKnob == this) {
            
            // Linear
            if (linear) {
                angle = dangle + 0.025*(dposition.y - m.y);
                angle = std::min( std::max(angle, MIN_ANGLE),MAX_ANGLE);
            }
            // Non linear
            else {
                float a = atan2f(dy, dx);
                // Normalize to 0..2PI
                if (a < 0.0f)
                    a += 2.0f * PI;
                // Accept only usable arc
                if (a >= MIN_ANGLE && a <= MAX_ANGLE) {
                    angle = a;
                }
            }

            // Value mapping
            float t = (angle - MIN_ANGLE) / RANGE;
            interpolateColors(t);
            kvalue = (u32)(t * 100.0f);
        }
    }

    void drawBody(bool active, bool muted) {
        Color baseCol = muted ? Darken(base, 0.35f) : base;
        Color ringCol = muted ? Darken(ring, 0.4f) : ring;

        if (!muted && (active || hovered))
            DrawCircleV(position, radius * 1.2f, glow);

        DrawCircleV(position, radius, baseCol);

        if (!muted) {
            DrawRing(
                position,
                radius * 0.9f,
                radius * (active ? 1.1f : 1.0f),
                0, 360, 64,
                ringCol
            );
        }
    }

    void drawIndicator(bool active, bool muted) {
        Color ind = muted ? Darken(indicator, 0.4f)
                          : (active ? WHITE : indicator);

        DrawCircle(
            position.x + cosf(angle) * radius * 0.8f,
            position.y + sinf(angle) * radius * 0.8f,
            radius * (active ? 0.1f : 0.07f),
            ind
        );
    }

    void drawValue(bool muted) {
        Color txt = muted ? Darken(RAYWHITE, 0.4f) : RAYWHITE;

        DrawText(
            TextFormat("%03i", kvalue),
            position.x - 18,
            position.y + radius * 1.35f,
            radius * 0.4f,
            txt
        );
    }

    void interpolateColors(float fac) {
        // linear interpolation
        ring.r = (unsigned char)(minring.r + (maxring.r - minring.r)*fac);
        ring.g = (unsigned char)(minring.g + (maxring.g - minring.g)*fac);
        ring.b = (unsigned char)(minring.b + (maxring.b - minring.b)*fac);
        

        glow.r = (unsigned char)(minglow.r + (maxglow.r - minglow.r)*fac);
        glow.g = (unsigned char)(minglow.g + (maxglow.g - minglow.g)*fac);
        glow.b = (unsigned char)(minglow.b + (maxglow.b - minglow.b)*fac);
    }
};

// --------------------------------------------------
// Demo
// --------------------------------------------------
int main() {
    InitWindow(800, 400, "Final Knob (Proper Angle Logic)");
    SetTargetFPS(60);

    Knob k1({200, 200}, 75, true);
    Knob k2({400, 200}, 75, true);
    Knob k3({600, 200}, 75, true);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({35, 35, 35, 255});

        k1.draw();
        k2.draw();
        k3.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}