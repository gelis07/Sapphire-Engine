#pragma once
#include "Scenes.h"
#include <memory>
#include "UI/Windows.h"

namespace Viewport{
    inline std::shared_ptr<Scene> CurrentScene = std::make_shared<Scene>(); // The loaed scene
    inline GLFWwindow* window;
    inline float DeltaTime;
    class Views{
        public:
            std::shared_ptr<Object> SelectedObj;
            glm::vec4 BackgroundColor = glm::vec4(0,0,0,1);
            std::shared_ptr<Object> MainCamera; // The camera of the game
            void SceneEditor();
            void PlayWindow();
            bool Paused = true;

            glm::vec2 ViewportPoint;
            glm::vec2 TextureSize;
            void OnApplicationStart();
        private:
            unsigned int PlayWindowTexture; 
            unsigned int PlayWindowFBO;
            unsigned int ViewportFBO;
            unsigned int ViewportTexture;
            ImVec2 ScaleWindow();
            // void ShowGrid();
            bool PlayWindowRender(unsigned int texture);
            bool ViewportRender(unsigned int texture);
            // void HandleObject(std::shared_ptr<Object> SelectedObj, std::vector<std::shared_ptr<Object>> &Objects);
            ImGuiContext* ViewportContext;
            bool ClickedOnObj;
            bool FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
            bool Start = true;
            glm::vec2 Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj

    };
}
