#ifndef JET_CAM_VIEW_
#define JET_CAM_VIEW_

#include <application.h>
#include "DXFParseAdaptor/DXFParseAdaptor.h"
#include <dxf/dxflib/dl_dxf.h>

class jetCamView{
    private:
        struct preferences_data_t{
            float background_color[3] = { 0.0f, 0.0f, 0.0f };
        };
        EasyRender::EasyRenderGui *menu_bar;
        static void ZoomEventCallback(nlohmann::json e);
        static void ViewMatrixCallback(PrimativeContainer *p);
        static void MouseEventCallback(PrimativeContainer* c,nlohmann::json e);
        static void RenderUI(void *p);

        FILE *dxf_fp;
        DL_Dxf *dl_dxf;
        DXFParseAdaptor *DXFcreationInterface;
        bool DxfFileOpen(std::string filename);
        static bool DxfFileParseTimer(void *p);
        
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