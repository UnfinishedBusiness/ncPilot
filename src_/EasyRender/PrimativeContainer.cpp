#include "PrimativeContainer.h"

void PrimativeContainer::process_mouse(float mpos_x, float mpos_y)
{
    if (this->type == "line")
    {
        this->line->process_mouse(mpos_x, mpos_y);
        if (this->line->mouse_event != NULL)
        {
            this->properties->mouse_callback(this, this->line->mouse_event);
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