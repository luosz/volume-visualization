// Data
uniform sampler1D TransferFunction;
uniform sampler3D VolumeData;
uniform sampler2D RayEnd;
uniform sampler2D RayStart;

// Lighting Stuff
const vec3 LightPosition = vec3(0.0, 1.0, 1.0);
const float SpecularContribution = 0.3;
const float DiffuseContribution = 1.0 - SpecularContribution;
const float cellSize = 1.0 / 256.0;

// RayCasting stuff
const float stepSize = 0.001;

vec3 traverseVector;
vec3 ec;

// Compute the Normal around the current voxel
vec3 getNormal(vec3 at)
{
    vec3 n = vec3(texture3D(VolumeData, at - vec3(cellSize, 0.0, 0.0)).w - texture3D(VolumeData, at + vec3(cellSize, 0.0, 0.0)).w,
                  texture3D(VolumeData, at - vec3(0.0, cellSize, 0.0)).w - texture3D(VolumeData, at + vec3(0.0, cellSize, 0.0)).w,
                  texture3D(VolumeData, at - vec3(0.0, 0.0, cellSize)).w - texture3D(VolumeData, at + vec3(0.0, 0.0, cellSize)).w
                 );
    
    return normalize(n);
}

float getLightIntensity(vec3 norm)
{
    float LightIntensity = 1.0;
    vec3 lightVec = normalize(LightPosition - ec);
    vec3 viewVec = normalize(-ec);

    vec3 reflectVec = reflect(-lightVec, norm);
    float diffuse = max(dot(lightVec, norm), 0.3);
    float spec = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.2);
        spec = pow(spec, 16.0);            
    }
    
    return SpecularContribution*spec + DiffuseContribution*diffuse;
}

void main(void)
{

    // Get the end point of the ray (from the front-culled faces rendering)
    vec3 rayStart = texture2D(RayStart, gl_TexCoord[0].st).xyz;
    vec3 rayEnd = texture2D(RayEnd, gl_TexCoord[0].st).xyz;
    
    // Get a vector from back to front
    traverseVector = rayEnd - rayStart;

    // The maximum length of the ray
    float maxLength = length(traverseVector);
      
    // Construct a ray in the correct direction and of step length
    ec = normalize(traverseVector);
    vec3 step = stepSize * ec;
    vec3 ray = vec3(0.0, 0.0, 0.0);
   
        // The color accumulation buffer
    vec4 acc = vec4(0.0, 0.0, 0.0, 0.0);

    // Holds current voxel color
    vec4 voxelColor;

    // Lighting
    float LightIntensity = 1.0;
    
    // Advance ray
    for (int i = 0; i < int(1.0/stepSize) + 1; ++i)
    {
        if (length(ray) >= maxLength || acc.a >= 1.0) 
        {
            acc.a = 1.0;
            break;
        }

        LightIntensity = getLightIntensity(getNormal(ray + rayStart));
        
        voxelColor = texture1D(TransferFunction, texture3D(VolumeData, ray + rayStart).w);

        // Accumulate RGB : acc.rgb = voxelColor.rgb*voxelColor.a + (1.0 - voxelColor.a)*acc.rgb;
        acc.rgb = mix(acc.rgb, voxelColor.rgb, voxelColor.a)*LightIntensity;

        // Accumulate Opacity: acc.a = acc.a + (1.0 - acc.a)*voxelColor.a;
        acc.a = mix(voxelColor.a, 1.0, acc.a);

        ray += step;

    }

    gl_FragColor = acc;
    return;
}
