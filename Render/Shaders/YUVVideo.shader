Shader "AN/YUVVideo"
{
    Properties
    {
        _MainTex("src tex", 2D) = "white" {}
        _YTex("Y tex", 2D) = "white" {}
        _UTex("U tex", 2D) = "white" {}
        _VTex("V tex", 2D) = "white" {}
    }
    SubShader
    {
        Tags { "RenderType" = "Opaque" "RenderPipeline" = "UniversalRenderPipeline" }

        HLSLINCLUDE

        cbuffer ANPerMaterial {
            SamplerState sampler_YTex;
            Texture2D _YTex;
            Texture2D _UTex;
            Texture2D _VTex;
        }


        ENDHLSL

        Pass
        {
            Tags { "LightMode" = "Forward" }

            Cull Off
            ZTest Always
            ZWrite Off

            HLSLPROGRAM

            #define vertex vertex_main // define vertex shader entry
            #define frag fragment_main // define fragment shader entry


			struct appdata_t {
				float4 vertex : POSITION;
				float2 texcoord : TEXCOORD0;
			};

			struct v2f {
				float4 vertex : SV_Position;
				float2 texcoord : TEXCOORD0;
			};



            v2f vertex(appdata_t v)
            {
				v2f o;
				o.vertex = v.vertex;
				o.texcoord = v.texcoord.xy;
				return o;
            }

            static const float3x3 YUVToRGBMatrix = float3x3(1.0, 1.0, 1.0,
                                                            0.0, -0.39465, 2.03211,
                                                            1.13983, -0.58060, 0.0);

            half4 frag(v2f i) : SV_TARGET
            {
                float3 yuv;
                yuv.x = _YTex.Sample(sampler_YTex, i.texcoord).r;
                yuv.y = _UTex.Sample(sampler_YTex, i.texcoord).r - 0.5;
                yuv.z = _VTex.Sample(sampler_YTex, i.texcoord).r - 0.5;


                // SDL2 BT709_SHADER_CONSTANTS
                // https://github.com/spurious/SDL-mirror/blob/4ddd4c445aa059bb127e101b74a8c5b59257fbe2/src/render/opengl/SDL_shaders_gl.c#L102
                const float3 Rcoeff = float3(1.1644,  0.000,  1.7927);
                const float3 Gcoeff = float3(1.1644, -0.2132, -0.5329);
                const float3 Bcoeff = float3(1.1644,  2.1124,  0.000);

                float3 rgb;
                yuv.x = yuv.x - 0.0625;
                rgb.r = dot(yuv, Rcoeff);
                rgb.g = dot(yuv, Gcoeff);
                rgb.b = dot(yuv, Bcoeff);

//                 float3 rgb = mul(yuv, YUVToRGBMatrix);

                return half4(rgb, 1.0);
            }
            ENDHLSL
        }
    }
}