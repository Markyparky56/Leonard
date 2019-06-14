#pragma once
#include "AppBase.hpp"
#include "EASTL\string.h"

class TriangleApp : public AppBase
{
public:
  TriangleApp()
  {
    properties.name = "hello";
  }

  virtual bool Init() override;
  virtual void Exit() override;
  virtual bool Load() override;
  virtual void Unload() override;
  virtual void Update(float dt) override;
  virtual void Draw() override;

  struct Properties : public PropertiesBase
  {
    eastl::string name;
  } properties;
};
