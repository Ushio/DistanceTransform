#include "pr.hpp"
#include <iostream>
#include <memory>
#include <array>
template <class T>
inline constexpr T ss_max(T x, T y)
{
    return (x < y) ? y : x;
}

template <class T>
inline constexpr T ss_min(T x, T y)
{
    return (y < x) ? y : x;
}

int main() {
    using namespace pr;

    SetDataDir(ExecutableDir());

    Config config;
    config.ScreenWidth = 1920;
    config.ScreenHeight = 1080;
    config.SwapInterval = 1;
    Initialize(config);

    Camera3D camera;
    camera.origin = { 4, 4, 4 };
    camera.lookat = { 0, 0, 0 };
    camera.zUp = true;

    double e = GetElapsedTime();

    Image2DRGBA8 src;
    //src.load("src.png");
    src.load("src_large.png");

    ITexture *tex = CreateTexture();
    tex->upload(src);

    std::vector<glm::ivec2> d_vec_map(src.width() * src.height());
    int large = std::max(src.width(), src.height());

    auto len2 = [](glm::ivec2 v)
    {
        return v.x * v.x + v.y * v.y;
    };

    std::vector<int> l1_map(src.width() * src.height());

    for (;;)
    {
        int width = src.width();
        int height = src.height();

        Stopwatch sw;
        for (int y = 0; y < src.height(); y++)
        {
            for (int x = 0; x < src.width(); x++)
            {
                int index = y * src.width() + x;
                int val = 128 < src(x, y).x ? 0 : large;
                d_vec_map[index] = glm::ivec2(val, val);
                l1_map[index] = 128 < src(x, y).x ? 0 : width + height;
            }
        }

        for (int y = 1; y < height - 1; y++)
        {
            int prev_c = width * height;
            int cy = y;

            int* c_head = l1_map.data() + cy * width;
            int* u_head = l1_map.data() + (cy - 1) * width;
            for (int x = 1; x < width - 1; x++)
            {
                int cx = x;
                
                int c = c_head[cx];
                int u = u_head[cx];
                c = ss_min(prev_c + 1, c);
                c = ss_min(u + 1, c);

                c_head[cx] = c;
                prev_c = c;
            }
        }

        for (int y = 1; y < height - 1; y++)
        {
            int prev_c = width * height;
            int cy = height - y - 1;

            int* c_head = l1_map.data() + cy * width;
            int* b_head = l1_map.data() + (cy + 1) * width;
            for (int x = 1; x < width - 1; x++)
            {
                int cx = width - x - 1;
                
                int c = c_head[cx];
                int b = b_head[cx];
                c = ss_min(prev_c + 1, c);
                c = ss_min(b + 1, c);

                c_head[cx] = c;
                prev_c = c;
            }
        }
#if 0
        std::array<glm::ivec2, 4> step1 = {
            glm::ivec2{-1, -1},
            glm::ivec2{ 0, -1},
            glm::ivec2{-1,  0},
            glm::ivec2{+1, -1},
        };
        std::array<glm::ivec2, 1> step2 = {
            glm::ivec2{+1, 0},
        };

        std::array<glm::ivec2, 4> step3 = {
            glm::ivec2{+1,  0},
            glm::ivec2{-1, +1},
            glm::ivec2{ 0, +1},
            glm::ivec2{+1, +1},
        };

        std::array<glm::ivec2, 1> step4 = {
            glm::ivec2{-1, 0},
        };

        // 1

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int cx = x;
                int cy = y;
                int index_c = cy * width + cx;
                glm::ivec2 c = d_vec_map[index_c];
                int clen2 = len2(c);

                for (glm::ivec2 mv : step1)
                {
                    int src_x = glm::clamp(cx + mv.x, 0, width - 1);
                    int src_y = glm::clamp(cy + mv.y, 0, height - 1);
                    int src_idx = src_y * width + src_x;
                    glm::ivec2 next = d_vec_map[src_idx] + glm::abs(mv);
                    int nextlen2 = len2(next);
                    if (nextlen2 < clen2)
                    {
                        clen2 = nextlen2;
                        c = next;
                    }
                    //if (len2(next) < len2(c))
                    //{
                    //    c = next;
                    //}
                }
                d_vec_map[index_c] = c;
            }
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int cx = width - x - 1;
                int cy = y;
                int index_c = cy * width + cx;
                glm::ivec2 c = d_vec_map[index_c];
                int clen2 = len2(c);
                for (glm::ivec2 mv : step2)
                {
                    int src_x = glm::clamp(cx + mv.x, 0, width - 1);
                    int src_y = glm::clamp(cy + mv.y, 0, height - 1);
                    int src_idx = src_y * width + src_x;
                    glm::ivec2 next = d_vec_map[src_idx] + glm::abs(mv);
                    //if (len2(next) < len2(c))
                    //{
                    //    c = next;
                    //}
                    int nextlen2 = len2(next);
                    if (nextlen2 < clen2)
                    {
                        clen2 = nextlen2;
                        c = next;
                    }
                }
                d_vec_map[index_c] = c;
            }
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int cx = width - x - 1;
                int cy = height - y - 1;
                int index_c = cy * width + cx;
                glm::ivec2 c = d_vec_map[index_c];
                int clen2 = len2(c);

                for (glm::ivec2 mv : step3)
                {
                    int src_x = glm::clamp(cx + mv.x, 0, width - 1);
                    int src_y = glm::clamp(cy + mv.y, 0, height - 1);
                    int src_idx = src_y * width + src_x;
                    glm::ivec2 next = d_vec_map[src_idx] + glm::abs(mv);
                    //if (len2(next) < len2(c))
                    //{
                    //    c = next;
                    //}
                    int nextlen2 = len2(next);
                    if (nextlen2 < clen2)
                    {
                        clen2 = nextlen2;
                        c = next;
                    }
                }
                d_vec_map[index_c] = c;
            }
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int cx = x;
                int cy = y;
                int index_c = cy * width + cx;
                glm::ivec2 c = d_vec_map[index_c];
                int clen2 = len2(c);

                for (glm::ivec2 mv : step4)
                {
                    int src_x = glm::clamp(cx + mv.x, 0, width - 1);
                    int src_y = glm::clamp(cy + mv.y, 0, height - 1);
                    int src_idx = src_y * width + src_x;
                    glm::ivec2 next = d_vec_map[src_idx] + glm::abs(mv);
                    //if (len2(next) < len2(c))
                    //{
                    //    c = next;
                    //}
                    int nextlen2 = len2(next);
                    if (nextlen2 < clen2)
                    {
                        clen2 = nextlen2;
                        c = next;
                    }
                }
                d_vec_map[index_c] = c;
            }
        }

#endif

        printf("%f\n", sw.elapsed());

    }

    Image2DRGBA8 distanceMap;
    distanceMap.allocate(src.width(), src.height());
    //for (int y = 0; y < src.height(); y++)
    //{
    //    for (int x = 0; x < src.width(); x++)
    //    {
    //        int index_c = y * src.width() + x;
    //        glm::ivec2 c = d_vec_map[index_c];
    //        int len = len2(c);

    //        int val = glm::clamp(len, 0, 255);
    //        distanceMap(x, y) = {
    //            val,
    //            val,
    //            val,
    //            255
    //        };
    //    }
    //}
    for (int y = 0; y < src.height(); y++)
    {
        for (int x = 0; x < src.width(); x++)
        {
            int index_c = y * src.width() + x;
            int len = l1_map[index_c];

            int val = glm::clamp(len * len, 0, 255);
            distanceMap(x, y) = {
                val,
                val,
                val,
                255
            };
        }
    }
    distanceMap.saveAsPng("distance.png");

    tex->upload(distanceMap);

    while (pr::NextFrame() == false) {
        if (IsImGuiUsingMouse() == false) {
            UpdateCameraBlenderLike(&camera);
        }

        ClearBackground(0.1f, 0.1f, 0.1f, 1);

        BeginCamera(camera);

        PushGraphicState();

        DrawGrid(GridAxis::XY, 1.0f, 10, { 128, 128, 128 });
        DrawXYZAxis(1.0f);

        PopGraphicState();
        EndCamera();

        BeginImGui();

        ImGui::SetNextWindowSize({ 700, 800 }, ImGuiCond_Once);
        ImGui::Begin("Panel");
        ImGui::Text("fps = %f", GetFrameRate());

        static int threshold = 4;
        if (ImGui::SliderInt("threshold", &threshold, 1, 500))
        {
            for (int y = 0; y < src.height(); y++)
            {
                for (int x = 0; x < src.width(); x++)
                {
                    int index_c = y * src.width() + x;
                    //glm::ivec2 c = d_vec_map[index_c];
                    //int len = len2(c);

                    int len = l1_map[index_c];

                    int val = len < threshold ? 255 : 0;
                    distanceMap(x, y) = {
                        val,
                        val,
                        val,
                        255
                    };
                }
            }
            tex->upload(distanceMap);
            distanceMap.saveAsPng("dilated.png");
        }

        ImGui::Image(tex, ImVec2(tex->width(), tex->height()));

        ImGui::End();

        EndImGui();
    }

    pr::CleanUp();
}
