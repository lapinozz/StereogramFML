#ifndef STEREOGRAME_H
#define STEREOGRAME_H

#include <string>
#include <vector>

#include <functional>

#include <SFML/Graphics.hpp>

class Stereograme : public sf::RenderTexture, public sf::Drawable, public sf::Transformable
{
    public:
        Stereograme();
        virtual ~Stereograme() = default;

        static std::string textHighlight(const std::string& text, int width = 63);
        static std::string ascii(const std::vector<char>& depthMap, size_t width, const std::string& pattern, size_t depth = 7);

        void setPatternTexture(const sf::Texture& texture);

        void useExternalTexture(const sf::Texture& texture);
        void useInternalTexture();

        void setDepthScale(float scale);
        void setPixelRepeat(float repeat);

        void setDrawDots(bool drawDots, float dotsRadius = 12.f, float dotOffsetFromTop = 8.f, bool recompute = true);

        void setInvertedDepth(bool invertDepth, bool recompute = true);
        void setShowDepthImage(bool showDepthImage, bool recompute = true);

        void setAddColour(bool addColour, bool doubleColour = true, bool recompute = true);

        void setAnimate(bool animated, bool recompute = true);

        void setRandomBackDrop(bool randomBackDrop, bool perLine = true, bool recompute = true);

        const sf::Texture& getInternalTexture() const;
        const sf::Texture& getCurrentTexture() const;

        void setCurrentTime(float timeSinceStart);

        void recompile();

        void update(float deltaTime);
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;
        using sf::RenderTexture::draw;

    private:
        using sf::RenderTexture::getTexture;

        float mTime;

        sf::Sprite mSprite;
        mutable sf::Shader mShader;

        float mDepthScale = 8.0f;
        float mPixelrepeat = 96.0f;

        const sf::Texture* mPatternTexture = nullptr;

        bool mDrawDots              = true;
        float mDotsRadius           = 12.f;
        float mDotsOffset           = 8.f;

        bool mInvertDepth           = false;
        bool mShowDepthImage        = false;

        bool mAddColour             = false;
        bool mDoubleColour          = false;

        bool mAnimate               = true;

        bool mRandomBackDrop        = false;
        bool mRandomBackDropPerLine = false;


};

static const auto DEFINE_DRAW_DOTS                 = "DRAW_DOTS";

static const auto DEFINE_INVERT_DEPTH              = "INVERT_DEPTH";
static const auto DEFINE_SHOW_DEPTH_IMAGE          = "SHOW_DEPTH_IMAGE";

static const auto DEFINE_ADD_COLOUR                = "ADD_COLOUR";
static const auto DEFINE_DOUBLE_COLOUR             = "DOUBLE_COLOUR";

static const auto DEFINE_ANIMATE                   = "ANIMATE";

static const auto DEFINE_RANDOM_BACK_DROP          = "RANDOM_BACKDROP_OFFSET";
static const auto DEFINE_RANDOM_BACK_DROP_PER_LINE = "RANDOM_BACKDROP_OFFSET_PER_LINE";

static const std::string SHADER_DEFINE             = "#define";

static const auto shaderString = R"(// Stereogram - @P_Malin

// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// A shader to generate a random dot stereogram or "magic eye" picture.
// http://en.wikipedia.org/wiki/Autostereogram

// The default is for "wall eyed" viewing.
// Remove this define if (like me) you prefer "cross eyed" viewing.
//#define INVERT_DEPTH

//#define SHOW_DEPTH_IMAGE

//#define ADD_COLOUR
//#define ANIMATE

uniform vec2      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;          // input channel. XX = 2D/Cube
uniform float     iPixelRepeat;
uniform float     iDepthScale;

//#ifdef ADD_COLOUR
//	#define DOUBLE_COLOUR
//#endif

//#ifdef ANIMATE
////	#define RANDOM_BACKDROP_OFFSET
////	#define RANDOM_BACKDROP_OFFSET_PER_LINE
//#endif

//#define DRAW_DOTS

// colour settings
const float fFuzziness = 0.25; // Colour blurry falloff (must be > 0.0)
const float fNoiseDesaturation = 0.5;
const float fColourDesaturation = 0.75;

float GetTime()
{
#ifdef ANIMATE
	return iGlobalTime;
#else
	return 0.0;
#endif
}

vec2 Random2(float f)
{
	return fract( sin( vec2(123.456, 78.9012) * f ) * vec2(3456.7890, 123.4567) );
}

// w is depth, rgb is colour
vec4 GetDepth( vec2 vPixel, vec4 vPrev )
{
	vec4 vResult = vPrev;

	vec2 vUV = (vPixel / iResolution.xy);
	vUV = vec2(vUV.x, vUV.y);

//    vResult.rgb = mix(vResult.rgb, texture2D(iChannel1, vUV).rgb, smoothstep(fFuzziness, -fFuzziness, 1.0));
	vResult.rgb = texture2D(iChannel1, vUV).rgb;
    //vResult.rgb = mix(texture2D(iChannel1, vUV).rgb, vResult.rgb, 0.5);
//    vResult.rgb = mix(vResult.rgb, vPrev.rgb, 0.5);
	vResult.w = vResult.b;
    return vResult;
}

vec4 GetStereogramDepth(vec2 vPixel, vec4 vPrev)
{
	// Adjust pixel co-ordinates to be in centre of strip
	return GetDepth(vPixel - vec2( iPixelRepeat * 0.5, 0.0), vPrev);
}

vec3 Stereogram(vec2 vPixel)
{
	vec2 vInitialPixel = vPixel;
	vec2 vIntPixel = vInitialPixel;

	// This is an arbitrary number, enough to make sure we will reach the edge of the screen
	for(int i=0; i<64; i++)
	{
		// Step left  iPixelRepeat minus depth...
		vec4 vDepth = GetStereogramDepth(vIntPixel, vec4(0.0));
		float fOffset = - iPixelRepeat;

		#ifndef INVERT_DEPTH
		fOffset -= vDepth.w * iDepthScale;
		#else
		fOffset += vDepth.w * iDepthScale;
		#endif

		vIntPixel.x = vIntPixel.x + fOffset;

		// ...until we fall of the screen
		if(vIntPixel.x < 0.0)
		{
			break;
		}
	}

	vIntPixel.x = mod(vIntPixel.x,  iPixelRepeat);

	vec2 vUV = (vIntPixel + 0.5) /  iPixelRepeat;

	vec3 vResult;

	#ifdef RANDOM_BACKDROP_OFFSET
		vUV += Random2(iGlobalTime);
	#endif // RANDOM_BACKDROP_OFFSET

	#ifdef RANDOM_BACKDROP_OFFSET_PER_LINE
		vUV += Random2(iGlobalTime + vUV.y * iResolution.y);
	#endif

	const float fMipLod = -32.0;
    vResult = texture2D(iChannel0, fract(vec2(vUV)), fMipLod).rgb;

	#ifdef ADD_COLOUR
	vec4 vColour = vec4(0.0, 0.0, 0.0, 0.0);
	vColour = GetStereogramDepth(vInitialPixel, vColour);

	#ifdef DOUBLE_COLOUR
	vColour = GetStereogramDepth(vInitialPixel + vec2(iPixelRepeat, 0.0), vColour);
	#endif // DOUBLE_COLOUR

	vResult = mix(vResult, vec3(1.0), fNoiseDesaturation); // desaturate noise
	vColour.rgb = mix(vColour.rgb, vec3(1.0), fColourDesaturation); // desaturate colour
	vResult = vResult * vColour.rgb;
	#endif

	return vResult;
}

vec3 ImageColour(vec2 vPixelCoord)
{
	vec3 vColour = Stereogram(vPixelCoord);

	#ifdef DRAW_DOTS

	    {
	vec2 vToCentre = vPixelCoord - vec2((iResolution.x / 2.0) -  iPixelRepeat * 0.5, iResolution.y - fRadius - fOffset);
	float fLength = length(vToCentre);
	float fAngle = atan(vToCentre.x, vToCentre.y);
	vec3 vDotColour = vec3(0.1, 0.3, 0.5);
	float fSpiral = abs( 0.5 - fract(fLength * 0.2 + fAngle * 3.14159 * 2.0 * 0.05 - iGlobalTime));
	vDotColour = mix(vDotColour, vec3(1.0, 1.0, 1.0), fSpiral);
	vColour = mix(vColour, vDotColour, smoothstep(fRadius, fRadius - 2.0, length(vToCentre)));
	}


	    {
	vec2 vToCentre = vPixelCoord - vec2((iResolution.x / 2.0) +  iPixelRepeat * 0.5, iResolution.y - fRadius - fOffset);
	float fLength = length(vToCentre);
	float fAngle = atan(vToCentre.x, vToCentre.y);
	vec3 vDotColour = vec3(0.1, 0.3, 0.5);
	float fSpiral = abs( 0.5 - fract(fLength * 0.2 + fAngle * 3.14159 * 2.0 * 0.05 - iGlobalTime));
	vDotColour = mix(vDotColour, vec3(1.0, 1.0, 1.0), fSpiral);
	vColour = mix(vColour, vDotColour, smoothstep(fRadius, fRadius - 2.0, length(vToCentre)));
	}
	#endif

	#ifdef SHOW_DEPTH_IMAGE
	vec4 vImage = GetDepth(vPixelCoord, vec4(0.0));
	vColour = vec3(vImage.w);

	#ifdef ADD_COLOUR
	vColour = vec3(vImage.rbg);
	#endif

	#endif

	return vColour;
}

void main()
{
	gl_FragColor = vec4(ImageColour(gl_FragCoord.xy),1.0);
})";


#endif // STEREOGRAME_
