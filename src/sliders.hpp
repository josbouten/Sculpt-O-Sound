#pragma once
using namespace std;

void initialize_slider_levels(float start_level[NR_OF_BANDS]);
float equal_loudness_value(int);
float min_equal_loudness_value(void);

struct MySlider_01 : SvgSlider 
{
	MySlider_01() 
    {
		Vec margin = Vec(0, 0);
		maxHandlePos = Vec(0, -4).plus(margin);
		minHandlePos = Vec(0, 33).plus(margin);

        background->svg = APP->window->loadSvg(asset::plugin(thePlugin, "res/mschack/mschack_sliderBG_01.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = APP->window->loadSvg(asset::plugin(thePlugin, "res/mschack/mschack_sliderKNOB_01.svg"));
		handle->wrap();
	}
};

struct Slider02_10x15 : SvgSlider
{
	Slider02_10x15() 
    {
		Vec margin = Vec(4, 0);
		maxHandlePos = Vec(-3, 0).plus(margin);
		minHandlePos = Vec(-3, 60).plus(margin);

        background->svg = APP->window->loadSvg(asset::plugin(thePlugin, "res/mschack/mschack_sliderBG_02.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = APP->window->loadSvg(asset::plugin(thePlugin, "res/mschack/mschack_Slider02_10x15.svg"));
		handle->wrap();
	}

    enum Type {
        ATTACK_TIME,
        RELEASE_TIME,
        LEVEL,
        PAN
    };
};

struct SliderWithId: Slider02_10x15
{
    int id;
    Type type;

/*
    void onChange(EventChange &e) override {
      // Do your own thang.
      printf("Slider change detected for type %d slider with number: %d\n", type, id);
      //printf("Slider change detected for %s slider %d\n", typeName[(int)type], id);
      // Pass event to super class.
      SvgSlider::onChange(e);
    }

    void onHoverKey(EventHoverKey &e) override {
        printf("Hovering above %d, %d!\n", id, type);
        EventMouseDown emd;
        emd.button = 1;
        SvgSlider::onMouseDown(emd);
        SvgSlider::onHoverKey(e);
    }
*/
};
