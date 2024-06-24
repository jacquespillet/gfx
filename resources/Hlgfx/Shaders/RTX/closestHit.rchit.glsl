#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

void main()
{
  // TODO: 
  // Stop using UUID, and use simple IDs instead
  // Change the way we serialize the project (Use the same as in gpupt...)
  // Mesh.h : Store a GeometryID and a MaterialID instead of pointers, that will point to the objects in the Project object.

  // During scene creation, if RTX is enabled, create a bindless descriptor set that will contain all the vertex buffers in the project.
  
  // Bindless vertex buffer that stores all the mesh vertex buffers, and store the MeshID into instance custom index
  // Bindless material uniform buffers, so we can access them from here
  // Bindless textures in order to sample any texture from the scene.



  const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  hitValue = barycentricCoords;
}
