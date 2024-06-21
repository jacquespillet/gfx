#define SceneDescriptorSetBinding 0
#define CameraDescriptorSetBinding 1
#define ModelDescriptorSetBinding 2
#define MaterialDescriptorSetBinding 3
#define GBufferDescriptorSetBinding 4
#define ReflectionsDescriptorSetBinding 5

//Each of the following bindings must be unique
//Because there's no descriptor set in opengl and d3d so no overlapping possible
#define CameraBinding 0
#define ModelBinding 1
#define SceneBinding 2
#define MaterialDataBinding 3

#define BaseColorTextureBinding 4
#define MetallicRoughnessTextureBinding 5
#define OcclusionTextureBinding 6
#define NormalTextureBinding 7
#define EmissiveTextureBinding 8 


#define PointLight 0
#define DirectionalLight 1
#define SpotLight 2
#define AreaLight 2

#define ShadowMapsBinding 10

#define GBufferPositionBinding 11
#define GBufferNormalBinding 12
#define GBufferAlbedoBinding 13
#define GBufferEmissionBinding 14
#define GBufferReflectionBinding 15

#define MaxLights 16