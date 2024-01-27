#include "Animation.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "stb_image_write.h"
#include "Editor.h"

static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;

SapphireRenderer::Animation::Animation(const std::string& AnimationFile, const std::string& TexturePath)
{
    AnimFile = AnimationFile;
    Texture.Load(TexturePath, true);
    KeyFramesData = readKeyFramePairsFromBinaryFile(AnimationFile);
}
void SapphireRenderer::Animation::SelectKeyFrame(VertexBuffer& VBO)
{
    if(Engine::GameTime >= LastRecoredTime + KeyFramesData[CurrentKeyFrameIdx].TimeStamp){
        CurrentKeyFrameIdx++;
        if(CurrentKeyFrameIdx >= KeyFramesData.size()){
            CurrentKeyFrameIdx = 0;
            LastRecoredTime = Engine::GameTime;
        }
        VBO.Bind();
        VBO.SubData(KeyFramesData[CurrentKeyFrameIdx].vertices.size() * sizeof(glm::vec2) * 2.0f, (GLbyte*)KeyFramesData[CurrentKeyFrameIdx].vertices.data());
        VBO.Unbind();
    }
}
std::vector<SapphireRenderer::KeyFramePair> SapphireRenderer::Animation::readKeyFramePairsFromBinaryFile(const std::string& filename){
    std::vector<KeyFramePair> keyFrames;

    std::ifstream file(filename, std::ios::binary);

    if (file.is_open()) {
        // Read the number of key frames
        size_t numKeyFrames;
        file.read(reinterpret_cast<char*>(&numKeyFrames), sizeof(size_t));

        // Read each KeyFramePair
        for (size_t i = 0; i < numKeyFrames; ++i) {
            KeyFramePair keyFrame;

            // Read TimeStamp
            file.read(reinterpret_cast<char*>(&keyFrame.TimeStamp), sizeof(float));

            // Read the number of vertices
            size_t numVertices;
            file.read(reinterpret_cast<char*>(&numVertices), sizeof(size_t));

            // Read vertices
            keyFrame.vertices.resize(numVertices);
            file.read(reinterpret_cast<char*>(keyFrame.vertices.data()), sizeof(Vertex) * numVertices);

            // Read path string
            size_t pathLength;
            file.read(reinterpret_cast<char*>(&pathLength), sizeof(size_t));

            char* pathBuffer = new char[pathLength + 1];
            file.read(pathBuffer, pathLength);
            pathBuffer[pathLength] = '\0';
            keyFrame.path = new std::string(pathBuffer);

            delete[] pathBuffer;

            // Add the keyFrame to the vector
            keyFrames.push_back(keyFrame);
        }

        file.close();
    } else {
        // Handle error if the file cannot be opened
        std::cerr << "Error: Unable to open file for reading." << std::endl;
    }

    return keyFrames;
}
float KeyFrameWidthSum(const std::vector<SapphireRenderer::KeyFrameData>& keyframes){
    float sum;
    for (size_t i = 0; i < keyframes.size(); i++)
    {
        sum += keyframes[i].width;
    }
    return sum;
}
float KeyFrameMaxHeight(const std::vector<SapphireRenderer::KeyFrameData>& keyframes){
    float MaxHeight = keyframes[0].height;
    for (size_t i = 0; i < keyframes.size(); i++)
    {
        MaxHeight = std::max(MaxHeight, keyframes[i].height);
    }
    return MaxHeight;
}
void writeKeyFramePairsToBinaryFile(const std::vector<SapphireRenderer::KeyFramePair>& keyframes, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);

    if (file.is_open()) {
        // Write the number of key frames
        size_t numKeyFrames = keyframes.size();
        file.write(reinterpret_cast<const char*>(&numKeyFrames), sizeof(size_t));

        // Write each KeyFramePair
        for (const SapphireRenderer::KeyFramePair& keyFrame : keyframes) {
            // Write TimeStamp
            file.write(reinterpret_cast<const char*>(&keyFrame.TimeStamp), sizeof(float));

            // Write the number of vertices
            size_t numVertices = keyFrame.vertices.size();
            file.write(reinterpret_cast<const char*>(&numVertices), sizeof(size_t));

            // Write vertices
            file.write(reinterpret_cast<const char*>(keyFrame.vertices.data()), sizeof(Vertex) * numVertices);

            // Write path string
            size_t pathLength = keyFrame.path->length();
            file.write(reinterpret_cast<const char*>(&pathLength), sizeof(size_t));
            file.write(keyFrame.path->c_str(), pathLength);
        }

        file.close();
    } else {
        // Handle error if the file cannot be opened
        std::cerr << "Error: Unable to open file for writing." << std::endl;
    }
}
void SapphireRenderer::Animation::Export(const std::vector<KeyFrame*>& MainKeyframes, const std::string& name)
{
    stbi_set_flip_vertically_on_load(true);
    stbi_flip_vertically_on_write(true);
    std::vector<KeyFrameData> keyframeData;
    for (size_t i = 0; i < MainKeyframes.size(); i++)
    {
        KeyFrameData keyframe;
        int width, height, channels;
        keyframe.Data = stbi_load(MainKeyframes[i]->path.c_str(), &width, &height, &channels, 4);
        keyframe.width = width;
        keyframe.height = height;
        keyframe.channels = channels;
        keyframeData.push_back(keyframe);
    }

    float width = KeyFrameWidthSum(keyframeData); // Desired width
    float height = KeyFrameMaxHeight(keyframeData); // Desired height
    int channels;

    unsigned char* whiteImage = new unsigned char[(int)width * (int)height * 4]; // RGBA image
    float lastKeyFrameWidth = 0;
    for (size_t i = 0; i < keyframeData.size(); i++)
    {
        if(i != 0)
            lastKeyFrameWidth += keyframeData[i-1].width;
        for (int y = 0; y < keyframeData[i].height; y++) {
            for (int x = 0; x < keyframeData[i].width; x++) {
                int textureSheetIndex = (y * width + (x + lastKeyFrameWidth)) * 4;
                int image1Index = (y * keyframeData[i].width + x) * 4;

                for (int c = 0; c < 4; c++) {
                    whiteImage[textureSheetIndex + c] = keyframeData[i].Data[image1Index + c];
                }
            }
        }
    }

    // Save the final image as a PNG
    if (stbi_write_png((Engine::GetMainPath() + (name + ".png").c_str()).c_str(), width, height, 4, whiteImage, width * 4) == 0) {
        // Handle the error, e.g., print an error message
        printf("Error saving the final image as a PNG file\n");
    } else {
        printf("Final image saved as final_image.png\n");
    }
    float lastKeyFrameWidth1 = 0;
    std::vector<KeyFramePair> KeyFramesData;
    for (size_t i = 0; i < keyframeData.size(); i++)
    {
        KeyFramePair test;
        test.TimeStamp = MainKeyframes[i]->TimeStamp;
        test.path = new std::string(MainKeyframes[i]->path);
        if(i != 0)
            lastKeyFrameWidth1 += keyframeData[i-1].width / width;
        
        float currentKeyFrameWidth = lastKeyFrameWidth1 + keyframeData[i].width / width;
        float currentKeyFrameHeight = keyframeData[i].height / height;
                                // positions         // texture coords
        test.vertices.push_back({glm::vec2(0.5f,  0.5f), glm::vec2(currentKeyFrameWidth, currentKeyFrameHeight)});
        test.vertices.push_back({glm::vec2(0.5f,  -0.5f), glm::vec2(currentKeyFrameWidth, 0.0f)});
        test.vertices.push_back({glm::vec2(-0.5f,  -0.5f), glm::vec2(lastKeyFrameWidth1, 0.0f)});
        test.vertices.push_back({glm::vec2(-0.5f,  0.5f), glm::vec2(lastKeyFrameWidth1, currentKeyFrameHeight)});
        KeyFramesData.push_back(test);
    }
    writeKeyFramePairsToBinaryFile(KeyFramesData, Engine::GetMainPath() + name);
}

void SapphireRenderer::Animation::SetSelectedAnimation(std::optional<Animation>& CurrentAnimation)
{
    CurrentAnimation = *this;
}
