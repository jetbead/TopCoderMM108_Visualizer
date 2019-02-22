#ifndef GRID_VISUALIZER_HH
#define GRID_VISUALIZER_HH

#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace grid_visualizer {

struct GridInfo {
  int H, W;
  std::string message;
  std::vector<std::string> field;
  GridInfo() : H(0), W(0) {}
  GridInfo(int H, int W) : H(H), W(W), field(H, std::string(W, ' ')) {}
};

class GridVisualizer {
  const int WINDOW_HEIGHT = 800;
  const int WINDOW_WIDTH = 800;
  const int FONT_SIZE = 12;
  const char* FONT_PATH = "/Library/Fonts/Arial.ttf";  // for mac

  SDL_Window* mWindow;
  SDL_Renderer* mRenderer;
  TTF_Font* mFont;

  bool mIsRunning;
  int mTicksCount;

  std::vector<GridInfo> history;
  int mIndex;

  void initialize();
  void finalize();
  void processInput();
  void update();
  void render();

  void renderText(const std::string& text, int y, int x, Uint8 r, Uint8 g,
                  Uint8 b);

 public:
  GridVisualizer();
  ~GridVisualizer();

  void addGridInfo(const GridInfo& info);
  void run();
};

GridVisualizer::GridVisualizer()
    : mWindow(nullptr), mIsRunning(true), mTicksCount(0), mIndex(0) {
  initialize();
}
GridVisualizer::~GridVisualizer() { finalize(); }

void GridVisualizer::initialize() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "Error: cannot initialize SDL" << std::endl;
    std::cerr << SDL_GetError() << std::endl;
    return;
  }

  if (TTF_Init() != 0) {
    std::cerr << "Error: cannot initialize TTF" << std::endl;
    std::cerr << TTF_GetError() << std::endl;
    return;
  }

  if (!(mWindow = SDL_CreateWindow("Grid Visualizer", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, WINDOW_HEIGHT,
                                   WINDOW_WIDTH, SDL_WINDOW_OPENGL))) {
    std::cerr << "Error: cannot create window" << std::endl;
    std::cerr << SDL_GetError() << std::endl;
    return;
  }

  mRenderer = SDL_CreateRenderer(
      mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!(mFont = TTF_OpenFont(FONT_PATH, FONT_SIZE))) {
    std::cerr << "Error: cannot load font" << std::endl;
    std::cerr << TTF_GetError() << std::endl;
    return;
  }
}

void GridVisualizer::finalize() {
  SDL_DestroyRenderer(mRenderer);
  SDL_DestroyWindow(mWindow);
  TTF_CloseFont(mFont);
  TTF_Quit();
  SDL_Quit();
}

void GridVisualizer::addGridInfo(const GridInfo& info) {
  history.emplace_back(info);
}

void GridVisualizer::run() {
  while (mIsRunning) {
    processInput();
    update();
    render();
  }
}

void GridVisualizer::processInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        mIsRunning = false;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          mIsRunning = false;
        }
        int speed = 1;
        if (event.key.keysym.mod & KMOD_SHIFT) {
          speed = 10;
        }
        if (event.key.keysym.sym == SDLK_LEFT) {
          mIndex = std::max(0, mIndex - speed);
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
          mIndex = std::min((int)history.size() - 1, mIndex + speed);
        }
        if (event.key.keysym.sym == SDLK_a) {
          mIndex = 0;
        }
        if (event.key.keysym.sym == SDLK_l) {
          mIndex = (int)history.size() - 1;
        }
        break;
    }
  }
}

void GridVisualizer::update() {
  while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 33))
    ;

  // deltaTime base update
  double deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0;
  if (deltaTime > 0.05) deltaTime = 0.05;
  mTicksCount = SDL_GetTicks();
  // ...
}

void GridVisualizer::render() {
  // background clear
  SDL_SetRenderDrawColor(mRenderer, 240, 248, 255, 255);
  SDL_RenderClear(mRenderer);

  // no history
  if (history.size() == 0) {
    SDL_RenderPresent(mRenderer);
    return;
  }

  // render info message
  std::stringstream ss;
  ss << "Index " << mIndex + 1 << " / " << history.size() << ": "
     << history[mIndex].message;
  renderText(ss.str().c_str(), 2, 2, 0, 0, 0);

  // render grid
  int base_y = FONT_SIZE + 10, base_x = 1;
  const int cell_size = FONT_SIZE + 2;
  for (int i = 0; i < history[mIndex].H; i++) {
    for (int j = 0; j < history[mIndex].W; j++) {
      // render cell
      /// outer box
      SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
      SDL_Rect cell_back_outer =
          (SDL_Rect){base_x + j * cell_size, base_y + i * cell_size,
                     cell_size + 1, cell_size + 1};
      SDL_RenderFillRect(mRenderer, &cell_back_outer);

      /// inner box
      if (history[mIndex].field[i][j] == '-')
        SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
      else
        SDL_SetRenderDrawColor(mRenderer, 230, 230, 250, 255);
      SDL_Rect cell_back_inner =
          (SDL_Rect){base_x + j * cell_size + 1, base_y + i * cell_size + 1,
                     cell_size - 1, cell_size - 1};
      SDL_RenderFillRect(mRenderer, &cell_back_inner);

      /// inner charactor
      if (history[mIndex].field[i][j] != '-') {
        std::stringstream tt;
        tt << history[mIndex].field[i][j];
        renderText(tt.str().c_str(), base_x + j * cell_size + 3,
                   base_y + i * cell_size + 1, 0, 0, 0);
      }
    }
  }

  SDL_RenderPresent(mRenderer);
}

void GridVisualizer::renderText(const std::string& text, int x, int y, Uint8 r,
                                Uint8 g, Uint8 b) {
  SDL_Color color = {r, g, b};
  SDL_Surface* surface = TTF_RenderUTF8_Blended(mFont, text.c_str(), color);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
  int ww, hh;
  SDL_QueryTexture(texture, nullptr, nullptr, &ww, &hh);
  SDL_Rect textRect = (SDL_Rect){0, 0, ww, hh};
  SDL_Rect pasteRect = (SDL_Rect){x, y, ww, hh};
  SDL_RenderCopy(mRenderer, texture, &textRect, &pasteRect);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}
};  // namespace grid_visualizer
#endif
