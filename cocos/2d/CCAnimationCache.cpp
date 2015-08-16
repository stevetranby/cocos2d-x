/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
CopyRight (c) 2013-2016 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "2d/CCAnimationCache.h"
#include "2d/CCSpriteFrameCache.h"
#include "platform/CCFileUtils.h"

// STEVE
#include "base/CCDirector.h"
#include "renderer/CCTextureCache.h"
#include "deprecated/CCString.h"
#include "deprecated/CCInteger.h"
#include "deprecated/CCArray.h"

using namespace std;

NS_CC_BEGIN

AnimationCache* AnimationCache::s_sharedAnimationCache = nullptr;

AnimationCache* AnimationCache::getInstance()
{
    if (! s_sharedAnimationCache)
    {
        s_sharedAnimationCache = new (std::nothrow) AnimationCache();
        s_sharedAnimationCache->init();
    }

    return s_sharedAnimationCache;
}

void AnimationCache::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(s_sharedAnimationCache);
}

bool AnimationCache::init()
{
    return true;
}

AnimationCache::AnimationCache()
{
}

AnimationCache::~AnimationCache()
{
    CCLOGINFO("deallocing AnimationCache: %p", this);
}

void AnimationCache::addAnimation(Animation *animation, const std::string& name)
{
    _animations.insert(name, animation);
}

void AnimationCache::removeAnimation(const std::string& name)
{
    if (name.empty())
        return;

    _animations.erase(name);
}

Animation* AnimationCache::getAnimation(const std::string& name)
{
    return _animations.at(name);
}

void AnimationCache::parseVersion1(const ValueMap& animations)
{
    SpriteFrameCache *frameCache = SpriteFrameCache::getInstance();

    for (auto iter = animations.cbegin(); iter != animations.cend(); ++iter)
    {
        const ValueMap& animationDict = iter->second.asValueMap();
        const ValueVector& frameNames = animationDict.at("frames").asValueVector();
        float delay = animationDict.at("delay").asFloat();
        Animation* animation = nullptr;

        if ( frameNames.empty() )
        {
            CCLOG("cocos2d: AnimationCache: Animation '%s' found in dictionary without any frames - cannot add to animation cache.", iter->first.c_str());
            continue;
        }

        ssize_t frameNameSize = frameNames.size();
        Vector<AnimationFrame*> frames(frameNameSize);

        for (auto& frameName : frameNames)
        {
            SpriteFrame* spriteFrame = frameCache->getSpriteFrameByName(frameName.asString());

            if ( ! spriteFrame ) {
                CCLOG("cocos2d: AnimationCache: Animation '%s' refers to frame '%s' which is not currently in the SpriteFrameCache. This frame will not be added to the animation.", iter->first.c_str(), frameName.asString().c_str());

                continue;
            }

            AnimationFrame* animFrame = AnimationFrame::create(spriteFrame, 1, ValueMap());
            frames.pushBack(animFrame);
        }

        if ( frames.empty() )
        {
            CCLOG("cocos2d: AnimationCache: None of the frames for animation '%s' were found in the SpriteFrameCache. Animation is not being added to the Animation Cache.", iter->first.c_str());
            continue;
        }
        else if ( frames.size() != frameNameSize )
        {
            CCLOG("cocos2d: AnimationCache: An animation in your dictionary refers to a frame which is not in the SpriteFrameCache. Some or all of the frames for the animation '%s' may be missing.", iter->first.c_str());
        }

        animation = Animation::create(frames, delay, 1);

        AnimationCache::getInstance()->addAnimation(animation, iter->first);
    }
}

void AnimationCache::parseVersion2(const ValueMap& animations)
{
    SpriteFrameCache *frameCache = SpriteFrameCache::getInstance();

    for (auto iter = animations.cbegin(); iter != animations.cend(); ++iter)
    {
        std::string name = iter->first;
        ValueMap& animationDict = const_cast<ValueMap&>(iter->second.asValueMap());

        const Value& loops = animationDict["loops"];
        bool restoreOriginalFrame = animationDict["restoreOriginalFrame"].asBool();

        ValueVector& frameArray = animationDict["frames"].asValueVector();

        if ( frameArray.empty() )
        {
            CCLOG("cocos2d: AnimationCache: Animation '%s' found in dictionary without any frames - cannot add to animation cache.", name.c_str());
            continue;
        }

        // Array of AnimationFrames
        Vector<AnimationFrame*> array(static_cast<int>(frameArray.size()));

        for (auto& obj : frameArray)
        {
            ValueMap& entry = obj.asValueMap();
            std::string spriteFrameName = entry["spriteframe"].asString();
            SpriteFrame *spriteFrame = frameCache->getSpriteFrameByName(spriteFrameName);

            if( ! spriteFrame ) {
                CCLOG("cocos2d: AnimationCache: Animation '%s' refers to frame '%s' which is not currently in the SpriteFrameCache. This frame will not be added to the animation.", name.c_str(), spriteFrameName.c_str());

                continue;
            }

            float delayUnits = entry["delayUnits"].asFloat();
            Value& userInfo = entry["notification"];

            AnimationFrame *animFrame = AnimationFrame::create(spriteFrame, delayUnits, userInfo.getType() == Value::Type::MAP ? userInfo.asValueMap() : ValueMapNull);

            array.pushBack(animFrame);
        }

        float delayPerUnit = animationDict["delayPerUnit"].asFloat();
        Animation *animation = Animation::create(array, delayPerUnit, loops.getType() != Value::Type::NONE ? loops.asInt() : 1);

        animation->setRestoreOriginalFrame(restoreOriginalFrame);

        AnimationCache::getInstance()->addAnimation(animation, name);
    }
}

void AnimationCache::addAnimationsWithDictionary(const ValueMap& dictionary,const std::string& plist)
{
    if ( dictionary.find("animations") == dictionary.end() )
    {
        CCLOG("cocos2d: AnimationCache: No animations were found in provided dictionary.");
        return;
    }
    
    const Value& animations = dictionary.at("animations");
    unsigned int version = 1;

    if( dictionary.find("properties") != dictionary.end() )
    {
        const ValueMap& properties = dictionary.at("properties").asValueMap();
        version = properties.at("format").asInt();
        const ValueVector& spritesheets = properties.at("spritesheets").asValueVector();

        for(const auto &value : spritesheets) {
            std::string path = FileUtils::getInstance()->fullPathFromRelativeFile(value.asString(),plist);
            SpriteFrameCache::getInstance()->addSpriteFramesWithFile(path);
        }
    }

    switch (version) {
        case 1:
            parseVersion1(animations.asValueMap());
            break;
        case 2:
            parseVersion2(animations.asValueMap());
            break;
        default:
            CCASSERT(false, "Invalid animation format");
    }
}

/** Read an NSDictionary from a plist file and parse it automatically for animations */
void AnimationCache::addAnimationsWithFile(const std::string& plist)
{
    CCASSERT(!plist.empty(), "Invalid texture file name");
    if (plist.empty()) {
        log("%s error:file name is empty!", __FUNCTION__);
        return;
    }
    
    ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(plist);

    CCASSERT( !dict.empty(), "CCAnimationCache: File could not be found");
    if (dict.empty()) {
        log("AnimationCache::addAnimationsWithFile error:%s not exist!", plist.c_str());
    }

    addAnimationsWithDictionary(dict,plist);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// MARK - STEVE Extensions

void AnimationCache::addAnimationWithName(const string& animName, const string& animFrameName, const vector<string>& animFrameIndicies, float frameDelay)
{
    AnimationCache *animCache = AnimationCache::getInstance();
    if(animCache->getAnimation(animName)) {
        return;
    }
    if(animFrameIndicies.empty()) {
        return;
    }

    Vector<SpriteFrame*> animSpriteFrames;
    for(auto frameStr : animFrameIndicies)
    {
        int index;
        stringstream(frameStr) >> index;
        auto frameName = StringUtils::format("%s-%03d.png", animFrameName.c_str(), index);
        SpriteFrame *spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if(spriteFrame) {
            animSpriteFrames.pushBack(spriteFrame);
        }
    }

    if(! animSpriteFrames.empty())
    {
        Animation *anim = Animation::createWithSpriteFrames(animSpriteFrames, frameDelay);
        animCache->addAnimation(anim, animName);
    }
}

#pragma  mark - spritesheet sequence

void AnimationCache::addAnimWithName(const string& animName, const string& frameFormat, const vector<string>& frameIndicies, float frameDelay)
{
    AnimationCache *animCache = AnimationCache::getInstance();
    if(animCache->getAnimation(animName))
    {
        CCLOG("already in cache");
        return;
    }
    if(frameIndicies.empty()) {
        return;
    }

    SpriteFrameCache *frameCache = SpriteFrameCache::getInstance();
    Vector<SpriteFrame*> animSpriteFrames;

    for(auto frameStr : frameIndicies)
    {
        int index;
        stringstream(frameStr) >> index;
        auto frameName = StringUtils::format(frameFormat.c_str(), index);
        auto nameOrFile = frameName;
        SpriteFrame* spriteFrame = frameCache->getSpriteFrameByName(nameOrFile);
        if (spriteFrame && spriteFrame->getTexture())
        {
            // CCLOG("adding spriteframe with franemname %s", frameName->getCString());
            animSpriteFrames.pushBack(spriteFrame);
        }
        else
        {
            auto tex = Director::getInstance()->getTextureCache()->addImage(nameOrFile);
            if(tex)
            {
                Rect rect;
                rect.size = tex->getContentSize();
                spriteFrame = SpriteFrame::create(nameOrFile, rect);
                if(spriteFrame) {
                    animSpriteFrames.pushBack(spriteFrame);
                }
            }
        }
    }

    if(animSpriteFrames.size() > 0)
    {
        //CCLOG("adding animation with name = %s", animName->getCString());
        Animation *anim = Animation::createWithSpriteFrames(animSpriteFrames, frameDelay);
        animCache->addAnimation(anim, animName);
    }
    else
    {
        CCLOG("no frames");
    }
}

NS_CC_END
