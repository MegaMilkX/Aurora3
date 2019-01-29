#ifndef SCENE_FROM_FBX_HPP
#define SCENE_FROM_FBX_HPP

#define NO_MIN_MAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <functional>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include "scene.hpp"
#include "transform.hpp"
#include "model.hpp"
#include "skin.hpp"
#include "animator.hpp"
#include "util/log.hpp"
#include "util/split.hpp"
#include "asset_params.hpp"

#include "resource/data_registry.h"
#include "resource/resource_factory.h"

inline void meshesFromAssimpScene(
    const aiScene* ai_scene, 
    Scene* scene, 
    std::shared_ptr<Skeleton> skeleton,
    AssetParams& asset_params,
    const std::string& dirname
) {
    unsigned int mesh_count = ai_scene->mNumMeshes;
    for(unsigned int i = 0; i < mesh_count; ++i) {
        aiMesh* ai_mesh = ai_scene->mMeshes[i];
        int32_t vertexCount = ai_mesh->mNumVertices;
        int32_t triCount = ai_mesh->mNumFaces;
        int32_t indexCount = triCount * 3;
        std::vector<float> vertices;
        std::vector<uint32_t> indices;
        std::vector<std::vector<float>> normal_layers;
        normal_layers.resize(1);
        std::vector<float> tangent;
        std::vector<float> bitangent;
        std::vector<std::vector<float>> uv_layers;
        std::vector<std::vector<float>> rgb_layers;
        std::vector<gfxm::vec4> boneIndices;
        std::vector<gfxm::vec4> boneWeights;
        for(unsigned j = 0; j < vertexCount; ++j) {
            aiVector3D& v = ai_mesh->mVertices[j];
            vertices.emplace_back(v.x);
            vertices.emplace_back(v.y);
            vertices.emplace_back(v.z);
        }
        for(unsigned j = 0; j < triCount; ++j) {
            aiFace& f = ai_mesh->mFaces[j];
            for(unsigned k = 0; k < f.mNumIndices; ++k) {
                indices.emplace_back(f.mIndices[k]);
            }
        }
        for(unsigned j = 0; j < vertexCount; ++j) {
            aiVector3D& n = ai_mesh->mNormals[j];
            normal_layers[0].emplace_back(n.x);
            normal_layers[0].emplace_back(n.y);
            normal_layers[0].emplace_back(n.z);
        }
        for(unsigned j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
            if(!ai_mesh->HasTextureCoords(j)) break;
            std::vector<float> uv_layer;
            for(unsigned k = 0; k < vertexCount; ++k) {
                aiVector3D& uv = ai_mesh->mTextureCoords[j][k];
                uv_layer.emplace_back(uv.x);
                uv_layer.emplace_back(uv.y);
            }
            uv_layers.emplace_back(uv_layer);
        }

        if(ai_mesh->mNumBones) {
            boneIndices.resize(vertexCount);
            boneWeights.resize(vertexCount);
            for(unsigned j = 0; j < ai_mesh->mNumBones; ++j) {
                unsigned int bone_index = j;
                aiBone* bone = ai_mesh->mBones[j];
                skeleton->addBone(bone->mName.C_Str(), *(gfxm::mat4*)&bone->mOffsetMatrix);
                
                for(unsigned k = 0; k < bone->mNumWeights; ++k) {
                    aiVertexWeight& w = bone->mWeights[k];
                    gfxm::vec4& indices_ref = boneIndices[w.mVertexId];
                    gfxm::vec4& weights_ref = boneWeights[w.mVertexId];
                    for(unsigned l = 0; l < 4; ++l) {
                        if(weights_ref[l] == 0.0f) {
                            indices_ref[l] = (float)bone_index;
                            weights_ref[l] = w.mWeight;
                            break;
                        }
                    }
                }
            }
        }

        tangent.resize(vertexCount * 3);
        bitangent.resize(vertexCount * 3);
        if(ai_mesh->mTangents && ai_mesh->mBitangents) {
            for(unsigned j = 0; j < vertexCount; ++j) {
                aiVector3D& n = ai_mesh->mTangents[j];
                tangent[j * 3] = (n.x);
                tangent[j * 3 + 1] = (n.y);
                tangent[j * 3 + 2] = (n.z);
            }
            for(unsigned j = 0; j < vertexCount; ++j) {
                aiVector3D& n = ai_mesh->mBitangents[j];
                bitangent[j * 3] = (n.x);
                bitangent[j * 3 + 1] = (n.y);
                bitangent[j * 3 + 2] = (n.z);
            }
        }

        

        std::shared_ptr<Mesh> mesh_ref(new Mesh());
        mesh_ref->Name(MKSTR(i << ".geo"));
        mesh_ref->Storage(Resource::LOCAL);

        mesh_ref->mesh.setAttribData(gl::POSITION, vertices.data(), vertices.size() * sizeof(float));
        if(normal_layers.size() > 0) {
            mesh_ref->mesh.setAttribData(gl::NORMAL, normal_layers[0].data(), normal_layers[0].size() * sizeof(float));
        }
        if(uv_layers.size() > 0) {
            mesh_ref->mesh.setAttribData(gl::UV, uv_layers[0].data(), uv_layers[0].size() * sizeof(float));
        }
        if(!boneIndices.empty() && !boneWeights.empty()) {
            mesh_ref->mesh.setAttribData(gl::BONE_INDEX4, boneIndices.data(), boneIndices.size() * sizeof(gfxm::vec4));
            mesh_ref->mesh.setAttribData(gl::BONE_WEIGHT4, boneWeights.data(), boneWeights.size() * sizeof(gfxm::vec4));
        }
        mesh_ref->mesh.setAttribData(gl::TANGENT, tangent.data(), tangent.size() * sizeof(float));
        mesh_ref->mesh.setAttribData(gl::BITANGENT, bitangent.data(), bitangent.size() * sizeof(float));

        mesh_ref->mesh.setIndices(indices.data(), indices.size());

        std::string fname = MKSTR(dirname << "\\" << i << ".mesh");
        mesh_ref->write_to_file(MKSTR(dirname << "\\" << i << ".mesh"));
        GlobalDataRegistry().Add(
            fname,
            DataSourceRef(new DataSourceFilesystem(get_module_dir() + "\\" + fname))
        );
    }
}

inline void animNodeFromAssimpNode(aiNodeAnim* ai_node_anim, AnimNode& node) {
    for(unsigned k = 0; k < ai_node_anim->mNumPositionKeys; ++k) {
        auto& ai_v = ai_node_anim->mPositionKeys[k].mValue;
        float t = (float)ai_node_anim->mPositionKeys[k].mTime;
        gfxm::vec3 v = { ai_v.x, ai_v.y, ai_v.z };
        node.t[t] = v;
    }
    for(unsigned k = 0; k < ai_node_anim->mNumRotationKeys; ++k) {
        auto& ai_v = ai_node_anim->mRotationKeys[k].mValue;
        float t = (float)ai_node_anim->mRotationKeys[k].mTime;
        gfxm::quat v = { ai_v.x, ai_v.y, ai_v.z, ai_v.w };
        node.r[t] = v;
    }
    for(unsigned k = 0; k < ai_node_anim->mNumScalingKeys; ++k) {
        auto& ai_v = ai_node_anim->mScalingKeys[k].mValue;
        float t = (float)ai_node_anim->mScalingKeys[k].mTime;
        gfxm::vec3 v = { ai_v.x, ai_v.y, ai_v.z };
        node.s[t] = v;
    }
}

inline void animationsFromAssimpScene(
    const aiScene* ai_scene,
    Scene* scene,
    std::shared_ptr<Skeleton> skeleton,
    AssetParams& asset_params,
    const std::string& dirname
) {
    for(unsigned i = 0; i < ai_scene->mNumAnimations; ++i) {
        aiAnimation* ai_anim = ai_scene->mAnimations[i];
        double fps = ai_anim->mTicksPerSecond;
        double len = ai_anim->mDuration;
        
        for(unsigned j = 0; j < ai_anim->mNumChannels; ++j) {
            aiNodeAnim* ai_node_anim = ai_anim->mChannels[j];
            skeleton->addBone(ai_node_anim->mNodeName.C_Str(), gfxm::mat4(1.0f));
        }

        AssetParams anim_asset_params = 
            asset_params.get_object("Animation").get_object(ai_anim->mName.C_Str());
        bool is_additive = anim_asset_params.get_bool("Additive", false);
        bool enable_root_motion = anim_asset_params.get_bool("EnableRootMotion", false);
        std::string root_motion_node_name = anim_asset_params.get_string("RootMotionNode", "");

        std::shared_ptr<Animation> anim(new Animation());
        anim->Storage(Resource::GLOBAL);
        anim->Name(ai_anim->mName.C_Str());

        anim->length                = (float)len;
        anim->fps                   = (float)fps;
        anim->root_motion_enabled   = enable_root_motion;
        anim->root_motion_node_name = root_motion_node_name;
        
        for(unsigned j = 0; j < ai_anim->mNumChannels; ++j) {
            aiNodeAnim* ai_node_anim = ai_anim->mChannels[j];
            std::string bone_node = ai_node_anim->mNodeName.C_Str();
            Skeleton::Bone* bone = skeleton->getBone(bone_node);
            if(!bone) continue;

            if(root_motion_node_name == bone_node) {
                animNodeFromAssimpNode(ai_node_anim, anim->getRootMotionNode());
                continue;
            }

            AnimNode& node = anim->getNode(bone_node);

            animNodeFromAssimpNode(ai_node_anim, node);
        }

        if(is_additive) {
            // TODO: Convert to additive?
        }

        std::string fname = MKSTR(dirname << "\\" << replace_reserved_chars(ai_anim->mName.C_Str(), '_') << ".anim");
        anim->write_to_file(fname);
        LOG("New anim file: " << fname);
        GlobalDataRegistry().Add(
            fname,
            DataSourceRef(new DataSourceFilesystem(get_module_dir() + "\\" + fname))
        );
    }
}

inline void resourcesFromAssimpScene(
    const aiScene* ai_scene, 
    Scene* scene, 
    std::shared_ptr<Skeleton> skeleton,
    AssetParams& asset_params,
    const std::string& dirname
) {
    meshesFromAssimpScene(ai_scene, scene, skeleton, asset_params, dirname);
    animationsFromAssimpScene(ai_scene, scene, skeleton, asset_params, dirname);
}

inline void objectFromAssimpNode(
    const aiScene* ai_scene, 
    aiNode* node, 
    SceneObject* root, 
    SceneObject* object, 
    size_t base_mesh_index, 
    std::vector<std::function<void(void)>>& tasks,
    std::shared_ptr<Skeleton> skeleton,
    const std::string& dirname
) {
    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        objectFromAssimpNode(ai_scene, node->mChildren[i], root, object->createChild(), base_mesh_index, tasks, skeleton, dirname);
    }

    object->setName(node->mName.C_Str());
    object->get<Transform>()->setTransform(gfxm::transpose(*(gfxm::mat4*)&node->mTransformation));

    if(node->mNumMeshes > 0) {
        Model* m = object->get<Model>();
        m->mesh = getResource<Mesh>(MKSTR(dirname << "\\" << node->mMeshes[0] << ".mesh"));
        // TODO: Set material
        aiMesh* ai_mesh = ai_scene->mMeshes[node->mMeshes[0]];
        if(ai_mesh->mNumBones) {
            Skin* skin = object->get<Skin>();
            for(unsigned j = 0; j < ai_mesh->mNumBones; ++j) {
                aiBone* bone = ai_mesh->mBones[j];
                std::string name(bone->mName.data, bone->mName.length);

                tasks.emplace_back([skin, bone, name, object, root, skeleton](){
                    SceneObject* so = root->findObject(name);
                    if(so) {
                        skin->bones.emplace_back(so->get<Transform>()->getId());
                        skin->bind_pose.emplace_back(gfxm::transpose(*(gfxm::mat4*)&bone->mOffsetMatrix));
                        // TODO: Refactor?
                        skeleton->addBone(bone->mName.C_Str(), *(gfxm::mat4*)&bone->mOffsetMatrix);
                    }
                });
            }
        }
    }
}

inline gfxm::vec2 sampleSphericalMap(gfxm::vec3 v) {
    const gfxm::vec2 invAtan = gfxm::vec2(0.1591f, 0.3183f);
    gfxm::vec2 uv = gfxm::vec2(atan2f(v.z, v.x), asinf(v.y));
    uv *= invAtan;
    uv = gfxm::vec2(uv.x + 0.5, uv.y + 0.5);
    return uv;
}

inline bool sceneFromFbx(const std::string& filename, Scene* scene, SceneObject* root_node = 0) {
    // Create subdir to store all resources extracted from this file
    
    std::string dirname = filename;
    std::replace( dirname.begin(), dirname.end(), '.', '_'); 
    CreateDirectoryA(dirname.c_str(), 0);
    

    AssetParams asset_params;
    asset_params.load(filename + ".asset_params");
    
    const aiScene* ai_scene = aiImportFile(
        filename.c_str(), 
        aiProcess_GenSmoothNormals |
        aiProcess_GenUVCoords |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_LimitBoneWeights |
        aiProcess_GlobalScale
    );

    std::vector<aiMesh*> meshes_with_fallback_uv;
    unsigned int mesh_count = ai_scene->mNumMeshes;
    for(unsigned int i = 0; i < mesh_count; ++i) {
        aiMesh* ai_mesh = ai_scene->mMeshes[i];

        if(ai_mesh->HasTextureCoords(0) == false) {
            meshes_with_fallback_uv.emplace_back(ai_mesh);
            ai_mesh->mTextureCoords[0] = new aiVector3D[ai_mesh->mNumVertices];
            // Generate uvs
            LOG("No uvs found, generating...")
            std::vector<float> uvs;
            for(unsigned j = 0; j < ai_mesh->mNumVertices; ++j) {
                gfxm::vec2 uv = sampleSphericalMap(
                    gfxm::normalize(gfxm::vec3(
                        ai_mesh->mVertices[j].x,
                        ai_mesh->mVertices[j].y,
                        ai_mesh->mVertices[j].z
                    ))
                );
                ai_mesh->mTextureCoords[0][j].x = uv.x;
                ai_mesh->mTextureCoords[0][j].y = uv.y;
            }
            LOG("Done");
        }
    }

    ai_scene = aiApplyPostProcessing(
        ai_scene,
        aiProcess_CalcTangentSpace
    );

    if(!ai_scene) {
        LOG_WARN("Failed to import " << filename);
        return false;
    }
    aiNode* ai_rootNode = ai_scene->mRootNode;
    if(!ai_rootNode) {
        LOG_WARN("Fbx import: No root node (" << filename << ")");
        return false;
    }

    SceneObject* root = root_node == 0 ? scene->getRootObject() : root_node;
    if(ai_scene->mMetaData) {
        double scaleFactor = 1.0;
        if(ai_scene->mMetaData->Get("UnitScaleFactor", scaleFactor)) {
            if(scaleFactor == 0.0) scaleFactor = 1.0f;
            scaleFactor *= 0.01;
            root->get<Transform>()->scale((float)scaleFactor);
            if(root->getParent()) {
                gfxm::vec3 a = root->get<Transform>()->scale();
                gfxm::vec3 b = root->getParent()->get<Transform>()->scale();
                root->get<Transform>()->scale(gfxm::vec3(
                    a.x / b.x, a.y / b.y, a.z / b.z
                ));
            }
        } else {
            // TODO ?
        }
    }
    auto base_mesh_index = scene->localResourceCount<Mesh>();
    auto base_anim_index = scene->localResourceCount<Animation>();

    std::vector<std::function<void(void)>> deferred_tasks;

    std::shared_ptr<Skeleton> skeleton(new Skeleton());
    skeleton->Storage(Resource::LOCAL);
    skeleton->Name("Skeleton");

    resourcesFromAssimpScene(ai_scene, scene, skeleton, asset_params, dirname);

    for(unsigned i = 0; i < ai_rootNode->mNumChildren; ++i) {
        objectFromAssimpNode(
            ai_scene, 
            ai_rootNode->mChildren[i],
            root, 
            root->createChild(), 
            base_mesh_index, 
            deferred_tasks,
            skeleton,
            dirname
        );
    }

    std::string skel_res_name = MKSTR(dirname << "\\" << "Skeleton" << ".skel");
    skeleton->write_to_file(skel_res_name);
    GlobalDataRegistry().Add(
        skel_res_name,
        DataSourceRef(new DataSourceFilesystem(get_module_dir() + "\\" + skel_res_name))
    );

    if(ai_scene->mNumAnimations > 0) {
        auto animator = root->get<Animator>();
        animator->setSkeleton(getResource<Skeleton>(skel_res_name));
    }

    for(unsigned int i = 0; i < ai_scene->mNumAnimations; ++i) {
        auto animator = root->get<Animator>();
        std::string fname = MKSTR(dirname << "\\" << replace_reserved_chars(ai_scene->mAnimations[i]->mName.C_Str(), '_') << ".anim");
        animator->addAnim(getResource<Animation>(fname));

        //animator->addAnim(scene->getLocalResource<Animation>(base_anim_index + i));
    }

    for(auto& t : deferred_tasks) {
        t();
    }

    std::vector<std::string> tokens = split(filename, '\\');
    if(!tokens.empty()) {
        std::string name = tokens[tokens.size() - 1];
        tokens = split(name, '.');
        name = name.substr(0, name.find_last_of("."));
        root->setName(name);
    } else {
        root->setName("object");
    }

    LOG("Fbx skeleton size: " << skeleton->boneCount());

    for(auto m : meshes_with_fallback_uv) {
        delete[] m->mTextureCoords[0];
        m->mTextureCoords[0] = 0;
    }
    aiReleaseImport(ai_scene);

    return true;
}

#endif
