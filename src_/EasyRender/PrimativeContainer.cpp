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
    if (this->type == "text")
    {
        this->text->process_mouse(mpos_x, mpos_y);
        if (this->text->mouse_event != NULL)
        {
            if (this->text->properties->mouse_callback != NULL)
            {
                this->text->properties->mouse_callback(this, this->text->mouse_event);
            }
            this->text->mouse_event = NULL;
        }
    }
    if (this->type == "image")
    {
        this->image->process_mouse(mpos_x, mpos_y);
        if (this->image->mouse_event != NULL)
        {
            if (this->image->properties->mouse_callback != NULL)
            {
                this->image->properties->mouse_callback(this, this->image->mouse_event);
            }
            this->image->mouse_event = NULL;
        }
    }
}
void PrimativeContainer::render()
{
    if (this->type == "line")
    {
        this->line->render();
    }
    if (this->type == "text")
    {
        this->text->render();
    }
    if (this->type == "image")
    {
        this->image->render();
    }
}