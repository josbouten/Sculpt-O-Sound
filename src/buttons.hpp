#pragma once

struct ButtonUp : SvgSwitch
{
    ButtonUp()
    {
        momentary = true;
        // The first svg is the one shown when the button is not pressed.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnUp_1.svg")));
        // The second svg is the one shown when the button is pressed and held down.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnUp_2.svg")));
        sw->wrap();
    }
};

struct ButtonDown: SvgSwitch
{
    ButtonDown() 
    {
        momentary = true;
        // The first svg is the one shown when the button is not pressed.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnDwn_1.svg")));
        // The second svg is the one shown when the button is pressed and held down.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnDwn_2.svg")));
        sw->wrap();
    }
};

struct ButtonCenter: SvgSwitch
{
    ButtonCenter() 
    {
        momentary = true;
        // The first svg is the one shown when the button is not pressed.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnCenter_1.svg")));
        // The second svg is the one shown when the button is pressed and held down.
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnCenter_2.svg")));
        sw->wrap();
    }
};
