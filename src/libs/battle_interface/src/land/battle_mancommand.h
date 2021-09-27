#pragma once

#include "../battle_command.h"

class BIManCommandList : public BICommandList
{
  public:
    BIManCommandList(ATTRIBUTES &pA, VDX9RENDER &rs);

    void FillIcons() override;
    void Init() override;

  protected:
    long CommandAdding();
    long UserIconsAdding();
    long AbilityAdding();
    long AddCancelIcon();
};
