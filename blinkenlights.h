// goes in config/esphome folder with ESPHome YAML files

static const int DigitCount = 10;
static const int DigitWidth = 3;
static const int DigitHeight = 4;
static const bool DigitMap[DigitCount][DigitHeight][DigitWidth] = {
    {
        // 0
        {1, 1, 1},
        {1, 0, 1},
        {1, 0, 1},
        {1, 1, 1}
    },
    {
        // 1
        {0, 1, 0},
        {1, 1, 0},
        {0, 1, 0},
        {1, 1, 1}
    },
    {
        // 2
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 0},
        {1, 1, 1}
    },
    {
        // 3
        {1, 1, 1},
        {0, 0, 1},
        {0, 1, 1},
        {1, 1, 1}
    },
    {
        // 4
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 1},
        {0, 0, 1}
    },
    {
        // 5
        {1, 1, 1},
        {1, 1, 0},
        {0, 0, 1},
        {1, 1, 0}
    },
    {
        // 6
        {0, 1, 1},
        {1, 0, 0},
        {1, 1, 1},
        {1, 1, 1}
    },
    {
        // 7
        {1, 1, 1},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 0}
    },
    {
        // 8
        {1, 1, 1},
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
    },
    {
        // 9
        {1, 1, 1},
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 0}
    }
};

// will be affected by lots of factors
static esphome::Color WARMWHITE(255, 180, 94);
static esphome::Color YELLOW(230, 184, 0);
static esphome::Color AMBERBRIGHT(255, 153, 0);
static esphome::Color AMBER(230, 132, 0);
static esphome::Color AMBERDIM(194, 117, 0);
static esphome::Color AMBERORANGE(255, 153, 0);
static esphome::Color ORANGE(230, 127, 0);
static esphome::Color ORANGEDIMMEST(204, 117, 0);
static esphome::Color BLACK(0, 0, 0);
static esphome::Color Colors[] = {YELLOW, AMBERBRIGHT, AMBER, AMBERDIM, AMBERORANGE, ORANGE, ORANGEDIMMEST};
static const int ColorCount = 7;

struct Blinkendigit {
    int value;
    int x;
    int y;
};

class Blinkenlights {
    private:
        // display state
        esphome::display::Display* _display;
        int _width;
        int _height;
        esphome::Color **_pixelColors;
        // blink state
        int **_pixelDelay;
        int _speed = 0;
        int _baseDelay = 100;
        // value display state
        Blinkendigit* _valueDigits;
        int _valueDigitsCount = 0;
        int _valueDelay = 0;

    public:
        Blinkenlights(esphome::display::Display* display, int width, int height) {
            _display = display;
            _width = width;
            _height = height;
            _pixelColors = new esphome::Color*[_width];
            _pixelDelay = new int*[_width];

            for (int w = 0; w < width; w++)
            {
                _pixelColors[w] = new esphome::Color[_height];
                _pixelDelay[w] = new int[_height];

                for (int h = 0; h < height; h++) {
                    _pixelColors[w][h] = Colors[random(0, ColorCount)];
                    _pixelDelay[w][h] = random(_baseDelay * 4, _baseDelay * 8);
                    _display->draw_pixel_at(w, h, _pixelColors[w][h]);
                }
            }
        }

        // TODO: ~Blinkenlights() {}

        void showDigits(Blinkendigit digits[], int count, int delay = 500) {
            _valueDigits = digits;
            _valueDigitsCount = max(0, count);
            _valueDelay = max(0, delay);

            for (int i = 0; i < _valueDigitsCount; i++) {
                ESP_LOGD("blinkenlights", "[%d]: {%d, %d, %d}", i, _valueDigits[i].value, _valueDigits[i].x, _valueDigits[i].y);
            };
        }

        void setSpeed(int speed) {
            _speed = speed;
            _baseDelay = (0.013 * speed * speed) - (2.25 * speed) + 100;
        }

        void refresh() {
            if (_valueDelay > 0)
                _valueDelay--;
            else
                _valueDigitsCount = 0;

            for (int w = 0; w < _width; w++)
            {
                for (int h = 0; h < _height; h++) {
                    if (--_pixelDelay[w][h] > 0)
                        continue;

                    // check if in digit
                    int digitIndex = -1;
                    for (int i = 0; i < _valueDigitsCount; i++) {
                        if (w >= _valueDigits[i].x && w < _valueDigits[i].x + DigitWidth &&
                            h >= _valueDigits[i].y && h < _valueDigits[i].y + DigitHeight) {
                            digitIndex = i;
                            break;
                        }
                    }

                    // display digit
                    if (_valueDelay > 0 && digitIndex > -1) {
                        Blinkendigit digit = _valueDigits[digitIndex];
                        _pixelColors[w][h] = DigitMap[digit.value][h - digit.y][w - digit.x] ? WARMWHITE : BLACK;
                    }
                    // or toggle blinky
                    else {
                        _pixelColors[w][h] = (_pixelColors[w][h] == BLACK || _pixelColors[w][h] == WARMWHITE)
                            ? Colors[random(0, ColorCount)]
                            : BLACK;
                    }

                    _pixelDelay[w][h] = random(_baseDelay, _baseDelay * 2);
                    _display->draw_pixel_at(w, h, _pixelColors[w][h]);
                }
            }
        }
};
