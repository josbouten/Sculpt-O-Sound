/*
struct LButton : SvgSwitch, MomentarySwitch {
    LButton() {
        addFrame(SVG::load(assetPlugin(plugin, "res/L.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/Ldown.svg")));
        sw->wrap();
        box.size = sw->box.size;
    }

    void onMouseDown(EventMouseDown &e) override {
       //if ( rack::windowIsShiftPressed() ) { 
           //printf("shift pressed\n");
        //}
#ifdef DEBUGMSG
       if (e.button == 1) {
            printf("Button pressed down\n");
       } else {
            printf("Button value %d\n", e.button);
       }
#endif
       SvgSwitch::onMouseDown(e);
    }

    void onHoverKey(EventHoverKey &e) override {
        printf("Hovering!\n");
        EventMouseDown emd;
        emd.button = 1;
        SvgSwitch::onMouseDown(emd);
        SvgSwitch::onHoverKey(e);
    }
    
    void onChange(EventChange &e) override {
      // Do your own thang.
      //printf("Button change detected!\n");
      // Pass event to super class.
      SvgSwitch::onChange(e);
    }
};

*/
