#pragma once

class AppBase
{
public:
  virtual ~AppBase() = 0;

  // App component start up & shutdown
  virtual bool Init() = 0;
  virtual void Exit() = 0;

  // App components load & unload
  virtual bool Load() = 0;
  virtual void Unload() = 0;

  // App logic & drawing
  virtual void Update(float dt) = 0;
  virtual void Draw() = 0;
  
  // Inherit from and extend as required in derived class
  struct PropertiesBase {
    int width = -1;
    int height = -1;
    bool fullscreen = false;
  };
};
