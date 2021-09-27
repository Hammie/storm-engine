#pragma once

#include "../battle_command.h"

class WMShipCommandList : public BICommandList
{
  public:
    WMShipCommandList(ATTRIBUTES &pA, VDX9RENDER &rs);

    void FillIcons() override;

  protected:
    long CommandAdding();
};
