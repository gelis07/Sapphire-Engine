#include "LuaUtilities.h"
#include "Engine/Engine.h"
#include "Editor/Editor.h"
//* I tried my best at trying to find a more clean way of making these functions but this is the solution I ended up with.

static std::unordered_map<std::string, int> Keys;


int LuaUtilities::log(lua_State* L) {
    int n = lua_gettop(L);
    if (n < 1 || n > 2) {
        return luaL_error(L, "Expected 1 or 2 arguments, got %d", n);
    }

    const char* message = luaL_checkstring(L, 1);
    const char* Type = "Info";
    if (n == 2) {
        Type = luaL_checkstring(L, 2);
    }
    if(std::string(Type) == "Info"){
        SapphireEngine::Log(std::string(message), SapphireEngine::Info);
    }else if(std::string(Type) == "Warning"){
        SapphireEngine::Log(std::string(message), SapphireEngine::Warning);
    }else if(std::string(Type) == "Error"){
        SapphireEngine::Log(std::string(message), SapphireEngine::Error);
    }else{
        std::stringstream error;
        error << Type << " type doesn't exist";
        return luaL_error(L, error.str().c_str());
    }
    return 0;
}
int LuaUtilities::Clamp(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }
    float min = luaL_checknumber(L, 1);
    float max = luaL_checknumber(L, 2);

    float value = std::min(std::max(min,value) , max);
    lua_pushnumber(L, value);

    return 1;
}
int LuaUtilities::GetDeltaTime(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }

    lua_pushnumber(L, Engine::GetDeltaTime());

    return 1;
}
int LuaUtilities::SetObject(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }

    luaL_newmetatable(L, "ObjectMetaTable");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);

    ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
    *ud = ObjectRef(-1);

    luaL_getmetatable(L, "ObjectMetaTable");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int LuaUtilities::OverlapCircle(lua_State *L)
{;
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float radius = (float)luaL_checknumber(L, 3);
    std::vector<SapphirePhysics::RigidBody*> rigidbodies;
    for (auto &&rb : SapphirePhysics::RigidBody::Rigibodies)
    {
        CollisionData cd;
        if(SapphirePhysics::CollisionDetection::StaticCirclexRectangle(rb, glm::vec3(x,y,0), radius, cd))
        {
            rigidbodies.push_back(rb);
        }
    }

    lua_newtable(L);
    for (int i = 0; i < rigidbodies.size(); ++i) {
        ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
        *ud = rigidbodies[i]->Parent;
        luaL_getmetatable(L, "ObjectMetaTable");
        lua_setmetatable(L, -2);
        // Set the ObjectRef userdata into the Lua table at index i + 1
        lua_rawseti(L, -2, i + 1);
    }
    

    return 1;
}
int mult(lua_State *L)
{
    lua_pushstring(L, "x");
    lua_gettable(L, -3);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -3);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);
    float multiplier = luaL_checknumber(L, 2);

    lua_newtable(L);
    lua_pushnumber(L, x * multiplier);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, y*multiplier);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int div(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -3);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -3);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);
    float multiplier = luaL_checknumber(L, 2);
    lua_newtable(L);
    lua_pushnumber(L, x / multiplier);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, y/multiplier);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int norm(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);
    float length = glm::sqrt(pow(x,2)+ pow(y,2));
    lua_newtable(L);
    lua_pushnumber(L, x / length);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, y/ length);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int length(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);
    float length = glm::sqrt(pow(x,2)+ pow(y,2));
    lua_pushnumber(L, length);
    return 1;
}
int tostring(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);

    std::string vector = "x: " + std::to_string(x) + ", y: " + std::to_string(y);
    lua_pushstring(L, vector.c_str());
    return 1;
}
int unm(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float x = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float y = lua_tonumber(L, -1);
    lua_pop(L,1);

    lua_newtable(L);
    lua_pushnumber(L, -x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, -y);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}

int add(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -3);
    float xLeft = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -3);
    float yLeft = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float xRight = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float yRight = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_newtable(L);
    lua_pushnumber(L, xRight + xLeft);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, yRight + yLeft);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int sub(lua_State* L){
    lua_pushstring(L, "x");
    lua_gettable(L, -3);
    float xLeft = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -3);
    float yLeft = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    float xRight = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    float yRight = lua_tonumber(L, -1);
    lua_pop(L,1);
    lua_newtable(L);
    lua_pushnumber(L, xLeft - xRight);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, yLeft - yRight);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int index(lua_State* L){
    lua_newtable(L);
    lua_getfield(L, -2, "x");
    lua_pushnumber(L, lua_tonumber(L, -1));
    lua_setfield(L, -1, "x");
    lua_pop(L, 1);
    lua_getfield(L, -2, "y");
    lua_pushnumber(L, lua_tonumber(L, -1));
    lua_setfield(L, -1, "y");
    lua_pop(L, 1);
    return 1;
}
int LuaUtilities::Vector(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }

    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);

    luaL_newmetatable(L, "Vector");

    // lua_pushstring(L, "__index");
    // lua_pushcfunction(L, index);
    // lua_settable(L, -3);
    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, tostring);
    lua_settable(L, -3);
    lua_pushstring(L, "__unm");
    lua_pushcfunction(L, unm);
    lua_settable(L, -3);
    lua_pushstring(L, "__add");
    lua_pushcfunction(L, add);
    lua_settable(L, -3);
    lua_pushstring(L, "__mul");
    lua_pushcfunction(L, mult);
    lua_settable(L, -3);
    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, sub);
    lua_settable(L, -3);
    lua_pushstring(L, "__div");
    lua_pushcfunction(L, div);
    lua_settable(L, -3);

    lua_newtable(L);

    lua_pushnumber(L, x);
    lua_setfield(L, -2, "x");

    lua_pushcfunction(L, norm);
    lua_setfield(L, -2, "normalize");

    lua_pushcfunction(L, length);
    lua_setfield(L, -2, "length");

    lua_pushnumber(L, y);
    lua_setfield(L, -2, "y");

    luaL_getmetatable(L, "Vector");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    return 1;
}
int LuaUtilities::KeyPress(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }

    const char* message = luaL_checkstring(L, 1);
    if (Keys.find(std::string(message)) != Keys.end()) {
        bool test = Engine::app->GetInput(Keys[std::string(message)]);
        lua_pushboolean(L, test);
    }
    else {
        std::stringstream ss;
        ss << "The key " << std::string(message) << " doesn't exist!"; 
        return luaL_error(L, ss.str().c_str());
    }
    return 1;
}
int LuaUtilities::KeyPressDown(lua_State * L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }

    const char* message = luaL_checkstring(L, 1);
    if (Keys.find(std::string(message)) != Keys.end()) {
        bool test = Engine::app->GetInputDown(Keys[std::string(message)]);
        lua_pushboolean(L, test);
    }
    else {
        std::stringstream ss;
        ss << "The key " << std::string(message) << " doesn't exist!"; 
        return luaL_error(L, ss.str().c_str());
    }
    return 1;
}
int LuaUtilities::Click(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }
    bool test = Engine::app->GetMouseInputDown(GLFW_MOUSE_BUTTON_1);
    lua_pushboolean(L, test);
    return 1;
}
int LuaUtilities::LoadScene(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }

    const char* Scene = luaL_checkstring(L, 1);

    // Engine::GetActiveScene().Load(std::string(Scene) + ".scene");    
    Engine::ShouldLoadScene = std::string(Scene) + ".scene";
    return 0;
}
int LuaUtilities::GetObject(lua_State * L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }

    const char* ObjNameCstr = luaL_checkstring(L, 1);
    std::string ObjName = std::string(ObjNameCstr);
    bool foundObj = false;
    for (size_t i = 0; i < Engine::GetActiveScene().Objects.size(); i++)
    {
        if(ObjName == Engine::GetActiveScene().Objects[i].Name){
            ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
            *ud = Engine::GetActiveScene().Objects[i].GetRef();
            luaL_getmetatable(L, "ObjectMetaTable");
            lua_istable(L, -1);
            lua_setmetatable(L, -2);
            return 1;
        }
    }
    if(!foundObj)
        Log("The object with the name \"" + ObjName + "\" cannot be found", SapphireEngine::Error);
    return 0;
}
int LuaUtilities::DeleteObject(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        SapphireEngine::Log(std::string("Expected 1 argument, got %d", n), SapphireEngine::Error);
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }
    ObjectRef* object = (ObjectRef*)lua_touserdata(L, -1);
    if((*object)->Name == "MainCamera") return 0;
    Engine::GetActiveScene().DeleteRuntime(object->Get());
    return 0;
}
int LuaUtilities::GetCurrentScene(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        SapphireEngine::Log(std::string("Expected 0 argument, got %d", n), SapphireEngine::Error);
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }
    lua_pushstring(L, (Engine::GetActiveScene().SceneFile.substr(0, Engine::GetActiveScene().SceneFile.length() - 6)).c_str());
    return 1;
}
int LuaUtilities::GetCameraPos(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }
    lua_newtable(L);

    const glm::vec3& CameraPos = Engine::GetCameraObject()->GetComponent<Transform>()->GetPosition();
    lua_pushnumber(L, CameraPos.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, CameraPos.y);
    lua_setfield(L, -2, "y");

    return 1;
}
int LuaUtilities::GetMouseCoord(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }
    lua_newtable(L);
    double x,y;
    glfwGetCursorPos(Engine::app->GetWindow(), &x, &y);
    int width, height;
    glfwGetWindowSize(Engine::app->GetWindow(), &width, &height);
    glm::vec2 FinalCoords = (Editor::WindowPos + Editor::WindowSize / 2.0f) - glm::vec2(x,y);
    // glm::vec2 FinalCoords = glm::vec2(width, height) / 2.0f - glm::vec2(x,y);
    FinalCoords = FinalCoords TOUNITS;
    FinalCoords.x *= -1;
    FinalCoords += glm::vec2(Engine::GetCameraObject()->GetComponent<Transform>()->GetPosition());
    lua_pushnumber(L, FinalCoords.x );
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, FinalCoords.y);
    lua_setfield(L, -2, "y");
    return 1;
}
int LuaUtilities::SetCameraPos(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }
    lua_Number x = lua_tonumber(L, -2);
    lua_Number y = lua_tonumber(L, -1);

    Engine::GetCameraObject()->GetComponent<Transform>()->SetPosition(glm::vec3(x, y, 0));
    return 0;
}
int LuaUtilities::GetCameraSize(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 0 argument, got %d", n);
    }
    lua_newtable(L);

    const glm::vec3& CameraSize = Engine::GetCameraObject()->GetComponent<Transform>()->GetSize();
    lua_pushnumber(L, CameraSize.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, CameraSize.y);
    lua_setfield(L, -2, "y");

    return 1;
}
int LuaUtilities::SetCameraSize(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }
    lua_Number x = lua_tonumber(L, -2);
    lua_Number y = lua_tonumber(L, -1);

    Engine::GetCameraObject()->GetComponent<Transform>()->SetSize(glm::vec3(x, y, 0));
    return 0;
}
int LuaUtilities::CreateObject(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }
    const char* ObjName = lua_tostring(L, -2);
    const char* ObjShape = lua_tostring(L, -1);
    Object obj = Object(std::string(ObjName));
    Component::ComponentTypeRegistry["Renderer"](&obj);
    Component::ComponentTypeRegistry["Rigidbody"](&obj);
    ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
    *ud = Engine::GetActiveScene().Add(std::move(obj));
    luaL_getmetatable(L, "ObjectMetaTable");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);

    return 1;
}
int LuaUtilities::GetTime(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_error(L, "Expected 2 argument, got %d", n);
    }

    lua_pushnumber(L, Engine::GameTime);

    return 1;
}
int LuaUtilities::LoadObjectPrefab(lua_State *L)
{
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", n);
    }
    const char* ObjName = lua_tostring(L, -1);

    // std::shared_ptr<Object> obj = Object::CreateObject(std::string(ObjName));
    Object* obj = Object::LoadPrefab(std::string(ObjName) + ".obj");
    lua_pushlightuserdata(L, obj);
    luaL_getmetatable(L, "ObjectMetaTable");
    lua_setmetatable(L, -2);
    return 1;
}

int LuaUtilities::luaopen_SapphireEngine(lua_State *L)
{
    Keys = 
    {
        {"space", GLFW_KEY_SPACE},
        {"apostrophe", GLFW_KEY_APOSTROPHE},
        {"comma", GLFW_KEY_COMMA},
        {"minus", GLFW_KEY_MINUS},
        {"period", GLFW_KEY_PERIOD},
        {"slash", GLFW_KEY_SLASH},
        {"0", GLFW_KEY_0},
        {"1", GLFW_KEY_1},
        {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3},
        {"4", GLFW_KEY_4},
        {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6},
        {"7", GLFW_KEY_7},
        {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9},
        {"semicolon", GLFW_KEY_SEMICOLON},
        {"equal", GLFW_KEY_EQUAL},
        {"a", GLFW_KEY_A},
        {"b", GLFW_KEY_B},
        {"c", GLFW_KEY_C},
        {"d", GLFW_KEY_D},
        {"e", GLFW_KEY_E},
        {"f", GLFW_KEY_F},
        {"g", GLFW_KEY_G},
        {"h", GLFW_KEY_H},
        {"i", GLFW_KEY_I},
        {"j", GLFW_KEY_J},
        {"k", GLFW_KEY_K},
        {"l", GLFW_KEY_L},
        {"m", GLFW_KEY_M},
        {"n", GLFW_KEY_N},
        {"o", GLFW_KEY_O},
        {"p", GLFW_KEY_P},
        {"q", GLFW_KEY_Q},
        {"r", GLFW_KEY_R},
        {"s", GLFW_KEY_S},
        {"t", GLFW_KEY_T},
        {"u", GLFW_KEY_U},
        {"v", GLFW_KEY_V},
        {"w", GLFW_KEY_W},
        {"x", GLFW_KEY_X},
        {"y", GLFW_KEY_Y},
        {"z", GLFW_KEY_Z},
        {"left_bracket", GLFW_KEY_LEFT_BRACKET},
        {"backslash", GLFW_KEY_BACKSLASH},
        {"right_bracket", GLFW_KEY_RIGHT_BRACKET},
        {"grave_accent", GLFW_KEY_GRAVE_ACCENT},
        {"escape", GLFW_KEY_ESCAPE},
        {"enter", GLFW_KEY_ENTER},
        {"tab", GLFW_KEY_TAB},
        {"backspace", GLFW_KEY_BACKSPACE},
        {"insert", GLFW_KEY_INSERT},
        {"delete", GLFW_KEY_DELETE},
        {"right", GLFW_KEY_RIGHT},
        {"left", GLFW_KEY_LEFT},
        {"down", GLFW_KEY_DOWN},
        {"up", GLFW_KEY_UP},
        {"page_up", GLFW_KEY_PAGE_UP},
        {"page_down", GLFW_KEY_PAGE_DOWN},
        {"home", GLFW_KEY_HOME},
        {"end", GLFW_KEY_END},
        {"caps_lock", GLFW_KEY_CAPS_LOCK},
        {"scroll_lock", GLFW_KEY_SCROLL_LOCK},
        {"num_lock", GLFW_KEY_NUM_LOCK},
        {"print_screen", GLFW_KEY_PRINT_SCREEN},
        {"pause", GLFW_KEY_PAUSE},
        {"f1", GLFW_KEY_F1},
        {"f2", GLFW_KEY_F2},
        {"f3", GLFW_KEY_F3},
        {"f4", GLFW_KEY_F4},
        {"f5", GLFW_KEY_F5},
        {"f6", GLFW_KEY_F6},
        {"f7", GLFW_KEY_F7},
        {"f8", GLFW_KEY_F8},
        {"f9", GLFW_KEY_F9},
        {"f10", GLFW_KEY_F10},
        {"f11", GLFW_KEY_F11},
        {"f12", GLFW_KEY_F12},
        {"f13", GLFW_KEY_F13},
        {"f14", GLFW_KEY_F14},
        {"f15", GLFW_KEY_F15},
        {"f16", GLFW_KEY_F16},
        {"f17", GLFW_KEY_F17},
        {"f18", GLFW_KEY_F18},
        {"f19", GLFW_KEY_F19},
        {"f20", GLFW_KEY_F20},
        {"f21", GLFW_KEY_F21},
        {"f22", GLFW_KEY_F22},
        {"f23", GLFW_KEY_F23},
        {"f24", GLFW_KEY_F24},
        {"f25", GLFW_KEY_F25},
        {"kp_0", GLFW_KEY_KP_0},
        {"kp_1", GLFW_KEY_KP_1},
        {"kp_2", GLFW_KEY_KP_2},
        {"kp_3", GLFW_KEY_KP_3},
        {"kp_4", GLFW_KEY_KP_4},
        {"kp_5", GLFW_KEY_KP_5},
        {"kp_6", GLFW_KEY_KP_6},
        {"kp_7", GLFW_KEY_KP_7},
        {"kp_8", GLFW_KEY_KP_8},
        {"kp_9", GLFW_KEY_KP_9},
        {"kp_decimal", GLFW_KEY_KP_DECIMAL},
        {"kp_divide", GLFW_KEY_KP_DIVIDE},
        {"kp_multiply", GLFW_KEY_KP_MULTIPLY},
        {"kp_subtract", GLFW_KEY_KP_SUBTRACT},
        {"kp_add", GLFW_KEY_KP_ADD},
        {"kp_enter", GLFW_KEY_KP_ENTER},
        {"kp_equal", GLFW_KEY_KP_EQUAL},
        {"left_shift", GLFW_KEY_LEFT_SHIFT},
        {"left_control", GLFW_KEY_LEFT_CONTROL},
        {"left_alt", GLFW_KEY_LEFT_ALT},
        {"left_super", GLFW_KEY_LEFT_SUPER},
        {"right_shift", GLFW_KEY_RIGHT_SHIFT},
        {"right_control", GLFW_KEY_RIGHT_CONTROL},
        {"right_alt", GLFW_KEY_RIGHT_ALT},
        {"right_super", GLFW_KEY_RIGHT_SUPER},
        {"menu", GLFW_KEY_MENU}
    };
    // Create a table to hold our functions
    lua_newtable(L);

    // Add the log function to the table
    lua_pushcfunction(L, log);
    lua_setfield(L, -2, "Log");
    lua_pushcfunction(L, KeyPress);
    lua_setfield(L, -2, "KeyPress");
    lua_pushcfunction(L, KeyPressDown);
    lua_setfield(L, -2, "KeyPressDown");
    lua_pushcfunction(L, Click);
    lua_setfield(L, -2, "Click");
    lua_pushcfunction(L, GetMouseCoord);
    lua_setfield(L, -2, "GetMousePos");
    lua_pushcfunction(L, GetObject);
    lua_setfield(L, -2, "GetObject");
    lua_pushcfunction(L, DeleteObject);
    lua_setfield(L, -2, "DeleteObject");
    lua_pushcfunction(L, GetCurrentScene);
    lua_setfield(L, -2, "GetCurrentScene");
    lua_pushcfunction(L, GetCameraPos);
    lua_setfield(L, -2, "GetCameraPos");
    lua_pushcfunction(L, SetCameraPos);
    lua_setfield(L, -2, "SetCameraPos");
    lua_pushcfunction(L, OverlapCircle);
    lua_setfield(L, -2, "OverlapCircle");

    lua_pushcfunction(L, GetCameraSize);
    lua_setfield(L, -2, "GetCameraSize");
    lua_pushcfunction(L, SetCameraSize);
    lua_setfield(L, -2, "SetCameraSize");

    lua_pushcfunction(L, GetDeltaTime);
    lua_setfield(L, -2, "GetDeltaTime");
    lua_pushcfunction(L, GetTime);
    lua_setfield(L, -2, "GetTime");

    lua_pushcfunction(L, LoadScene);
    lua_setfield(L, -2, "LoadScene");
    lua_pushcfunction(L, CreateObject);
    lua_setfield(L, -2, "CreateObject");
    lua_pushcfunction(L, LoadObjectPrefab);
    lua_setfield(L, -2, "LoadObjectPrefab");
    lua_pushcfunction(L, Clamp);
    lua_setfield(L, -2, "Clamp");
    lua_pushcfunction(L, SetObject);
    lua_setfield(L, -2, "SetObject");
    lua_pushcfunction(L, Vector);
    lua_setfield(L, -2, "Vector");

    // Return the table
    return 1;
}
