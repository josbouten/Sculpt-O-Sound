#pragma once

struct ButtonUp : SvgSwitch
{
    ButtonUp()
    {
        //momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnUp.svg")));
        sw->wrap();
    }
};

struct ButtonDown: SvgSwitch
{
    ButtonDown() 
    {
        //momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnDwn.svg")));
        sw->wrap();
    }
};

struct ButtonCenter: SvgSwitch
{
    ButtonCenter() 
    {
        //momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/tresamigos/BtnCenter.svg")));
        sw->wrap();
    }
};
