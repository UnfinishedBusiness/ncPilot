#include "PrimativeContainer.h"
#include "logging/loguru.h"

void PrimativeContainer::process_mouse(float mpos_x, float mpos_y)
{
    if (this->type == "line")
    {
        this->line->process_mouse(mpos_x, mpos_y);
        if (this->line->mouse_event != NULL)
        {
            if (this->line->properties->mouse_callback != NULL)
            {
                this->line->properties->mouse_callback(this, this->line->mouse_event);
            }
            this->line->mouse_event = NULL;
        }
    }
}
void PrimativeContainer::render()
{
    if (this->type == "line")
    {
        this->line->render();
    }
}