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
    if (this->type == "path")
    {
        this->path->process_mouse(mpos_x, mpos_y);
        if (this->path->mouse_event != NULL)
        {
            if (this->path->properties->mouse_callback != NULL)
            {
                this->path->properties->mouse_callback(this, this->path->mouse_event);
            }
            this->path->mouse_event = NULL;
        }
    }
    if (this->type == "arc")
    {
        this->arc->process_mouse(mpos_x, mpos_y);
        if (this->arc->mouse_event != NULL)
        {
            if (this->arc->properties->mouse_callback != NULL)
            {
                this->arc->properties->mouse_callback(this, this->arc->mouse_event);
            }
            this->arc->mouse_event = NULL;
        }
    }
    if (this->type == "circle")
    {
        this->circle->process_mouse(mpos_x, mpos_y);
        if (this->circle->mouse_event != NULL)
        {
            if (this->circle->properties->mouse_callback != NULL)
            {
                this->circle->properties->mouse_callback(this, this->circle->mouse_event);
            }
            this->circle->mouse_event = NULL;
        }
    }
    if (this->type == "box")
    {
        this->box->process_mouse(mpos_x, mpos_y);
        if (this->box->mouse_event != NULL)
        {
            if (this->box->properties->mouse_callback != NULL)
            {
                this->box->properties->mouse_callback(this, this->box->mouse_event);
            }
            this->box->mouse_event = NULL;
        }
    }
}
void PrimativeContainer::render()
{
    if (this->properties->matrix_callback != NULL)
    {
        this->properties->matrix_callback(this);
    }
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
    if (this->type == "path")
    {
        this->path->render();
    }
    if (this->type == "arc")
    {
        this->arc->render();
    }
    if (this->type == "circle")
    {
        this->circle->render();
    }
    if (this->type == "box")
    {
        this->box->render();
    }
}