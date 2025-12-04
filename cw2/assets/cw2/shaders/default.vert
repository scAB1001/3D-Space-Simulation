#version 430

// Vertex attributes for colored objects
layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;

// Uniforms
layout( location = 0 ) uniform mat4 uProjCameraWorld;
layout( location = 1 ) uniform mat3 uNormalMatrix;

// Outputs to fragment shader
out vec3 v2fColor;
out vec3 v2fNormal;

void main()
{
    // Pass through vertex color
    v2fColor = iColor;

    // Transform normal using normal matrix
    v2fNormal = normalize( uNormalMatrix * iNormal );

    // Transform position
    gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );
}