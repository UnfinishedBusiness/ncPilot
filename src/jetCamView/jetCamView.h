#ifndef JET_CAM_VIEW_
#define JET_CAM_VIEW_

#include <application.h>
#include "DXFParsePathAdaptor/DXFParsePathAdaptor.h"
#include <dxf/dxflib/dl_dxf.h>

class jetCamView{
    private:
        struct duplicate_part_t{
            bool visable;
        };
        struct preferences_data_t{
            float background_color[3] = { 0.0f, 0.0f, 0.0f };
        };
        EasyRender::EasyRenderGui *menu_bar;
        static void ZoomEventCallback(nlohmann::json e);
        static void ViewMatrixCallback(PrimativeContainer *p);
        static void MouseCallback(nlohmann::json e);
        static void MouseEventCallback(PrimativeContainer* c,nlohmann::json e);
        static void KeyCallback(nlohmann::json e);
        static void RenderUI(void *p);
        
        double_point_t last_mouse_click_position;
        bool left_click_state;
        bool tab_state;
        #define JETCAM_TOOL_CONTOUR 0
        #define JETCAM_TOOL_NESTING 1
        #define JETCAM_TOOL_POINT 2
        int CurrentTool;
        FILE *dxf_fp;
        DL_Dxf *dl_dxf;
        DXFParsePathAdaptor *DXFcreationInterface;
        bool DxfFileOpen(std::string filename, std::string name);
        static bool DxfFileParseTimer(void *p);

        float ProgressWindowProgress;
        EasyRender::EasyRenderGui *ProgressWindowHandle;
        void ShowProgressWindow(bool v);
        static void RenderProgressWindow(void *p);
        
    public:

        preferences_data_t preferences;
        EasyPrimative::Box *material_plane;

        jetCamView(){
            dxf_fp = NULL;
        };
        void PreInit();
        void Init();
        void Tick();
        void MakeActive();
        void Close();
};

#endif