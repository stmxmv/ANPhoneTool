#ifndef AN_REALTIMELIGHTS_HLSL
#define AN_REALTIMELIGHTS_HLSL

// Abstraction over Light shading data.
struct Light {
    half3   direction;
    half3   color;
    float   distanceAttenuation; // full-float precision required on some platforms
    half    shadowAttenuation;
    uint    layerMask;
};



#endif//AN_REALTIMELIGHTS_HLSL