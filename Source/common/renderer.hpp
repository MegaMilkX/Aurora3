#ifndef RENDERER_BASE_HPP
#define RENDERER_BASE_HPP

#include "render_viewport.hpp"
#include "gfxm.hpp"
#include "gl/indexed_mesh.hpp"
#include "shader_factory.hpp"
#include "resource/texture2d.h"
#include "resource/cube_map.hpp"

#include "resource/resource_tree.hpp"

#include "draw_list.hpp"

#include "gl/uniform_buffers.hpp"

#include "util/animation/curve.h"

#include "data_headers/ibl_brdf_lut.png.h"

#include "render_state.hpp"

#include "draw_primitive.hpp"

extern int dbg_renderBufferId;

class ktWorld;
class Renderer {
public:
    Renderer() {
        {
            env_map.reset(new CubeMap());
            irradiance_map.reset(new CubeMap());

            curve<gfxm::vec3> curv;
            curv[0.0f] = gfxm::vec3(0 / 255.0f, 0 / 255.0f, 0 / 255.0f);
            curv[.5f] = gfxm::vec3(3 / 255.0f, 3 / 255.0f, 5 / 255.0f);
            curv[.55f] = gfxm::vec3(255 / 255.0f, 74 / 255.0f, 5 / 255.0f);
            curv[.60f] = gfxm::vec3(160 / 255.0f, 137 / 255.0f, 129 / 255.0f);
            curv[.7f] = gfxm::vec3(1 / 255.0f, 44 / 255.0f, 95 / 255.0f);
            curv[1.0f] = gfxm::vec3(1 / 255.0f, 11 / 255.0f, 36 / 255.0f);
            setSkyGradient(curv);
        }
        
        tex_ibl_brdf_lut.reset(new Texture2D());
        dstream dstrm;
        dstrm.setBuffer(std::vector<char>(ibl_brdf_lut_png, ibl_brdf_lut_png + sizeof(ibl_brdf_lut_png)));
        tex_ibl_brdf_lut->deserialize(dstrm, sizeof(ibl_brdf_lut_png));

        prog_quad = ShaderFactory::getOrCreate(
            "screen_quad",
            #include "../common/shaders/v_screen_quad.glsl"
            ,
            #include "../common/shaders/f_plain_texture.glsl"
        );
        prog_skybox = ShaderFactory::getOrCreate(
            "skybox",
            #include "../common/shaders/v_skybox.glsl"
            ,
            #include "../common/shaders/f_skybox.glsl"
        );
    }
    virtual ~Renderer() {}

    RenderState& getState() { return state; }

    void beginFrame(RenderViewport* vp, gfxm::mat4& proj, gfxm::mat4& view);

    void endFrame(RenderViewport* vp, bool draw_final_on_screen, bool draw_skybox);

    template<typename T>
    void drawMultiple(gl::ShaderProgram* p, const T* elements, size_t count) {
        getState().setProgram(p);
        for(size_t i = 0; i < count; ++i) {
            auto& e = elements[i];

            getState().setMaterial(e.material);

            e.bind(getState());

            glBindVertexArray(e.vao);
            glDrawElements(GL_TRIANGLES, e.indexCount, GL_UNSIGNED_INT, (GLvoid*)e.indexOffset);
        }
    }

    void drawWorld(RenderViewport* vp, ktWorld* world);
    void drawToScreen(GLuint textureId);

    virtual void draw(RenderViewport* vp, gfxm::mat4& proj, gfxm::mat4& view, const DrawList& draw_list, bool draw_final_on_screen = false, bool draw_skybox = true) = 0;

    void setSkyGradient(curve<gfxm::vec3> grad);
protected:
    RenderState state;

    gl::ShaderProgram* prog_quad;
    gl::ShaderProgram* prog_skybox;

    std::shared_ptr<Texture2D> tex_ibl_brdf_lut;
    std::shared_ptr<CubeMap> env_map;
    std::shared_ptr<CubeMap> irradiance_map;
};  

class RendererPBR : public Renderer {
    gl::ShaderProgram* prog_gbuf_solid;
    gl::ShaderProgram* prog_gbuf_skin;
    gl::ShaderProgram* prog_light_pass;

public:
    RendererPBR();

    virtual void draw(RenderViewport* vp, gfxm::mat4& proj, gfxm::mat4& view, const DrawList& draw_list, bool draw_final_on_screen = false, bool draw_skybox = true);
};

#endif
