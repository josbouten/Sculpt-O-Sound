#pragma once

//-----------------------------------------------------
// Sliders
//
//-----------------------------------------------------
struct MySlider_01 : SVGFader 
{
	MySlider_01() 
    {

		Vec margin = Vec(0, 0);
		maxHandlePos = Vec(0, -4).plus(margin);
		minHandlePos = Vec(0, 33).plus(margin);

        background->svg = SVG::load(assetPlugin(plugin,"res/mschack/mschack_sliderBG_01.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = SVG::load(assetPlugin(plugin,"res/mschack/mschack_sliderKNOB_01.svg"));
		handle->wrap();
	}
};

struct Slider02_10x15 : SVGFader 
{
	Slider02_10x15() 
    {
		Vec margin = Vec(4, 0);
		maxHandlePos = Vec(-3, 0).plus(margin);
		minHandlePos = Vec(-3, 60).plus(margin);

        background->svg = SVG::load(assetPlugin(plugin,"res/mschack/mschack_sliderBG_02.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = SVG::load(assetPlugin(plugin,"res/mschack/mschack_Slider02_10x15.svg"));
		handle->wrap();
	}
};

struct Slider02_10x15WithId : Slider02_10x15 
{
    int sliderId;
	Slider02_10x15 thisSlider = Slider02_10x15();

	Slider02_10x15WithId(int id) 
    {
        sliderId = id;
	    Slider02_10x15();
    }
};

