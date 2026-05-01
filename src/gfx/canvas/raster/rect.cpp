#include "rasterCanvas.hpp"

void wgfx::RasterCanvas::rectRoundedFlatAligned(RectCommand const &cmd)
{
    Rgba8 color = cmd.paint.color.toRgba8();
    float roundeness = core::min(cmd.radius, 1.0f);

    float radius = cmd.rect.width() > cmd.rect.height() ? cmd.rect.width() : cmd.rect.height();
    radius *= roundeness / 2.f;
    for (long y = 0; y < cmd.rect.height(); y++)
    {

        // x = r cos ((y / r) * pi/2) for the top left
        // x = width - r cos ((y / r) * pi/2) for the top right
        // x =  r cos (((height-y) / r) * pi/2) for the bottom left
        // x = width - r cos (((height-y) / r) * pi/2) for the bottom right

        float start_x = cmd.rect.start.x;
        float end_x = cmd.rect.end.x;


        // top
        if(y <= radius)
        {
          //  start_x  = radius * cosf(((float)y/radius) * M_PI);
            start_x  = radius -  sqrtf(radius * radius - (radius - (float)y) * (radius - (float)y));
            //log::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;

        }
        else if(y >= cmd.rect.height() - radius)
        {

            start_x  = radius -  sqrtf(radius * radius - (cmd.rect.height() - radius - (float)y) * (cmd.rect.height() - radius - (float)y));
            //log::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;

       }
        else
        {
            start_x = 0;
            end_x = cmd.rect.width();
        }



        float fs2 = floorf(end_x);
        float fs1 = floorf(start_x);
        for (long x = (long)(start_x + 1.f); x < (long)fs2; x++)
        {
            colorize(x + cmd.rect.start.x, y + cmd.rect.start.y, color);
        }

        blend((long)start_x + cmd.rect.start.x, y + cmd.rect.start.y, color, (1.0f - ((start_x)-fs1)));
        blend((long)end_x + cmd.rect.start.x, y + cmd.rect.start.y, color, ((end_x - fs2)));
    }
}
void wgfx::RasterCanvas::rectFlatAligned(RectCommand const &cmd)
{
    Rgba8 color = cmd.paint.color.toRgba8();
       for (long y = cmd.rect.start.y; y < cmd.rect.end.y; y++)
       {
           for (long x = cmd.rect.start.x; x < cmd.rect.end.y; x++)
           {
               buffer[x + y * width] = color;
           }
       }
}

void wgfx::RasterCanvas::rect(RectCommand const &cmd)
{

    if (cmd.radius <= 0.0001f)
    {
        rectFlatAligned(cmd);
    }
    else
    {
        rectRoundedFlatAligned(cmd);
    }
}
