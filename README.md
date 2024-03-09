**Sapphire Engine**

Sapphire Engine is a light-weight 2D game engine with a visual editor

Features:

* Box2D physics simulations.
* Lua scripting.
* Clean UI for the editor.
* Easy to use.

This is a project I made for educational purposes and is clearly not made for anyone to actually make a game and to release it.
Feel free to dive in to the code and have a look for yourself, its really easy to get started and have this as starting point on reading new code.

As of writting this, I'm a 16 year old high school student in Greece interested on computer science and this was an experiment for me in computer graphics
and generally on the technology behind game engines. I started this project because before this, I was making video games using the Unity game engine
and was intrested on how Unity worked.

# Lua scripting refrence

## Introduction

Welcome to the SapphireEngine reference guide! This guide provides documentation for the Lua scripting API of SapphireEngine. SapphireEngine is a powerful game engine that allows you to create games and interactive experiences.

## Lua functions provided

SapphireEngine provides several Lua utility functions that you can use in your scripts. Here's an overview of these utilities:

- `Log(message: string [, type: string])`: Logs a message with an optional type (Info, Warning, Error).
- `KeyPress(key: string)`: Checks if a key is currently pressed.
- `KeyPressDown(key: string)`: Checks if a key was pressed down in the current frame.
- `Click()`: Checks if the left mouse button was clicked.
- `GetMousePos()`: Retrieves the current mouse position.
- `GetObject(objectName: string)`: Retrieves a reference to an object by name.
- `DeleteObject(objectRef: userdata)`: Deletes the specified object from the scene.
- `GetCurrentScene()`: Retrieves the name of the currently active scene.
- `GetCameraPos()`: Retrieves the position of the camera.
- `SetCameraPos(x: number, y: number)`: Sets the position of the camera.
- `OverlapCircle(x: number, y: number, radius: number)`: Performs collision detection with a circle shape.
- `GetCameraSize()`: Retrieves the size of the camera viewport.
- `SetCameraSize(width: number, height: number)`: Sets the size of the camera viewport.
- `GetDeltaTime()`: Retrieves the time elapsed since the last frame.
- `GetTime()`: Retrieves the current game time.
- `LoadScene(sceneName: string)`: Loads a new scene by name.
- `CreateObject(objectName: string, shape: string)`: Creates a new object with the specified name and shape.
- `LoadObjectPrefab(prefabName: string)`: Loads a prefab object by name.
- `Clamp(value: number, min: number, max: number)`: Clamps a value within a specified range.
- `SetObject()`: Its a function to initialize variable as SapphireEngine objects from the lua script so the editor can show the variable accordingly.

## Transform Component

* `Move(float x, float y)` moves the object x units horizontally, y units vertically
* `Rotate(float angle)` rotates the object a certain amount
* `LookAt(float x, float y)` returns an angle in radians that points at a point in space
* `SetPosition(float x, float y)` sets the world position of an object
* `SetSize(float x, float y)` sets the size of an object
* `SetRotation(float angle)` sets the rotation of an object

Warning: The Move and Rotate function differ from SetRotation and SetPosition from that fact that the move function will start from the original position
and offset it by and amount, on the other hand SetPosition ignores the original position and just sets the position value as given.


## Rigidbody Component

* `Impulse(float x, float y)` applies an impulse to the object.
* `Raycast(float x, float y, optional string tag)` sends out a ray from the objects position to the specified point. Returns whether the object hit, the object refrence and the collision point.
* `SetFriction(float amount)` sets the coefficient of friction to an object/
* `SetVelocity(flaot x, flaot y)` sets the velocity of an object.
* `GetVelocity()` returns the velocity of an object.


You can also call functions from only an object

* `RemoveComponent(string name)` removes a component from the name provided
* `AddComponent(string path)` Adds a lua component from a given path (from the relative path to the project's directory)

## Usage

To use these utilities, simply call the corresponding function from your Lua scripts. Here's an example of how you might use the `Log` function:

```lua
SapphireEngine.Log("Hello, world!", "Info")
```
