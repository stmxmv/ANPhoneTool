#ifndef SHADOW_SAMPLING_TENT_INCLUDED
#define SHADOW_SAMPLING_TENT_INCLUDED
// ------------------------------------------------------------------
//  PCF Filtering Tent Functions
// ------------------------------------------------------------------

// Assuming a isoceles right angled triangle of height "triangleHeight" (as drawn below).
// This function return the area of the triangle above the first texel.
//
// |\      <-- 45 degree slop isosceles right angled triangle
// | \
// ----    <-- length of this side is "triangleHeight"
// _ _ _ _ <-- texels
float SampleShadow_GetTriangleTexelArea(float triangleHeight)
{
    return triangleHeight - 0.5;
}

// Assuming a isoceles triangle of 1.5 texels height and 3 texels wide lying on 4 texels.
// This function return the area of the triangle above each of those texels.
//    |    <-- offset from -0.5 to 0.5, 0 meaning triangle is exactly in the center
//   / \   <-- 45 degree slop isosceles triangle (ie tent projected in 2D)
//  /   \
// _ _ _ _ <-- texels
// X Y Z W <-- result indices (in computedArea.xyzw and computedAreaUncut.xyzw)
void SampleShadow_GetTexelAreas_Tent_3x3(float offset, out float4 computedArea, out float4 computedAreaUncut)
{
    // Compute the exterior areas
    float offset01SquaredHalved = (offset + 0.5) * (offset + 0.5) * 0.5;
    computedAreaUncut.x = computedArea.x = offset01SquaredHalved - offset;
    computedAreaUncut.w = computedArea.w = offset01SquaredHalved;

    // Compute the middle areas
    // For Y : We find the area in Y of as if the left section of the isoceles triangle would
    // intersect the axis between Y and Z (ie where offset = 0).
    computedAreaUncut.y = SampleShadow_GetTriangleTexelArea(1.5 - offset);
    // This area is superior to the one we are looking for if (offset < 0) thus we need to
    // subtract the area of the triangle defined by (0,1.5-offset), (0,1.5+offset), (-offset,1.5).
    float clampedOffsetLeft = min(offset,0);
    float areaOfSmallLeftTriangle = clampedOffsetLeft * clampedOffsetLeft;
    computedArea.y = computedAreaUncut.y - areaOfSmallLeftTriangle;

    // We do the same for the Z but with the right part of the isoceles triangle
    computedAreaUncut.z = SampleShadow_GetTriangleTexelArea(1.5 + offset);
    float clampedOffsetRight = max(offset,0);
    float areaOfSmallRightTriangle = clampedOffsetRight * clampedOffsetRight;
    computedArea.z = computedAreaUncut.z - areaOfSmallRightTriangle;
}

// Assuming a isoceles triangle of 1.5 texels height and 3 texels wide lying on 4 texels.
// This function return the weight of each texels area relative to the full triangle area.
void SampleShadow_GetTexelWeights_Tent_3x3(float offset, out float4 computedWeight)
{
    float4 dummy;
    SampleShadow_GetTexelAreas_Tent_3x3(offset, computedWeight, dummy);
    computedWeight *= 0.44444;//0.44 == 1/(the triangle area)
}

// Assuming a isoceles triangle of 2.5 texel height and 5 texels wide lying on 6 texels.
// This function return the weight of each texels area relative to the full triangle area.
//  /       \
// _ _ _ _ _ _ <-- texels
// 0 1 2 3 4 5 <-- computed area indices (in texelsWeights[])
void SampleShadow_GetTexelWeights_Tent_5x5(float offset, out float3 texelsWeightsA, out float3 texelsWeightsB)
{
    // See _UnityInternalGetAreaPerTexel_3TexelTriangleFilter for details.
    float4 computedArea_From3texelTriangle;
    float4 computedAreaUncut_From3texelTriangle;
    SampleShadow_GetTexelAreas_Tent_3x3(offset, computedArea_From3texelTriangle, computedAreaUncut_From3texelTriangle);

    // Triangle slope is 45 degree thus we can almost reuse the result of the 3 texel wide computation.
    // the 5 texel wide triangle can be seen as the 3 texel wide one but shifted up by one unit/texel.
    // 0.16 is 1/(the triangle area)
    texelsWeightsA.x = 0.16 * (computedArea_From3texelTriangle.x);
    texelsWeightsA.y = 0.16 * (computedAreaUncut_From3texelTriangle.y);
    texelsWeightsA.z = 0.16 * (computedArea_From3texelTriangle.y + 1);
    texelsWeightsB.x = 0.16 * (computedArea_From3texelTriangle.z + 1);
    texelsWeightsB.y = 0.16 * (computedAreaUncut_From3texelTriangle.z);
    texelsWeightsB.z = 0.16 * (computedArea_From3texelTriangle.w);
}

// Assuming a isoceles triangle of 3.5 texel height and 7 texels wide lying on 8 texels.
// This function return the weight of each texels area relative to the full triangle area.
//  /           \
// _ _ _ _ _ _ _ _ <-- texels
// 0 1 2 3 4 5 6 7 <-- computed area indices (in texelsWeights[])
void SampleShadow_GetTexelWeights_Tent_7x7(float offset, out float4 texelsWeightsA, out float4 texelsWeightsB)
{
    // See _UnityInternalGetAreaPerTexel_3TexelTriangleFilter for details.
    float4 computedArea_From3texelTriangle;
    float4 computedAreaUncut_From3texelTriangle;
    SampleShadow_GetTexelAreas_Tent_3x3(offset, computedArea_From3texelTriangle, computedAreaUncut_From3texelTriangle);

    // Triangle slope is 45 degree thus we can almost reuse the result of the 3 texel wide computation.
    // the 7 texel wide triangle can be seen as the 3 texel wide one but shifted up by two unit/texel.
    // 0.081632 is 1/(the triangle area)
    texelsWeightsA.x = 0.081632 * (computedArea_From3texelTriangle.x);
    texelsWeightsA.y = 0.081632 * (computedAreaUncut_From3texelTriangle.y);
    texelsWeightsA.z = 0.081632 * (computedAreaUncut_From3texelTriangle.y + 1);
    texelsWeightsA.w = 0.081632 * (computedArea_From3texelTriangle.y + 2);
    texelsWeightsB.x = 0.081632 * (computedArea_From3texelTriangle.z + 2);
    texelsWeightsB.y = 0.081632 * (computedAreaUncut_From3texelTriangle.z + 1);
    texelsWeightsB.z = 0.081632 * (computedAreaUncut_From3texelTriangle.z);
    texelsWeightsB.w = 0.081632 * (computedArea_From3texelTriangle.w);
}

// 3x3 Tent filter (45 degree sloped triangles in U and V)
void SampleShadow_ComputeSamples_Tent_3x3(float4 shadowMapTexture_TexelSize, float2 coord, out float fetchesWeights[4], out float2 fetchesUV[4])
{
    // tent base is 3x3 base thus covering from 9 to 12 texels, thus we need 4 bilinear PCF fetches
    float2 tentCenterInTexelSpace = coord.xy * shadowMapTexture_TexelSize.zw;
    float2 centerOfFetchesInTexelSpace = floor(tentCenterInTexelSpace + 0.5);
    float2 offsetFromTentCenterToCenterOfFetches = tentCenterInTexelSpace - centerOfFetchesInTexelSpace;

    // find the weight of each texel based
    float4 texelsWeightsU, texelsWeightsV;
    SampleShadow_GetTexelWeights_Tent_3x3(offsetFromTentCenterToCenterOfFetches.x, texelsWeightsU);
    SampleShadow_GetTexelWeights_Tent_3x3(offsetFromTentCenterToCenterOfFetches.y, texelsWeightsV);

    // each fetch will cover a group of 2x2 texels, the weight of each group is the sum of the weights of the texels
    float2 fetchesWeightsU = texelsWeightsU.xz + texelsWeightsU.yw;
    float2 fetchesWeightsV = texelsWeightsV.xz + texelsWeightsV.yw;

    // move the PCF bilinear fetches to respect texels weights
    float2 fetchesOffsetsU = texelsWeightsU.yw / fetchesWeightsU.xy + float2(-1.5,0.5);
    float2 fetchesOffsetsV = texelsWeightsV.yw / fetchesWeightsV.xy + float2(-1.5,0.5);
    fetchesOffsetsU *= shadowMapTexture_TexelSize.xx;
    fetchesOffsetsV *= shadowMapTexture_TexelSize.yy;

    float2 bilinearFetchOrigin = centerOfFetchesInTexelSpace * shadowMapTexture_TexelSize.xy;
    fetchesUV[0] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.x);
    fetchesUV[1] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.x);
    fetchesUV[2] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.y);
    fetchesUV[3] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.y);

    fetchesWeights[0] = fetchesWeightsU.x * fetchesWeightsV.x;
    fetchesWeights[1] = fetchesWeightsU.y * fetchesWeightsV.x;
    fetchesWeights[2] = fetchesWeightsU.x * fetchesWeightsV.y;
    fetchesWeights[3] = fetchesWeightsU.y * fetchesWeightsV.y;
}

// 5x5 Tent filter (45 degree sloped triangles in U and V)
void SampleShadow_ComputeSamples_Tent_5x5(float4 shadowMapTexture_TexelSize, float2 coord, out float fetchesWeights[9], out float2 fetchesUV[9])
{
    // tent base is 5x5 base thus covering from 25 to 36 texels, thus we need 9 bilinear PCF fetches
    float2 tentCenterInTexelSpace = coord.xy * shadowMapTexture_TexelSize.zw;
    float2 centerOfFetchesInTexelSpace = floor(tentCenterInTexelSpace + 0.5);
    float2 offsetFromTentCenterToCenterOfFetches = tentCenterInTexelSpace - centerOfFetchesInTexelSpace;

    // find the weight of each texel based on the area of a 45 degree slop tent above each of them.
    float3 texelsWeightsU_A, texelsWeightsU_B;
    float3 texelsWeightsV_A, texelsWeightsV_B;
    SampleShadow_GetTexelWeights_Tent_5x5(offsetFromTentCenterToCenterOfFetches.x, texelsWeightsU_A, texelsWeightsU_B);
    SampleShadow_GetTexelWeights_Tent_5x5(offsetFromTentCenterToCenterOfFetches.y, texelsWeightsV_A, texelsWeightsV_B);

    // each fetch will cover a group of 2x2 texels, the weight of each group is the sum of the weights of the texels
    float3 fetchesWeightsU = float3(texelsWeightsU_A.xz, texelsWeightsU_B.y) + float3(texelsWeightsU_A.y, texelsWeightsU_B.xz);
    float3 fetchesWeightsV = float3(texelsWeightsV_A.xz, texelsWeightsV_B.y) + float3(texelsWeightsV_A.y, texelsWeightsV_B.xz);

    // move the PCF bilinear fetches to respect texels weights
    float3 fetchesOffsetsU = float3(texelsWeightsU_A.y, texelsWeightsU_B.xz) / fetchesWeightsU.xyz + float3(-2.5,-0.5,1.5);
    float3 fetchesOffsetsV = float3(texelsWeightsV_A.y, texelsWeightsV_B.xz) / fetchesWeightsV.xyz + float3(-2.5,-0.5,1.5);
    fetchesOffsetsU *= shadowMapTexture_TexelSize.xxx;
    fetchesOffsetsV *= shadowMapTexture_TexelSize.yyy;

    float2 bilinearFetchOrigin = centerOfFetchesInTexelSpace * shadowMapTexture_TexelSize.xy;
    fetchesUV[0] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.x);
    fetchesUV[1] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.x);
    fetchesUV[2] = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.x);
    fetchesUV[3] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.y);
    fetchesUV[4] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.y);
    fetchesUV[5] = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.y);
    fetchesUV[6] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.z);
    fetchesUV[7] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.z);
    fetchesUV[8] = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.z);

    fetchesWeights[0] = fetchesWeightsU.x * fetchesWeightsV.x;
    fetchesWeights[1] = fetchesWeightsU.y * fetchesWeightsV.x;
    fetchesWeights[2] = fetchesWeightsU.z * fetchesWeightsV.x;
    fetchesWeights[3] = fetchesWeightsU.x * fetchesWeightsV.y;
    fetchesWeights[4] = fetchesWeightsU.y * fetchesWeightsV.y;
    fetchesWeights[5] = fetchesWeightsU.z * fetchesWeightsV.y;
    fetchesWeights[6] = fetchesWeightsU.x * fetchesWeightsV.z;
    fetchesWeights[7] = fetchesWeightsU.y * fetchesWeightsV.z;
    fetchesWeights[8] = fetchesWeightsU.z * fetchesWeightsV.z;
}

// 7x7 Tent filter (45 degree sloped triangles in U and V)
void SampleShadow_ComputeSamples_Tent_7x7(float4 shadowMapTexture_TexelSize, float2 coord, out float fetchesWeights[16], out float2 fetchesUV[16])
{
    // tent base is 7x7 base thus covering from 49 to 64 texels, thus we need 16 bilinear PCF fetches
    float2 tentCenterInTexelSpace = coord.xy * shadowMapTexture_TexelSize.zw;
    float2 centerOfFetchesInTexelSpace = floor(tentCenterInTexelSpace + 0.5);
    float2 offsetFromTentCenterToCenterOfFetches = tentCenterInTexelSpace - centerOfFetchesInTexelSpace;

    // find the weight of each texel based on the area of a 45 degree slop tent above each of them.
    float4 texelsWeightsU_A, texelsWeightsU_B;
    float4 texelsWeightsV_A, texelsWeightsV_B;
    SampleShadow_GetTexelWeights_Tent_7x7(offsetFromTentCenterToCenterOfFetches.x, texelsWeightsU_A, texelsWeightsU_B);
    SampleShadow_GetTexelWeights_Tent_7x7(offsetFromTentCenterToCenterOfFetches.y, texelsWeightsV_A, texelsWeightsV_B);

    // each fetch will cover a group of 2x2 texels, the weight of each group is the sum of the weights of the texels
    float4 fetchesWeightsU = float4(texelsWeightsU_A.xz, texelsWeightsU_B.xz) + float4(texelsWeightsU_A.yw, texelsWeightsU_B.yw);
    float4 fetchesWeightsV = float4(texelsWeightsV_A.xz, texelsWeightsV_B.xz) + float4(texelsWeightsV_A.yw, texelsWeightsV_B.yw);

    // move the PCF bilinear fetches to respect texels weights
    float4 fetchesOffsetsU = float4(texelsWeightsU_A.yw, texelsWeightsU_B.yw) / fetchesWeightsU.xyzw + float4(-3.5,-1.5,0.5,2.5);
    float4 fetchesOffsetsV = float4(texelsWeightsV_A.yw, texelsWeightsV_B.yw) / fetchesWeightsV.xyzw + float4(-3.5,-1.5,0.5,2.5);
    fetchesOffsetsU *= shadowMapTexture_TexelSize.xxxx;
    fetchesOffsetsV *= shadowMapTexture_TexelSize.yyyy;

    float2 bilinearFetchOrigin = centerOfFetchesInTexelSpace * shadowMapTexture_TexelSize.xy;
    fetchesUV[0]  = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.x);
    fetchesUV[1]  = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.x);
    fetchesUV[2]  = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.x);
    fetchesUV[3]  = bilinearFetchOrigin + float2(fetchesOffsetsU.w, fetchesOffsetsV.x);
    fetchesUV[4]  = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.y);
    fetchesUV[5]  = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.y);
    fetchesUV[6]  = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.y);
    fetchesUV[7]  = bilinearFetchOrigin + float2(fetchesOffsetsU.w, fetchesOffsetsV.y);
    fetchesUV[8]  = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.z);
    fetchesUV[9]  = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.z);
    fetchesUV[10] = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.z);
    fetchesUV[11] = bilinearFetchOrigin + float2(fetchesOffsetsU.w, fetchesOffsetsV.z);
    fetchesUV[12] = bilinearFetchOrigin + float2(fetchesOffsetsU.x, fetchesOffsetsV.w);
    fetchesUV[13] = bilinearFetchOrigin + float2(fetchesOffsetsU.y, fetchesOffsetsV.w);
    fetchesUV[14] = bilinearFetchOrigin + float2(fetchesOffsetsU.z, fetchesOffsetsV.w);
    fetchesUV[15] = bilinearFetchOrigin + float2(fetchesOffsetsU.w, fetchesOffsetsV.w);

    fetchesWeights[0]  = fetchesWeightsU.x * fetchesWeightsV.x;
    fetchesWeights[1]  = fetchesWeightsU.y * fetchesWeightsV.x;
    fetchesWeights[2]  = fetchesWeightsU.z * fetchesWeightsV.x;
    fetchesWeights[3]  = fetchesWeightsU.w * fetchesWeightsV.x;
    fetchesWeights[4]  = fetchesWeightsU.x * fetchesWeightsV.y;
    fetchesWeights[5]  = fetchesWeightsU.y * fetchesWeightsV.y;
    fetchesWeights[6]  = fetchesWeightsU.z * fetchesWeightsV.y;
    fetchesWeights[7]  = fetchesWeightsU.w * fetchesWeightsV.y;
    fetchesWeights[8]  = fetchesWeightsU.x * fetchesWeightsV.z;
    fetchesWeights[9]  = fetchesWeightsU.y * fetchesWeightsV.z;
    fetchesWeights[10] = fetchesWeightsU.z * fetchesWeightsV.z;
    fetchesWeights[11] = fetchesWeightsU.w * fetchesWeightsV.z;
    fetchesWeights[12] = fetchesWeightsU.x * fetchesWeightsV.w;
    fetchesWeights[13] = fetchesWeightsU.y * fetchesWeightsV.w;
    fetchesWeights[14] = fetchesWeightsU.z * fetchesWeightsV.w;
    fetchesWeights[15] = fetchesWeightsU.w * fetchesWeightsV.w;
}
#endif
