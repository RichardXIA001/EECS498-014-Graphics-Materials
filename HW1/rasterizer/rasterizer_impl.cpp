#include <cstdint>

#include "image.hpp"
#include "loader.hpp"
#include "rasterizer.hpp"
#include <random>
#include <iostream>
// TODO
bool IsPixelInsideTriangle(float x, float y, Triangle trig)
{
    // Convert trig positions to vec3 by dropping the w component from vec4
    trig.Homogenize();
    glm::vec3 vec_a = glm::vec3(trig.pos[0]);  // Convert directly from vec4 to vec3
    glm::vec3 vec_b = glm::vec3(trig.pos[1]);
    glm::vec3 vec_c = glm::vec3(trig.pos[2]);

    glm::vec3 vec_p = glm::vec3(x, y, 0.0f);

    // Calculate cross products
    glm::vec3 cross_product_1 = glm::cross(vec_b - vec_a, vec_p - vec_a);
    glm::vec3 cross_product_2 = glm::cross(vec_c - vec_b, vec_p - vec_b);
    glm::vec3 cross_product_3 = glm::cross(vec_a - vec_c, vec_p - vec_c);

    // Check if the point is inside the triangle by comparing the sign of the z-component of the cross products
    float z1 = cross_product_1.z;
    float z2 = cross_product_2.z;
    float z3 = cross_product_3.z;

    // Check if all signs are the same and non-zero
    if ((z1 > 0 && z2 > 0 && z3 > 0) || (z1 < 0 && z2 < 0 && z3 < 0) || (z1 * z2 * z3 == 0))
    {
        return true;
    }
    return false;
}

std::vector<glm::vec3> GenerateSamplesInPixel(uint32_t x, uint32_t y, uint32_t spp)
{
    // Generate spp samples in the pixel with uniform distribution in the range [0, 1]
  // Vector to store the generated samples
    std::vector<glm::vec3> samples;

    // Create a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister random number engine
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);  // Uniform distribution in range [0, 1]
    // std::normal_distribution<float> dis(0.5f, 1.0f);  // Normal distribution with mean 0 and standard deviation 1
    // Generate 'spp' samples
    for (uint32_t i = 0; i < spp; ++i)
    {
        // Generate random (x, y) coordinates within the pixel
        float sample_x = dis(gen);  // Random float between 0 and 1 for x-axis
        float sample_y = dis(gen);  // Random float between 0 and 1 for y-axis

        // Create a sample point (x + sample_x, y + sample_y, z)
        // We use the pixel location `(x, y)` and offset by a random sample within the pixel's unit square
        samples.push_back(glm::vec3(x + sample_x, y + sample_y, 0.0f));  // z = 0 for now
    }

    return samples;
}


void Rasterizer::DrawPixel(uint32_t x, uint32_t y, Triangle trig, AntiAliasConfig config, uint32_t spp, Image& image, Color color)
{
    if (config == AntiAliasConfig::NONE)            // if anti-aliasing is off
    {
        if (IsPixelInsideTriangle(x + 0.5, y + 0.5, trig))
        {
            image.Set(x, y, color);
        }   

    }
    else if (config == AntiAliasConfig::SSAA)       // if anti-aliasing is on
    {
        // Generate spp samples in the pixel
        std::vector<glm::vec3> samples = GenerateSamplesInPixel(x, y, spp);
        int count = 0;
        for (uint32_t i = 0; i < spp; i++)
        {
            if (IsPixelInsideTriangle(samples[i].x, samples[i].y, trig))
            {
                ++count;
            }
        }
        // Set the color of the sample
        image.Set(x, y, color * (count / spp));
    }
    return;
}

// TODO
void Rasterizer::AddModel(MeshTransform transform, glm::mat4 rotation)
{
    /* model.push_back( model transformation constructed from translation, rotation and scale );*/
    
    // Translation matrix
    glm::mat4 translation = glm::mat4(1.0f);
    translation[3][0] = transform.translation.x;
    translation[3][1] = transform.translation.y;
    translation[3][2] = transform.translation.z;
    
    // Scale matrix
    glm::mat4 scale = glm::mat4(1.0f);
    scale[0][0] = transform.scale.x;
    scale[1][1] = transform.scale.y;
    scale[2][2] = transform.scale.z;

    // glm::mat4 rot = glm::toMat4(transform.rotation);   // Quaternion to rotation matrix


    // Model matrix
    glm::mat4 model = translation * rotation * scale;
    this->model.push_back(model);

    return;
}

// TODO
void Rasterizer::SetView()
{
    const Camera& camera = this->loader.GetCamera();
    glm::vec3 cameraPos = camera.pos;
    glm::vec3 cameraLookAt = camera.lookAt;
    glm::vec3 cameraUp = camera.up;

    // TODO change this line to the correct view matrix
    this->view = glm::mat4(1.);

    // Translate first
    glm::mat4 translate = glm::mat4(1.0f);
    translate[3][0] = -cameraPos.x;
    translate[3][1] = -cameraPos.y;
    translate[3][2] = -cameraPos.z;

    // Rotate
    // normailize the camera coordinate vector
    glm::mat4 rotate = glm::mat4(1.0f);
    cameraUp = glm::normalize(cameraUp);
    glm::vec3 cameraView = glm::normalize(cameraLookAt - cameraPos);
    
    glm::vec3 auxiliary_axis = glm::cross(cameraView, cameraUp);

    rotate[0][0] = auxiliary_axis.x;
    rotate[1][0] = auxiliary_axis.y;
    rotate[2][0] = auxiliary_axis.z;

    rotate[0][1] = cameraUp.x;
    rotate[1][1] = cameraUp.y;
    rotate[2][1] = cameraUp.z;

    rotate[0][2] = -cameraView.x;
    rotate[1][2] = -cameraView.y;
    rotate[2][2] = -cameraView.z;

    this->view = rotate * translate;

    return;
}

// TODO
void Rasterizer::SetProjection()
{
    const Camera& camera = this->loader.GetCamera();

    float nearClip = -camera.nearClip;                   // near clipping distance, strictly positive
    float farClip = -camera.farClip;                     // far clipping distance, strictly positive
    
    float width = camera.width;
    float height = camera.height;
    
    // TODO change this line to the correct projection matrix
    this->projection = glm::mat4(1.0f);

    glm::mat4 M_persp = glm::mat4(0.0f);

    M_persp[0][0] = nearClip;
    M_persp[1][1] = nearClip;
    M_persp[2][2] = nearClip + farClip;
    M_persp[3][2] = -nearClip * farClip;
    M_persp[2][3] = 1.0f;

    glm::mat4 M_ortho = glm::mat4(1.0f);

    glm::mat4 M_scale = glm::mat4(1.0f);
    glm::mat4 M_translate = glm::mat4(1.0f);

    M_scale[0][0] = 2.0f / width;
    M_scale[1][1] = 2.0f / height;
    M_scale[2][2] = 2.0f / (nearClip - farClip);

    M_translate[3][2] = -(farClip + nearClip) / 2.0f;
    // M_translate[3][1] = - height / 2.0f;
    // M_translate[3][0] = - width / 2.0f;
    
    M_ortho = M_scale * M_translate;

    this->projection = M_ortho * M_persp;

    return;
}

// TODO
void Rasterizer::SetScreenSpace()
{
    float width = this->loader.GetWidth();
    float height = this->loader.GetHeight();

    // TODO change this line to the correct screenspace matrix
    this->screenspace = glm::mat4(1.);

    this->screenspace[3][0] = width / 2.0f;
    this->screenspace[3][1] = height / 2.0f;

    this->screenspace[0][0] = width / 2.0f;
    this->screenspace[1][1] = height / 2.0f;

    return;
}

// TODO
glm::vec3 Rasterizer::BarycentricCoordinate(glm::vec2 pos, Triangle trig)
{
    return glm::vec3();
}

// TODO
float Rasterizer::zBufferDefault = float();

// TODO
void Rasterizer::UpdateDepthAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, ImageGrey& ZBuffer)
{

    float result;
    ZBuffer.Set(x, y, result);

    return;
}

// TODO
void Rasterizer::ShadeAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, Image& image)
{

    Color result;
    image.Set(x, y, result);

    return;
}
