#include "rack.hpp"

#pragma once

// For debug purposes only !
//#define DEBUG

using namespace rack;

template <typename BASE>
struct LedLight : BASE {
    LedLight() {
      //this->box.size = Vec(20.0, 20.0);
      this->box.size = mm2px(Vec(6.0, 6.0));
    }
};

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file

extern Model *modelVocode_O_Matic;

#include "dsp/digital.hpp"
#include <sstream>
#include <iomanip>

struct MsDisplayWidget : TransparentWidget {
    
  int *value;
  std::shared_ptr<Font> font;
    
  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };
    
  void draw(NVGcontext *vg) override
  { 
    // Background
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
    NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.5);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);
    
    std::stringstream to_display;
    to_display << std::right  << std::setw(5) << *value;
    
    Vec textPos = Vec(4.0f, 17.0f);
    
    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);
    
    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);
    
    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  } 
};  

struct MsDisplayWidget1 : TransparentWidget {
    
  int *value;
  std::shared_ptr<Font> font;
    
  MsDisplayWidget1() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };
    
  void draw(NVGcontext *vg) override
  { 
    // Background
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
    NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 1.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.5);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);
    
    std::stringstream to_display;
    to_display << std::right  << std::setw(1) << *value;
    
    Vec textPos = Vec(4.0f, 17.0f);
    
    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 8));
    nvgText(vg, textPos.x, textPos.y, "~", NULL);
    
    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 8));
    nvgText(vg, textPos.x, textPos.y, "\\", NULL);
    
    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  } 
};  
struct MsDisplayWidget2 : TransparentWidget {
    
  int *value;
  std::shared_ptr<Font> font;
    
  MsDisplayWidget2() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };
    
  void draw(NVGcontext *vg) override
  { 
    // Background
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
    NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 2.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.5);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);
    
    std::stringstream to_display;
    to_display << std::right  << std::setw(2) << *value;
    
    Vec textPos = Vec(4.0f, 17.0f);
    
    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 8));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);
    
    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 8));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);
    
    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  } 
};  
