## How to compile addon

Sources contains all necessary libraries for compiling addon. Simply open solution in VisualStudio 2015, switch to ```Release``` profile, and all should be done successfully. Binary dll-s are in archive in release page.


## How to use for other integrations

This repository can be used for creating integrations of other renderers into Softimage. Delete the folder ```render_osp``` and you will obtain minimal universal integration of empty renderer. It contains the following features:
* Control all states of the render process
* Call update only for objects which changed between two render sessions
* Save output image by using [stb_write](https://github.com/nothings/stb) and [tinyexr](https://github.com/syoyo/tinyexr) libraries

To integrate another render you should create the inheritance class of the ```RenderEngineBase``` class and implement the following virtual functions:
```
virtual XSI::CStatus render_option_define(XSI::CustomProperty &property);
virtual XSI::CStatus render_options_update(XSI::PPGEventContext &event_context);
virtual XSI::CStatus render_option_define_layout(XSI::Context &context);

virtual XSI::CStatus pre_render_engine();
virtual XSI::CStatus pre_scene_process();  
virtual XSI::CStatus create_scene();
virtual XSI::CStatus update_scene(XSI::X3DObject &xsi_object, const UpdateType update_type); 
virtual XSI::CStatus update_scene(const XSI::Material &xsi_material);
virtual XSI::CStatus update_scene(const XSI::SIObject &si_object, const UpdateType update_type);
virtual XSI::CStatus update_scene_render();
virtual void render();
virtual XSI::CStatus post_render_engine();
virtual void abort();
virtual void clear_engine();
```

See comments and example to understand what each function should do. Additionally you should change the name of the addon for all callbacks in ```si_callbacks\si_callbacks.cpp``` file (and also change the name in the main start-point file ```SoftimageOspRay.cpp```).