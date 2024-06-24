#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

struct vertex
{
  vec4 PositionUvX;
  vec4 NormalUvY; 
  vec4 Tangent; 
};
layout (set = 5, binding = 20, std430) buffer VerticesBuffer
{
    vertex Vertices[];
};

layout (set = 5, binding = 21, std430) buffer OffsetsBuffer
{
    uint Offsets[];
};

layout (set = 5, binding = 22, std430) buffer IndexBuffer
{
    uint Indices[];
};


hitAttributeEXT vec2 attribs;

void main()
{
  // TODO: 
  // Stop using UUID, and use simple IDs instead                                                                                        OK
  // Mesh.h : Store a GeometryID and a MaterialID instead of pointers, that will point to the objects in the Project object.            OK

  // During scene creation, if RTX is enabled, create a bindless descriptor set that will contain all the vertex buffers in the project.
  
  // Bindless vertex buffer that stores all the mesh vertex buffers, and store the MeshID into instance custom index
  // Bindless material uniform buffers, so we can access them from here
  // Bindless textures in order to sample any texture from the scene.
  
  uint Offset = Offsets[gl_InstanceCustomIndexEXT];

  uint Primitive = gl_PrimitiveID * 3;
  uint Inx0 = Indices[Offset + Primitive + 0];
  uint Inx1 = Indices[Offset + Primitive + 1];
  uint Inx2 = Indices[Offset + Primitive + 2];

  vertex V0 = Vertices[Inx0];
  vertex V1 = Vertices[Inx1];
  vertex V2 = Vertices[Inx2];

  vec2 UV0 = vec2(V0.PositionUvX.w, V0.NormalUvY.w);
  vec2 UV1 = vec2(V1.PositionUvX.w, V1.NormalUvY.w);
  vec2 UV2 = vec2(V2.PositionUvX.w, V2.NormalUvY.w);


  const vec3 Barycentric = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  vec2 UV = Barycentric.x * UV0
                + Barycentric.y * UV1
                + Barycentric.z * UV2;
  // vec3 Normal = Barycentric.x * V0.NormalUvY.xyz
  //               + Barycentric.y * V1.NormalUvY.xyz
  //               + Barycentric.z * V2.NormalUvY.xyz;
  //               + Barycentric.z * V2.NormalUvY.xyz;

  // vec3 Normal = V0.NormalUvY.xyz;
  hitValue = vec3(UV, 0);
  // hitValue = Normal;
}
