#ifndef __XPICTURES_H__
#define __XPICTURES_H__

#include "..\\common_h\\attributes.h"

class CXPICTURES
{
  public:
    CXPICTURES();
    ~CXPICTURES();

    void SetPictures(Attribute *pAttributes);
    void ChangePicture(char *sAttributeName, char *sNewImageName);
    void ReleaseAllPictures();
};

#endif
