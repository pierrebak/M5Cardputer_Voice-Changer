
/*
 * M5Cardputer High-Pitch Voice Changer with Enhanced UI
 * * This sketch records audio continuously. Pressing 'Go' (the right big button on top of the device ) applies a 
 * * high-pitch effect and plays it back.
 * * The UI has been updated with icons, colors, and a dynamic
 * * recording indicator for a more attractive look.
 * * Hardware: M5Cardputer
 * * Dependencies:
 * - M5Cardputer Board Support Package for Arduino IDE.
 */

#include <M5Cardputer.h>

// ===================================
// == Audio Configuration           ==
// ===================================
static constexpr const size_t record_number     = 512;
static constexpr const size_t record_length     = 240;
static constexpr const size_t record_size       = record_number * record_length;
static constexpr const size_t record_samplerate = 10000; // ~12 second recording

static int16_t *rec_data; // Buffer for recording and processing

// ===================================
// == UI Configuration              ==
// ===================================
#define BG_COLOR      0x002040 // Dark Blue
#define TITLE_COLOR   TFT_CYAN
#define ACCENT_COLOR  TFT_YELLOW
#define TEXT_COLOR    TFT_WHITE
#define REC_COLOR     TFT_RED

// ===================================
// == Function Prototypes           ==
// ===================================
void drawHomeScreen(bool isRecording);
void drawPlaybackScreen();
void drawMicrophoneIcon(int x, int y);
void drawSpeakerIcon(int x, int y);

// ===================================
// == Arduino Setup Function        ==
// ===================================
void setup(void) {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    rec_data = (int16_t*)heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_8BIT);

    if (rec_data == NULL) {
        M5Cardputer.Lcd.fillScreen(BLACK);
        M5Cardputer.Lcd.setTextColor(RED);
        M5Cardputer.Lcd.setTextSize(2);
        M5Cardputer.Lcd.setCursor(10, 10);
        M5Cardputer.Lcd.println("Failed to allocate memory!");
        while(1);
    }

    memset(rec_data, 0, record_size * sizeof(int16_t));
    M5Cardputer.Speaker.setVolume(255);

    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();

    drawHomeScreen(true);
}

// ===================================
// == Arduino Loop Function         ==
// ===================================
void loop(void) {
    M5Cardputer.update();

    // Continuously record audio into the circular buffer
    if (M5Cardputer.Mic.isEnabled()) {
        static size_t rec_record_idx = 0;
        M5Cardputer.Mic.record(&rec_data[rec_record_idx * record_length], record_length, record_samplerate);
        if (++rec_record_idx >= record_number) {
            rec_record_idx = 0;
        }

        // Blinking REC indicator for visual feedback
        static uint32_t lastBlink = 0;
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            static bool recVisible = true;
            M5Cardputer.Lcd.setTextColor(recVisible ? REC_COLOR : BG_COLOR, BG_COLOR);
            M5Cardputer.Lcd.setTextSize(2);
            M5Cardputer.Lcd.setCursor(180, 10);
            M5Cardputer.Lcd.print("REC");
            recVisible = !recVisible;
        }
    }

    // When the 'A' button is clicked, play the sound with the high-pitch effect
    if (M5Cardputer.BtnA.wasClicked()) {
        M5Cardputer.Mic.end();
        
        drawPlaybackScreen();
        M5Cardputer.Speaker.begin();

        uint32_t playback_rate = record_samplerate * 1.5;
        M5Cardputer.Speaker.playRaw(rec_data, record_size, playback_rate);

        while (M5Cardputer.Speaker.isPlaying()) {
            M5Cardputer.update();
            delay(10);
        }

        M5Cardputer.Speaker.end();
        M5Cardputer.Mic.begin();
        drawHomeScreen(true);
    }
}

// ===================================
// == Display Functions             ==
// ===================================
void drawHomeScreen(bool isRecording) {
    M5Cardputer.Lcd.fillScreen(BG_COLOR);
    
    // Title
    M5Cardputer.Lcd.setTextColor(TITLE_COLOR, BG_COLOR);
    M5Cardputer.Lcd.setTextSize(2);
    M5Cardputer.Lcd.setCursor(20, 10);
    M5Cardputer.Lcd.println("Voice Changer");
    M5Cardputer.Lcd.drawFastHLine(10, 35, 220, TFT_DARKGREY);

    // Microphone Icon
    drawMicrophoneIcon(105, 50);

    // Instructions
    M5Cardputer.Lcd.setTextColor(TEXT_COLOR, BG_COLOR);
    M5Cardputer.Lcd.setTextSize(1);
    M5Cardputer.Lcd.setCursor(25, 115);
    M5Cardputer.Lcd.println("Press 'Go' to play");
}

void drawPlaybackScreen() {
    M5Cardputer.Lcd.fillScreen(BG_COLOR);

    // Title
    M5Cardputer.Lcd.setTextColor(ACCENT_COLOR, BG_COLOR);
    M5Cardputer.Lcd.setTextSize(3);
    M5Cardputer.Lcd.setCursor(45, 10);
    M5Cardputer.Lcd.println("PLAYING");
    M5Cardputer.Lcd.drawFastHLine(10, 45, 220, TFT_DARKGREY);

    // Speaker Icon
    drawSpeakerIcon(105, 60);
}

void drawMicrophoneIcon(int x, int y) {
    M5Cardputer.Lcd.fillRoundRect(x, y, 30, 50, 15, TEXT_COLOR);
    M5Cardputer.Lcd.fillRect(x+5, y-5, 20, 10, TEXT_COLOR);
    M5Cardputer.Lcd.fillRect(x-5, y+50, 40, 5, TEXT_COLOR);
    M5Cardputer.Lcd.fillRect(x+12, y+55, 6, 10, TEXT_COLOR);
}

void drawSpeakerIcon(int x, int y) {
    M5Cardputer.Lcd.fillRect(x, y, 20, 40, TEXT_COLOR);
    M5Cardputer.Lcd.fillTriangle(x, y, x-20, y+10, x, y+40, TEXT_COLOR);
    M5Cardputer.Lcd.drawFastVLine(x+25, y+10, 20, TEXT_COLOR);
    M5Cardputer.Lcd.drawFastVLine(x+30, y+5, 30, TEXT_COLOR);
}

