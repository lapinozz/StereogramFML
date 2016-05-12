#include "stereograme.h"

#include <iostream>
#include <algorithm>

Stereograme::Stereograme()
{
//    useInternalTexture(); ///Need to be called AFTER sf::RenderTexture::create

    recompile();
}

void Stereograme::setPatternTexture(const sf::Texture& texture)
{
    mPatternTexture = &texture;
    mShader.setParameter("iChannel0", texture);
}

void Stereograme::useExternalTexture(const sf::Texture& texture)
{
    mSprite.setTexture(texture, true);
}

void Stereograme::useInternalTexture()
{
    useExternalTexture(getInternalTexture());
}

void Stereograme::setDepthScale(float scale)
{
    mDepthScale = scale;
    mShader.setParameter("iDepthScale", scale); // 8.0
}

void Stereograme::setPixelRepeat(float repeat)
{
    mPixelrepeat = repeat;
    mShader.setParameter("iPixelRepeat", repeat); // 96.0
}

void Stereograme::setDrawDots(bool drawDots, float dotsRadius, float dotOffsetFromTop, bool recompute)
{
    mDrawDots = drawDots;
    mDotsRadius = dotsRadius;
    mDotsOffset = dotOffsetFromTop;

    if(recompute)
        recompile();
}

void Stereograme::setInvertedDepth(bool invertDepth, bool recompute)
{
    mInvertDepth = invertDepth;

    if(recompute)
        recompile();
}

void Stereograme::setShowDepthImage(bool showDepthImage, bool recompute)
{
    mShowDepthImage = showDepthImage;

    if(recompute)
        recompile();
}

void Stereograme::setAddColour(bool addColour, bool doubleColour, bool recompute)
{
    mAddColour = addColour;
    mDoubleColour = doubleColour;

    if(recompute)
        recompile();
}

void Stereograme::setAnimate(bool animated, bool recompute)
{
    mAnimate = animated;

    if(recompute)
        recompile();
}

void Stereograme::setRandomBackDrop(bool randomBackDrop, bool perLine, bool recompute)
{
    mRandomBackDrop = randomBackDrop;
    mRandomBackDropPerLine = perLine;

    if(recompute)
        recompile();
}

const sf::Texture& Stereograme::getInternalTexture() const
{
    return getTexture();
}

const sf::Texture& Stereograme::getCurrentTexture() const
{
    return *mSprite.getTexture();
}

void Stereograme::setCurrentTime(float timeSinceStart)
{
    mTime = timeSinceStart;
    mShader.setParameter("iGlobalTime", mTime);
}

void Stereograme::recompile()
{
    std::string additionalInclude;

    if(mDrawDots)
    {
        additionalInclude += "float fRadius =" + std::to_string(mDotsRadius) + ";\n";
        additionalInclude += "float fOffset =" + std::to_string(mDotsOffset) + ";\n";

        additionalInclude += SHADER_DEFINE + DEFINE_DRAW_DOTS + "\n";
    }

    if(mInvertDepth)
        additionalInclude += SHADER_DEFINE + DEFINE_INVERT_DEPTH + "\n";

    if(mShowDepthImage)
        additionalInclude += SHADER_DEFINE + DEFINE_SHOW_DEPTH_IMAGE + "\n";

    if(mAddColour)
    {
        additionalInclude += SHADER_DEFINE + DEFINE_ADD_COLOUR + "\n";

        if(mDoubleColour)
            additionalInclude += SHADER_DEFINE + DEFINE_DOUBLE_COLOUR + "\n";
    }

    if(mAnimate)
        additionalInclude += SHADER_DEFINE + DEFINE_ANIMATE + "\n";

    if(mRandomBackDrop)
        additionalInclude += SHADER_DEFINE + DEFINE_RANDOM_BACK_DROP + "\n";
    if(mRandomBackDropPerLine)
        additionalInclude += SHADER_DEFINE + DEFINE_RANDOM_BACK_DROP_PER_LINE + "\n";

    mShader.loadFromMemory(additionalInclude + shaderString, sf::Shader::Fragment);
    mShader.setParameter("iChannel1", sf::Shader::CurrentTexture);

    setDepthScale(mDepthScale);
    setPixelRepeat(mPixelrepeat);

    if(mPatternTexture)
        setPatternTexture(*mPatternTexture);
}

void Stereograme::update(float deltaTime)
{
    mTime += deltaTime;
    mShader.setParameter("iGlobalTime", mTime);
}

void Stereograme::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.shader = &mShader;

    mShader.setParameter("iResolution", (sf::Vector2f)mSprite.getTexture()->getSize());

    target.draw(mSprite, states);
}

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin (reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end (reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }

std::string Stereograme::textHighlight(const std::string& text, int width)
{
    std::vector<int> spacePos;

    std::string output;
    size_t currentLineCharCount = 0;

    size_t lastPos = 0;
    size_t currentPos;
    while(true)
    {
        currentPos = text.find(' ', lastPos);
        if(currentPos == std::string::npos)
            currentPos = text.size() - 1;

        std::string word = text.substr(lastPos, currentPos - lastPos);

        if(word.size() == 1 && word[0] == ' ')
        {
            lastPos++;
            continue;
        }

        currentLineCharCount += word.size() + 1;
        if(currentLineCharCount > width)
        {
            output.pop_back(); // Remove last space
            spacePos.erase(spacePos.begin(), spacePos.begin() + 1); // Don't take into acount first word
            size_t spaceToPlace = width - (currentLineCharCount - word.size() - 2);
            size_t spaceLeft = spaceToPlace % spacePos.size();
            spaceToPlace /= spacePos.size();

            std::string spaceString;
            for(size_t x = 0; x < spaceToPlace; x++)
                spaceString += " ";

            for(auto pos : reverse(spacePos))
                output.insert(pos, spaceString);

            spaceString.clear();
            for(size_t x = 0; x < spaceLeft; x++)
                spaceString += " ";

            output.insert(spacePos.front(), spaceString);

            spacePos.clear();

            currentLineCharCount = word.size() + 1;
            output += "\n";
        }

        spacePos.push_back(output.size());
        output += word + " ";

        lastPos = currentPos + 1;

        if(currentPos == text.size() - 1)
            break;
    }

    lastPos = 0;
    while(true)
    {
        currentPos = output.find('\n', lastPos);
        if(currentPos == std::string::npos)
            currentPos = output.size() - 1;

        std::string line = output.substr(lastPos, currentPos - lastPos);
        size_t flagPos = 0;
        while((flagPos = line.find('*', flagPos)) != std::string::npos)
        {
            size_t lineDiff = 0;
            while(output[lastPos + flagPos] == '*')
            {
                output[lastPos + flagPos] = ' ';
                line.erase(flagPos - lineDiff, 1);
                auto spacePos = line.find(' ', flagPos);
                if(spacePos == std::string::npos)
                    spacePos = line.size();

                line.insert(spacePos + lineDiff, " ");

                flagPos++;
                lineDiff++;
            }
        }

        output.insert(currentPos, line);
        output.insert(currentPos, "  ");

        if(line.size() < width)//last line
        {
            for(size_t x = 0; x < width - line.size(); x++)
                output.insert(currentPos, " ");

            break;
        }

        lastPos = currentPos + line.size() + 3;
    }

    return output;
}


std::string Stereograme::ascii(const std::vector<char>& depthMap, size_t width, const std::string& pattern, size_t depth)
{
    std::string output;

    std::string currentPattern;
    size_t currentPatternOffset = 0;
    size_t lineOffset = 0;
    for(size_t x = 0; x < depthMap.size(); x++)
    {
        if(x % width == 0)
        {
            output += "\n";
            lineOffset = 0;
            currentPattern = pattern.substr(currentPatternOffset, depth);
            currentPatternOffset += depth;
            if(currentPattern.size() < depth)
            {
                currentPatternOffset = depth - currentPattern.size();
                currentPattern += pattern.substr(0, currentPatternOffset);
            }
        }

        //the max value for the depth map is depth - 1
//        std::rotate(currentPattern.begin(), currentPattern.begin() + depthMap[x], currentPattern.end());
        lineOffset += depthMap[x];
//        output += currentPattern[(x + lineOffset) % (depth)];
//        output += currentPattern[(x + depthMap[x]) % (depth)];
        output += currentPattern[(x + lineOffset) % (depth - depthMap[x])];
//        output += currentPattern[x % depth];
    }

    output.erase(0, 1);

    return output;
}
