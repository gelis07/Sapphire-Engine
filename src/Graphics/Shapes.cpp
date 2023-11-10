#include "Shapes.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "stb_image_write.h"

static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
//This is a complicated name but its just the shape constructor
SapphireRenderer::Shape::Shape(const SapphireRenderer::Shader& shader, std::vector<Vertex>&& Vertices, const std::string& path) : Shader(shader), VertexArray(), VertexBuffer(), IndexBuffer(), Texture(std::make_optional<SapphireRenderer::Texture>(path, true))
{
    Shader = shader;
    VertexArray.Bind();
    VertexBuffer.Bind();
    IndexBuffer.Bind();


    VertexBuffer.AssignData(16 * sizeof(float), (GLbyte*)Vertices.data(), GL_DYNAMIC_DRAW);

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    IndexBuffer.AssignData(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_STATIC_DRAW);
    Shader = SapphireRenderer::BasicShader;
    Texture = std::nullopt;

    VertexArray.Unbind();
    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
}

void SapphireRenderer::Shape::RenderShape(const Transform& transform,const glm::vec4& Color,const glm::vec3 &CamPos, const glm::mat4& view, float CameraZoom, bool OutLine ,bool WireFrame, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms)
{
    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->GetSize());
    // m_Projection = glm::ortho( -WindowSize.x/2.0f / CameraZoom, WindowSize.x/2.0f / CameraZoom, -WindowSize.y / 2.0f / CameraZoom, WindowSize.y / 2.0f / CameraZoom, -1.0f, 1.0f);
    m_Projection = glm::ortho( 0.0f, WindowSize.x / CameraZoom, 0.0f, WindowSize.y / CameraZoom, -1.0f, 1.0f);
     

    Shader.Bind();
    VertexArray.Bind();
    IndexBuffer.Bind();
    //Here is the standard model view projection matrix
    const glm::vec3& Pos = transform.GetPosition();
    glm::mat4 mvp = m_Projection * view * transform.GetModel();
    Shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    if(Texture){
        Texture.value().SetAsActive();
        Texture.value().Bind();
        Shader.SetUniform("u_Texture", (int)Texture.value().GetSlot());
    }
    SetUpUniforms(Shader);

    if(OutLine){
        Shader.SetUniform("u_Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }else{
        Shader.SetUniform("u_Color", Color);
    }
    if(!WireFrame){
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }else{
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
    if(Texture) Texture.value().Unbind(); 
    Shader.Unbind();
    VertexArray.Unbind();
    IndexBuffer.Unbind();
}

void SapphireRenderer::Shape::Load(const std::string &path, bool flip)
{
    Shader = SapphireRenderer::TextureShader;
    if(Texture)
    {
        Texture.value().Load(path, true);
    }else{
        Texture = SapphireRenderer::Texture(path, true);
    }
}
const glm::vec2 SapphireRenderer::Shape::GetTextureDimensions() const
{
    if(!Texture)
        return glm::vec2(0);
    else
        return Texture.value().GetDimensions();
}


SapphireRenderer::Rectangle::Rectangle(const SapphireRenderer::Shader &shader, const std::string &path): Shape(shader,{
    {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
    {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
    {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
    {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
}, path) 
{

    ShapeType = RectangleT;
}

//? Maybe make it the default functon for the shape class?
void SapphireRenderer::Rectangle::Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame){
    RenderShape(transform, Color,CamPos,view,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  });
            //                                       ^The rectangle doesn't have any extra uniforms
}

SapphireRenderer::Circle::Circle(const SapphireRenderer::Shader &shader): Shape(shader,{
    {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
    {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
    {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
    {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
}) 
{
    ShapeType = CircleT;
}

void SapphireRenderer::Circle::Render(const Transform &transform, const glm::vec4 &Color, const glm::vec3 &CamPos, const glm::mat4 &view, float CameraZoom, bool OutLine, bool WireFrame)
{
    const glm::vec3& ObjectSize = transform.GetSize();
    const glm::vec3& ObjectPos = transform.GetPosition();

    glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CamPos.x, ObjectPos.y - ObjectSize.y/2 + CamPos.y);
    std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
        shader.SetUniform("RectWidth", ObjectSize.x);
        shader.SetUniform("RectHeight", ObjectSize.y);
        shader.SetUniform("StartPoint", StartPos);
        shader.SetUniform("CameraZoom", CameraZoom);
    };

    RenderShape(transform, Color,CamPos,view,CameraZoom,OutLine, WireFrame, Uniforms);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}



void SapphireRenderer::Animation::SaveAnim(const std::string& filename) {

    std::vector<KeyFrame> MainKeyframes;
    for (size_t i = 0; i < KeyFrames.size(); i++)
    {
        KeyFrame keyframe;
        keyframe.TimeStamp = KeyFrames[i].TimeStamp;
        keyframe.path = *KeyFrames[i].path;
        MainKeyframes.push_back(keyframe);
    }
    
    KeyFrames.clear();
    std::vector<KeyFrameData> keyframeData;
    for (size_t i = 0; i < MainKeyframes.size(); i++)
    {
        KeyFrameData keyframe;
        int width, height, channels;
        keyframe.Data = stbi_load(MainKeyframes[i].path.c_str(), &width, &height, &channels, 4);
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
    if (stbi_write_png("hi.png", width, height, 4, whiteImage, width * 4) == 0) {
        // Handle the error, e.g., print an error message
        printf("Error saving the final image as a PNG file\n");
    } else {
        printf("Final image saved as final_image.png\n");
    }
    float lastKeyFrameWidth1 = 0;
    for (size_t i = 0; i < keyframeData.size(); i++)
    {
        KeyFramePair newKeyframe;
        newKeyframe.TimeStamp = MainKeyframes[i].TimeStamp;
        newKeyframe.path = new std::string(MainKeyframes[i].path);
        if(i != 0)
            lastKeyFrameWidth1 += keyframeData[i-1].width / width;
        
        float currentKeyFrameWidth = lastKeyFrameWidth1 + keyframeData[i].width / width;
        float currentKeyFrameHeight = keyframeData[i].height / height;
        std::vector<Vertex> Vertices;
        Vertices.push_back({glm::vec2(0.5f,  0.5f),glm::vec2(currentKeyFrameWidth, currentKeyFrameHeight)});
        Vertices.push_back({glm::vec2(0.5f,  0.5f), glm::vec2(currentKeyFrameWidth, 0.0f)});
        Vertices.push_back({glm::vec2(-0.5f, -0.5f), glm::vec2(lastKeyFrameWidth1, 0.0f)});
        Vertices.push_back({glm::vec2(-0.5f,  0.5f), glm::vec2(lastKeyFrameWidth1, currentKeyFrameHeight)});
        newKeyframe.vertices = Vertices;
        KeyFrames.push_back(newKeyframe);
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return;
    }

    for (KeyFramePair keyframe : KeyFrames) {
        // Write TimeStamp
        file.write(reinterpret_cast<char*>(&keyframe.TimeStamp), sizeof(float));

        // Serialize and write vertices
        keyframe.serializeVertices(file);

        // Serialize and write the string
        keyframe.serializeString(file);
    }

    file.close();
}

void SapphireRenderer::Animation::SaveAnim(const std::string &filename, const std::vector<KeyFrame>& aKeyFrames)
{
    const std::vector<KeyFrame>& MainKeyframes = aKeyFrames;
    
    KeyFrames.clear();
    std::vector<KeyFrameData> keyframeData;
    for (size_t i = 0; i < MainKeyframes.size(); i++)
    {
        KeyFrameData keyframe;
        int width, height, channels;
        keyframe.Data = stbi_load(MainKeyframes[i].path.c_str(), &width, &height, &channels, 4);
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
    if (stbi_write_png("hi.png", width, height, 4, whiteImage, width * 4) == 0) {
        // Handle the error, e.g., print an error message
        printf("Error saving the final image as a PNG file\n");
    } else {
        printf("Final image saved as final_image.png\n");
    }
    float lastKeyFrameWidth1 = 0;
    for (size_t i = 0; i < keyframeData.size(); i++)
    {
        KeyFramePair newKeyframe;
        newKeyframe.TimeStamp = MainKeyframes[i].TimeStamp;
        newKeyframe.path = new std::string(MainKeyframes[i].path);
        if(i != 0)
            lastKeyFrameWidth1 += keyframeData[i-1].width / width;
        
        float currentKeyFrameWidth = lastKeyFrameWidth1 + keyframeData[i].width / width;
        float currentKeyFrameHeight = keyframeData[i].height / height;
        std::vector<Vertex> Vertices;
        Vertices.push_back({glm::vec2(0.5f,  0.5f),glm::vec2(currentKeyFrameWidth, currentKeyFrameHeight)});
        Vertices.push_back({glm::vec2(0.5f,  0.5f), glm::vec2(currentKeyFrameWidth, 0.0f)});
        Vertices.push_back({glm::vec2(-0.5f, -0.5f), glm::vec2(lastKeyFrameWidth1, 0.0f)});
        Vertices.push_back({glm::vec2(-0.5f,  0.5f), glm::vec2(lastKeyFrameWidth1, currentKeyFrameHeight)});
        newKeyframe.vertices = Vertices;
        KeyFrames.push_back(newKeyframe);
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return;
    }

    for (KeyFramePair keyframe : KeyFrames) {
        // Write TimeStamp
        file.write(reinterpret_cast<char*>(&keyframe.TimeStamp), sizeof(float));

        // Serialize and write vertices
        keyframe.serializeVertices(file);

        // Serialize and write the string
        keyframe.serializeString(file);
    }

    file.close();
}

std::vector<SapphireRenderer::KeyFramePair> SapphireRenderer::Animation::LoadAnim(const std::string& filename){
    std::vector<KeyFramePair> keyframes;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file for reading." << std::endl;
        return keyframes;
    }

    while (!file.eof()) {
        KeyFramePair keyframe;

        // Read TimeStamp
        file.read(reinterpret_cast<char*>(&keyframe.TimeStamp), sizeof(float));

        // Deserialize and read vertices
        keyframe.deserializeVertices(file);

        // Deserialize and read the string
        keyframe.deserializeString(file);

        keyframes.push_back(keyframe);
    }

    file.close();
    return keyframes;
}

void SapphireRenderer::Animation::SelectKeyFrame()
{
    if(glfwGetTime() >= LastRecoredTime + KeyFrames[CurrentKeyFrameIdx].TimeStamp){
        VertexBuffer.Bind();
        VertexBuffer.SubData(sizeof(KeyFrames[CurrentKeyFrameIdx].vertices), (GLbyte*)KeyFrames[CurrentKeyFrameIdx].vertices.data());
        VertexBuffer.Unbind();
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer.GetID()));
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(KeyFrames[CurrentKeyFrameIdx].vertices), KeyFrames[CurrentKeyFrameIdx].vertices.data()));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CurrentKeyFrameIdx++;
        if(CurrentKeyFrameIdx >= KeyFrames.size()){
            CurrentKeyFrameIdx = 0;
            LastRecoredTime = glfwGetTime();
        }
    }
}

SapphireRenderer::Animation::Animation(std::vector<KeyFrame>&& aKeyFrames, std::vector<Vertex>&& Vertices, const SapphireRenderer::Shader &shader, const std::string &path) : Shape(shader,std::move(Vertices),path)
{
    SaveAnim(Engine::Get().GetMainPath() + "/NewName.anim", aKeyFrames);
    ShapeType = RectangleT;
    quickSort(KeyFrames, 0, KeyFrames.size() - 1);
    this->Load("hi.png");
    SetShader(SapphireRenderer::AnimationShader);
}

SapphireRenderer::Animation::Animation(std::vector<KeyFramePair>&& aKeyFrames, std::vector<Vertex>&& Vertices, const SapphireRenderer::Shader &shader, const std::string &path) : Shape(shader,std::move(Vertices),path), KeyFrames(aKeyFrames)
{
    ShapeType = RectangleT;
    quickSort(KeyFrames, 0, KeyFrames.size() - 1);
    this->Load(path);
    SetShader(SapphireRenderer::AnimationShader);
}

void SapphireRenderer::Animation::Render(const Transform &transform, const glm::vec4 &Color, const glm::vec3 &CamPos, const glm::mat4 &view, float CameraZoom, bool OutLine, bool WireFrame)
{
    SelectKeyFrame();
    RenderShape(transform, Color,CamPos,view,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  });
}

int SapphireRenderer::Animation::partition(std::vector<KeyFramePair> &arr, int start, int end)
{

    float pivot = arr[start].TimeStamp;

    int count = 0;
    for (int i = start + 1; i <= end; i++) {
        if (arr[i].TimeStamp <= pivot)
            count++;
    }

    // Giving pivot element its correct position
    int pivotIndex = start + count;
    std::swap(arr[pivotIndex], arr[start]);

    // Sorting left and right parts of the pivot element
    int i = start, j = end;

    while (i < pivotIndex && j > pivotIndex) {

        while (arr[i].TimeStamp <= pivot) {
            i++;
        }

        while (arr[j].TimeStamp > pivot) {
            j--;
        }

        if (i < pivotIndex && j > pivotIndex) {
            std::swap(arr[i++], arr[j--]);
        }
    }

    return pivotIndex;
}

void SapphireRenderer::Animation::quickSort(std::vector<KeyFramePair> &arr, int start, int end)
{
    // base case
    if (start >= end)
        return;

    // partitioning the array
    int p = partition(arr, start, end);

    // Sorting the left part
    quickSort(arr, start, p - 1);

    // Sorting the right part
    quickSort(arr, p + 1, end);
}

float SapphireRenderer::Animation::KeyFrameWidthSum(const std::vector<KeyFrameData> &keyframes)
{
    float sum;
    for (size_t i = 0; i < keyframes.size(); i++)
    {
        sum += keyframes[i].width;
    }
    return sum;
}

float SapphireRenderer::Animation::KeyFrameMaxHeight(const std::vector<KeyFrameData> &keyframes)
{
    float MaxHeight = keyframes[0].height;
    for (size_t i = 0; i < keyframes.size(); i++)
    {
        MaxHeight = std::max(MaxHeight, keyframes[i].height);
    }
    return MaxHeight;
}