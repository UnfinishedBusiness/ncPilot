#include "PrimativeContainer.h"
#include "../logging/loguru.h"

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
    if (this->type == "part")
    {
        this->part->process_mouse(mpos_x, mpos_y);
        if (this->part->mouse_event != NULL)
        {
            if (this->part->properties->mouse_callback != NULL)
            {
                this->part->properties->mouse_callback(this, this->part->mouse_event);
            }
            this->part->mouse_event = NULL;
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
    if (this->type == "part")
    {
        this->part->render();
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
void PrimativeContainer::destroy()
{
    if (this->type == "line")
    {
        this->line->destroy();
        delete this->line;
    }
    if (this->type == "text")
    {
        this->text->destroy();
        delete this->text;
    }
    if (this->type == "image")
    {
        this->image->destroy();
        delete this->image;
    }
    if (this->type == "path")
    {
        this->path->destroy();
        delete this->path;
    }
    if (this->type == "part")
    {
        this->part->destroy();
        delete this->part;
    }
    if (this->type == "arc")
    {
        this->arc->destroy();
        delete this->arc;
    }
    if (this->type == "circle")
    {
        this->circle->destroy();
        delete this->circle;
    }
    if (this->type == "box")
    {
        this->box->destroy();
        delete this->box;
    }
}
nlohmann::json PrimativeContainer::serialize()
{
    nlohmann::json j;
    if (this->type == "line")
    {
        j = this->line->serialize();
    }
    if (this->type == "text")
    {
        j = this->text->serialize();
    }
    if (this->type == "image")
    {
        j = this->image->serialize();
    }
    if (this->type == "path")
    {
        j = this->path->serialize();
    }
    if (this->type == "part")
    {
        j = this->part->serialize();
    }
    if (this->type == "arc")
    {
        j = this->arc->serialize();
    }
    if (this->type == "circle")
    {
        j = this->circle->serialize();
    }
    if (this->type == "box")
    {
        j = this->box->serialize();
    }
    j["type"] = this->type;
    nlohmann::json p;
    p["visable"] = this->properties->visable;
    p["zindex"] = this->properties->zindex;
    p["color"]["r"] = this->properties->color[0];
    p["color"]["g"] = this->properties->color[1];
    p["color"]["b"] = this->properties->color[2];
    p["color"]["a"] = this->properties->color[3];
    p["scale"] = this->properties->scale;
    p["offset"]["x"] = this->properties->offset[0];
    p["offset"]["y"] = this->properties->offset[1];
    p["offset"]["z"] = this->properties->offset[2];
    p["angle"] = this->properties->angle;
    p["id"] = this->properties->id;
    p["view"] = this->properties->view;
    p["mouse_over_padding"] = this->properties->mouse_over_padding;
    p["data"] = this->properties->data;
    j["properties"] = p;
    return j;
}