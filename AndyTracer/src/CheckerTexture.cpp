#include "CheckerTexture.h"

Color CheckerTexture::GetTextureColor(float u, float v) const
{
    int x = (int)(u * fils);  
    int y = (int)(v * cols);  

    Color c;
    if (x % 2 == 0 && y % 2 == 0)
        c = texture1->GetTextureColor(u,v);
    else if (x % 2 == 0 && y % 2 != 0)
        c = texture2->GetTextureColor(u, v);
    else if (x % 2 != 0 && y % 2 == 0)
        c = texture2->GetTextureColor(u, v);
    else if (x % 2 != 0 && y % 2 != 0)
        c = texture1->GetTextureColor(u, v);

    return c;
}
